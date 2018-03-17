// *****************************************************************************
//
// Project: Beast Feed'Her
//
// Module: Feeding configuration class.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// *****************************************************************************
//
//        Copyright (c) 2016-2018, Martin Garon, All rights reserved.
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
#include "FeedCfgRec.h"

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

FeedCfgRec::FeedCfgRec()
  : DBRec()
  , mRec{0} {

  // Ctor body left intentionally empty.
}


uint8_t FeedCfgRec::GetTimedFeedPeriod(void) const {
  return mRec.mTimedFeedPeriod;
}


bool FeedCfgRec::IsWebFeedingEnable(void) const {
  return mRec.mIsWebFeedingEnable;
}


bool FeedCfgRec::IsAutoPetFeedingEnable(void) const {
  return mRec.mIsAutoPetFeedingEnable;
}


void FeedCfgRec::SetTimedFeedPeriod(uint8_t aPeriod) {
  mRec.mTimedFeedPeriod = aPeriod;
}


void FeedCfgRec::SetIsWebFeedingEnabled(bool aIsEnabled) {
  mRec.mIsWebFeedingEnable = aIsEnabled;
}


void FeedCfgRec::SetIsAutoPetFeedingEnabled(bool aIsEnabled) {
  mRec.mIsAutoPetFeedingEnable = aIsEnabled;
}


bool FeedCfgRec::IsSane(void) {
  if (!IsCRCGood(reinterpret_cast<uint8_t *>(&mRec), sizeof(mRec))) {
    return false;
  }

  // Check magic value.
  if (('C' != mRec.mMagic[0])
      || ('F' != mRec.mMagic[1])
      || ('G' != mRec.mMagic[2])) {
    return false;
  }

  return true;
}


bool FeedCfgRec::IsDirty(void) const {
  return mIsDirty;
}


void FeedCfgRec::ResetDflt(void) {

  // Set magic.
  mRec.mMagic[0] = 'C';
  mRec.mMagic[1] = 'F';
  mRec.mMagic[2] = 'G';

  mRec.mTimedFeedPeriod = 2;
  mRec.mIsWebFeedingEnable     = true;
  mRec.mIsAutoPetFeedingEnable = true;

  mRec.mCRC = ComputeCRC(reinterpret_cast<uint8_t *>(&mRec), sizeof(mRec));
  mIsDirty = true;
}


unsigned int FeedCfgRec::GetRecSize(void) const {
  return sizeof(struct RecStructTag);
}


// Trivial serialization function.
void FeedCfgRec::Serialize(uint8_t * const aDataPtr) const {

  memcpy(aDataPtr, &mRec, GetRecSize());
}


// Trivial serialization function.
void FeedCfgRec::Deserialize(uint8_t const *aDataPtr) {

  memcpy(&mRec, aDataPtr, GetRecSize());
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
