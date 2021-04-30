#pragma once
// *******************************************************************************
//
// Project: LwIP
//
// Module: LM3S6965 low-level Ethernet driver.
//
// *******************************************************************************

//! \file
//! \brief Ethernet driver class.
//! \ingroup lwip

// ******************************************************************************
//
//        Copyright (c) 2015-2021, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include "LwIPDrv.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class EthDrv
    : public LwIPDrv {
public:
    EthDrv(unsigned int aIndex, EthernetAddress const &aEthernetAddress, unsigned int aPBufQueueSize);
    ~EthDrv() {}

    // LwIPDrv interface.
    void DisableAllInt(void) override;
    void EnableAllInt(void) override;

private:
    // LwIPDrv interface.
    err_t EtherIFOut(struct netif * const aNetIF, struct pbuf * const aPBuf);
    void Rd(void);
    void Wr(void);

    err_t EtherIFInit(struct netif * const aNetIF) override;
    void ISR(void) override;

    // Local interface.
    void LowLevelTx(struct pbuf * const aPBuf);
    struct pbuf *LowLevelRx(void);
    void FreePBuf(struct pbuf * const aPBuf);

    bool IsTxEmpty(void) const;

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

    PBufQ &GetPBufQ(void) const { return *mPBufQ; }

    // Queue of pbufs for transmission.
    PBufQ *mPBufQ = nullptr;
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
