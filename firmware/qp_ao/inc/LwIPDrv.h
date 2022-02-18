#pragma once
// *******************************************************************************
//
// Project: LwIP
//
// Module: LwIP Ethernet driver.
//
// *******************************************************************************

//! \file
//! \brief LwIP Ethernet Driver class.
//! \ingroup lwip

// ******************************************************************************
//
//        Copyright (c) 2015-2021, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include "net/EthernetAddress.h"
#include "net/IPAddress.h"

#include "lwip/err.h"
#include "lwip/netif.h"

#include <vector>

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// Forward declarations
namespace QP {
    class QActive;
}


class LwIPDrv {
public:
    virtual ~LwIPDrv() = default;

    static void StaticInit(
        QP::QActive * const aAO,
        bool const aUseDHCP,
        IPAddress const aIPAddress,
        IPAddress const aSubnetMask,
        IPAddress const aGWAddress
    );
    static void StaticISR(unsigned int const aIndex);
    virtual void Rd(void) = 0;
    virtual void Wr(void) = 0;

    static EthernetAddress const &StaticGetMACAddress(unsigned int const aIndex);
    static IPAddress StaticGetIPAddress(unsigned int const aIndex);
    static IPAddress StaticGetSubnetMask(unsigned int const aIndex);
    static IPAddress StaticGetDefaultGW(unsigned int const aIndex);

    static void DNSFoundCallback(
        const char *aName,
        const ip_addr_t *aIPAddr,
        void *aArgs
    );

    EthernetAddress const &GetMACAddress(void) const;
    IPAddress GetIPAddress(void) const;
    IPAddress GetSubnetMask(void) const;
    IPAddress GetDefaultGW(void) const;
    void StartIPCfg(void);

    virtual void PHYISR(void) {/*DoNothing();*/}
    virtual void DisableAllInt(void) = 0;
    virtual void EnableAllInt(void) = 0;

protected:
    LwIPDrv(unsigned int const aIndex, EthernetAddress const &aEthernetAddress);

    void PostRxEvent(void);
    void PostTxEvent(void);
    void PostOverrunEvent(void);
    void PostNetIFChangedEvent(bool const aIsUp);
    void PostLinkChangedEvent(bool const aIsUp);
    void PostPHYInterruptEvent(void);

    unsigned int GetIndex(void) const {return mMyIndex;}
    struct netif &GetNetIF(void) {return mNetIF;}
    QP::QActive &GetAO(void) const {return *mAO;}
    bool IsUsingDHCP(void) const{return mUseDHCP;}
    void SetAO(QP::QActive * const aAO) {mAO = aAO;}
    void UseDHCP(bool aUseDHCP) {mUseDHCP = aUseDHCP;}

private:
    void DrvInit(
        QP::QActive * const aAO,
        bool const aUseDHCP,
        IPAddress const aIPAddress,
        IPAddress const aSubnetMask,
        IPAddress const aGWAddress
    );

    // Static functions to hook to 'C' code.
    // Specific to an Ethernet IF.
    static err_t StaticEtherIFInit(struct netif * const aNetIF);
    static err_t StaticEtherIFOut(struct netif * const aNetIF, struct pbuf * const aPBuf);

#if LWIP_NETIF_STATUS_CALLBACK
    static void StaticStatusCallback(struct netif * const aNetIF);
    virtual void StatusCallback(struct netif * const aNetIF) {static_cast<void>(aNetIF);}
#endif // LWIP_NETIF_STATUS_CALLBACK

#if LWIP_NETIF_LINK_CALLBACK
    static void StaticLinkCallback(struct netif * const aNetIF);
    virtual void LinkCallback(struct netif * const aNetIF) { static_cast<void>(aNetIF);}
#endif // LWIP_NETIF_LINK_CALLBACK

#if LWIP_NETIF_EXT_STATUS_CALLBACK
    static void StaticExtCallback(
        struct netif * const aNetIF,
        netif_nsc_reason_t const aReason,
        netif_ext_callback_args_t const *aArgs
    );
    virtual void ExtCallback(
        netif_nsc_reason_t const aReason,
        netif_ext_callback_args_t const *aArgs
    );
#endif // LWIP_NETIF_EXT_STATUS_CALLBACK

    virtual err_t EtherIFOut(struct pbuf * const aPBuf) = 0;

    virtual err_t EtherIFInit(struct netif * const aNetIF) = 0;
    virtual void ISR(void) = 0;

    LwIPDrv(LwIPDrv const &) = delete;
    LwIPDrv const &operator=(LwIPDrv const &) = delete;

    static std::vector<LwIPDrv *> sVector;

    unsigned int const mMyIndex;
    EthernetAddress mEthernetAddress;
    struct netif mNetIF;
    netif_ext_callback_t mExtCallback;
    bool mUseDHCP;
    QP::QActive *mAO = nullptr;
};

// ******************************************************************************
//                            EXPORTED VARIABLES
// ******************************************************************************

// ******************************************************************************
//                                 EXTERNS
// ******************************************************************************

// ******************************************************************************
//                            EXPORTED FUNCTIONS
// ******************************************************************************

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
