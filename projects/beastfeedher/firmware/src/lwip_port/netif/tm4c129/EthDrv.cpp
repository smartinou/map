// *****************************************************************************
//
// Project: LwIP
//
// Module: TivaWare low-level Ethernet driver.
//
// *****************************************************************************

//! \file
//! \brief Ethernet driver class.
//! Compatible with TivaWare library.
//! \ingroup lwip

// *****************************************************************************
//
//        Copyright (c) 2015-2021, Martin Garon, All rights reserved.
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
#include <inc/hw_emac.h>
#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <driverlib/emac.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <driverlib/sysctl.h>

#include "netif/etharp.h"
#include "netif/tm4c129/EthDrv.h"

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

// Sanity Check:  This interface driver will NOT work if the following defines
// are incorrect.
#if (PBUF_LINK_HLEN != 14)
#error "PBUF_LINK_HLEN must be 14 for this interface driver!"
#endif

#if (ETH_PAD_SIZE != 0)
#error "ETH_PAD_SIZE must be 0 for this interface driver!"
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

EthDrv::EthDrv(
    unsigned int aIndex,
    EthernetAddress const &aEthernetAddress,
    unsigned int aBufQueueSize
)   : LwIPDrv(aIndex, aEthernetAddress)
    , mRxDescriptors()
    , mTxDescriptors() {

    // Ctor body.
}


void EthDrv::DisableAllInt(void) {

}


void EthDrv::EnableAllInt(void) {
    // Enable Ethernet TX and RX Packet Interrupts.
    MAP_EMACIntEnable(EMAC0_BASE, EMAC_INT_TRANSMIT);// | EMAC_INT_RECEIVE);

#if LINK_STATS
    MAP_EMACIntEnable(EMAC0_BASE, EMAC_INT_RX_OVERFLOW);
#endif
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

err_t EthDrv::EtherIFOut(struct pbuf * const aPBuf) {

    // Chain pbufs elements to transmit into as many descriptors:
    // Each pbuf element is attached to a descriptor of the tx chain.
    if (aPBuf != nullptr) {
        static constexpr bool sIsFirstElement = true;
        mTxDescriptors.SaveHead();
        bool lResult = LowLevelTx(aPBuf, sIsFirstElement);
        if (lResult) {
            // ethernet_output() must not release the pbuf after this call.
            // It will be released once the packet is out.
            pbuf_ref(aPBuf);
            // Unblock the transmitter potentially in suspended state.
            MAP_EMACTxDMAPollDemand(EMAC0_BASE);
        } else {
            // Error while assigning pbuf to descriptor chain.
            mTxDescriptors.RestoreHead();
            return ERR_BUF;
        }
    }

    return ERR_OK;
}


void EthDrv::Rd(void) {
    // TODO.
}


void EthDrv::Wr(void) {
    // We should never get here from a Tx interrupt.
}


// Initialize Ethernet IF as per TivaWare Driver doc under:
// 10.2.4.26 EMACPHYConfigSet
err_t EthDrv::EtherIFInit(struct netif * const aNetIF) {

    // Initialize the hardware...
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_EMAC0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_EPHY0);

    MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_EMAC0);
#if 0
    MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_EPHY0);

    // Ensure the MAC has completed its reset.
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_EMAC0)) {
    }
