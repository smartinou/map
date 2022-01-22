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
//        Copyright (c) 2015-2021, Martin Garon, All rights reserved.
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
    constexpr PeripheralDev(uint32_t aBaseAddr, uint32_t aClkRate) noexcept
        : mBaseAddr(aBaseAddr)
        , mClkRate(aClkRate)
        , mIDRegMapPtr(reinterpret_cast<id_reg_map_t const * const>(aBaseAddr + 0x0FE0))
        , mPeripheralID{0}
        , mPrimeCellID(0)
    {
        mPeripheralID[0]  = (mIDRegMapPtr->mPeripheralID[0] <<  0);
        mPeripheralID[0] |= (mIDRegMapPtr->mPeripheralID[1] <<  8);
        mPeripheralID[0] |= (mIDRegMapPtr->mPeripheralID[2] << 16);
        mPeripheralID[0] |= (mIDRegMapPtr->mPeripheralID[3] << 24);

        mPrimeCellID  = (mIDRegMapPtr->mPrimeCellID[0] <<  0);
        mPrimeCellID |= (mIDRegMapPtr->mPrimeCellID[1] <<  8);
        mPrimeCellID |= (mIDRegMapPtr->mPrimeCellID[2] << 16);
        mPrimeCellID |= (mIDRegMapPtr->mPrimeCellID[3] << 24);
    }

    constexpr uint32_t GetBaseAddr(void) const noexcept {return mBaseAddr;}
    constexpr uint32_t GetClkRate(void) const noexcept {return mClkRate;}
    constexpr uint32_t GetPeripheralID(void) const noexcept {return mPeripheralID[0];}
    constexpr uint32_t GetPrimCellID(void) const noexcept {return mPrimeCellID;}

private:
    typedef uint32_t volatile reg_t;

    struct ID_REG_MAP_STRUCT_TAG {
        reg_t mPeripheralID[4];
        reg_t mPrimeCellID[4];
    };
    typedef struct ID_REG_MAP_STRUCT_TAG id_reg_map_t;

    uint32_t const mBaseAddr;
    uint32_t const mClkRate;
    id_reg_map_t const * const mIDRegMapPtr;
    uint32_t mPeripheralID[1];
    uint32_t mPrimeCellID;
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
