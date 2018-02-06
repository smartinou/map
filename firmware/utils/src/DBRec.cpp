// *****************************************************************************
//
// Project: Beast Feed'Her
//
// Module: DB record base abstract class.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// *****************************************************************************
//
//        Copyright (c) 2015-2018, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// Standard Library.
#include <stddef.h>

// This project.
#include "DBRec.h"

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

DBRec::DBRec()
  : mIsDirty(false)
  , mNextRecPtr(nullptr) {

  // Ctor body left intentionally emtpy.
}


bool DBRec::IsDirty(void) const {
  return mIsDirty;
}


DBRec *DBRec::GetNextRec(void) const {
  return mNextRecPtr;
}


void DBRec::SetNextRec(DBRec * const aDBRecPtr) {
  mNextRecPtr = aDBRecPtr;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

uint8_t DBRec::ComputeCRC(uint8_t const *aDataPtr, unsigned int aSize) {
  uint8_t lCRC = 0;
  for (unsigned int lIx = 1; lIx < aSize; lIx++) {
    lCRC += aDataPtr[lIx];
  }

  return ~lCRC;
}


bool DBRec::IsCRCGood(uint8_t const *aDataPtr, unsigned int aSize) {
  uint8_t lCRC = 0;
  for (unsigned int lIx = 0; lIx < aSize; lIx++) {
    lCRC += aDataPtr[lIx];
  }

  lCRC++;

  // Total CRC (data + CRC) should yield 0.
  if (lCRC) {
    return false;
  }

  return true;
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
