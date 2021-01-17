// *****************************************************************************
//
// Project: LwIP
//
// Module: LM3S6965 low-level Ethernet driver.
//
// *****************************************************************************

//! \file
//! \brief Ethernet driver class.
//! \ingroup lwip

// *****************************************************************************
//
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// Standard Libraries.
#include <string.h>

// QP Library.
#include <qpcpp.h>

extern "C" {
// LwIP.
#include "lwip/opt.h"
#include "lwip/ip.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"

// TI Library.
#include <hw_ethernet.h>
#include <hw_ints.h>
#include <hw_memmap.h>
#include <hw_types.h>
#include <driverlib/ethernet.h>
#include <driverlib/interrupt.h>
#include <driverlib/sysctl.h>

#include "netif/etharp.h"
} // extern "C"

#include "Signals.h"

#include "LwIP_Events.h"
#include "EthDrv.h"

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

// Sanity Check:  This interface driver will NOT work if the following defines
// are incorrect.
#if (PBUF_LINK_HLEN != 16)
#error "PBUF_LINK_HLEN must be 16 for this interface driver!"
#endif

#if (ETH_PAD_SIZE != 2)
#error "ETH_PAD_SIZE must be 2 for this interface driver!"
#endif

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

// Setup processing for PTP (IEEE-1588).
#if LWIP_PTPD
void lwIPHostGetTime(u32_t *time_s, u32_t *time_ns);
#endif

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

EthDrv::EthDrv(unsigned int aIndex, EthernetAddress const &aEthernetAddress, unsigned int aBufQueueSize)
    : LwIPDrv(aIndex, aBufQueueSize) {

    // Set MAC address in the network interface...
    GetNetIF().hwaddr_len = NETIF_MAX_HWADDR_LEN;
    memcpy(&GetNetIF().hwaddr[0], aEthernetAddress.GetData(), NETIF_MAX_HWADDR_LEN);
}


// TODO: should return error.
void EthDrv::DrvInit(
    QP::QActive * const aAO,
    bool aUseDHCP,
    uint32_t aIPAddr,
    uint32_t aSubnetMask,
    uint32_t aGWAddr
) {
    // Save the active object associated with this driver.
    SetAO(aAO);

#if LWIP_NETIF_HOSTNAME
    // Initialize interface hostname.
    GetNetIF().hostname = "LwIP";
#endif
    GetNetIF().name[0] = 'Q';
    GetNetIF().name[1] = 'P';

    // Initialize the snmp variables and counters inside the struct netif.
    // The last argument should be replaced with your link speed, in units
    // of bits per second.
    NETIF_INIT_SNMP(&mNetIF, snmp_ifType_ethernet_csmacd, 1000000);

    // We directly use etharp_output() here to save a function call.
    // You can instead declare your own function an call etharp_output()
    // from it if you have to do some checks before sending (e.g. if link is available...)
    GetNetIF().output = &etharp_output;
    GetNetIF().linkoutput = &LwIPDrv::StaticEtherIFOut;

    ip_addr_t lIPAddr;
    ip_addr_t lSubnetMask;
    ip_addr_t lGWAddr;

    if (aUseDHCP) {
        IP4_ADDR(&lIPAddr, 0, 0, 0, 0);
        IP4_ADDR(&lSubnetMask, 0, 0, 0, 0);
        IP4_ADDR(&lGWAddr, 0, 0, 0, 0);
    } else if (IPADDR_ANY != (aIPAddr & aSubnetMask)) {
        // IP Address from persistence.
        lIPAddr.addr = htonl(aIPAddr);
        lSubnetMask.addr = htonl(aSubnetMask);
        lGWAddr.addr = htonl(aGWAddr);
    } else {
#if (LWIP_DHCP == 0) && (LWIP_AUTOIP == 0)
        // No mechanism of obtaining IP address specified, use static IP.
        IP4_ADDR(
            &lIPAddr,
            STATIC_IPADDR0, STATIC_IPADDR1,
            STATIC_IPADDR2, STATIC_IPADDR3
        );
        IP4_ADDR(
            &lSubnetMask,
            STATIC_NET_MASK0, STATIC_NET_MASK1,
            STATIC_NET_MASK2, STATIC_NET_MASK3
        );
        IP4_ADDR(
            &lGWAddr,
            STATIC_GW_IPADDR0, STATIC_GW_IPADDR1,
            STATIC_GW_IPADDR2, STATIC_GW_IPADDR3
        );
#else
        // Either DHCP or AUTOIP are configured, start with zero IP addresses.
        IP4_ADDR(&lIPAddr, 0, 0, 0, 0);
        IP4_ADDR(&lSubnetMask, 0, 0, 0, 0);
        IP4_ADDR(&lGWAddr, 0, 0, 0, 0);
#endif
    }

    // Add and configure the Ethernet interface with default settings.
    // Use AO* as the state.
    netif_add(
        &GetNetIF(),
	    &lIPAddr,
	    &lSubnetMask,
	    &lGWAddr,
	    &GetAO(),
	    &LwIPDrv::StaticEtherIFInit,
        &ip_input
    );

    // Bring the interface up.
    netif_set_default(&GetNetIF());
    netif_set_up(&GetNetIF());

#if (LWIP_DHCP != 0)
    // Start DHCP if configured in lwipopts.h.
    dhcp_start(&GetNetIF());
    // NOTE: If LWIP_AUTOIP is configured in lwipopts.h and
    // LWIP_DHCP_AUTOIP_COOP is set as well, the DHCP process will start
    // AutoIP after DHCP fails for 59 seconds.
#elif (LWIP_AUTOIP != 0)
    // Start AutoIP if configured in lwipopts.h.
    autoip_start(&mNetIF);
#endif

    // Enable Ethernet TX and RX Packet Interrupts.
    HWREG(ETH_BASE + MAC_O_IM) |= (ETH_INT_RX | ETH_INT_TX);

#if LINK_STATS
    HWREG(ETH_BASE + MAC_O_IM) |= ETH_INT_RXOF;
#endif
}


