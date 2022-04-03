#pragma once
// *******************************************************************************
//
// Project: LwIP
//
// Module: TivaWare low-level Ethernet driver.
//
// *******************************************************************************

//! \file
//! \brief Ethernet driver class.
//! Compatible with TivaWare library.
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

// LwIP.
#include "lwip/pbuf.h"

#include "LwIPDrv.h"

#include "RxDescriptor.h"
#include "TxDescriptor.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class EthDrv
    : public LwIPDrv {
public:
    EthDrv(
        unsigned int aIndex,
        EthernetAddress const &aEthernetAddress,
        unsigned int aBufQueueSize,
        uint32_t aSysClk
    );
    ~EthDrv() {}

    void Rd(void) override;
    void Wr(void) override {}
    void PHYISR(void) override;
    void DisableAllInt(void) override;
    void EnableAllInt(void) override;

private:
    // LwIP Interface.
    err_t EtherIFOut(struct pbuf * const aPBuf) override;
    err_t EtherIFInit(struct netif * const aNetIF) override;
    void ISR(void) override;

    // Local interface.
    struct pbuf *LowLevelRx(RxDescriptor * const aDescriptor, size_t aCumulatedLen = 0);

    RxDescriptorChain mRxRingBuf;
    TxRingBuf mTxRingBuf;
    uint32_t const mSysClk;
    uint8_t const mPHYAddr = EMAC_PHY_ADDR;
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
