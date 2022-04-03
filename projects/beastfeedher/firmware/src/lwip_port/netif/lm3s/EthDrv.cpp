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
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// Standard Libraries.
#include <string.h>

// QP Library.
#include <qpcpp.h>

// LwIP.
#include "lwip/opt.h"
#include "lwip/def.h"

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

EthDrv::EthDrv(unsigned int aIndex, EthernetAddress const &aEthernetAddress, unsigned int aPBufQueueSize)
    : LwIPDrv(aIndex, aEthernetAddress)
    , mPBufQ(new PBufQ(aPBufQueueSize)) {

    // Ctor body.
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
    EthernetIntEnable(ETH_BASE, ETH_INT_RX);
}


void EthDrv::Wr(void) {

    // TX fifo empty? Should be since we likely got here by TxEmpty int.
    if (IsTxEmpty()) {
        struct pbuf * const lPBuf = GetPBufQ().Get();
        // pbuf found in the queue?
        if (lPBuf != nullptr) {
            // Send and free the pbuf: lwIP knows nothing of it.
            LowLevelTx(lPBuf);
            pbuf_free(lPBuf);
        }
    }
}


void EthDrv::PHYISR(void) {
    // This PHYISR handler is called in normal "task" context.
    // EthDrv::ISR() makes call to read and clear interrupt status.

    unsigned long lPHYInt = EthernetPHYRead(ETH_BASE, PHY_MR17);
    if (lPHYInt & PHY_MR17_LSCHG_INT) {
        // Link status changed: determine new state.
        unsigned int lPHYStatus = EthernetPHYRead(ETH_BASE, PHY_MR1);
        if (lPHYStatus & PHY_MR1_LINK) {
            // Signal the link is up.
            PostLinkChangedEvent(true);
        //} else if (lPHYStatus & PHY_MR1_ANEGC) {
            // Not sure which should come 1st: link of autoneg?
        } else {
            // Signal the link is down.
            PostLinkChangedEvent(false);
        }
    }

    if ((lPHYInt & PHY_MR17_LSCHG_INT)
        || (lPHYInt & PHY_MR17_ANEGCOMP_INT)) {
        // Check for duplex and rate states.
        unsigned long lCfg = EthernetConfigGet(ETH_BASE);
        unsigned long lPHYDiag = EthernetPHYRead(ETH_BASE, PHY_MR18);
        if (lPHYDiag & PHY_MR18_DPLX) {
            // Enable duplex mode in MAC.
            lCfg |= ETH_CFG_TX_DPLXEN;
        } else {
            // Disable duplex mode in MAC.
            lCfg &= ~ETH_CFG_TX_DPLXEN;
        }

        // Nothing to force MAC in 10/100 rate!
        EthernetConfigSet(ETH_BASE, lCfg);
    }

    // Now that the interrupt source was likely cleared, clear the flag.
    // Re-enable PHY interrupt.
    EthernetIntEnable(ETH_BASE, ETH_INT_PHY);
}


void EthDrv::DisableAllInt(void) {
    EthernetIntDisable(ETH_BASE, ETH_INT_RX | ETH_INT_TX);

#if LINK_STATS
    EthernetIntDisable(ETH_BASE, ETH_INT_RXOF);
#endif
}


