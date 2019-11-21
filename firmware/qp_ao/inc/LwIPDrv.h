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
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include "net/EthernetAddress.h"

#include "lwip/err.h"
#include "lwip/netif.h"

#include <map>
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

    uint8_t const *GetMACAddress(void) const;
    uint32_t GetIPAddress(void) const;
    uint32_t GetSubnetMask(void) const;
    uint32_t GetDefaultGW(void) const;

private:
    // Internal PBuf Q-ring class.
    class PBufQ {
    public:
        PBufQ(unsigned int aQSize);

        bool IsEmpty(void) const;
        bool Put(struct pbuf * const aPBufPtr);
        struct pbuf *Get(void);

    private:
        struct pbuf **mPBufRing;
        unsigned int mRingSize;
        unsigned int mQWrIx;
        unsigned int mQRdIx;
        unsigned int mQOverflow;
    };

protected:
    // Static functions to hook to 'C' code.
    // Specific to an Ethernet IF.
    static err_t StaticEtherIFInit(struct netif * const aNetIF);
    static err_t StaticEtherIFOut(struct netif * const aNetIF, struct pbuf * const aPBuf);

    LwIPDrv(unsigned int mMyIndex, unsigned int aPBufQSize);

    virtual err_t EtherIFInit(struct netif * const aNetIF) = 0;
    virtual err_t EtherIFOut(struct netif * const aNetIF, struct pbuf * const aPBuf) = 0;

    virtual void DrvInit(
        QP::QActive * const aAO,
        bool aUseDHCP,
        uint32_t aIPAddress,
        uint32_t aSubnetMask,
        uint32_t aGWAddress
    ) = 0;
    virtual void Rd(void) = 0;
    virtual void Wr(void) = 0;
    virtual void ISR(void) = 0;

    PBufQ &GetPBufQ(void) const { return *mPBufQ; }
    unsigned int GetIndex(void) const { return mMyIndex; }
    struct netif &GetNetIF(void) { return mNetIF; }
    QP::QActive &GetAO(void) const { return *mAO; }
    void SetAO(QP::QActive * const aAO) { mAO = aAO; }

private:
    LwIPDrv(LwIPDrv const &) = delete;
    LwIPDrv const &operator=(LwIPDrv const &) = delete;

    static std::map<struct netif * const, LwIPDrv * const> sMap;
    static std::vector<LwIPDrv *> sVector;

    // Queue of pbufs for transmission.
    unsigned int mMyIndex;
    PBufQ       *mPBufQ = nullptr;
    struct netif mNetIF;
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
