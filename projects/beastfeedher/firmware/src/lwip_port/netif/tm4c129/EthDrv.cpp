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

EthDrv::EthDrv(
    unsigned int aIndex,
    EthernetAddress const &aEthernetAddress,
    unsigned int aBufQueueSize
)   : LwIPDrv(aIndex, aEthernetAddress, aBufQueueSize)
    , mRxDescriptors(EMAC0_BASE, 10, 540) {

    // Ctor body.
}


void EthDrv::DisableAllInt(void) {

}


void EthDrv::EnableAllInt(void) {
    // Enable Ethernet TX and RX Packet Interrupts.
    MAP_EMACIntEnable(EMAC0_BASE, EMAC_INT_RECEIVE);
    //HWREG(ETH_BASE + MAC_O_IM) |= (ETH_INT_RX | ETH_INT_TX);

#if LINK_STATS
    MAP_EMACIntEnable(EMAC0_BASE, EMAC_INT_RX_OVERFLOW);
#endif
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************
#if 0
#define RX_BUFFER_SIZE 1536
EthDrv::RxDescriptor::RxDescriptor(void * const aBuffer) {

    // Mark descriptor as unavailable to HW.
    mDescriptor.ui32CtrlStatus = 0;
    mDescriptor.ui32Count = (DES1_RX_CTRL_CHAINED |
        (RX_BUFFER_SIZE << DES1_RX_CTRL_BUFF1_SIZE_S));
    mDescriptor.pvBuffer1 = static_cast<struct pbuf * const>(aBuffer)->payload;
}


void EthDrv::RxDescriptor::GiveToHW(void) {
    mDescriptor.ui32CtrlStatus |= DES0_RX_CTRL_OWN;
}


bool EthDrv::RxDescriptor::IsHWOwned(void) const {
    return mDescriptor.ui32CtrlStatus & DES0_RX_CTRL_OWN;
}


bool EthDrv::RxDescriptor::IsFrameValid(void) const {
    return !(mDescriptor.ui32CtrlStatus & DES0_RX_STAT_ERR);
}


bool EthDrv::RxDescriptor::IsLastFrame(void) const {
    return mDescriptor.ui32CtrlStatus & DES0_RX_STAT_LAST_DESC;
}


int32_t EthDrv::RxDescriptor::GetFrameLen(void) const {
    int32_t lLen = mDescriptor.ui32CtrlStatus & DES0_RX_STAT_FRAME_LENGTH_M;
    lLen >>= DES0_RX_STAT_FRAME_LENGTH_S;
    return lLen;
}


void EthDrv::RxDescriptor::ChainTo(RxDescriptor * const aRxDescriptor) {
    // [MG] PAS SUR QUE CA MARCHE DE MEME.
    // [MG] DESCRIPTOR EST PRIVATE.
    // [MG] DEVRAIT RETOURNER POINTEUR.
    // [MG] OU FAIRE UNE FONCTION STATIQUE (EUH, POURQUOI?)
    mDescriptor.DES3.pLink = aRxDescriptor->mDescriptor;
}
#endif

EthDrv::RxDescriptorChain::RxDescriptorChain(uint32_t aBaseAddr, unsigned int aQty, unsigned int aPktSize) {

    // Fill the vector and map of descriptors.
    for (unsigned int lIx = 0; lIx < aQty; lIx++) {
        struct pbuf *lPBuf = pbuf_alloc(PBUF_RAW, aPktSize, PBUF_POOL);
        Descriptor *lDescriptor = new Descriptor();
        lDescriptor->SetBuffer(lPBuf);
        mRxDescriptors.push_back(lDescriptor);
        mMap[lPBuf] = lDescriptor;
    }

    // Chain the descriptors of the vector.
    for (unsigned int lIx = 0; lIx < (mRxDescriptors.size() - 1); lIx++) {
        mRxDescriptors[lIx]->ChainTo(mRxDescriptors[lIx + 1]);
    }

    mRxDescriptors.back()->ChainTo(mRxDescriptors.front());

    // Set the descriptor pointers in the hardware.
    Descriptor *lDescriptor = mRxDescriptors.front();
    MAP_EMACRxDMADescriptorListSet(aBaseAddr, lDescriptor);//mRxDescriptors.front());
}


EthDrv::RxDescriptorChain::~RxDescriptorChain() {

    // Free all pbufs and descriptors.
    for (auto lIt = mMap.begin(); lIt != mMap.end(); ++lIt) {
        pbuf_free(lIt->first);
        delete lIt->second;
    }
}


EthDrv::Descriptor &EthDrv::RxDescriptorChain::GetNext(void) {
    // Get the next free descriptor.
    Descriptor * const lDescriptor = mRxDescriptors[mIndex];
    mIndex++;
    if (mIndex >= mRxDescriptors.size()) {
        mIndex = 0;
    }
    return *lDescriptor;
}


void EthDrv::RxDescriptorChain::Init(void) {

    for (auto lIt = mRxDescriptors.begin(); lIt != mRxDescriptors.end(); ++lIt) {
        (*lIt)->GiveToHW();
    }
}


// Initialize Ethernet IF as per TivaWare Driver doc under:
// 10.2.4.26 EMACPHYConfigSet
err_t EthDrv::EtherIFInit(struct netif * const aNetIF) {

    // Initialize the hardware...
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_EMAC0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_EPHY0);

    MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_EMAC0);
    MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_EPHY0);

    // Ensure the MAC is completed its reset.
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_EMAC0)) {
    }

    // This driver uses the internal PHY.
    // Configure for use with the internal PHY.
    // Set the PHY type and configuration options.
    MAP_EMACPHYConfigSet(
        EMAC0_BASE,
        (EMAC_PHY_TYPE_INTERNAL |
        EMAC_PHY_INT_MDIX_EN |
        EMAC_PHY_AN_100B_T_FULL_DUPLEX)
    );

    // Reset the MAC to latch the PHY configuration.
    MAP_EMACReset(EMAC0_BASE);

    // Maximum transfer unit.
    aNetIF->mtu = 1500;

    // Set device capabilities.
    aNetIF->flags = (NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP);

    // Disable all Ethernet interrupts.
    uint32_t lAllInts =
        (EMAC_INT_PHY | EMAC_INT_EARLY_RECEIVE | EMAC_INT_BUS_ERROR
        | EMAC_INT_EARLY_TRANSMIT | EMAC_INT_RX_WATCHDOG | EMAC_INT_RX_STOPPED | EMAC_INT_RX_NO_BUFFER
        | EMAC_INT_RECEIVE | EMAC_INT_TX_UNDERFLOW | EMAC_INT_RX_OVERFLOW | EMAC_INT_TX_JABBER
        | EMAC_INT_TX_NO_BUFFER | EMAC_INT_TX_STOPPED | EMAC_INT_TRANSMIT);
    MAP_EMACIntDisable(EMAC0_BASE, lAllInts);

    // Acknolwedge all interrupts.
    // Clear any pending interrupts.
    MAP_EMACIntClear(EMAC0_BASE, MAP_EMACIntStatus(EMAC0_BASE, false));

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

    // Wait for the link to become active.
    // [MG] POTENTIAL DEADLOCK IF NO CABLE CONNECTED?
    static uint8_t constexpr sPHYAddr = 0;
    while ((MAP_EMACPHYRead(EMAC0_BASE, sPHYAddr, EPHY_BMSR) & EPHY_BMSR_LINKSTAT) == 0) {
    }

    // Set MAC filtering options. We receive all broadcast and multicast
    // packets along with those addressed specifically for us.
    MAP_EMACFrameFilterSet(
        EMAC0_BASE,
        (EMAC_FRMFILTER_SADDR |
            //EMAC_FRMFILTER_PASS_MULTICAST |
            EMAC_FRMFILTER_PASS_NO_CTRL)
    );

    // Initialize the Ethernet DMA descriptors.
    mRxDescriptors.Init();

    // Enable the Ethernet MAC transmitter and receiver.
    MAP_EMACTxEnable(EMAC0_BASE);
    MAP_EMACRxEnable(EMAC0_BASE);

    return ERR_OK;
}