err_t EthDrv::EtherIFInit(struct netif * const aNetIF) {

    // Initialize the hardware...
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);
    SysCtlPeripheralReset(SYSCTL_PERIPH_ETH);

    // Set the MAC address.
    EthernetMACAddrSet(ETH_BASE, &aNetIF->hwaddr[0]);

    // Maximum transfer unit.
    aNetIF->mtu = 1500;

    // Set device capabilities.
    aNetIF->flags = (NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP);

    // Disable all Ethernet interrupts.
    HWREG(ETH_BASE + MAC_O_IM) &=
        ~(ETH_INT_PHY | ETH_INT_MDIO | ETH_INT_RXER
            | ETH_INT_RXOF | ETH_INT_TX | ETH_INT_TXER | ETH_INT_RX
    );

    // Acknolwedge all interrupts.
    HWREG(ETH_BASE + MAC_O_IACK) = HWREG(ETH_BASE + MAC_O_RIS);

    // Initialize the Ethernet Controller.
    EthernetInitExpClk(ETH_BASE, SysCtlClockGet());

    // configure the Ethernet Controller for normal operation
    // - Enable TX Duplex Mode
    // - Enable TX Padding
    // - Enable TX CRC Generation
    // - Enable RX Multicast Reception
    EthernetConfigSet(
        ETH_BASE,
        (ETH_CFG_TX_DPLXEN | ETH_CFG_TX_CRCEN | ETH_CFG_TX_PADEN | ETH_CFG_RX_AMULEN)
    );

    // Enable Ethernet transmitter and receiver.
    EthernetEnable(ETH_BASE);

    return ERR_OK;
}


void EthDrv::Rd(void) {

    // New packet received into the pbuf?
    struct pbuf * const lPBuf = LowLevelRx();
    if (lPBuf != nullptr) {
        // pbuf handled?
        if (ethernet_input(lPBuf, &GetNetIF()) != ERR_OK) {
            // Free the pbuf.
            pbuf_free(lPBuf);
        }
        // Try to output a packet if TX fifo is empty and pbuf is available.
        Wr();
    }

    // Re-enable the RX interrupt.
    HWREG(ETH_BASE + MAC_O_IM) |= ETH_INT_RX;
}


void EthDrv::Wr(void) {

    // TX fifo empty? Should be since we likely got here by TxEmpty int.
    if ((HWREG(ETH_BASE + MAC_O_TR) & MAC_TR_NEWTX) == 0) {
        struct pbuf * const lPBuf = GetPBufQ().Get();
        // pbuf found in the queue?
        if (lPBuf != nullptr) {
            // Send and free the pbuf: lwIP knows nothing of it.
            LowLevelTx(lPBuf);
            pbuf_free(lPBuf);
        }
    }
}


