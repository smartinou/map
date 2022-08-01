#pragma once
// *******************************************************************************
//
// Project: ARM Cortex-M.
//
// Module: CoreLink Peripherals.
//
// *******************************************************************************

//! \file
//! \brief CoreLink peripheral SPI device class declaration.
//! \ingroup corelink_peripherals

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

#include "CoreLinkPeripheralDev.h"
#include "ISPIMasterDev.h"
#include "SPISlaveCfg.h"
//#include "SSIPinCfg.h"

namespace CoreLink {

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \class SPIMasterDev
//! \brief SPI device
class SPIMasterDev
    : public ISPIMasterDev
    , public PeripheralDev {

public:
    struct SSIGPIO {
        uint32_t mClkPinCfg;
        uint32_t mDat0PinCfg;
        uint32_t mDat1PinCfg;
        unsigned long mPort;
        unsigned int mClkPin;
        unsigned int mRxPin;
        unsigned int mTxPin;
    };

    explicit SPIMasterDev(
        uint32_t aBaseAddr,
        uint32_t aClkRate,
        SSIGPIO const &aSSIGPIO
    ) noexcept;

    // ISPIMasterDev interface.
    void RdData(
        uint8_t aAddr,
        uint8_t * aData,
        size_t aLen,
        SPISlaveCfg const &aSPICfg
    ) const final;

    void RdData(
        uint8_t * aData,
        size_t aLen,
        SPISlaveCfg const &aSPICfg
    ) const final;

    void WrData(
        uint8_t aAddr,
        uint8_t const * aData,
        size_t aLen,
        SPISlaveCfg const &aSPICfg
    ) const final;

    void WrData(
        uint8_t const * aData,
        size_t aLen,
        SPISlaveCfg const &aSPICfg
    ) const final;

    [[maybe_unused]] auto PushPullByte(uint8_t aByte) const -> uint8_t final;
    [[maybe_unused]] auto PushPullByte(
        uint8_t aByte,
        SPISlaveCfg const &aSPICfg
    ) const -> uint8_t final;

private:
    static void SetPins(SSIGPIO const &aSSIGPIO);
    void SetCfg(SPISlaveCfg const &aSPISlaveCfg) const;

    static auto ToNativeProtocol(
        SPISlaveCfg::protocol_t const aProtocol
    ) -> unsigned int;

    // Non-owning pointer acting as cached last pointer.
    mutable SPISlaveCfg const *mLastSPICfg = nullptr;
};


} // namespace CoreLink

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
