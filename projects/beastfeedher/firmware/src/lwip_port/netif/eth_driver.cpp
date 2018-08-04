/*
 * lwIP Ethernet driver for Stellaris MCUs, QP/Vanilla kernel
 * Copyright (c) 2009 Quantum Leaps, LLC, www.state-machine.com
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 */

/*
 * Copyright (c) 2008 Luminary Micro, Inc.
 *
 * This file is dervied from the ``ethernetif.c'' skeleton Ethernet network
 * interface driver for lwIP.
 */

#include "qpcpp.h"

#include "lwip/opt.h"
#include "lwip/init.h"
#include "lwip/ip.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"

#include "netif/etharp.h"
#include "netif/eth_driver.h"

#include "inc/hw_ethernet.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/ethernet.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"

#include <string.h>                                         /* for memcpy() */

#if 0
enum EthDriverSignals {
  LWIP_SLOW_TICK_SIG,
    LWIP_RX_READY_SIG,
    LWIP_TX_READY_SIG,
    LWIP_RX_OVERRUN_SIG
};
#else
#include "BSP.h"
#endif

/**
 * Sanity Check:  This interface driver will NOT work if the following defines
 * are incorrect.
 */
#if (PBUF_LINK_HLEN != 16)
#error "PBUF_LINK_HLEN must be 16 for this interface driver!"
#endif
#if (ETH_PAD_SIZE != 2)
#error "ETH_PAD_SIZE must be 2 for this interface driver!"
#endif

/**
 * Setup processing for PTP (IEEE-1588).
 */
#if LWIP_PTPD
void lwIPHostGetTime(u32_t *time_s, u32_t *time_ns);
#endif

#ifndef TX_PBUF_QUEUE_LEN
#define TX_PBUF_QUEUE_LEN 8
#endif

/* Helper queue of pbufs */
typedef struct PbufQueueTag {
    struct pbuf *ring[TX_PBUF_QUEUE_LEN];
    uint8_t qwrite;
    uint8_t qread;
    uint8_t overflow;
} PbufQueue;

static void PbufQueue_ctor(PbufQueue *me);
static uint8_t PbufQueue_put(PbufQueue *me, struct pbuf *p);
static struct pbuf *PbufQueue_get(PbufQueue *me);
#define PbufQueue_isEmpty(me_) ((me_)->qwrite == (me_)->qread)

/*..........................................................................*/
static struct netif l_netif;                /* the single network interface */
static QActive *l_active;      /* active object associated with this driver */
static PbufQueue l_txq;                  /* queue of pbufs for transmission */

static err_t ethernetif_init(struct netif *netif);
static err_t ethernetif_output(struct netif *netif, struct pbuf *p);
static struct pbuf *low_level_receive(void);
static void low_level_transmit(struct pbuf *p);

/*..........................................................................*/
void ISR_Ethernet(void) {
    unsigned long eth_stat = HWREG(ETH_BASE + MAC_O_RIS);

    HWREG(ETH_BASE + MAC_O_IACK) = eth_stat; /* clear the interrupt sources */

    eth_stat &= HWREG(ETH_BASE + MAC_O_IM);/* mask only the enabled sources */

    if ((eth_stat & ETH_INT_RX) != 0) {
        static QEvent const evt_eth_rx = { SIG_LWIP_RX_READY, 0 };
        //QActive_postFIFO(l_active, &evt_eth_rx);          /* send to the AO */
        l_active->POST(&evt_eth_rx, 0);          /* send to the AO */

        HWREG(ETH_BASE + MAC_O_IM) &= ~ETH_INT_RX;    /* disable further RX */
    }
    if ((eth_stat & ETH_INT_TX) != 0) {
        static QEvent const evt_eth_tx = { SIG_LWIP_TX_READY, 0 };
        //QActive_postFIFO(l_active, &evt_eth_tx);          /* send to the AO */
        l_active->POST(&evt_eth_tx, 0);          /* send to the AO */
    }
#if LINK_STATS
    if ((eth_stat & ETH_INT_RXOF) != 0) {
        static QEvent const evt_eth_er = { SIG_LWIP_RX_OVERRUN, 0 };
        //QActive_postFIFO(l_active, &evt_eth_er);          /* send to the AO */
        l_active->POST(&evt_eth_er, 0);          /* send to the AO */
    }
#endif
}

