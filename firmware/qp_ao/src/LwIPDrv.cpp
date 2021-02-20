// *****************************************************************************
//
// Project: LwIP
//
// Module: LwIP Ethernet driver.
//
// *******************************************************************************

//! \file
//! \brief LwIP Ethernet Driver class.
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
#include "lwip/init.h"
#include "lwip/def.h"
#include "lwip/err.h"
#include "lwip/dns.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"
#include "netif/etharp.h"
} // extern "C"

#include "Signals.h"

#include "LwIP_Events.h"
#include "LwIPDrv.h"

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

//std::map<struct netif * const, LwIPDrv * const> LwIPDrv::sMap;
std::vector<LwIPDrv *> LwIPDrv::sVector({nullptr});

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

void LwIPDrv::StaticInit(
    QP::QActive * const aAO,
    bool aUseDHCP,
    uint32_t aIPAddress,
    uint32_t aSubnetMask,
    uint32_t aGWAddress
) {
    // Go through all registered network drivers and call their Init() function.
    // For now, this forces to have the same IP address scheme, but for now it's fine.
    // Could at least increment IP address?
    for (std::vector<LwIPDrv *>::iterator lIt = sVector.begin(); lIt != sVector.end(); ++lIt) {
        (*lIt)->DrvInit(aAO, aUseDHCP, aIPAddress, aSubnetMask, aGWAddress);
    }
}


void LwIPDrv::StaticRd(unsigned int aIndex) {
    sVector[aIndex]->Rd();
}


void LwIPDrv::StaticWr(unsigned int aIndex) {
    sVector[aIndex]->Wr();
}


void LwIPDrv::StaticISR(unsigned int aIndex) {
    sVector[aIndex]->ISR();
}


uint8_t const *LwIPDrv::StaticGetMACAddress(unsigned int aIndex) {
    return sVector[aIndex]->GetMACAddress();
}


uint32_t LwIPDrv::StaticGetIPAddress(unsigned int aIndex) {
    return sVector[aIndex]->GetIPAddress();
}


uint32_t LwIPDrv::StaticGetSubnetMask(unsigned int aIndex) {
    return sVector[aIndex]->GetSubnetMask();
}


uint32_t LwIPDrv::StaticGetDefaultGW(unsigned int aIndex) {
    return sVector[aIndex]->GetDefaultGW();
}


void LwIPDrv::DNSFoundCallback(
    const char *aName,
    const ip_addr_t *aIPAddr,
    void *aArgs
) {
    LwIPDrv * const lThis = static_cast<LwIPDrv * const>(aArgs);

    // DNS found a matching IP address.
    LwIP::Event::HostNameFound * const lEvent = Q_NEW(
        LwIP::Event::HostNameFound,
        LWIP_HOST_NAME_FOUND_SIG,
        lThis->GetIndex(),
        aName,
        aIPAddr
    );

    lThis->GetAO().POST(lEvent, lThis);
}


void LwIPDrv::StaticStatusCallback(struct netif * const aNetIF) {
    // Check the current state of the network interface.
    // We end up here as a result to call to either:
    // netif_set_up(), netif_set_down(), netif_set_ipaddr().
    // Signal the AO to react to it.
    LwIPDrv * const lThis = static_cast<LwIPDrv * const>(aNetIF->state);
    static QP::QEvt const sNetIFEvent(LWIP_NETIF_CHANGED_SIG);
    lThis->GetAO().POST(&sNetIFEvent, lThis);
}


void LwIPDrv::StaticLinkCallback(struct netif * const aNetIF) {
    // We end up here as a result of call to either:
    // netif_set_link_up(), netif_set_link_down().
    // Signal the AO to react to it.
    LwIPDrv * const lThis = static_cast<LwIPDrv * const>(aNetIF->state);
    static QP::QEvt const sLinkEvent(LWIP_LINK_CHANGED_SIG);
    lThis->GetAO().POST(&sLinkEvent, lThis);
}


uint8_t const *LwIPDrv::GetMACAddress(void) const {
    return &mNetIF.hwaddr[0];
}


uint32_t LwIPDrv::GetIPAddress(void) const {
    return mNetIF.ip_addr.addr;
}


uint32_t LwIPDrv::GetSubnetMask(void) const {
    return mNetIF.netmask.addr;
}


