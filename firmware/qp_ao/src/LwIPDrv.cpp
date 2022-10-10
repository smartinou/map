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
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
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
#include "lwip/init.h"
#include "lwip/def.h"
#include "lwip/err.h"
#include "lwip/dns.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/snmp.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"
#include "netif/etharp.h"

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

std::vector<LwIPDrv::Ptr> LwIPDrv::sNetDrives{};

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

void LwIPDrv::StaticInit(
    // [MG] CONSIDER shared_ptr<QP::QActive>
    QP::QActive * const aAO,
    bool const aUseDHCP,
    IPAddress const aIPAddress,
    IPAddress const aSubnetMask,
    IPAddress const aGWAddress
) {
    // Go through all registered network drivers and call their Init() function.
    // For now, this forces to have the same IP address scheme, but for now it's fine.
    // Could at least increment IP address?
    for (auto &lNetDrv : sNetDrives) {
        lNetDrv->DrvInit(aAO, aUseDHCP, aIPAddress, aSubnetMask, aGWAddress);
    }
}


void LwIPDrv::StaticISR(unsigned int const aIndex) {
    sNetDrives.at(aIndex)->ISR();
}


EthernetAddress const &LwIPDrv::StaticGetMACAddress(unsigned int const aIndex) {
    return sNetDrives.at(aIndex)->GetMACAddress();
}


IPAddress LwIPDrv::StaticGetIPAddress(unsigned int const aIndex) {
    return sNetDrives.at(aIndex)->GetIPAddress();
}


IPAddress LwIPDrv::StaticGetSubnetMask(unsigned int const aIndex) {
    return sNetDrives.at(aIndex)->GetSubnetMask();
}


IPAddress LwIPDrv::StaticGetDefaultGW(unsigned int const aIndex) {
    return sNetDrives.at(aIndex)->GetDefaultGW();
}


void LwIPDrv::DNSFoundCallback(
    const char *aName,
    const ip_addr_t *aIPAddr,
    void *aArgs
) {
    // DNS found a matching IP address.
    LwIP::Event::HostNameFound * const lEvent = Q_NEW(
        LwIP::Event::HostNameFound,
        LWIP_HOST_NAME_FOUND_SIG,
        aName,
        aIPAddr
    );

    auto const lThis {static_cast<LwIPDrv *>(aArgs)};
    lThis->GetAO().POST(lEvent, lThis);
}


#if LWIP_NETIF_STATUS_CALLBACK
void LwIPDrv::StaticStatusCallback(struct netif * const aNetIF) {
    // Check the current state of the network interface.
    // We end up here as a result to call to either:
    // netif_set_up(), netif_set_down(), netif_set_ipaddr().
    auto const lThis {static_cast<LwIPDrv *>(aNetIF->state)};
    lThis->StatusCallback(aNetIF);
}
#endif // LWIP_NETIF_STATUS_CALLBACK


#if LWIP_NETIF_LINK_CALLBACK
void LwIPDrv::StaticLinkCallback(struct netif * const aNetIF) {
    // We end up here as a result of call to either:
    // netif_set_link_up(), netif_set_link_down().
    auto const lThis {static_cast<LwIPDrv *>(aNetIF->state)};
    lThis->LinkCallback(aNetIF);
}
#endif // LWIP_NETIF_LINK_CALLBACK


#if LWIP_NETIF_EXT_STATUS_CALLBACK
void LwIPDrv::StaticExtCallback(
    struct netif * const aNetIF,
    netif_nsc_reason_t const aReason,
    netif_ext_callback_args_t const *aArgs) {
    // We end up here as a result of call to either: LWIP_NSC_* (see netif.h)
    auto const lThis {static_cast<LwIPDrv *>(aNetIF->state)};
    lThis->ExtCallback(aReason, aArgs);
}