/*..........................................................................*/
struct netif *eth_driver_init(QActive *active,
			      bool     aUseDHCP,
			      uint32_t aIPAddr,
			      uint32_t aSubnetMask,
			      uint32_t aGWAddr,
                              u8_t     macaddr[NETIF_MAX_HWADDR_LEN])
{
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gwaddr;

    lwip_init();                                /* nitialize the lwIP stack */

                             /* set MAC address in the network interface... */
    l_netif.hwaddr_len = NETIF_MAX_HWADDR_LEN;
    memcpy(&l_netif.hwaddr[0], macaddr, NETIF_MAX_HWADDR_LEN);

    l_active = active; /*save the active object associated with this driver */

#if LWIP_NETIF_HOSTNAME
    l_netif.hostname = "lwIP";             /* initialize interface hostname */
#endif
    l_netif.name[0] = 'Q';
    l_netif.name[1] = 'P';

    /*
    * Initialize the snmp variables and counters inside the struct netif.
    * The last argument should be replaced with your link speed, in units
    * of bits per second.
    */
    NETIF_INIT_SNMP(&l_netif, snmp_ifType_ethernet_csmacd, 1000000);

    /* We directly use etharp_output() here to save a function call.
    * You can instead declare your own function an call etharp_output()
    * from it if you have to do some checks before sending (e.g. if link
    * is available...) */
    l_netif.output     = &etharp_output;

    l_netif.linkoutput = &ethernetif_output;

    PbufQueue_ctor(&l_txq);                 /* initialize the TX pbuf queue */

    if (aUseDHCP) {
      IP4_ADDR(&ipaddr,  0, 0, 0, 0);
      IP4_ADDR(&netmask, 0, 0, 0, 0);
      IP4_ADDR(&gwaddr,  0, 0, 0, 0);
    } else if (IPADDR_ANY != (aIPAddr & aSubnetMask)) {
      // IP Address from persistence.
      ipaddr.addr  = htonl(aIPAddr);
      netmask.addr = htonl(aSubnetMask);
      gwaddr.addr  = htonl(aGWAddr);
    } else {
#if (LWIP_DHCP == 0) && (LWIP_AUTOIP == 0)
          /* No mechanism of obtaining IP address specified, use static IP: */
    IP4_ADDR(&ipaddr,  STATIC_IPADDR0,    STATIC_IPADDR1,
                       STATIC_IPADDR2,    STATIC_IPADDR3);
    IP4_ADDR(&netmask, STATIC_NET_MASK0,  STATIC_NET_MASK1,
                       STATIC_NET_MASK2,  STATIC_NET_MASK3);
    IP4_ADDR(&gwaddr,  STATIC_GW_IPADDR0, STATIC_GW_IPADDR1,
                       STATIC_GW_IPADDR2, STATIC_GW_IPADDR3);
#else
     /* either DHCP or AUTOIP are configured, start with zero IP addresses: */
    IP4_ADDR(&ipaddr,  0, 0, 0, 0);
    IP4_ADDR(&netmask, 0, 0, 0, 0);
    IP4_ADDR(&gwaddr,  0, 0, 0, 0);
#endif
    }

          /* add and configure the Ethernet interface with default settings */
    netif_add(&l_netif,
              &ipaddr, &netmask, &gwaddr,            /* configured IP addresses */
              active,                /* use this active object as the state */
              &ethernetif_init,        /* Ethernet interface initialization */
              &ip_input);                   /* standard IP input processing */

    netif_set_default(&l_netif);

    netif_set_up(&l_netif);                       /* bring the interface up */

    if (aUseDHCP) {
#if (LWIP_DHCP != 0)
      dhcp_start(&l_netif);
#else // LWIP_DHCP
#error "Requires LWIP_DHCP set to '1'"
#endif
    } else if (IPADDR_ANY == (aIPAddr & aSubnetMask)) {
      // No static IP address already set.
#if (LWIP_AUTOIP != 0)
    autoip_start(&l_netif);     /* start AutoIP if configured in lwipopts.h */
#endif
    }

    /* Enable Ethernet TX and RX Packet Interrupts. */
    HWREG(ETH_BASE + MAC_O_IM) |= (ETH_INT_RX | ETH_INT_TX);

#if LINK_STATS
    HWREG(ETH_BASE + MAC_O_IM) |= ETH_INT_RXOF;
#endif

    return &l_netif;
}
/*..........................................................................*/
void eth_driver_read(void) {
    struct pbuf *p = low_level_receive();
    if (p != NULL) {                  /* new packet received into the pbuf? */
        if (ethernet_input(p, &l_netif) != ERR_OK) {   /* pbuf not handled? */
            LWIP_DEBUGF(NETIF_DEBUG, ("eth_driver_input: input error\n"));
            pbuf_free(p);                                  /* free the pbuf */
        }

        /* try to output a packet if TX fifo is empty and pbuf is available */
        if ((HWREG(ETH_BASE + MAC_O_TR) & MAC_TR_NEWTX) == 0) {
            p = PbufQueue_get(&l_txq);
            if (p != NULL) {
                low_level_transmit(p);
                pbuf_free(p);    /* free the pbuf, lwIP knows nothing of it */
            }
        }
    }

    HWREG(ETH_BASE + MAC_O_IM) |= ETH_INT_RX; /* re-enable the RX interrupt */
}
/*..........................................................................*/
void eth_driver_write(void) {
    if ((HWREG(ETH_BASE + MAC_O_TR) & MAC_TR_NEWTX) == 0) {/*TX fifo empty? */
        struct pbuf *p = PbufQueue_get(&l_txq);
        if (p != NULL) {                        /* pbuf found in the queue? */
            low_level_transmit(p);
            pbuf_free(p);        /* free the pbuf, lwIP knows nothing of it */
        }
    }
}