// This function will either write the pbuf into the Stellaris TX FIFO,
// or will put the packet in the TX queue of pbufs for subsequent
// transmission when the transmitter becomes idle.
//
// @param netif the lwip network interface structure for this ethernetif
// @param p the pbuf to send
// @return ERR_OK if the packet could be sent
//         an err_t value if the packet couldn't be sent
err_t EthDrv::EtherIFOut(struct netif * const aNetIF, struct pbuf * const aPBuf) {

    // Nothing in the TX queue?
    // TX empty?
    if (GetPBufQ().IsEmpty() && ((HWREG(ETH_BASE + MAC_O_TR) & MAC_TR_NEWTX) == 0)) {
        // Send the pbuf right away.
        LowLevelTx(aPBuf);
        // The pbuf will be freed by the lwIP code.
    } else {
        // Otherwise post the pbuf to the transmit queue.
        // Could the TX queue take the pbuf?
        if (GetPBufQ().Put(aPBuf)) {
            // Reference the pbuf to spare it from freeing.
            pbuf_ref(aPBuf);
        } else {
            // No room in the queue.
            // The pbuf will be freed by the lwIP code.
            return ERR_MEM;
        }
    }

    return ERR_OK;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

void EthDrv::ISR(void) {

    unsigned long lEthStatus = HWREG(ETH_BASE + MAC_O_RIS);

    // Clear the interrupt sources.
    HWREG(ETH_BASE + MAC_O_IACK) = lEthStatus; 

    // Mask only the enabled sources.
    lEthStatus &= HWREG(ETH_BASE + MAC_O_IM);

    if ((lEthStatus & ETH_INT_RX) != 0) {
        static LwIP::Event::Interrupt const sRxEvent(LWIP_RX_READY_SIG, GetIndex());
        // Send to the AO.
        GetAO().POST(&sRxEvent, this);
        // Disable further RX.
        HWREG(ETH_BASE + MAC_O_IM) &= ~ETH_INT_RX;
    }
  
    if ((lEthStatus & ETH_INT_TX) != 0) {
        static LwIP::Event::Interrupt const sTxEvent(LWIP_TX_READY_SIG, GetIndex());
        // Send to the AO.
        GetAO().POST(&sTxEvent, this);
    }
#if LINK_STATS
    if ((lEthStatus & ETH_INT_RXOF) != 0) {
        static LwIP::Event::Interrupt const sOverrunEvent(LWIP_RX_OVERRUN_SIG, GetIndex());
        // Send to the AO.
        GetAO().POST(&sOverrunEvent, this);
    }
#endif
}


// This function should do the actual transmission of the packet. The packet is
// contained in the pbuf that is passed to the function. This pbuf might be
// chained.
//
// @param p the MAC packet to send (e.g. IP packet including MAC addr and type)
// @return ERR_OK if the packet could be sent
//         an err_t value if the packet couldn't be sent
// @note This function MUST be called with interrupts disabled or with the
//       Stellaris Ethernet transmit fifo protected.
void EthDrv::LowLevelTx(struct pbuf * const aPBuf) {

    // Fill in the first two bytes of the payload data (configured as padding
    // with ETH_PAD_SIZE = 2) with the total length of the payload data
    // (minus the Ethernet MAC layer header).
    unsigned short * const lPayload = static_cast<unsigned short * const>(aPBuf->payload);
    *lPayload = aPBuf->tot_len - 16;

    // Initialize the gather register.
    unsigned int  lByteGatherIx = 0;
    unsigned long lWordGather = 0;
    unsigned char * const lWordGatherPtr = reinterpret_cast<unsigned char *>(&lWordGather);

    // Copy data from the pbuf(s) into the TX Fifo.
    for (struct pbuf *lQueuePtr = aPBuf; lQueuePtr != nullptr; lQueuePtr = lQueuePtr->next) {
        // Intialize a char pointer and index to the pbuf payload data.
        unsigned char * const lByteBufPtr = reinterpret_cast<unsigned char *>(lQueuePtr->payload);
        unsigned int lByteBufIx = 0;

        // If the gather buffer has leftover data from a previous pbuf
        // in the chain, fill it up and write it to the Tx FIFO.
        while ((lByteBufIx < lQueuePtr->len) && (lByteGatherIx != 0)) {
            // Copy a byte from the pbuf into the gather buffer.
            lWordGatherPtr[lByteGatherIx] = lByteBufPtr[lByteBufIx++];

            // Increment the gather buffer index modulo 4.
            lByteGatherIx = ((lByteGatherIx + 1) % 4);
        }

        // If the gather index is 0 and the pbuf index is non-zero,
        // we have a gather buffer to write into the Tx FIFO.
        if ((lByteGatherIx == 0) && (lByteBufIx != 0)) {
            HWREG(ETH_BASE + MAC_O_DATA) = lWordGather;
            lWordGather = 0;
        }

        // Initialze a long pointer into the pbuf for 32-bit access.
        unsigned long *lULongBufPtr = reinterpret_cast<unsigned long *>(&lByteBufPtr[lByteBufIx]);

        // Copy words of pbuf data into the Tx FIFO, but don't go past
        // the end of the pbuf.
        while ((lByteBufIx + 4) <= lQueuePtr->len) {
            HWREG(ETH_BASE + MAC_O_DATA) = *lULongBufPtr++;
            lByteBufIx += 4;
        }

        // Check if leftover data in the pbuf and save it in the gather
        // buffer for the next time.
        while (lByteBufIx < lQueuePtr->len) {
            // Copy a byte from the pbuf into the gather buffer.
            lWordGatherPtr[lByteGatherIx] = lByteBufPtr[lByteBufIx++];

            // Increment the gather buffer index modulo 4.
            lByteGatherIx = ((lByteGatherIx + 1) % 4);
        }
    }

    // Send any leftover data to the FIFO.
    // Wakeup the transmitter.
    HWREG(ETH_BASE + MAC_O_DATA) = lWordGather;
    HWREG(ETH_BASE + MAC_O_TR) = MAC_TR_NEWTX;

    LINK_STATS_INC(link.xmit);
}


// This function will read a single packet from the Stellaris ethernet
// interface, if available, and return a pointer to a pbuf.  The timestamp
// of the packet will be placed into the pbuf structure.
// * @return pointer to pbuf packet if available, nullptr otherswise.
struct pbuf *EthDrv::LowLevelRx(void) {

#if LWIP_PTPD
    u32_t time_s;
    u32_t time_ns;
    // Get the current timestamp if PTPD is enabled.
    lwIPHostGetTime(&time_s, &time_ns);
#endif

    // Check if a packet is available, if not, return nullptr packet.
    if ((HWREG(ETH_BASE + MAC_O_NP) & MAC_NP_NPR_M) == 0) {
        return nullptr;
    }

    // Obtain the size of the packet and put it into the "len" variable.
    // Note:  The length returned in the FIFO length position includes the
    // two bytes for the length + the 4 bytes for the FCS.
    u32_t lTemp = HWREG(ETH_BASE + MAC_O_DATA);
    u16_t lLen  = lTemp & 0xFFFF;

    // We allocate a pbuf chain of pbufs from the pool.
    struct pbuf * const lPBuf = pbuf_alloc(PBUF_RAW, lLen, PBUF_POOL);

    // If a pbuf was allocated, read the packet into the pbuf.
    if (lPBuf != nullptr) {
        // Place the first word into the first pbuf location.
        unsigned long * const lPayload = static_cast<unsigned long * const>(lPBuf->payload);
        *lPayload = lTemp;
        lPBuf->payload = reinterpret_cast<char *>(lPBuf->payload) + 4;
        lPBuf->len -= 4;

        // Process all but the last buffer in the pbuf chain.
        struct pbuf *lQueuePtr = lPBuf;
        do {
            // Setup a byte pointer into the payload section of the pbuf.
            unsigned long *lULongPtr = static_cast<unsigned long *>(lQueuePtr->payload);

            // Read data from FIFO into the current pbuf
            // (assume pbuf length is modulo 4)
            for (unsigned int lIx = 0; lIx < lQueuePtr->len; lIx += 4) {
	            *lULongPtr++ = HWREG(ETH_BASE + MAC_O_DATA);
            }

            // Link in the next pbuf in the chain.
            lQueuePtr = lQueuePtr->next;
        } while (lQueuePtr != nullptr);

        // Restore the first pbuf parameters to their original values.
        lPBuf->payload = reinterpret_cast<char *>(lPBuf->payload) - 4;
        lPBuf->len += 4;

        // Adjust the link statistics.
        LINK_STATS_INC(link.recv);

#if LWIP_PTPD
        // Place the timestamp in the PBUF.
        lQueuePtr->time_s = time_s;
        lQueuePtr->time_ns = time_ns;
#endif
    } else {
        // If no pbuf available, just drain the RX fifo.
        for (unsigned int lIx = 4; lIx < lLen; lIx += 4) {
            lTemp = HWREG(ETH_BASE + MAC_O_DATA);
        }

        // Adjust the link statistics.
        LINK_STATS_INC(link.memerr);
        LINK_STATS_INC(link.drop);
    }

    return lPBuf;
}


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
#endif // NETIF_DEBUG

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