void EthDrv::ISR(void) {
#if 0
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

#else
    // Get and clear the interrupt sources.
    uint32_t lStatus = MAP_EMACIntStatus(EMAC0_BASE, true);
    MAP_EMACIntClear(EMAC0_BASE, lStatus);

    if ((lStatus & EMAC_INT_RECEIVE)) {
        // Send to the AO.
        PostRxEvent();
        // Disable further RX.
        MAP_EMACIntDisable(EMAC0_BASE, EMAC_INT_RECEIVE);
    }

#if LINK_STATS
    if (lStatus & EMAC_INT_RX_OVERFLOW) {
        // Send to the AO.
        PostOverrunEvent();
    }
#endif
#endif
}


void EthDrv::EnableRxInt(void) {
    MAP_EMACIntEnable(EMAC0_BASE, EMAC_INT_RECEIVE);
}


bool EthDrv::IsTxEmpty(void) const {
    return true;//((HWREG(ETH_BASE + MAC_O_TR) & MAC_TR_NEWTX) == 0);
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
#if 0
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
#endif
    LINK_STATS_INC(link.xmit);
}


// This function will read a single packet from the Stellaris ethernet
// interface, if available, and return a pointer to a pbuf.  The timestamp
// of the packet will be placed into the pbuf structure.
// * @return pointer to pbuf packet if available, nullptr otherswise.
struct pbuf *EthDrv::LowLevelRx(void) {

    Descriptor &lDescriptor = mRxDescriptors.GetNext();

    // SW should always own the current descriptor since rx interrupt triggered this call.
    // If not, then the pointer got out of sync?
    if (!lDescriptor.IsHWOwned()) {
        if (lDescriptor.IsFrameValid()) {
            // Get the pbuf assigned as payload of the descriptor.
            struct pbuf *lPBuf = static_cast<struct pbuf *>(lDescriptor.GetBuffer());
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
        EthDrv::Descriptor * const lDescriptor = mRxDescriptors.GetDescriptor(lQueuePtr);
        lDescriptor->GiveToHW();
        // Link in the next pbuf in the chain.
        lQueuePtr = lQueuePtr->next;
    //} while (lQueuePtr != nullptr);
    } while (lQueuePtr->len != lQueuePtr->tot_len);

    // Finally, free the pbuf.
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
