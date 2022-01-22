#pragma once
// *******************************************************************************
//
// Project: Utils.
//
// Module: Feeding configuration DB.
//
// *******************************************************************************

//! \file
//! \brief Feeding configuration DB class.
//! \ingroup module_group

// ******************************************************************************
//
//        Copyright (c) 2016-2021, Martin Garon, All rights reserved.
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

//! \brief Feeding configuratino database.
class FeedCfgRec
    : public DBRec {
public:
    FeedCfgRec();
    ~FeedCfgRec();

    // DBRec interface.
    bool IsSane(void) const override;
    void ResetDflt(void) override;

    // Extended object's interface.
    uint8_t GetManualFeedWaitPeriod(void) const;
    uint8_t GetManualFeedMaxFeedPeriod(void) const;
    uint8_t GetTimedFeedPeriod(void) const;
    bool IsManualFeedEnable(void) const;
    bool IsTimedFeedEnable(void) const;
    bool UseSystemTime(void) const;

    void SetTimedFeedPeriod(uint8_t aPeriod);
    void SetManualFeedEnabled(bool aIsEnabled);
    void SetTimedFeedEnabled(bool aIsEnabled);
    void SetUseSystemTime(bool aUseSystemTime);

private:
    // DBRec interface.
    size_t GetRecSize(void) const override;
    void Serialize(uint8_t * const aDataPtr) const override;
    void Deserialize(uint8_t const * const aDataPtr) override;
    void UpdateCRC(void) override;

    struct RecData {
        struct BaseRec mBase;
        uint8_t mManualFeedWaitPeriod;
        uint8_t mManualFeedMaxFeedPeriod;
        uint8_t mTimedFeedPeriod;
        bool mIsManualFeedEnable;
        bool mIsTimedFeedEnable;
        bool mUseSystemTime;
    };

    struct RecData mRec;

    static char constexpr sMagic[3] = { 'C', 'F', 'G' };
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