#endif
    // This driver uses the internal PHY.
    // Configure for use with the internal PHY.
    // Set the PHY type and configuration options.
    MAP_EMACPHYConfigSet(
        EMAC0_BASE,
        (EMAC_PHY_TYPE_INTERNAL |
            EMAC_PHY_INT_MDIX_EN |
            EMAC_PHY_AN_100B_T_FULL_DUPLEX)
    );

    // Configure PHY interrupts.
    // Listen to "Change of link status" interrupt.
    static uint8_t constexpr sPHYAddr = EMAC_PHY_ADDR;
    MAP_EMACPHYWrite(EMAC0_BASE, sPHYAddr, EPHY_SCR, EPHY_SCR_INTEN);
    MAP_EMACPHYWrite(EMAC0_BASE, sPHYAddr, EPHY_MISR1, EPHY_MISR1_LINKSTATEN);
    // Reset the MAC to latch the PHY configuration.
    //MAP_EMACReset(EMAC0_BASE);

    // Maximum transfer unit.
    aNetIF->mtu = 1500;

    // Set device capabilities.
    // NETIF_FLAG_UP and NETIF_FLAG_LINK_UP will be set when turning on the
    // network interface and when the link comes up respectively.
    aNetIF->flags |= (NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP);

    // Disable all Ethernet interrupts.
    uint32_t lAllInts =
        // PHY interrupts.
        (EMAC_INT_PHY
        // Normal interrupts.
        | EMAC_INT_TRANSMIT | EMAC_INT_TX_NO_BUFFER | EMAC_INT_RECEIVE | EMAC_INT_EARLY_RECEIVE
        // Abnormal interrupts.
        | EMAC_INT_TX_STOPPED | EMAC_INT_TX_JABBER | EMAC_INT_RX_OVERFLOW | EMAC_INT_TX_UNDERFLOW
        | EMAC_INT_RX_NO_BUFFER | EMAC_INT_RX_STOPPED | EMAC_INT_RX_WATCHDOG | EMAC_INT_EARLY_TRANSMIT
        | EMAC_INT_BUS_ERROR);
    MAP_EMACIntDisable(EMAC0_BASE, lAllInts);

    // Acknowledge all interrupts.
    // Clear any pending interrupts.
    uint32_t lIntStatus = MAP_EMACIntStatus(EMAC0_BASE, false);
    MAP_EMACIntClear(EMAC0_BASE, lIntStatus);
    MAP_EMACIntEnable(EMAC0_BASE, EMAC_INT_PHY);

    // Initialize the MAC and set the DMA mode.
    static uint32_t constexpr sRxBurstSize = 4;
    static uint32_t constexpr sTxBurstSize = 4;
    static uint32_t constexpr sDescSkipSize = 0;
    MAP_EMACInit(
        EMAC0_BASE,
        SysCtlClockGet(),
        EMAC_BCONFIG_MIXED_BURST | EMAC_BCONFIG_PRIORITY_FIXED,
        sRxBurstSize,
        sTxBurstSize,
        sDescSkipSize
    );

    // Set MAC configuration options.
    static uint32_t constexpr sRxMaxFrameSize = 0;
    MAP_EMACConfigSet(
        EMAC0_BASE,
        (EMAC_CONFIG_FULL_DUPLEX |
            EMAC_CONFIG_CHECKSUM_OFFLOAD |
            EMAC_CONFIG_7BYTE_PREAMBLE |
            EMAC_CONFIG_IF_GAP_96BITS |
            EMAC_CONFIG_USE_MACADDR0 |
            EMAC_CONFIG_SA_FROM_DESCRIPTOR |
            EMAC_CONFIG_BO_LIMIT_1024),
        (EMAC_MODE_RX_STORE_FORWARD |
            EMAC_MODE_TX_STORE_FORWARD |
            EMAC_MODE_TX_THRESHOLD_64_BYTES |
            EMAC_MODE_RX_THRESHOLD_64_BYTES),
        sRxMaxFrameSize
    );

    // Program the hardware with its MAC address (for filtering).
    MAP_EMACAddrSet(EMAC0_BASE, 0, &aNetIF->hwaddr[0]);

    // Set MAC filtering options. We receive all broadcast and multicast
    // packets along with those addressed specifically for us.
    MAP_EMACFrameFilterSet(
        EMAC0_BASE,
        (EMAC_FRMFILTER_SADDR |
            //EMAC_FRMFILTER_PASS_MULTICAST |
            EMAC_FRMFILTER_PASS_NO_CTRL)
    );

    // Initialize the Ethernet DMA descriptors.
    static constexpr unsigned int sDescriptorQty = 8;
    RxDescriptor *lRxList = mRxDescriptors.Create(EMAC0_BASE, sDescriptorQty, 540);
    MAP_EMACRxDMADescriptorListSet(EMAC0_BASE, lRxList);
    TxDescriptor *lTxList = mTxDescriptors.Create(sDescriptorQty);
    MAP_EMACTxDMADescriptorListSet(EMAC0_BASE, lTxList);

    // Enable the Ethernet MAC transmitter and receiver.
    MAP_EMACTxEnable(EMAC0_BASE);
    //MAP_EMACRxEnable(EMAC0_BASE);
    return ERR_OK;
}


