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
//        Copyright (c) 2015-2020, Martin Garon, All rights reserved.
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
#include "lwip/def.h"
} // extern "C"

// TI Library.
#include <inc/hw_ethernet.h>
#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <driverlib/ethernet.h>
#include <driverlib/sysctl.h>

#include "netif/etharp.h"

#include "netif/lm3s/EthDrv.h"

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
    : LwIPDrv(aIndex, aEthernetAddress, aBufQueueSize) {

    // Ctor body.
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

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


void EthDrv::ISR(void) {

    unsigned long lEthStatus = HWREG(ETH_BASE + MAC_O_RIS);

    // Clear the interrupt sources.
    HWREG(ETH_BASE + MAC_O_IACK) = lEthStatus; 

    // Mask only the enabled sources.
    lEthStatus &= HWREG(ETH_BASE + MAC_O_IM);

    if ((lEthStatus & ETH_INT_RX) != 0) {
        // Send to the AO.
        PostRxEvent();
        // Disable further RX.
        HWREG(ETH_BASE + MAC_O_IM) &= ~ETH_INT_RX;
    }
  
    if ((lEthStatus & ETH_INT_TX) != 0) {
        // Send to the AO.
        PostTxEvent();
    }
#if LINK_STATS
    if ((lEthStatus & ETH_INT_RXOF) != 0) {
        // Send to the AO.
        PostOverrunEvent();
    }
#endif

#if 0
    if ((lEthStatus & ETH_INT_PHY) != 0) {
        unsigned long lPHYInt = EthernetPHYRead(ETH_BASE, PHY_MR17);
        if (lPHYInt | PHY_MR17_LSCHG_INT) {
            // Link status changed: determine new state.
            unsigned int lPHYStatus = EthernetPHYRead(ETH_BASE, PHY_MR1);
            if (lPHYStatus | PHY_MR1_LINK) {
                // Signal the link is up. Callback will do the rest.
                netif_set_link_up(&GetNetIF());
            } else if (lPHYStatus | PHY_MR1_ANEGC) {
                // Not sure which should come 1st: link of autoneg?
            } else if ((lPHYStatus & PHY_MR1_LINK) == 0) {
                // Signal the link is up. Callback will do the rest.
                netif_set_link_down(&GetNetIF());
            }
        }
    }
#endif
}


void EthDrv::EnableRxInt(void) {
    HWREG(ETH_BASE + MAC_O_IM) |= ETH_INT_RX;
    //EthernetIntEnable(ETH_BASE, ETH_INT_RX);
}


void EthDrv::EnableAllInt(void) {
    // Enable PHY interrupts: auto-negotiation complete, link status change.
    //EthernetPHYWrite(ETH_BASE, PHY_MR17, PHY_MR17_LSCHG_IE | PHY_MR17_ANEGCOMP_IE);

    // Enable Ethernet TX and RX Packet Interrupts.
    HWREG(ETH_BASE + MAC_O_IM) |= (ETH_INT_RX | ETH_INT_TX | ETH_INT_PHY);

#if LINK_STATS
    HWREG(ETH_BASE + MAC_O_IM) |= ETH_INT_RXOF;
#endif
}


bool EthDrv::IsTxEmpty(void) const {
    return ((HWREG(ETH_BASE + MAC_O_TR) & MAC_TR_NEWTX) == 0);
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
    //for (struct pbuf *lQueuePtr = aPBuf; lQueuePtr->len != lQueuePtr->tot_len; lQueuePtr = lQueuePtr->next) {
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
        // 2 bytes represent the total length of the received Ethernet frame,
        // including the FCS and Frame Length bytes.
        // 2 bytes represent the DA bytes 1 and 2.
        // To save all 4 bytes in the payload, ETH_PAD_SIZE must be set to 2.
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
        // [MG] DOUBLE CHECK: THIS SHOULD BE:
        //} while (lQueuePtr->len != lQueuePtr->tot_len);

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


void EthDrv::FreePBuf(struct pbuf * const aPBuf) {
    // Free the pbuf.
    pbuf_free(aPBuf);
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
