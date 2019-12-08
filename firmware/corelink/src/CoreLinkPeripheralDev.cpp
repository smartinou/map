// *****************************************************************************
//
// Project: ARM Cortex-M.
//
// Module: CoreLink Peripherals.
//
// *****************************************************************************

//! \file
//! \brief Generic CoreLink peripheral device base class.
//! \ingroup corelink_peripherals

// *****************************************************************************
//
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include "CoreLinkPeripheralDev.h"

using namespace CoreLink;

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

PeripheralDev::PeripheralDev(uint32_t aBaseAddr)
    : mBaseAddr(aBaseAddr)
    , mPeripheralID{ 0 }
    , mPrimeCellID(0) {

  id_reg_map_t const * const lIDRegMapPtr = reinterpret_cast<id_reg_map_t const * const>(mBaseAddr + 0x0FE0);

  mPeripheralID[0]  = (lIDRegMapPtr->mPeripheralID[0] <<  0);
  mPeripheralID[0] |= (lIDRegMapPtr->mPeripheralID[1] <<  8);
  mPeripheralID[0] |= (lIDRegMapPtr->mPeripheralID[2] << 16);
  mPeripheralID[0] |= (lIDRegMapPtr->mPeripheralID[3] << 24);

  mPrimeCellID  = (lIDRegMapPtr->mPrimeCellID[0] <<  0);
  mPrimeCellID |= (lIDRegMapPtr->mPrimeCellID[1] <<  8);
  mPrimeCellID |= (lIDRegMapPtr->mPrimeCellID[2] << 16);
  mPrimeCellID |= (lIDRegMapPtr->mPrimeCellID[3] << 24);
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
