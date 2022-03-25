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

#include "inc/GPIO.h"

#include "ISPISlaveCfg.h"


namespace CoreLink {

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class SPISlaveCfg final
    : public ISPISlaveCfg {
public:
    constexpr explicit SPISlaveCfg(
        GPIO const &aGPIO,
        protocol_t const aProtocol = PROTOCOL::MOTO_0,
        unsigned int const aBitRate = 0,
        unsigned int const aDataWidth = 8
    ) noexcept
        : mProtocol(aProtocol)
        , mBitRate(aBitRate)
        , mDataWidth(aDataWidth)
        , mCSnGPIO(aGPIO) {}

    ~SPISlaveCfg() = default;

    // ISPISlaveCfg interface.
    void SetBitRate(unsigned int const aBitRate) final {mBitRate = aBitRate;}

    constexpr protocol_t GetProtocol(void) const final {return mProtocol;}
    constexpr unsigned int GetBitRate(void) const final {return mBitRate;}
    constexpr unsigned int GetDataWidth(void) const final {return mDataWidth;}

    void AssertCSn(void) const final;
    void DeassertCSn(void) const final;

    void InitCSnGPIO(void) const final;

private:
    SPISlaveCfg() = delete;

    protocol_t const mProtocol;
    unsigned long mBitRate;
    unsigned long const mDataWidth;
    GPIO const mCSnGPIO;
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
