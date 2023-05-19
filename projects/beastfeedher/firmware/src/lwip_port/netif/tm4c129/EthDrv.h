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
    [[nodiscard]] explicit EthDrv(
        UseCreateFunc aDummy,
        unsigned int aIndex,
        EthernetAddress const &aEthernetAddress,
        std::weak_ptr<QP::QActive> aAO,
        unsigned int aRingBufSize,
        uint32_t aSysClk
    ) noexcept;

    void Rd() noexcept override;
    void Wr() noexcept override {/*DoNothing();*/}
    void PHYISR() noexcept override;
    void DisableAllInt() noexcept override;
    void EnableAllInt() noexcept override;

private:
    // LwIP Interface.
    [[nodiscard]] auto EtherIFOut(struct pbuf * aPBuf) noexcept -> err_t override;
    [[nodiscard]] auto EtherIFInit(struct netif * aNetIF) noexcept -> err_t override;
    void ISR() noexcept override;

    // Local interface.
    [[nodiscard]] struct pbuf *LowLevelRx(RxDescriptor * aDescriptor, size_t aCumulatedLen = 0);

    RxDescriptorChain mRxRingBuf{};
    TxRingBuf mTxRingBuf{};
    unsigned int mRingBufSize{8};
    uint32_t mSysClk{};
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