void LwIPDrv::ExtCallback(
    netif_nsc_reason_t const aReason,
    netif_ext_callback_args_t const *aArgs
) {
    static_cast<void>(aArgs);

#if LWIP_NETIF_STATUS_CALLBACK
    if (aReason | LWIP_NSC_STATUS_CHANGED) {
        StatusCallback(aNetIF);
    } else
#endif // LWIP_NETIF_STATUS_CALLBACK
#if LWIP_NETIF_LINK_CALLBACK
    if (aReason | LWIP_NSC_LINK_CHANGED) {
        LinkCallback(aNetIF);
    } else
#endif // LWIP_NETIF_LINK_CALLBACK
    if (aReason
        & (LWIP_NSC_IPV4_ADDRESS_CHANGED
            | LWIP_NSC_IPV4_NETMASK_CHANGED
            | LWIP_NSC_IPV4_GATEWAY_CHANGED)
    ) {
        // Some components of IP address changed.
        // Could cast aArgs to ipv4_changed* for old addresses.
        LwIP::Event::IPAddressChanged * const lEvent = Q_NEW(
            LwIP::Event::IPAddressChanged,
            LWIP_IP_CHANGED_SIG,
            IPAddress(ip4_addr_get_u32(netif_ip_addr4(&mNetIF))),
            IPAddress(ip4_addr_get_u32(netif_ip_netmask4(&mNetIF))),
            IPAddress(ip4_addr_get_u32(netif_ip_gw4(&mNetIF)))
        );
#ifdef Q_SPY
        static QP::QSpyId const sLwIPDrvExtCallback = {0U};
#endif // Q_SPY
        QP::QF::PUBLISH(lEvent, &sLwIPDrvExtCallback);
    } else {
        // Discard all other reasons (netif.h):
        // LWIP_NSC_NONE, LWIP_NSC_NETIF_ADDED, LWIP_NSC_NETIF_REMOVED
    }
}
#endif // LWIP_NETIF_EXT_STATUS_CALLBACK


EthernetAddress const &LwIPDrv::GetMACAddress(void) const {
    return mEthernetAddress;
}


IPAddress LwIPDrv::GetIPAddress(void) const {
    return IPAddress(ip4_addr_get_u32(netif_ip_addr4(&mNetIF)));
}


IPAddress LwIPDrv::GetSubnetMask(void) const {
    return IPAddress(ip4_addr_get_u32(netif_ip_netmask4(&mNetIF)));
}


IPAddress LwIPDrv::GetDefaultGW(void) const {
    return IPAddress(ip4_addr_get_u32(netif_ip_gw4(&mNetIF)));
}


