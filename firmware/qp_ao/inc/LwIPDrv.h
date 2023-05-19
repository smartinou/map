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
        bool aUseDHCP,
        IPAddress aIPAddress,
        IPAddress aSubnetMask,
        IPAddress aGWAddress
    ) noexcept;
    static void StaticISR(unsigned int aIndex) noexcept;
    virtual void Rd() noexcept = 0;
    virtual void Wr() noexcept = 0;

    [[nodiscard]] static auto StaticGetMACAddress(unsigned int aIndex) noexcept -> EthernetAddress const &;
    [[nodiscard]] static auto StaticGetIPAddress(unsigned int aIndex) noexcept -> IPAddress;
    [[nodiscard]] static auto StaticGetSubnetMask(unsigned int aIndex) noexcept -> IPAddress;
    [[nodiscard]] static auto StaticGetDefaultGW(unsigned int aIndex) noexcept -> IPAddress;

    static void DNSFoundCallback(
        const char *aName,
        const ip_addr_t *aIPAddr,
        void *aArgs
    ) noexcept;

    auto GetMACAddress() const noexcept -> EthernetAddress const &;
    auto GetIPAddress() const noexcept -> IPAddress;
    auto GetSubnetMask() const noexcept -> IPAddress;
    auto GetDefaultGW() const noexcept -> IPAddress;
    void StartIPCfg() noexcept;

    virtual void PHYISR() noexcept {/*DoNothing();*/}
    virtual void DisableAllInt() noexcept = 0;
    virtual void EnableAllInt() noexcept = 0;

protected:
    struct UseCreateFunc {
        explicit UseCreateFunc() noexcept = default;
    };
    explicit LwIPDrv(
        UseCreateFunc /* Dummy */,
        unsigned int aIndex,
        EthernetAddress const &aEthernetAddress,
        std::weak_ptr<QP::QActive> aAO
    ) noexcept;

    void PostRxEvent() noexcept;
    void PostTxEvent() noexcept;
    void PostOverrunEvent() noexcept;
    void PostNetIFChangedEvent(bool aIsUp) noexcept;
    void PostLinkChangedEvent(bool aIsUp) noexcept;
    void PostPHYInterruptEvent() noexcept;

    [[nodiscard]] auto GetNetIF() noexcept -> struct netif & {return mNetIF;}
    [[nodiscard]] auto GetAO() const noexcept -> QP::QActive & {return *(mAO.lock().get());}
    [[nodiscard]] bool IsUsingDHCP() const noexcept {return mUseDHCP;}
    void UseDHCP(bool const aUseDHCP) noexcept {mUseDHCP = aUseDHCP;}

private:
    void DrvInit(
        bool aUseDHCP,
        IPAddress aIPAddress,
        IPAddress aSubnetMask,
        IPAddress aGWAddress
    ) noexcept;

    // Static functions to hook to 'C' code.
    // Specific to an Ethernet IF.
    static auto StaticEtherIFInit(struct netif * aNetIF) noexcept -> err_t;
    static auto StaticEtherIFOut(struct netif * aNetIF, struct pbuf * aPBuf) noexcept -> err_t;

#if LWIP_NETIF_STATUS_CALLBACK
    static void StaticStatusCallback(struct netif * aNetIF) noexcept;
    virtual void StatusCallback([[maybe_unused]] struct netif * aNetIF) noexcept {}
#endif // LWIP_NETIF_STATUS_CALLBACK

#if LWIP_NETIF_LINK_CALLBACK
    static void StaticLinkCallback(struct netif * aNetIF) noexcept;
    virtual void LinkCallback([[maybe_unused]] struct netif * aNetIF) noexcept {}
#endif // LWIP_NETIF_LINK_CALLBACK

#if LWIP_NETIF_EXT_STATUS_CALLBACK
    static void StaticExtCallback(
        struct netif * aNetIF,
        netif_nsc_reason_t const aReason,
        netif_ext_callback_args_t const *aArgs
    ) noexcept;
    virtual void ExtCallback(
        netif_nsc_reason_t const aReason,
        netif_ext_callback_args_t const *aArgs
    ) noexcept;
#endif // LWIP_NETIF_EXT_STATUS_CALLBACK

    virtual auto EtherIFOut(struct pbuf * aPBuf) noexcept -> err_t = 0;
    virtual auto EtherIFInit(struct netif * aNetIF) noexcept -> err_t = 0;
    virtual void ISR() noexcept = 0;

    using Ptr = std::unique_ptr<LwIPDrv>;
    static std::vector<Ptr> sNetDrives;

    unsigned int mMyIndex{};
    EthernetAddress mEthernetAddress{};
    struct netif mNetIF{};
    netif_ext_callback_t mExtCallback{};
    bool mUseDHCP{false};
    std::weak_ptr<QP::QActive> mAO{};
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
