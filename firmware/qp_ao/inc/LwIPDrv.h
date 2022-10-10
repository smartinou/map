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

#include <memory>
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
    LwIPDrv(LwIPDrv const &) = delete;
    LwIPDrv &operator=(LwIPDrv const &) = delete;
    virtual ~LwIPDrv() = default;

    template <typename T, typename...Args>
    static void Create(Args&&... aArgs) {
        sNetDrives.emplace_back(
            std::make_unique<T>(
                typename T::UseCreateFunc{},
                std::forward<Args>(aArgs)...
            )
        );
    }

    static void StaticInit(
        QP::QActive * aAO,
        bool aUseDHCP,
        IPAddress aIPAddress,
        IPAddress aSubnetMask,
        IPAddress aGWAddress
    );
    static void StaticISR(unsigned int aIndex);
    virtual void Rd() = 0;
    virtual void Wr() = 0;

    static EthernetAddress const &StaticGetMACAddress(unsigned int aIndex);
    static IPAddress StaticGetIPAddress(unsigned int aIndex);
    static IPAddress StaticGetSubnetMask(unsigned int aIndex);
    static IPAddress StaticGetDefaultGW(unsigned int aIndex);

    static void DNSFoundCallback(
        const char *aName,
        const ip_addr_t *aIPAddr,
        void *aArgs
    );

    EthernetAddress const &GetMACAddress() const;
    IPAddress GetIPAddress() const;
    IPAddress GetSubnetMask() const;
    IPAddress GetDefaultGW() const;
    void StartIPCfg();

    virtual void PHYISR() {/*DoNothing();*/}
    virtual void DisableAllInt() = 0;
    virtual void EnableAllInt() = 0;

protected:
    struct UseCreateFunc {
        explicit UseCreateFunc() = default;
    };
    explicit LwIPDrv(
        UseCreateFunc /* Dummy */,
        unsigned int aIndex,
        EthernetAddress const &aEthernetAddress
    ) noexcept;

    void PostRxEvent();
    void PostTxEvent();
    void PostOverrunEvent();
    void PostNetIFChangedEvent(bool aIsUp);
    void PostLinkChangedEvent(bool aIsUp);
    void PostPHYInterruptEvent();

    unsigned int GetIndex() const {return mMyIndex;}
    struct netif &GetNetIF() {return mNetIF;}
    QP::QActive &GetAO() const {return *mAO;}
    bool IsUsingDHCP() const {return mUseDHCP;}
    // [MG] IS THIS REQUIRED?
    void SetAO(QP::QActive * const aAO) {mAO = aAO;}
    void UseDHCP(bool const aUseDHCP) {mUseDHCP = aUseDHCP;}

private:
    void DrvInit(
        //[MG] CONSIDER shared_ptr<QP::QActive>
        QP::QActive * aAO,
        bool aUseDHCP,
        IPAddress aIPAddress,
        IPAddress aSubnetMask,
        IPAddress aGWAddress
    );

    // Static functions to hook to 'C' code.
    // Specific to an Ethernet IF.
    static err_t StaticEtherIFInit(struct netif * aNetIF);
    static err_t StaticEtherIFOut(struct netif * aNetIF, struct pbuf * aPBuf);

#if LWIP_NETIF_STATUS_CALLBACK
    static void StaticStatusCallback(struct netif * aNetIF);
    virtual void StatusCallback(struct netif * aNetIF) {static_cast<void>(aNetIF);}
#endif // LWIP_NETIF_STATUS_CALLBACK

#if LWIP_NETIF_LINK_CALLBACK
    static void StaticLinkCallback(struct netif * aNetIF);
    virtual void LinkCallback(struct netif * aNetIF) { static_cast<void>(aNetIF);}
#endif // LWIP_NETIF_LINK_CALLBACK

#if LWIP_NETIF_EXT_STATUS_CALLBACK
    static void StaticExtCallback(
        struct netif * aNetIF,
        netif_nsc_reason_t const aReason,
        netif_ext_callback_args_t const *aArgs
    );
    virtual void ExtCallback(
        netif_nsc_reason_t const aReason,
        netif_ext_callback_args_t const *aArgs
    );
#endif // LWIP_NETIF_EXT_STATUS_CALLBACK

    virtual err_t EtherIFOut(struct pbuf * aPBuf) = 0;

    virtual err_t EtherIFInit(struct netif * aNetIF) = 0;
    virtual void ISR() = 0;

    using Ptr = std::unique_ptr<LwIPDrv>;
    static std::vector<Ptr> sNetDrives;

    unsigned int mMyIndex{};
    EthernetAddress mEthernetAddress{};
    struct netif mNetIF{};
    netif_ext_callback_t mExtCallback{};
    bool mUseDHCP{false};
    // [MG] CONSIDER shared_ptr OR weak_prt HERE.
    QP::QActive *mAO{nullptr};
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
