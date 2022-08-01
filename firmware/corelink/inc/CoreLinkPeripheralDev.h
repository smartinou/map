#pragma once
// *******************************************************************************
//
// Project: ARM Cortex-M.
//
// Module: CoreLink Peripherals.
//
// *******************************************************************************

//! \file
//! \brief Generic CoreLink peripheral device base class.
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

#include <cstdint>

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

namespace CoreLink {


//! \class PeripheralDev
//! \brief CoreLink Peripheral Device base class.
//!
//! This is the base class from which all CoreLink peripheral device drivers
//! are derived.
//!
class PeripheralDev {
protected:
    constexpr explicit PeripheralDev(
        uint32_t const aBaseAddr,
        uint32_t const aClkRate
    ) noexcept
        : mBaseAddr(aBaseAddr)
        , mClkRate(aClkRate)
        , mPeripheralID{
            [&]() -> uint32_t {
                id_reg_map_t const * const lIDRegMapPtr =
                    reinterpret_cast<id_reg_map_t const *>(aBaseAddr + 0x0FE0);
                uint32_t lID = (lIDRegMapPtr->mPeripheralID[0] <<  0);
                lID |= (lIDRegMapPtr->mPeripheralID[1] <<  8);
                lID |= (lIDRegMapPtr->mPeripheralID[2] << 16);
                lID |= (lIDRegMapPtr->mPeripheralID[3] << 24);
                return lID;
            } ()
        }
        , mPrimeCellID(
            [&]() -> uint32_t {
                id_reg_map_t const * const lIDRegMapPtr =
                    reinterpret_cast<id_reg_map_t const *>(aBaseAddr + 0x0FE0);
                uint32_t lID = (lIDRegMapPtr->mPrimeCellID[0] <<  0);
                lID |= (lIDRegMapPtr->mPrimeCellID[1] <<  8);
                lID |= (lIDRegMapPtr->mPrimeCellID[2] << 16);
                lID |= (lIDRegMapPtr->mPrimeCellID[3] << 24);
                return lID;
            } ()
        ) {}

    constexpr uint32_t GetBaseAddr(void) const noexcept {return mBaseAddr;}
    constexpr uint32_t GetClkRate(void) const noexcept {return mClkRate;}
    constexpr uint32_t GetPeripheralID(void) const noexcept {return mPeripheralID[0];}
    constexpr uint32_t GetPrimCellID(void) const noexcept {return mPrimeCellID;}

private:
    typedef uint32_t reg_t;

    struct ID_REG_MAP_STRUCT_TAG {
        reg_t mPeripheralID[4];
        reg_t mPrimeCellID[4];
    };
    typedef struct ID_REG_MAP_STRUCT_TAG id_reg_map_t;

    uint32_t const mBaseAddr;
    uint32_t const mClkRate;
    uint32_t const mPeripheralID[1];
    uint32_t const mPrimeCellID;
};


} // namespace

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
