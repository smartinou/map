#pragma once
// *******************************************************************************
//
// Project: Utils.
//
// Module: Feeding configuration.
//
// *******************************************************************************

//! \file
//! \brief Feeding configuration class class.
//! \ingroup module_group

// ******************************************************************************
//
//        Copyright (c) 2016-2019, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include "DBRec.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Brief description.
//! Details follow...
//! ...here.
class FeedCfgRec
    : public DBRec {
public:
    FeedCfgRec();
    ~FeedCfgRec();

    // DBRec.
    bool IsSane(void) override;
    void ResetDflt(void) override;

    // Extended object's interface.
    uint8_t GetTimedFeedPeriod(void) const;
    bool    IsWebFeedingEnable(void) const;
    bool    IsAutoPetFeedingEnable(void) const;

    void SetTimedFeedPeriod(uint8_t aPeriod);
    void SetIsWebFeedingEnabled(bool aIsEnabled);
    void SetIsAutoPetFeedingEnabled(bool aIsEnabled);

private:
    // DBRec.
    unsigned int GetRecSize(void) const override;
    void Serialize(uint8_t * const aDataPtr) const override;
    void Deserialize(uint8_t const * const aDataPtr) override;

    struct RecStructTag {
        uint8_t mCRC;
        char    mMagic[3];
        uint8_t mTimedFeedPeriod;
        bool    mIsWebFeedingEnable;
        bool    mIsAutoPetFeedingEnable;
    };

    struct RecStructTag mRec;
};

// ******************************************************************************
//                            EXPORTED VARIABLES
// ******************************************************************************

// ******************************************************************************
//                                 EXTERNS
// ******************************************************************************

// ******************************************************************************
//                            EXPORTED FUNCTIONS
// ******************************************************************************

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