/*..........................................................................*/
/*
 * This function will either write the pbuf into the Stellaris TX FIFO,
 * or will put the packet in the TX queue of pbufs for subsequent
 * transmission when the transmitter becomes idle.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the pbuf to send
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 */
static err_t ethernetif_output(struct netif *netif, struct pbuf *p) {
    if (PbufQueue_isEmpty(&l_txq) &&            /* nothing in the TX queue? */
        ((HWREG(ETH_BASE + MAC_O_TR) & MAC_TR_NEWTX) == 0))    /* TX empty? */
    {
        low_level_transmit(p);                  /* send the pbuf right away */
        /* the pbuf will be freed by the lwIP code */
    }
    else {                 /* otherwise post the pbuf to the transmit queue */
        if (PbufQueue_put(&l_txq, p)) { /*could the TX queue take the pbuf? */
            pbuf_ref(p);     /* reference the pbuf to spare it from freeing */
        }
        else {                                      /* no room in the queue */
            /* the pbuf will be freed by the lwIP code */
            return ERR_MEM;
        }
    }
    return ERR_OK;
}

/*==========================================================================*/

static err_t ethernetif_init(struct netif *netif) {
                                              /* initialize the hardware... */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);
    SysCtlPeripheralReset(SYSCTL_PERIPH_ETH);

    EthernetMACAddrSet(ETH_BASE, &netif->hwaddr[0]); /* set the MAC address */

    netif->mtu = 1500;                             /* maximum transfer unit */

                                                 /* set device capabilities */
    netif->flags = (NETIF_FLAG_BROADCAST
                    | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP);

                                         /* disable all Ethernet interrupts */
    HWREG(ETH_BASE + MAC_O_IM) &= ~(ETH_INT_PHY | ETH_INT_MDIO | ETH_INT_RXER
                                    | ETH_INT_RXOF | ETH_INT_TX | ETH_INT_TXER
                                    | ETH_INT_RX);
                                              /* acknolwedge all interrupts */
    HWREG(ETH_BASE + MAC_O_IACK) = HWREG(ETH_BASE + MAC_O_RIS);

    /* Initialize the Ethernet Controller. */
    EthernetInitExpClk(ETH_BASE, SysCtlClockGet());

    /* configure the Ethernet Controller for normal operation
    * - Enable TX Duplex Mode
    * - Enable TX Padding
    * - Enable TX CRC Generation
    * - Enable RX Multicast Reception
    */
    EthernetConfigSet(ETH_BASE, (ETH_CFG_TX_DPLXEN | ETH_CFG_TX_CRCEN
                                 | ETH_CFG_TX_PADEN | ETH_CFG_RX_AMULEN));

    EthernetEnable(ETH_BASE);   /* enable Ethernet transmitter and receiver */

    return ERR_OK;
}

