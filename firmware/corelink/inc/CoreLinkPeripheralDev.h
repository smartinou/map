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
//        Copyright (c) 2015-2020, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <stdint.h>

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

namespace CoreLink {


//! \class PeripheralDev
// \brief CoreLink Peripheral Device base class.
//
// This is the base class from which all CoreLink peripheral device drivers
// are derived.
//
class PeripheralDev {
public:
    virtual ~PeripheralDev() {}

protected:
    PeripheralDev(uint32_t aBaseAddr, uint32_t aClkRate);

    uint32_t GetBaseAddr(void) const { return mBaseAddr; }
    uint32_t GetClkRate(void) const {return mClkRate;}

private:
    typedef uint32_t volatile reg_t;

    struct ID_REG_MAP_STRUCT_TAG {
        reg_t mPeripheralID[4];
        reg_t mPrimeCellID[4];
    };
    typedef struct ID_REG_MAP_STRUCT_TAG id_reg_map_t;

    uint32_t mBaseAddr;
    uint32_t mClkRate;
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
