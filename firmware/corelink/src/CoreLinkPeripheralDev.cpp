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
//        Copyright (c) 2015-2016, Martin Garon, All rights reserved.
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

//uint32_t PeripheralDev::mExpectedPeripheralID[1];

//uint32_t PeripheralDev::mExpectedPrimeCellID;

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

PeripheralDev::PeripheralDev(uint32_t aBaseAddr):
  mBaseAddr(aBaseAddr),
  //mPeripheralID(0),
  mPrimeCellID(0) {

  //mBaseAddr = aBaseAddr;

  id_reg_map_t *lIDRegMapPtr = reinterpret_cast<id_reg_map_t *>(mBaseAddr + 0x0FE0);
#if 1
  mPeripheralID[0]  = (lIDRegMapPtr->mPeripheralID[0] <<  0);
  mPeripheralID[0] |= (lIDRegMapPtr->mPeripheralID[1] <<  8);
  mPeripheralID[0] |= (lIDRegMapPtr->mPeripheralID[2] << 16);
  mPeripheralID[0] |= (lIDRegMapPtr->mPeripheralID[3] << 24);

  mPrimeCellID  = (lIDRegMapPtr->mPrimeCellID[0] <<  0);
  mPrimeCellID |= (lIDRegMapPtr->mPrimeCellID[1] <<  8);
  mPrimeCellID |= (lIDRegMapPtr->mPrimeCellID[2] << 16);
  mPrimeCellID |= (lIDRegMapPtr->mPrimeCellID[3] << 24);
#endif
}


PeripheralDev::~PeripheralDev() {

}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