/*..........................................................................*/
/*
* This function should do the actual transmission of the packet. The packet is
* contained in the pbuf that is passed to the function. This pbuf might be
* chained.
*
* @param p the MAC packet to send (e.g. IP packet including MAC addr and type)
* @return ERR_OK if the packet could be sent
*         an err_t value if the packet couldn't be sent
* @note This function MUST be called with interrupts disabled or with the
*       Stellaris Ethernet transmit fifo protected.
*/
static void low_level_transmit(struct pbuf *p) {
    int iBuf;
    unsigned char *pucBuf;
    unsigned long *pulBuf;
    struct pbuf *q;
    int iGather;
    unsigned long ulGather;
    unsigned char *pucGather;

    /**
    * Fill in the first two bytes of the payload data (configured as padding
    * with ETH_PAD_SIZE = 2) with the total length of the payload data
    * (minus the Ethernet MAC layer header).
    */
    *((unsigned short *)(p->payload)) = p->tot_len - 16;

    /* Initialize the gather register. */
    iGather = 0;
    pucGather = (unsigned char *)&ulGather;
    ulGather = 0;

    /* Copy data from the pbuf(s) into the TX Fifo. */
    for (q = p; q != NULL; q = q->next) {
        /* Intialize a char pointer and index to the pbuf payload data. */
        pucBuf = (unsigned char *)q->payload;
        iBuf = 0;

        /**
        * If the gather buffer has leftover data from a previous pbuf
        * in the chain, fill it up and write it to the Tx FIFO.
        */
        while ((iBuf < q->len) && (iGather != 0)) {
            /* Copy a byte from the pbuf into the gather buffer. */
            pucGather[iGather] = pucBuf[iBuf++];

            /* Increment the gather buffer index modulo 4. */
            iGather = ((iGather + 1) % 4);
        }

        /**
        * If the gather index is 0 and the pbuf index is non-zero,
        * we have a gather buffer to write into the Tx FIFO.
        */
        if ((iGather == 0) && (iBuf != 0)) {
            HWREG(ETH_BASE + MAC_O_DATA) = ulGather;
            ulGather = 0;
        }

        /* Initialze a long pointer into the pbuf for 32-bit access. */
        pulBuf = (unsigned long *)&pucBuf[iBuf];

        /**
        * Copy words of pbuf data into the Tx FIFO, but don't go past
        * the end of the pbuf.
        */
        while ((iBuf + 4) <= q->len) {
            HWREG(ETH_BASE + MAC_O_DATA) = *pulBuf++;
            iBuf += 4;
        }

        /**
        * Check if leftover data in the pbuf and save it in the gather
        * buffer for the next time.
        */
        while (iBuf < q->len) {
            /* Copy a byte from the pbuf into the gather buffer. */
            pucGather[iGather] = pucBuf[iBuf++];

            /* Increment the gather buffer index modulo 4. */
            iGather = ((iGather + 1) % 4);
        }
    }

    /* Send any leftover data to the FIFO. */
    HWREG(ETH_BASE + MAC_O_DATA) = ulGather;

    /* Wakeup the transmitter. */
    HWREG(ETH_BASE + MAC_O_TR) = MAC_TR_NEWTX;

    LINK_STATS_INC(link.xmit);
}

/*..........................................................................*/
/*
 * This function will read a single packet from the Stellaris ethernet
 * interface, if available, and return a pointer to a pbuf.  The timestamp
 * of the packet will be placed into the pbuf structure.
 *
 * @return pointer to pbuf packet if available, NULL otherswise.
 */
static struct pbuf *low_level_receive(void) {
    struct pbuf *p, *q;
    u16_t len;
    u32_t temp;
    int i;

#if LWIP_PTPD
    u32_t time_s, time_ns;

    /* Get the current timestamp if PTPD is enabled */
    lwIPHostGetTime(&time_s, &time_ns);
#endif

    /* Check if a packet is available, if not, return NULL packet. */
    if ((HWREG(ETH_BASE + MAC_O_NP) & MAC_NP_NPR_M) == 0) {
        return (struct pbuf *)0;
    }

    /**
    * Obtain the size of the packet and put it into the "len" variable.
    * Note:  The length returned in the FIFO length position includes the
    * two bytes for the length + the 4 bytes for the FCS.
    */
    temp = HWREG(ETH_BASE + MAC_O_DATA);
    len = temp & 0xFFFF;

    /* We allocate a pbuf chain of pbufs from the pool. */
    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

