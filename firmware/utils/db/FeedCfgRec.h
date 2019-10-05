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
    bool IsSane(void) const override;
    void ResetDflt(void) override;

    // Extended object's interface.
    uint8_t GetManualFeedWaitPeriod(void) const;
    uint8_t GetManualFeedMaxFeedPeriod(void) const;
    uint8_t GetTimedFeedPeriod(void) const;
    bool IsManualFeedEnable(void) const;
    bool IsTimedFeedEnable(void) const;

    void SetTimedFeedPeriod(uint8_t aPeriod);
    void SetManualFeedEnabled(bool aIsEnabled);
    void SetTimedFeedEnabled(bool aIsEnabled);

private:
    // DBRec.
    size_t GetRecSize(void) const override;
    void Serialize(uint8_t * const aDataPtr) const override;
    void Deserialize(uint8_t const * const aDataPtr) override;

    struct RecData {
        struct BaseRec mBase;
        uint8_t mManualFeedWaitPeriod;
        uint8_t mManualFeedMaxFeedPeriod;
        uint8_t mTimedFeedPeriod;
        bool mIsManualFeedEnable;
        bool mIsTimedFeedEnable;
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