void EthDrv::ISR(void) {
    // Get and clear the interrupt sources.
    uint32_t lStatus = MAP_EMACIntStatus(EMAC0_BASE, true);
    MAP_EMACIntClear(EMAC0_BASE, lStatus);

    // Process normal interrupts.
    if (lStatus & EMAC_INT_RECEIVE) {
        // Send to the AO.
        PostRxEvent();
        // Disable further RX.
        MAP_EMACIntDisable(EMAC0_BASE, EMAC_INT_RECEIVE);
    }

    if (lStatus & EMAC_INT_TRANSMIT) {
        // Frame transmission is complete.
        // Try to advance the end pointer of the free descriptor list.
        //mTxDescriptors.UpdateFreeList();
        TxDescriptor * const lTxDMACurrentDescriptor =
            static_cast<TxDescriptor * const>(MAP_EMACTxDMACurrentDescriptorGet(EMAC0_BASE));
        struct pbuf *lPBuf = mTxDescriptors.GetPBufs(lTxDMACurrentDescriptor);
        if (lPBuf) {
            pbuf_free(lPBuf);
        }
    }

    // Process abnormal interrupts.
#if LINK_STATS
    if (lStatus & EMAC_INT_RX_OVERFLOW) {
        // Send to the AO.
        PostOverrunEvent();
    }
#endif

    // Process PHY interrupts.
    if (lStatus & EMAC_INT_PHY) {
        // Reading the interrupt status clears the bits.
        static constexpr uint8_t sPHYAddr = EMAC_PHY_ADDR;
        uint16_t lEPHYMISR1 = MAP_EMACPHYRead(EMAC0_BASE, sPHYAddr, EPHY_MISR1);
        if (lEPHYMISR1 & EPHY_MISR1_LINKSTAT) {
            uint16_t lStatus = MAP_EMACPHYRead(EMAC0_BASE, sPHYAddr, EPHY_BMSR);
            // Call the proper netif function. This will internally set the right flag.
            // Eventually, this will also trigger a callback.
            if (lStatus & EPHY_BMSR_LINKSTAT) {
                //static const LwIP::Event::NetStatusChanged sEvent(LWIP_LINK_CHANGED_SIG, &GetNetIF(), true);
                //GetAO().POST(&sEvent, this);
                PostNetIFChangedEvent(true);
            } else {
                //static const LwIP::Event::NetStatusChanged sEvent(LWIP_LINK_CHANGED_SIG, &GetNetIF(), false);
                //GetAO().POST(&sEvent, this);
                PostNetIFChangedEvent(false);
            }
        }
    }
}


void EthDrv::EnableRxInt(void) {
    MAP_EMACIntEnable(EMAC0_BASE, EMAC_INT_RECEIVE);
}


// This function should do the actual transmission of the packet. The packet is
// contained in the pbuf that is passed to the function. This pbuf might be
// chained.
//
// @param p the MAC packet to send (e.g. IP packet including MAC addr and type)
// @return true if descriptors were allocated for the chain of pbufs.
//         false otherwise.
bool EthDrv::LowLevelTx(struct pbuf * const aPBuf, bool aIsFirstPBuf) {

    // Before start, keep track of Head, in case we need to revert back.
    TxDescriptor * const lDescriptor = mTxDescriptors.PutPBufs(aPBuf, aIsFirstPBuf);
    if (lDescriptor) {
        // Operation succeeded. Check if this is a chained pbuf.
        if (aPBuf->next != nullptr) {
            // There's more pbuf element: try to assign it to a descriptor right away.
            bool lResult = LowLevelTx(aPBuf->next, false);
            if (!lResult) {
                // There was an error while attempting the next link:
                // Reassign the pointer to the current descriptor.
                // Propagate error back to top of chain.
                return false;
            }
        } else {
            // This is the end of the chain of pbufs. Mark it.
            lDescriptor->SetFrameEnd();
        }

        if (aIsFirstPBuf) {
            // This is the 1st descriptor of the chain. Mark it.
            lDescriptor->SetFrameStart();
        }

        // Give to HW last: this way, if failing to allocate a descriptor in the chain,
        // it will be easier to reclaim them for SW usage.
        lDescriptor->GiveToHW();

        // Move to handling of TRANSMIT_INT.
        LINK_STATS_INC(link.xmit);
        return true;
    }

    return false;
}


