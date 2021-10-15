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

// LwIP.
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/stats.h"

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
#include "netif/tm4c129/CustomPBuf.h"

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
    unsigned int aBufQueueSize,
    uint32_t aSysClk
)   : LwIPDrv(aIndex, aEthernetAddress)
    , mRxRingBuf()
    , mTxRingBuf()
    , mSysClk(aSysClk) {

    // Ctor body.
}


void EthDrv::Rd(void) {
    // New packet received into the pbuf?
    //struct pbuf * const lPBuf = LowLevelRx();
    // Get a free descriptor from the ring buffer.
    RxDescriptor *lDescriptor = mRxRingBuf.GetNext();
    while (lDescriptor != nullptr) {
        struct pbuf * const lPBuf = LowLevelRx(lDescriptor);
        if (lPBuf != nullptr) {
            // pbuf handled?
            if (ethernet_input(lPBuf, &GetNetIF()) != ERR_OK) {
                // If internal error, the pbuf is freed then?
                pbuf_free(lPBuf);
            }
        } else {
            // Could not create custom PBuf.
        }
        // Get a free descriptor from the ring buffer.
        lDescriptor = mRxRingBuf.GetNext();
    }

    // Re-enable the RX interrupt.
    MAP_EMACIntEnable(EMAC0_BASE, EMAC_INT_RECEIVE);
}

#if 0
void EthDrv::Wr(void) {
    // We should never get here from a Tx interrupt.
}
#endif

void EthDrv::PHYISR(void) {
    // This PHYISR handler is called in normal "task" context.
    // EthDrv::ISR() makes call to read and clear interrupt status.

    // Reading the interrupt status clears the bits.
    uint16_t lEPHYMISR1 = MAP_EMACPHYRead(EMAC0_BASE, EMAC_PHY_ADDR, EPHY_MISR1);
    if (lEPHYMISR1 & EPHY_MISR1_LINKSTAT) {
        uint16_t lBMSR = MAP_EMACPHYRead(EMAC0_BASE, EMAC_PHY_ADDR, EPHY_BMSR);
        if (lBMSR & EPHY_BMSR_LINKSTAT) {
            PostLinkChangedEvent(true);
        } else {
            PostLinkChangedEvent(false);
        }
    }

    if ((lEPHYMISR1 & EPHY_MISR1_SPEED)
        || (lEPHYMISR1 & EPHY_MISR1_DUPLEXM)
        || (lEPHYMISR1 & EPHY_MISR1_ANC)) {

        uint32_t lCfg = 0;
        uint32_t lMode = 0;
        uint32_t lRxMaxFrameSize = 0;
        MAP_EMACConfigGet(EMAC0_BASE, &lCfg, &lMode, &lRxMaxFrameSize);

        uint16_t lStatus = MAP_EMACPHYRead(EMAC0_BASE, EMAC_PHY_ADDR, EPHY_STS);
        if (lStatus & EPHY_STS_SPEED) {
            lCfg &= ~EMAC_CONFIG_100MBPS;
        } else {
            lCfg |= EMAC_CONFIG_100MBPS;
        }

        if (lStatus & EPHY_STS_DUPLEX) {
            lCfg |= EMAC_CONFIG_FULL_DUPLEX;
        } else {
            lCfg &= ~EMAC_CONFIG_FULL_DUPLEX;
        }

        MAP_EMACConfigSet(EMAC0_BASE, lCfg, lMode, lRxMaxFrameSize);
    }

    // Now that the interrupt source was likely cleared, clear the flag.
    // Re-enable PHY interrupt.
    MAP_EMACIntEnable(EMAC0_BASE, EMAC_INT_PHY);
}


void EthDrv::DisableAllInt(void) {
    MAP_EMACIntDisable(EMAC0_BASE, EMAC_INT_TRANSMIT | EMAC_INT_RECEIVE);
}


void EthDrv::EnableAllInt(void) {
    // Enable Ethernet TX and RX Packet Interrupts.
    MAP_EMACIntEnable(EMAC0_BASE, EMAC_INT_TRANSMIT | EMAC_INT_RECEIVE);

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
    TxDescriptor * const lTxDMACurrentDescriptor =
        static_cast<TxDescriptor * const>(EMACTxDMACurrentDescriptorGet(EMAC0_BASE));
    bool lResult = mTxRingBuf.PushPBuf(lTxDMACurrentDescriptor, aPBuf, true);
    if (lResult) {
        // Don't release the pbuf after this call.
        // It will be released once the packet is out.
        pbuf_ref(aPBuf);
        // Unblock the transmitter potentially in suspended state.
        EMACTxDMAPollDemand(EMAC0_BASE);
        return ERR_OK;
    }

    // Error while assigning pbuf to descriptor chain.
    return ERR_BUF;
}