void LwIPDrv::StartIPCfg(void) {
    if ((mNetIF.ip_addr.addr & mNetIF.netmask.addr) == 0) {
        // No IP address configured: set it automatically.
        if (IsUsingDHCP()) {
#if (LWIP_DHCP != 0)
            // Start DHCP if configured in lwipopts.h.
            dhcp_start(&GetNetIF());
            // NOTE: If LWIP_AUTOIP is configured in lwipopts.h and
            // LWIP_DHCP_AUTOIP_COOP is set as well, the DHCP process will start
            // AutoIP after DHCP fails for 59 seconds.
#endif
#if (LWIP_AUTOIP != 0)
        } else {
            // Start AutoIP if configured in lwipopts.h.
            autoip_start(&GetNetIF());
#endif
        }
    }
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

LwIPDrv::LwIPDrv(
    [[maybe_unused]] UseCreateFunc /* Dummy */,
    unsigned int const aIndex,
    EthernetAddress const &aEthernetAddress
) noexcept
    : mMyIndex{aIndex}
    , mEthernetAddress{aEthernetAddress}
    //, mNetIF{0}
    //, mExtCallback{0}
    //, mUseDHCP(false)
{
    // Set MAC address in the network interface...
    GetNetIF().hwaddr_len = NETIF_MAX_HWADDR_LEN;
    aEthernetAddress.GetData(&GetNetIF().hwaddr[0]);
}


void LwIPDrv::DrvInit(
    // [MG] CONSIDER shared_ptr<QP::QActive>
    QP::QActive * const aAO,
    bool const aUseDHCP,
    IPAddress const aIPAddr,
    IPAddress const aSubnetMask,
    IPAddress const aGWAddr
) {
    // Save the active object associated with this driver.
    SetAO(aAO);
    UseDHCP(aUseDHCP);

    ip_addr_t lIPAddr;
    ip_addr_t lSubnetMask;
    ip_addr_t lGWAddr;

    if (aUseDHCP) {
        ip4_addr_set_zero(&lIPAddr);
        ip4_addr_set_zero(&lSubnetMask);
        ip4_addr_set_zero(&lGWAddr);
    } else if (IPADDR_ANY != (aIPAddr.GetValue() & aSubnetMask.GetValue())) {
        // IP Address from persistence.
        ip4_addr_set_u32(&lIPAddr, aIPAddr.GetValue());
        ip4_addr_set_u32(&lSubnetMask, aSubnetMask.GetValue());
        ip4_addr_set_u32(&lGWAddr, aGWAddr.GetValue());
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
        ip4_addr_set_zero(&lIPAddr);
        ip4_addr_set_zero(&lSubnetMask);
        ip4_addr_set_zero(&lGWAddr);
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

#if LWIP_NETIF_STATUS_CALLBACK
    // Set status callback.
    // Called after: netif_set_ipaddr(), netif_set_up(), netif_set_down().
    netif_set_status_callback(&GetNetIF(), LwIPDrv::StaticStatusCallback);
#endif // LWIP_NETIF_STATUS_CALLBACK

#if LWIP_NETIF_LINK_CALLBACK
    // Set link callback.
    // Called after netif_set_link_up(), netif_set_link_down().
    netif_set_link_callback(&GetNetIF(), LwIPDrv::StaticLinkCallback);
#endif // LWIP_NETIF_LINK_CALLBACK

#if LWIP_NETIF_EXT_STATUS_CALLBACK
    // Ext callback.
    netif_add_ext_callback(&mExtCallback, LwIPDrv::StaticExtCallback);
#endif // LWIP_NETIF_EXT_STATUS_CALLBACK

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
    // Enable one-self: this should be done by an external object.
    PostNetIFChangedEvent(true);
}


// Low-level init.
// Will be called by LwIP at init stage, everytime a netif is added.
err_t LwIPDrv::StaticEtherIFInit(struct netif * const aNetIF) {
    auto const lThis {static_cast<LwIPDrv *>(aNetIF->state)};
    return lThis->EtherIFInit(aNetIF);
}


err_t LwIPDrv::StaticEtherIFOut(struct netif * const aNetIF, struct pbuf * const aPBuf) {
    auto const lThis {static_cast<LwIPDrv *>(aNetIF->state)};
    return lThis->EtherIFOut(aPBuf);
}


void LwIPDrv::PostRxEvent(void) {
    static LwIP::Event::Interrupt const sRxEvent(LWIP_RX_READY_SIG, &GetNetIF());
    // Send to the AO.
    GetAO().POST(&sRxEvent, this);
}


void LwIPDrv::PostTxEvent(void) {
    static LwIP::Event::Interrupt const sTxEvent(LWIP_TX_READY_SIG, &GetNetIF());
    // Send to the AO.
    GetAO().POST(&sTxEvent, this);
}


void LwIPDrv::PostOverrunEvent(void) {
    static LwIP::Event::Interrupt const sOverrunEvent(LWIP_RX_OVERRUN_SIG, &GetNetIF());
    // Send to the AO.
    GetAO().POST(&sOverrunEvent, this);
}


void LwIPDrv::PostNetIFChangedEvent(bool const aIsUp) {
    if (aIsUp) {
        static const LwIP::Event::NetStatusChanged sEvent(LWIP_NETIF_CHANGED_SIG, &GetNetIF(), true);
        GetAO().POST(&sEvent, this);
    } else {
        static const LwIP::Event::NetStatusChanged sEvent(LWIP_NETIF_CHANGED_SIG, &GetNetIF(), false);
        GetAO().POST(&sEvent, this);
    }
}


void LwIPDrv::PostLinkChangedEvent(bool const aIsUp) {
    if (aIsUp) {
        static const LwIP::Event::NetStatusChanged sEvent(LWIP_LINK_CHANGED_SIG, &GetNetIF(), true);
        GetAO().POST(&sEvent, this);
    } else {
        static const LwIP::Event::NetStatusChanged sEvent(LWIP_LINK_CHANGED_SIG, &GetNetIF(), false);
        GetAO().POST(&sEvent, this);
    }
}


void LwIPDrv::PostPHYInterruptEvent(void) {
    static LwIP::Event::Interrupt const sEvent(LWIP_PHY_INT_SIG, &GetNetIF());
    GetAO().POST(&sEvent, this);
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
