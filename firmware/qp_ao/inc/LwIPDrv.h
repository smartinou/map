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
    virtual ~LwIPDrv() {}

    static void StaticInit(
        QP::QActive * const aAO,
        bool aUseDHCP,
        uint32_t aIPAddress,
        uint32_t aSubnetMask,
        uint32_t aGWAddress
    );
    static void StaticRd(unsigned int aIndex);
    static void StaticWr(unsigned int aIndex);
    static void StaticISR(unsigned int aIndex);

    static uint8_t const *StaticGetMACAddress(unsigned int aIndex);
    static uint32_t StaticGetIPAddress(unsigned int aIndex);
    static uint32_t StaticGetSubnetMask(unsigned int aIndex);
    static uint32_t StaticGetDefaultGW(unsigned int aIndex);

    static void DNSFoundCallback(
        const char *aName,
        const ip_addr_t *aIPAddr,
        void *aArgs
    );

    uint8_t const *GetMACAddress(void) const;
    uint32_t GetIPAddress(void) const;
    uint32_t GetSubnetMask(void) const;
    uint32_t GetDefaultGW(void) const;
    void StartIPCfg(void);

    virtual void DisableAllInt(void) = 0;
    virtual void EnableAllInt(void) = 0;

protected:
    LwIPDrv(unsigned int aIndex, EthernetAddress const &aEthernetAddress);

    void PostRxEvent(void);
    void PostTxEvent(void);
    void PostOverrunEvent(void);
    void PostNetIFChangedEvent(bool aIsUp);
    void PostLinkChangedEvent(bool aIsUp);

    unsigned int GetIndex(void) const {return mMyIndex;}
    struct netif &GetNetIF(void) {return mNetIF;}
    QP::QActive &GetAO(void) const {return *mAO;}
    bool IsUsingDHCP(void) const{return mUseDHCP;}
    void SetAO(QP::QActive * const aAO) {mAO = aAO;}
    void UseDHCP(bool aUseDHCP) {mUseDHCP = aUseDHCP;}

private:
    void DrvInit(
        QP::QActive * const aAO,
        bool aUseDHCP,
        uint32_t aIPAddress,
        uint32_t aSubnetMask,
        uint32_t aGWAddress
    );

    // Static functions to hook to 'C' code.
    // Specific to an Ethernet IF.
    static err_t StaticEtherIFInit(struct netif * const aNetIF);
    static err_t StaticEtherIFOut(struct netif * const aNetIF, struct pbuf * const aPBuf);

    static void StaticStatusCallback(struct netif * const aNetIF);
    static void StaticLinkCallback(struct netif * const aNetIF);
    void StatusCallback(struct netif * const aNetIF);

    virtual err_t EtherIFOut(struct pbuf * const aPBuf) = 0;
    virtual void Rd(void) = 0;
    virtual void Wr(void) = 0;

    virtual err_t EtherIFInit(struct netif * const aNetIF) = 0;
    virtual void ISR(void) = 0;

    LwIPDrv(LwIPDrv const &) = delete;
    LwIPDrv const &operator=(LwIPDrv const &) = delete;

    static std::vector<LwIPDrv *> sVector;

    unsigned int mMyIndex;
    struct netif mNetIF;
    bool mUseDHCP;
    QP::QActive *mAO = nullptr;

    ip_addr_t mIPAddress;
    ip_addr_t mSubnetMask;
    ip_addr_t mGWAddress;
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