    /* If a pbuf was allocated, read the packet into the pbuf. */
    if (p != NULL) {
        /* Place the first word into the first pbuf location. */
        *(unsigned long *)p->payload = temp;
        p->payload = (char *)(p->payload) + 4;
        p->len -= 4;

        /* Process all but the last buffer in the pbuf chain. */
        q = p;
        do {
            /* Setup a byte pointer into the payload section of the pbuf. */
            unsigned long *ptr = (unsigned long *)q->payload;

            /**
            * Read data from FIFO into the current pbuf
            * (assume pbuf length is modulo 4)
            */
            for (i = 0; i < q->len; i += 4) {
                *ptr++ = HWREG(ETH_BASE + MAC_O_DATA);
            }

            /* Link in the next pbuf in the chain. */
            q = q->next;
        } while (q != NULL);

        /* Restore the first pbuf parameters to their original values. */
        p->payload = (char *)(p->payload) - 4;
        p->len += 4;

        /* Adjust the link statistics */
        LINK_STATS_INC(link.recv);

#if LWIP_PTPD
        /* Place the timestamp in the PBUF */
        p->time_s = time_s;
        p->time_ns = time_ns;
#endif
    }

    /* If no pbuf available, just drain the RX fifo. */
    else {
        for (i = 4; i < len; i+=4) {
            temp = HWREG(ETH_BASE + MAC_O_DATA);
        }

        /* Adjust the link statistics */
        LINK_STATS_INC(link.memerr);
        LINK_STATS_INC(link.drop);
    }

    return p;
}

/*..........................................................................*/
static void PbufQueue_ctor(PbufQueue *me) {
    me->qread    = 0;
    me->qwrite   = 0;
    me->overflow = 0;
}
/*..........................................................................*/
static struct pbuf *PbufQueue_get(PbufQueue *me) {
    struct pbuf *pBuf;

    if (PbufQueue_isEmpty(me)) {
        /* Return a NULL pointer if the queue is empty. */
        pBuf = (struct pbuf *)0;
    }
    else {
        /*
        * The queue is not empty so return the next frame from it
        * and adjust the read pointer accordingly.
        */
        pBuf = me->ring[me->qread];
        if ((++me->qread) == Q_DIM(me->ring)) {
            me->qread = 0;
        }
    }
    return pBuf;
}
/*..........................................................................*/
static uint8_t PbufQueue_put(PbufQueue *me, struct pbuf *p) {
    uint8_t next_qwrite = me->qwrite + 1;

    if (next_qwrite == Q_DIM(me->ring)) {
        next_qwrite = 0;
    }
    if (next_qwrite != me->qread) {
        /*
        * The queue isn't full so we add the new frame at the current
        * write position and move the write pointer.
        */
        me->ring[me->qwrite] = p;
        if ((++me->qwrite) == Q_DIM(me->ring)) {
            me->qwrite = 0;
        }
        return 1;                           /* successfully posted the pbuf */
    }
    else {
        /*
        * The stack is full so we are throwing away this value.
        * Keep track of the number of times this happens.
        */
        ++me->overflow;
        return 0;                                /* could not post the pbuf */
    }
}

extern "C" u32_t sys_now() {
  return 0;
}
/*..........................................................................*/
#if NETIF_DEBUG
/* Print an IP header by using LWIP_DEBUGF
 * @param p an IP packet, p->payload pointing to the IP header
 */
void eth_driver_debug_print(struct pbuf *p) {
    struct eth_hdr *ethhdr = (struct eth_hdr *)p->payload;
    u16_t *plen = (u16_t *)p->payload;

    LWIP_DEBUGF(NETIF_DEBUG, ("ETH header:\n"));
    LWIP_DEBUGF(NETIF_DEBUG, ("Packet Length:%5"U16_F" \n",*plen));
    LWIP_DEBUGF(NETIF_DEBUG, ("Destination: %02"X8_F"-%02"X8_F"-%02"X8_F
        "-%02"X8_F"-%02"X8_F"-%02"X8_F"\n",
        ethhdr->dest.addr[0],
        ethhdr->dest.addr[1],
        ethhdr->dest.addr[2],
        ethhdr->dest.addr[3],
        ethhdr->dest.addr[4],
        ethhdr->dest.addr[5]));
    LWIP_DEBUGF(NETIF_DEBUG, ("Source: %02"X8_F"-%02"X8_F"-%02"X8_F
        "-%02"X8_F"-%02"X8_F"-%02"X8_F"\n",
        ethhdr->src.addr[0],
        ethhdr->src.addr[1],
        ethhdr->src.addr[2],
        ethhdr->src.addr[3],
        ethhdr->src.addr[4],
        ethhdr->src.addr[5]));
    LWIP_DEBUGF(NETIF_DEBUG, ("Packet Type:0x%04"U16_F" \n", ethhdr->type));
}
#endif /* NETIF_DEBUG */
