// *****************************************************************************
//
// Project: Beast Feed'Her
//
// Module: DB holder class.
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
#include "DB.h"

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

DBRec       *DB::mRootDBRecPtr = nullptr;
unsigned int DB::mDBRecObjCnt  = 0;

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

void DB::AddRec(DBRec * const aDBRecPtr) {

  if (nullptr == mRootDBRecPtr) {
    mRootDBRecPtr = aDBRecPtr;
  } else {
    DBRec *lDBRecPtr = DB::mRootDBRecPtr;
    while (nullptr != lDBRecPtr->GetNextRec()) {
      lDBRecPtr = lDBRecPtr->GetNextRec();
    }
    lDBRecPtr->SetNextRec(aDBRecPtr);
  }

  mDBRecObjCnt++;
}


bool DB::IsSane(void) {
  return IsSane(mRootDBRecPtr);
}


bool DB::IsDirty(void) {
  return IsDirty(mRootDBRecPtr);
}


void DB::ResetDflt(void) {

  DBRec *lDBRecPtr = mRootDBRecPtr;
  while (nullptr != lDBRecPtr) {
    lDBRecPtr->ResetDflt();
    lDBRecPtr = lDBRecPtr->GetNextRec();
  }
}


unsigned int DB::GetSize(void) {

  DBRec *lDBRecPtr = mRootDBRecPtr;
  unsigned int lDBSize = 0;
  while (nullptr != lDBRecPtr) {
    lDBSize += lDBRecPtr->GetRecSize();
    lDBRecPtr = lDBRecPtr->GetNextRec();
  }

  return lDBSize;
}


void DB::Serialize(uint8_t *aDataPtr) {

  DBRec *lDBRecPtr = mRootDBRecPtr;
  while (nullptr != lDBRecPtr) {
    lDBRecPtr->Serialize(aDataPtr);
    unsigned int lSize = lDBRecPtr->GetRecSize();
    aDataPtr += lSize;
    lDBRecPtr = lDBRecPtr->GetNextRec();
  }
}


void DB::Deserialize(uint8_t const *aDataPtr) {

  DBRec *lDBRecPtr = mRootDBRecPtr;
  while (nullptr != lDBRecPtr) {
    lDBRecPtr->Deserialize(aDataPtr);
    unsigned int lSize = lDBRecPtr->GetRecSize();
    aDataPtr += lSize;
    lDBRecPtr = lDBRecPtr->GetNextRec();
  }
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

bool DB::IsSane(DBRec * const aDBRecPtr) {

  // Check current record.
  // Bail out the minute one record is not sane.
  bool lIsRecSane = aDBRecPtr->IsSane();
  if (false == lIsRecSane) {
    return false;
  } else if (nullptr == aDBRecPtr->GetNextRec()) {
    return true;
  }

  // Recursive call to end of record list.
  return lIsRecSane && IsSane(aDBRecPtr->GetNextRec());
}


bool DB::IsDirty(DBRec * const aDBRecPtr) {

  // Check current record.
  // Bail out the minute one record is dirty.
  bool lIsRecDirty = aDBRecPtr->IsDirty();
  if (true == lIsRecDirty) {
    return true;
  } else if (nullptr == aDBRecPtr->GetNextRec()) {
    return false;
  }

  // Recursive call to end of record list.
  return lIsRecDirty || IsDirty(aDBRecPtr->GetNextRec());
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