// This function will read a single packet from the Stellaris ethernet
// interface, if available, and return a pointer to a pbuf.  The timestamp
// of the packet will be placed into the pbuf structure.
// * @return pointer to pbuf packet if available, nullptr otherswise.
struct pbuf *EthDrv::LowLevelRx(void) {

    RxDescriptor &lDescriptor = mRxDescriptors.GetNext();

    // SW should always own the current descriptor since rx interrupt triggered this call.
    // If not, then the pointer got out of sync?
    if (!lDescriptor.IsHWOwned()) {
        if (lDescriptor.IsFrameValid()) {
            // Get the pbuf assigned as payload of the descriptor.
            // It should never be null.
            struct pbuf *lPBuf = mRxDescriptors.GetPBuf(&lDescriptor);
            if (!lDescriptor.IsLastFrame()) {
                // Get the next descriptor.
                struct pbuf *lTailPBuf = LowLevelRx();
                if (lTailPBuf != nullptr) {
                    pbuf_chain(lPBuf, lTailPBuf);
                } else {
                    // Somehow the next pbuf returned an error.
                    // Invalidate overall packet.
                    lDescriptor.GiveToHW();
                    return nullptr;
                }
            } else {
                // This is the last descriptor of the frame:
                // Adjust the link statistics.
                LINK_STATS_INC(link.recv);

#if LWIP_PTPD
                u32_t time_s = 0;
                u32_t time_ns = 0;
                // Get the current timestamp if PTPD is enabled.
                lwIPHostGetTime(&time_s, &time_ns);

                // Place the timestamp in the PBUF.
                lPBuf->time_s = time_s;
                lPBuf->time_ns = time_ns;
#endif
            }

            return lPBuf;
        }
    }

#if 0
    // Adjust the link statistics.
    LINK_STATS_INC(link.memerr);
    LINK_STATS_INC(link.drop);
#endif
    return nullptr;
}


void EthDrv::FreePBuf(struct pbuf *const aPBuf) {
    // Follow the chain of pbuf to give back RxDescriptors to HW.
    struct pbuf *lQueuePtr = aPBuf;
    do {
        EthDrv::RxDescriptor * const lDescriptor = mRxDescriptors.GetDescriptor(lQueuePtr);
        lDescriptor->GiveToHW();
        // Link in the next pbuf in the chain.
        lQueuePtr = lQueuePtr->next;
    //} while (lQueuePtr != nullptr);
    } while (lQueuePtr->len != lQueuePtr->tot_len);

    // Finally, free the pbuf. The reference count should be 0 afterward.
    pbuf_free(aPBuf);
}


EthDrv::RxDescriptor::RxDescriptor(struct pbuf * const aBuffer)
    : tEMACDMADescriptor {
        .ui32CtrlStatus = 0
        , .ui32Count = (DES1_RX_CTRL_CHAINED | (static_cast<uint32_t>(aBuffer->len) << DES1_RX_CTRL_BUFF1_SIZE_S))
        , .pvBuffer1 = aBuffer->payload
        , .DES3 = {0}
        , .ui32ExtRxStatus = 0
        , .ui32Reserved = 0
        , .ui32IEEE1588TimeLo = 0
        , .ui32IEEE1588TimeHi = 0
    }
    , mPBuf(aBuffer) {

    // Ctor body.
}


EthDrv::TxDescriptor::TxDescriptor()
    : tEMACDMADescriptor {
        .ui32CtrlStatus = DES0_TX_CTRL_INTERRUPT | DES0_TX_CTRL_CHAINED | DES0_TX_CTRL_IP_ALL_CKHSUMS
        , .ui32Count = 0
        , .pvBuffer1 = nullptr
        , .DES3 = {0}
        , .ui32ExtRxStatus = 0
        , .ui32Reserved = 0
        , .ui32IEEE1588TimeLo = 0
        , .ui32IEEE1588TimeHi = 0
    }
{
    // Ctor body.
}


void EthDrv::TxDescriptor::FreePBuf(void) {
    if (mPBuf != nullptr) {
        pbuf_free(mPBuf);
        pvBuffer1 = nullptr;
    }
}


