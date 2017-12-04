// *****************************************************************************
//
// Project: Beast Feed'Her
//
// Module: Master record class.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// *****************************************************************************
//
//        Copyright (c) 2016-2017, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// Standard Library.
#include <string.h>

// Common Library.
#include "DBRec.h"

// This project.
#include "MasterRec.h"

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

MasterRec::MasterRec(unsigned int aRecQty)
  : DBRec()
  , mMasterRec{0}
  , mRecQty(aRecQty)
  , mRecIx(0)
  , mDBRec{nullptr} {

  // Ctor body left intentionally empty.
  mDBRec= new DBRec *[aRecQty];
}


MasterRec::~MasterRec() {

  // Dtor body left intentionally empty.
}


bool MasterRec::IsDirty(void) const {
  bool lIsDirty = false;

  for (unsigned int lRecIx = 0; lRecIx < mRecIx; lRecIx++) {
    lIsDirty |= mDBRec[lRecIx]->IsDirty();
  }

  return lIsDirty;
}


bool MasterRec::IsSane(void) const {

  // Check magic value.
  if (('M' != mMasterRec.mMagic[0])
      || ('S' != mMasterRec.mMagic[1])
      || ('T' != mMasterRec.mMagic[2])
      || ('R' != mMasterRec.mMagic[3])) {
    return false;
  }

  for (unsigned int lRecIx = 0; lRecIx < mRecIx; lRecIx++) {
    if (!mDBRec[lRecIx]->IsSane()) {
      return false;
    }
  }

  return true;
}


unsigned int MasterRec::AddRec(DBRec * const aDBRecPtr) {
  mDBRec[mRecIx] = aDBRecPtr;
  return mRecIx++;
}


void MasterRec::ResetDflt(void) {

  mMasterRec.mMagic[0] = 'M';
  mMasterRec.mMagic[1] = 'S';
  mMasterRec.mMagic[2] = 'T';
  mMasterRec.mMagic[3] = 'R';

  mMasterRec.mVerMajor = VER_MAJOR;
  mMasterRec.mVerMinor = VER_MINOR;
  mMasterRec.mVerRev   = VER_REV;

  mMasterRec.mRecQty = 4;

  // Set record info.
  for (unsigned int lRecIx= 0; lRecIx < 3; lRecIx++) {
    mMasterRec.mRecInfo[lRecIx].mType   = 0;
    mMasterRec.mRecInfo[lRecIx].mOffset = 0;
    mMasterRec.mRecInfo[lRecIx].mSize   = 0;
  }

  // Nullify reserved fields.
  memset(&mMasterRec.mRsvd[0], 0, 12);

  // Reset all sub-records.
  for (unsigned int lRecIx = 0; lRecIx < mRecIx; lRecIx++) {
    mDBRec[lRecIx]->ResetDflt();
  }

  mIsDirty = true;
}


unsigned int MasterRec::GetRecSize(void) const {

  unsigned int lSize = sizeof(mMasterRec);
  for (unsigned int lRecIx = 0; lRecIx < mRecIx; lRecIx++) {
    lSize += mDBRec[lRecIx]->GetRecSize();
  }

  return lSize;
}


// Trivial serialization function.
void MasterRec::Serialize(uint8_t * const aDataPtr) const {

  uint8_t *lDataPtr = aDataPtr;
  memcpy(lDataPtr,
         reinterpret_cast<void const *>(&mMasterRec),
         sizeof(struct RecStructTag));
  lDataPtr += sizeof(struct RecStructTag);

  for (unsigned int lRecIx = 0; lRecIx < mRecIx; lRecIx++) {
    mDBRec[lRecIx]->Serialize(lDataPtr);
    lDataPtr += mDBRec[lRecIx]->GetRecSize();
  }
}


// Trivial serialization function.
void MasterRec::Deserialize(uint8_t const *aDataPtr) {

  memcpy(reinterpret_cast<void *>(&mMasterRec),
         aDataPtr,
         sizeof(struct RecStructTag));
  aDataPtr += sizeof(struct RecStructTag);

  for (unsigned int lRecIx = 0; lRecIx < mRecIx; lRecIx++) {
    mDBRec[lRecIx]->Deserialize(aDataPtr);
    aDataPtr += mDBRec[lRecIx]->GetRecSize();
  }
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