void EthDrv::EnableAllInt(void) {
    // Enable Ethernet TX and RX Packet Interrupts.
    EthernetIntEnable(ETH_BASE, ETH_INT_RX | ETH_INT_TX);

#if LINK_STATS
    EthernetIntEnable(ETH_BASE, ETH_INT_RXOF);
#endif
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// This function will either write the pbuf into the Stellaris TX FIFO,
// or will put the packet in the TX queue of pbufs for subsequent
// transmission when the transmitter becomes idle.
//
// @param netif the lwip network interface structure for this ethernetif
// @param p the pbuf to send
// @return ERR_OK if the packet could be sent
//         an err_t value if the packet couldn't be sent
err_t EthDrv::EtherIFOut(struct pbuf * const aPBuf) {

    // Nothing in the TX queue?
    // TX empty?
    if (GetPBufQ().IsEmpty() && IsTxEmpty()) {
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


err_t EthDrv::EtherIFInit(struct netif * const aNetIF) {

    // Initialize the hardware...
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);
    SysCtlPeripheralReset(SYSCTL_PERIPH_ETH);

    // Set the MAC address.
    EthernetMACAddrSet(ETH_BASE, &aNetIF->hwaddr[0]);

    // Configure PHY interrupts.
    // Listen to interrupts:
    //    -"Change of link status"
    //    -"Auto-negotiation complete"
    EthernetPHYWrite(
        ETH_BASE,
        PHY_MR17,
        PHY_MR17_LSCHG_IE | PHY_MR17_ANEGCOMP_IE
    );

    // Maximum transfer unit.
    aNetIF->mtu = 1500;

    // Set device capabilities.
    aNetIF->flags |= (NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP);

    // Disable all Ethernet interrupts.
    EthernetIntDisable(
        ETH_BASE,
        (ETH_INT_PHY | ETH_INT_MDIO | ETH_INT_RXER
            | ETH_INT_RXOF | ETH_INT_TX | ETH_INT_TXER | ETH_INT_RX)
    );

    // Acknolwedge all interrupts.
    static constexpr tBoolean sIsMasked = false;
    unsigned long lIntStatus = EthernetIntStatus(ETH_BASE, sIsMasked);
    EthernetIntClear(ETH_BASE, lIntStatus);
    EthernetIntEnable(ETH_BASE, ETH_INT_PHY);

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

    // Clear the interrupt sources.
    // Mask only the enabled sources.
    static constexpr tBoolean sIsMasked = true;
    unsigned long lIntStatus = EthernetIntStatus(ETH_BASE, sIsMasked);
    EthernetIntClear(ETH_BASE, lIntStatus);

    if ((lIntStatus & ETH_INT_RX) != 0) {
        // Send to the AO.
        PostRxEvent();
        // Disable further RX.
        EthernetIntDisable(ETH_BASE, ETH_INT_RX);
    }
  
    if ((lIntStatus & ETH_INT_TX) != 0) {
        // Send to the AO.
        PostTxEvent();
    }

#if LINK_STATS
    if ((lIntStatus & ETH_INT_RXOF) != 0) {
        // Send to the AO.
        PostOverrunEvent();
    }
#endif

    // Process PHY interrupts.
    if ((lIntStatus & ETH_INT_PHY) != 0) {
        // Handler will restore PHY interrupts once they are handled.
        PostPHYInterruptEvent();
    }
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


EthDrv::PBufQ::PBufQ(unsigned int aQSize)
    : mPBufRing(nullptr)
    , mRingSize(aQSize)
    , mQWrIx(0)
    , mQRdIx(0)
    , mQOverflow(0) {

    // Ctor body.
    mPBufRing = new struct pbuf *[aQSize];
}


bool EthDrv::PBufQ::IsEmpty(void) const {
    return (mQWrIx == mQRdIx);
}


bool EthDrv::PBufQ::Put(struct pbuf * const aPBufPtr) {
    unsigned int lNextQWr = mQWrIx + 1;

    if (lNextQWr == mRingSize) {
        lNextQWr = 0;
    }

    if (lNextQWr != mQRdIx) {
        // The queue isn't full so we add the new frame at the current
        // write position and move the write pointer.
        mPBufRing[mQWrIx] = aPBufPtr;
        if ((++mQWrIx) == mRingSize) {
            mQWrIx = 0;
        }

        // Successfully posted the pbuf.
        return true;
    } else {
        // The stack is full so we are throwing away this value.
        // Keep track of the number of times this happens.
        mQOverflow++;
        // Could not post the pbuf.
        return false;
    }
}


struct pbuf *EthDrv::PBufQ::Get(void) {
    struct pbuf *lPBuf = nullptr;

    if (!IsEmpty()) {
        // The queue is not empty so return the next frame from it.
        // Adjust the read pointer accordingly.
        lPBuf = mPBufRing[mQRdIx];
        if ((++mQRdIx) == mRingSize) {
            mQRdIx = 0;
        }
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
