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
//        Copyright (c) 2015-2016, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// Standard Library.
#include <stddef.h>

// TI Library.

// QP Library.

// Common Library.

// This project.
#include "DBRec.h"

//Q_DEFINE_THIS_FILE

//namespace BFH {

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

unsigned int DBRec::mDBObjCnt = 0;

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

DBRec::DBRec()
  : mIsDirty(false)
  , mDBObjIx(0) {

  mDBObjIx = mDBObjCnt;
  mDBObjCnt++;
}


unsigned int DBRec::GetMyIx(void) {
  return mDBObjIx;
}


bool DBRec::IsDirty(void) const {
  return mIsDirty;
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