EthDrv::TxDescriptor *EthDrv::TxRingBuf::Create(size_t aSize) {
    // Fill the map of descriptors.
    for (size_t lIx = 0; lIx < aSize; lIx++) {
        TxDescriptor *lDescriptor = new TxDescriptor;
        if (lDescriptor != nullptr) {
            if (mHead == nullptr) {
                // Add to empty.
                mHead = lDescriptor;
                lDescriptor->ChainTo(mHead);
            } else {
                // Insert at "end" of circular list.
                lDescriptor->ChainTo(static_cast<TxDescriptor *>(mHead->DES3.pLink));
                mHead->ChainTo(lDescriptor);
            }
            // Don't give descriptor to HW yet: this will be done when pbufs are assigned.
        } else {
            // Failed to allocated number of requested descriptors in the chain.
            // Free them and bailout.
            Free();
            return nullptr;
        }
    }

    mSize = aSize;
    mTail = mHead;
    mBkp = mHead;
    return mHead;
}


EthDrv::TxDescriptor *EthDrv::TxRingBuf::PutPBufs(struct pbuf *aPBuf, bool aIsFirstPBuf) {
    TxDescriptor *lDescriptor = mHead;
    if (aPBuf && !IsFull() && !lDescriptor->IsHWOwned()) {
        lDescriptor->SetPBuf(aPBuf);
        lDescriptor->SetPayload(aPBuf->payload);
        lDescriptor->SetLen(aPBuf->len);
        mHead = lDescriptor->GetNext();
        return lDescriptor;
    }

    // Either no pbuf, ring full or descriptor owned by HW.
    return nullptr;
}


struct pbuf *EthDrv::TxRingBuf::GetPBufs(TxDescriptor * const aCurrent) {

    // Should be SW-owned now, but check anyway.
    if ((mTail != aCurrent) && !mTail->IsHWOwned()) {
        // This descriptor was released from the HW.
        // Increment Tail as far as the pbuf spans.
        struct pbuf *lPBuf = mTail->GetPBuf();
        mTail = mTail->GetNext();
        while (lPBuf->next != nullptr) {
            // Opt: check that the stored pbuf for this descriptor is null as expected.
            lPBuf = lPBuf->next;
            mTail = mTail->GetNext();
        }

        return lPBuf;
    }

    // Reached current descriptor or hit a busy descriptor prematurely: bail out.
    return nullptr;
}


EthDrv::RxDescriptorChain::~RxDescriptorChain() {

    // Free all pbufs and descriptors. Use the map.
    for (auto lIt = mMap.begin(); lIt != mMap.end(); ++lIt) {
        pbuf_free(lIt->first);
        delete lIt->second;
    }
}


EthDrv::RxDescriptor &EthDrv::RxDescriptorChain::GetNext(void) {
    mCurrentDescriptor = mCurrentDescriptor->GetNext();
    return *mCurrentDescriptor;
}


EthDrv::RxDescriptor *EthDrv::RxDescriptorChain::Create(uint32_t aBaseAddr, unsigned int aChainSize, unsigned int aPktSize) {

    // Fill the map of descriptors.
    for (unsigned int lIx = 0; lIx < aChainSize; lIx++) {
        Add(aPktSize);
    }

    // Return the next descriptor already, since this is the one that will be requested
    // on the 1st call to lowLevelRx().
    return mCurrentDescriptor->GetNext();
}


void EthDrv::RxDescriptorChain::Add(unsigned int aPktSize) {

    struct pbuf *lPBuf = nullptr;
    if (aPktSize) {
        lPBuf = pbuf_alloc(PBUF_RAW, aPktSize, PBUF_RAM);
    }

    if (lPBuf != nullptr) {
        RxDescriptor *lDescriptor = new RxDescriptor(lPBuf);
        // Add to circular list. Leverage the chain pointer part of the descriptor.
        if (lDescriptor != nullptr) {
            if (mCurrentDescriptor == nullptr) {
                // Add to empty.
                mCurrentDescriptor = lDescriptor;
                lDescriptor->ChainTo(mCurrentDescriptor);
            } else {
                // Insert at "end" of circular list.
                lDescriptor->ChainTo(static_cast<RxDescriptor *>(mCurrentDescriptor->DES3.pLink));
                mCurrentDescriptor->ChainTo(lDescriptor);
            }
            lDescriptor->GiveToHW();

            // Keep a reference of the pbuf assigned to the descriptor.
            mMap[lPBuf] = lDescriptor;
        } else {
            pbuf_free(lPBuf);
        }
    }
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
