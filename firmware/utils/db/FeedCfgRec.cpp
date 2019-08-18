// *****************************************************************************
//
// Project: Utils.
//
// Module: Feeding configuration.
//
// *****************************************************************************

//! \file
//! \brief Feeding configuration class.
//! \ingroup utils_db

// *****************************************************************************
//
//        Copyright (c) 2016-2019, Martin Garon, All rights reserved.
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

    mRec.mTimedFeedPeriod = 2;
    mRec.mIsWebFeedingEnable     = true;
    mRec.mIsAutoPetFeedingEnable = true;

    mRec.mBase.mCRC = ComputeCRC(reinterpret_cast<uint8_t *>(&mRec), sizeof(struct RecData));
    SetIsDirty();
}


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

//
// Start of child methods.
//

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


// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
