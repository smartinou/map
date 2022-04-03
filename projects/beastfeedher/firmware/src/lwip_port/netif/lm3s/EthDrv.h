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
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
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
    void Rd(void) override;
    void Wr(void) override;
    void PHYISR(void) override;
    void DisableAllInt(void) override;
    void EnableAllInt(void) override;

private:
    // LwIPDrv interface.
    err_t EtherIFOut(struct pbuf * const aPBuf) override;

    err_t EtherIFInit(struct netif * const aNetIF) override;
    void ISR(void) override;

    // Local interface.
    void LowLevelTx(struct pbuf * const aPBuf);
    struct pbuf *LowLevelRx(void);

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