// Initialize Ethernet IF as per TivaWare Driver doc under:
// 10.2.4.26 EMACPHYConfigSet
err_t EthDrv::EtherIFInit(struct netif * const aNetIF) {

    // Initialize the hardware...
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_EMAC0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_EPHY0);

    MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_EMAC0);
    // This reset is handled in EMACPHYConfigSet().
    //MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_EPHY0);

    // Ensure the MAC has completed its reset.
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_EMAC0)) {
    }

    // This driver uses the internal PHY.
    // Configure for use with the internal PHY.
    // Set the PHY type and configuration options.
    MAP_EMACPHYConfigSet(
        EMAC0_BASE,
        EMAC_PHY_TYPE_INTERNAL | EMAC_PHY_INT_MDIX_EN | EMAC_PHY_AN_100B_T_FULL_DUPLEX
    );

    // Configure PHY interrupts.
    // Listen to interrupts:
    //    -"Change of link status"
    //    -"Change of Speed Status"
    //    -"Change of Duplex Status"
    MAP_EMACPHYWrite( EMAC0_BASE, EMAC_PHY_ADDR, EPHY_CFG1, EPHY_CFG1_DONE);
    uint16_t lSCR = MAP_EMACPHYRead(EMAC0_BASE, EMAC_PHY_ADDR, EPHY_SCR);
    lSCR |= EPHY_SCR_INTEN | EPHY_SCR_INTOE_EXT;
    MAP_EMACPHYWrite(EMAC0_BASE, EMAC_PHY_ADDR, EPHY_SCR, lSCR);
    MAP_EMACPHYWrite(
        EMAC0_BASE,
        EMAC_PHY_ADDR,
        EPHY_MISR1,
        EPHY_MISR1_LINKSTATEN | EPHY_MISR1_SPEEDEN | EPHY_MISR1_DUPLEXMEN | EPHY_MISR1_ANCEN
    );
    // Reset the MAC to latch the PHY configuration.
    // BUG: DOESN'T WORK IF LEFT HERE AS PER USER MANUAL.
    //MAP_EMACReset(EMAC0_BASE);

    // Maximum transfer unit.
    aNetIF->mtu = 1500;

    // Set device capabilities.
    // NETIF_FLAG_UP and NETIF_FLAG_LINK_UP will be set when turning on the
    // network interface and when the link comes up respectively.
    aNetIF->flags |= (NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP);

    // Disable all Ethernet interrupts.
    uint32_t lAllInts = (
        // PHY interrupts.
        EMAC_INT_PHY
        // Normal interrupts.
        | EMAC_INT_TRANSMIT | EMAC_INT_TX_NO_BUFFER | EMAC_INT_RECEIVE | EMAC_INT_EARLY_RECEIVE
        // Abnormal interrupts.
        | EMAC_INT_TX_STOPPED | EMAC_INT_TX_JABBER | EMAC_INT_RX_OVERFLOW | EMAC_INT_TX_UNDERFLOW
        | EMAC_INT_RX_NO_BUFFER | EMAC_INT_RX_STOPPED | EMAC_INT_RX_WATCHDOG | EMAC_INT_EARLY_TRANSMIT
        | EMAC_INT_BUS_ERROR
    );
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
        mSysClk, // Can't use SysCtlClockGet() with TM4C129!!!
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
            EMAC_CONFIG_STRIP_CRC |
            EMAC_CONFIG_CHECKSUM_OFFLOAD |
            EMAC_CONFIG_7BYTE_PREAMBLE |
            EMAC_CONFIG_IF_GAP_96BITS |
            EMAC_CONFIG_USE_MACADDR0 |
            EMAC_CONFIG_SA_FROM_DESCRIPTOR |
            EMAC_CONFIG_BO_LIMIT_1024 |
            EMAC_CONFIG_AUTO_CRC_STRIPPING),
        (EMAC_MODE_RX_STORE_FORWARD |
            EMAC_MODE_TX_STORE_FORWARD),
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
    CustomPBuf::Init();
    static constexpr unsigned int sDescriptorQty = 8;
    static constexpr size_t sBufferSize = 540;
    RxDescriptor * const lRxList = mRxRingBuf.Create(EMAC0_BASE, sDescriptorQty, sBufferSize);
    MAP_EMACRxDMADescriptorListSet(EMAC0_BASE, lRxList);
    TxDescriptor * const lTxList = mTxRingBuf.Create(sDescriptorQty);
    MAP_EMACTxDMADescriptorListSet(EMAC0_BASE, lTxList);

    // Enable the Ethernet MAC transmitter and receiver.
    MAP_EMACTxEnable(EMAC0_BASE);
    MAP_EMACRxEnable(EMAC0_BASE);
    return ERR_OK;
}


