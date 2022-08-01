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

namespace CoreLink {

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class SPISlaveCfg final {
public:
    enum class PROTOCOL {
        MOTO_0 = 0,
        MOTO_1,
        MOTO_2,
        MOTO_3,
        TI,
        NMW
    };

    using protocol_t = enum class PROTOCOL;
    constexpr explicit SPISlaveCfg(
        GPIO const &aGPIO,
        protocol_t const aProtocol,
        unsigned int const aBitRate,
        unsigned int const aDataWidth
    ) noexcept
        : mProtocol(aProtocol)
        , mBitRate(aBitRate)
        , mDataWidth(aDataWidth)
        , mCSnGPIO(aGPIO) {}

    // ISPISlaveCfg interface.
    constexpr void SetBitRate(unsigned int const aBitRate) noexcept {mBitRate = aBitRate;}

    [[nodiscard]] constexpr auto GetProtocol() const noexcept -> protocol_t {return mProtocol;}
    [[nodiscard]] constexpr auto GetBitRate() const noexcept -> unsigned int {return mBitRate;}
    [[nodiscard]] constexpr auto GetDataWidth() const noexcept -> unsigned int {return mDataWidth;}

    void AssertCSn() const noexcept;
    void DeassertCSn() const noexcept;

    void InitCSnGPIO() const noexcept;

private:
    protocol_t mProtocol{protocol_t::MOTO_0};
    unsigned long mBitRate{0};

    static constexpr auto sDfltDataWidth{8};
    unsigned long mDataWidth{sDfltDataWidth};
    GPIO mCSnGPIO;
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
