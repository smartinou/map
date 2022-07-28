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
//        Copyright (c) 2016-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// This project.
#include "FeedCfgRec.h"

// Standard Library.
#include <cstring>

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

//
// Start of DBRec interface.
//

auto FeedCfgRec::IsSane() const noexcept -> bool {

    bool const lIsMagicGood = IsMagicGood(mRec.mBase, sMagic);
    if (lIsMagicGood) {
        return IsCRCGood(
            {
                reinterpret_cast<uint8_t const * const>(&mRec),
                sizeof(struct RecData)
            }
        );
    }

    return false;
}


void FeedCfgRec::ResetDflt() noexcept {

    // Set magic.
    mRec.mBase.mMagic = sMagic;
    mRec.mManualFeedWaitPeriod = sDfltManualFeedWaitPeriod;
    mRec.mManualFeedMaxFeedPeriod = sDfltManualFeedMaxFeedPeriod;
    mRec.mTimedFeedPeriod = sDfltTimedFeedPeriod;
    mRec.mIsManualFeedEnable = true;
    mRec.mIsTimedFeedEnable = true;
    mRec.mUseSystemTime = true;

    SetIsDirty();
}


auto FeedCfgRec::GetManualFeedWaitPeriod() const noexcept -> uint8_t {
    return mRec.mManualFeedWaitPeriod;
}


auto FeedCfgRec::GetManualFeedMaxFeedPeriod() const noexcept -> uint8_t {
    return mRec.mManualFeedMaxFeedPeriod;
}


auto FeedCfgRec::GetTimedFeedPeriod() const noexcept -> uint8_t {
    return mRec.mTimedFeedPeriod;
}


auto FeedCfgRec::IsManualFeedEnable() const noexcept -> bool {
    return mRec.mIsManualFeedEnable;
}


auto FeedCfgRec::IsTimedFeedEnable() const noexcept -> bool {
    return mRec.mIsTimedFeedEnable;
}


auto FeedCfgRec::UseSystemTime() const noexcept -> bool {
    return mRec.mUseSystemTime;
}


void FeedCfgRec::SetTimedFeedPeriod(uint8_t const aPeriod) noexcept {
    mRec.mTimedFeedPeriod = aPeriod;
    SetIsDirty();
}


void FeedCfgRec::SetManualFeedEnabled(bool const aIsEnabled) noexcept {
    mRec.mIsManualFeedEnable = aIsEnabled;
    SetIsDirty();
}


void FeedCfgRec::SetTimedFeedEnabled(bool const aIsEnabled) noexcept {
    mRec.mIsTimedFeedEnable = aIsEnabled;
    SetIsDirty();
}


void FeedCfgRec::SetUseSystemTime(bool const aUseSystemTime) noexcept {
    mRec.mUseSystemTime = aUseSystemTime;
    SetIsDirty();
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

auto FeedCfgRec::GetRecSize() const noexcept -> size_t {
    return sizeof(struct RecData);
}


// Trivial serialization function.
void FeedCfgRec::Serialize(uint8_t * const aDataPtr) const {

    std::memcpy(aDataPtr, &mRec, GetRecSize());
}


// Trivial serialization function.
void FeedCfgRec::Deserialize(uint8_t const * const aDataPtr) {

    std::memcpy(&mRec, aDataPtr, GetRecSize());
}


void FeedCfgRec::UpdateCRC() noexcept {
    mRec.mBase.mCRC = ComputeCRC(
        {
            reinterpret_cast<uint8_t const * const>(mRec.mBase.mMagic.data()),
            sizeof(struct RecData) - sizeof(struct BaseRec)
        }
    );
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
