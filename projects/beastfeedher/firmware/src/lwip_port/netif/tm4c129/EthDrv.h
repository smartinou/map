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

class EthDrv final
    : public LwIPDrv {
protected:
    template<class T, typename...Args>
    friend void LwIPDrv::Create(Args&&... aArgs);

public:
    explicit EthDrv(
        UseCreateFunc aDummy,
        unsigned int aIndex,
        EthernetAddress const &aEthernetAddress,
        unsigned int aRingBufSize,
        uint32_t aSysClk
    ) noexcept;

    void Rd() override;
    void Wr() override {}
    void PHYISR() override;
    void DisableAllInt() override;
    void EnableAllInt() override;

private:
    // LwIP Interface.
    err_t EtherIFOut(struct pbuf * aPBuf) override;
    err_t EtherIFInit(struct netif * aNetIF) override;
    void ISR() override;

    // Local interface.
    struct pbuf *LowLevelRx(RxDescriptor * aDescriptor, size_t aCumulatedLen = 0);

    RxDescriptorChain mRxRingBuf{};
    TxRingBuf mTxRingBuf{};
    unsigned int mRingBufSize{8};
    uint32_t mSysClk{};
    uint8_t mPHYAddr{};
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
