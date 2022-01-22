// *****************************************************************************
//
// Project: Utils.
//
// Module: Feeding configuration DB.
//
// *****************************************************************************

//! \file
//! \brief Feeding configuration DB class.
//! \ingroup utils_db

// *****************************************************************************
//
//        Copyright (c) 2016-2021, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// Standard Library.
#include <string.h>

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

char constexpr FeedCfgRec::sMagic[3];

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

FeedCfgRec::FeedCfgRec()
    : DBRec()
    , mRec{0} {

    // Ctor body left intentionally empty.
}


FeedCfgRec::~FeedCfgRec() {
    // Dtor body left intentionally empty.
}


//
// Start of DBRec interface.
//

bool FeedCfgRec::IsSane(void) const {
    // Check magic.
    if (!IsMagicGood(reinterpret_cast<BaseRec const * const>(&mRec.mBase), sMagic)) {
        return false;
    }

    // Check CRC.
    if (!IsCRCGood(reinterpret_cast<uint8_t const * const>(&mRec), sizeof(struct RecData))) {
        return false;
    }

    return true;
}


void FeedCfgRec::ResetDflt(void) {

    // Set magic.
    mRec.mBase.mMagic[0] = sMagic[0];
    mRec.mBase.mMagic[1] = sMagic[1];
    mRec.mBase.mMagic[2] = sMagic[2];

    mRec.mManualFeedWaitPeriod = 2;
    mRec.mManualFeedMaxFeedPeriod = 5;
    mRec.mTimedFeedPeriod = 4;
    mRec.mIsManualFeedEnable = true;
    mRec.mIsTimedFeedEnable = true;
    mRec.mUseSystemTime = true;

    SetIsDirty();
}


uint8_t FeedCfgRec::GetManualFeedWaitPeriod(void) const {
    return mRec.mManualFeedWaitPeriod;
}


uint8_t FeedCfgRec::GetManualFeedMaxFeedPeriod(void) const
{
    return mRec.mManualFeedMaxFeedPeriod;
}


uint8_t FeedCfgRec::GetTimedFeedPeriod(void) const {
    return mRec.mTimedFeedPeriod;
}


bool FeedCfgRec::IsManualFeedEnable(void) const {
    return mRec.mIsManualFeedEnable;
}


bool FeedCfgRec::IsTimedFeedEnable(void) const {
    return mRec.mIsTimedFeedEnable;
}


bool FeedCfgRec::UseSystemTime(void) const {
    return mRec.mUseSystemTime;
}


void FeedCfgRec::SetTimedFeedPeriod(uint8_t aPeriod) {
    mRec.mTimedFeedPeriod = aPeriod;
    SetIsDirty();
}


void FeedCfgRec::SetManualFeedEnabled(bool aIsEnabled) {
    mRec.mIsManualFeedEnable = aIsEnabled;
    SetIsDirty();
}


void FeedCfgRec::SetTimedFeedEnabled(bool aIsEnabled) {
    mRec.mIsTimedFeedEnable = aIsEnabled;
    SetIsDirty();
}


void FeedCfgRec::SetUseSystemTime(bool aUseSystemTime) {
    mRec.mUseSystemTime = aUseSystemTime;
    SetIsDirty();
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

size_t FeedCfgRec::GetRecSize(void) const {
    return sizeof(struct RecData);
}


// Trivial serialization function.
void FeedCfgRec::Serialize(uint8_t * const aDataPtr) const {

    memcpy(aDataPtr, &mRec, GetRecSize());
}


// Trivial serialization function.
void FeedCfgRec::Deserialize(uint8_t const *aDataPtr) {

    memcpy(&mRec, aDataPtr, GetRecSize());
}


void FeedCfgRec::UpdateCRC() {
    mRec.mBase.mCRC = ComputeCRC(
        reinterpret_cast<uint8_t const * const>(&mRec),
        sizeof(struct RecData)
    );
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