void EthDrv::ISR(void) {
    // Get and clear the interrupt sources.
    static constexpr bool sIsMasked = true;
    uint32_t lStatus = MAP_EMACIntStatus(EMAC0_BASE, sIsMasked);
    MAP_EMACIntDisable(EMAC0_BASE, lStatus);
    MAP_EMACIntClear(EMAC0_BASE, lStatus);

    // Process normal interrupts.
    if (lStatus & EMAC_INT_RECEIVE) {
        // Send to the AO.
        PostRxEvent();
    }

    if (lStatus & EMAC_INT_TRANSMIT) {
        // Frame transmission is complete.
        // Try to advance the end pointer of the free descriptor list.
        TxDescriptor * const lTxDMACurrentDescriptor =
            static_cast<TxDescriptor * const>(EMACTxDMACurrentDescriptorGet(EMAC0_BASE));
        bool lResult = mTxRingBuf.PopPBuf(lTxDMACurrentDescriptor);
        if (lResult) {
            LINK_STATS_INC(link.xmit);
        } else {
            LINK_STATS_INC(link.err);
        }
        EMACIntEnable(EMAC0_BASE, EMAC_INT_TRANSMIT);
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
        // Handler will restore PHY interrupts once they are handled.
        PostPHYInterruptEvent();
    }
}


void EthDrv::EnableRxInt(void) {
    MAP_EMACIntEnable(EMAC0_BASE, EMAC_INT_RECEIVE);
}


// This function will read a single packet from the Stellaris ethernet
// interface, if available, and return a pointer to a pbuf.  The timestamp
// of the packet will be placed into the pbuf structure.
// * @return pointer to pbuf packet if available, nullptr otherswise.
struct pbuf *EthDrv::LowLevelRx(RxDescriptor * const aDescriptor) {

#if 0
    // SW should always own the current descriptor since rx interrupt triggered this call.
    // If not, then the pointer got out of sync?
    if (!lDescriptor.IsHWOwned()) {
        if (lDescriptor.IsFrameValid()) {
            // Get the pbuf assigned as payload of the descriptor.
            // It should never be null.
            struct pbuf *lPBuf = mRxRingBuf.GetPBuf(&lDescriptor);
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

#else

    if (!aDescriptor->IsHWOwned() && aDescriptor->IsFrameValid()) {
        // Get the pbuf assigned as payload of the descriptor.
        // It should never be null.
        // Create a custom pbuf.
        CustomPBuf *lCustomPBuf = nullptr;
        if (aDescriptor->IsFirst()) {
            // The chain of descriptor is attached to the 1st PBuf only.
            lCustomPBuf = CustomPBuf::New(aDescriptor);
        } else {
            lCustomPBuf = CustomPBuf::New();
        }

        // Recurse-call if this is not the last descriptor of the chain:
        // Reference and pbuf-chain all payloads of the chain of descriptors.
        if (lCustomPBuf != nullptr) {
            if (!aDescriptor->IsLast()) {
                RxDescriptor * const lNextDescriptor = mRxRingBuf.GetNext();
                struct pbuf_custom * const lHeadPBuf = lCustomPBuf->GetPBuf();
                struct pbuf * const lTailPBuf = LowLevelRx(lNextDescriptor);
                if ((lNextDescriptor != nullptr) && (lTailPBuf != nullptr)) {
                    pbuf_chain(&lHeadPBuf->pbuf, lTailPBuf);
                }
            } else {
                // Refer to the payload data of the descriptor.
                struct pbuf * const lPBuf = lCustomPBuf->Alloced();
                if (lPBuf != nullptr) {
                    return lPBuf;
                }
            }
        }
    }

    // Somehow the next pbuf returned an error.
    // Invalidate overall packet.
    aDescriptor->GiveToHW();
    return nullptr;

#endif
}

#if 0
void EthDrv::FreePBuf(struct pbuf *const aPBuf) {
    // Follow the chain of pbuf to give back RxDescriptors to HW.
    struct pbuf *lQueuePtr = aPBuf;
    do {
        EthDrv::RxDescriptor * const lDescriptor = mRxRingBuf.GetDescriptor(lQueuePtr);
        lDescriptor->GiveToHW();
        // Link in the next pbuf in the chain.
        lQueuePtr = lQueuePtr->next;
    //} while (lQueuePtr != nullptr);
    } while (lQueuePtr->len != lQueuePtr->tot_len);

    // Finally, free the pbuf. The reference count should be 0 afterward.
    pbuf_free(aPBuf);
}
#endif


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