uint32_t LwIPDrv::GetDefaultGW(void) const {
    return mNetIF.gw.addr;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

LwIPDrv::LwIPDrv(unsigned int aIndex, EthernetAddress const &aEthernetAddress, unsigned int aPBufQueueSize)
    : mMyIndex(aIndex)
    , mNetIF{0} {

    mPBufQ = new PBufQ(aPBufQueueSize);

    // Associate this <struct netif *, LwIPDrv>.
    LwIPDrv::sVector[aIndex] = this;

    // Set MAC address in the network interface...
    GetNetIF().hwaddr_len = NETIF_MAX_HWADDR_LEN;
    memcpy(&GetNetIF().hwaddr[0], aEthernetAddress.GetData(), NETIF_MAX_HWADDR_LEN);
}


void LwIPDrv::DrvInit(
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
    // Use *this* as the state.
    netif_add(
        &GetNetIF(),
	    &lIPAddr,
	    &lSubnetMask,
	    &lGWAddr,
	    this,
	    &LwIPDrv::StaticEtherIFInit,
        &ip_input
    );

    // Set status callback.
    // Called after: netif_set_ipaddr(), netif_set_up(), netif_set_down().
    netif_set_status_callback(&GetNetIF(), LwIPDrv::StaticStatusCallback);

    // Set link callback.
    // Called after netif_set_link_up(), netif_set_link_down().
    netif_set_link_callback(&GetNetIF(), LwIPDrv::StaticLinkCallback);

#if (LWIP_DNS != 0)
    // Add Google DNS.
    ip_addr_t lDNS;
    IP4_ADDR(&lDNS, 8, 8, 8, 8);
    dns_setserver(0, &lDNS);
    IP4_ADDR(&lDNS, 8, 8, 4, 4);
    dns_setserver(1, &lDNS);
    dns_init();
#endif

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
    autoip_start(&GetNetIF());
#endif

    // Everything is initialized: enable all interrupts.
    EnableAllInt();
}


// Low-level init.
// Will be called by LwIP at init stage, everytime a netif is added.
err_t LwIPDrv::StaticEtherIFInit(struct netif * const aNetIF) {
#if 0
    // Find the instance in hash that owns this struct netif.
    auto lIt = LwIPDrv::sMap.find(aNetIF);
    if (lIt != LwIPDrv::sMap.end()) {
        return lIt->second->EtherIFInit(aNetIF);
    }

    return ERR_ARG;
#else
    LwIPDrv * const lThis = static_cast<LwIPDrv * const>(aNetIF->state);
    return lThis->EtherIFInit(aNetIF);
#endif
}


err_t LwIPDrv::StaticEtherIFOut(struct netif * const aNetIF, struct pbuf * const aPBuf) {
#if 0
    // Find the instance in hash that owns this struct netif.
    auto lIt = LwIPDrv::sMap.find(aNetIF);
    if (lIt != LwIPDrv::sMap.end()) {
        return lIt->second->EtherIFOut(aNetIF, aPBuf);
    }

    return ERR_ARG;
#else
    LwIPDrv * const lThis = static_cast<LwIPDrv * const>(aNetIF->state);
    return lThis->EtherIFOut(aNetIF, aPBuf);
#endif
}


// This function will either write the pbuf into the Stellaris TX FIFO,
// or will put the packet in the TX queue of pbufs for subsequent
// transmission when the transmitter becomes idle.
//
// @param netif the lwip network interface structure for this ethernetif
// @param p the pbuf to send
// @return ERR_OK if the packet could be sent
//         an err_t value if the packet couldn't be sent
err_t LwIPDrv::EtherIFOut(struct netif * const aNetIF, struct pbuf * const aPBuf) {

    // Nothing in the TX queue?
    // TX empty?
    //if (GetPBufQ().IsEmpty() && ((HWREG(ETH_BASE + MAC_O_TR) & MAC_TR_NEWTX) == 0)) {
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


void LwIPDrv::Rd(void) {

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
    EnableRxInt();
}


void LwIPDrv::Wr(void) {

    // TX fifo empty? Should be since we likely got here by TxEmpty int.
    //if ((HWREG(ETH_BASE + MAC_O_TR) & MAC_TR_NEWTX) == 0) {
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
void LwIPDrv::PostRxEvent(void) {
    static LwIP::Event::Interrupt const sRxEvent(LWIP_RX_READY_SIG, GetIndex());
    // Send to the AO.
    GetAO().POST(&sRxEvent, this);
}


void LwIPDrv::PostTxEvent(void) {
    static LwIP::Event::Interrupt const sTxEvent(LWIP_TX_READY_SIG, GetIndex());
    // Send to the AO.
    GetAO().POST(&sTxEvent, this);
}


void LwIPDrv::PostOverrunEvent(void) {
    static LwIP::Event::Interrupt const sOverrunEvent(LWIP_RX_OVERRUN_SIG, GetIndex());
    // Send to the AO.
    GetAO().POST(&sOverrunEvent, this);
}


LwIPDrv::PBufQ::PBufQ(unsigned int aQSize)
    : mPBufRing(nullptr)
    , mRingSize(aQSize)
    , mQWrIx(0)
    , mQRdIx(0)
    , mQOverflow(0) {

    // Ctor body.
    mPBufRing = new struct pbuf *[aQSize];
}


bool LwIPDrv::PBufQ::IsEmpty(void) const {
    return (mQWrIx == mQRdIx);
}


bool LwIPDrv::PBufQ::Put(struct pbuf * const aPBufPtr) {
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


struct pbuf *LwIPDrv::PBufQ::Get(void) {
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

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
