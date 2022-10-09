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
//        Copyright (c) 2016-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
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

//! \brief Feeding configuration database.
class FeedCfgRec final
    : public DBRec {
protected:
    template<class T, typename...Args>
    friend auto DBRec::Create(Args&&... aArgs) -> std::shared_ptr<T>;

public:
    explicit FeedCfgRec(UseCreateFunc const aDummy) noexcept
        : DBRec{aDummy} {}

    // DBRec interface.
    [[nodiscard]] bool IsSane() const noexcept override;
    void ResetDflt() noexcept override;

    // Extended object's interface.
    [[nodiscard]] auto GetManualFeedWaitPeriod() const noexcept -> uint8_t;
    [[nodiscard]] auto GetManualFeedMaxFeedPeriod() const noexcept -> uint8_t;
    [[nodiscard]] auto GetTimedFeedPeriod() const noexcept -> uint8_t;
    [[nodiscard]] bool IsManualFeedEnable() const noexcept;
    [[nodiscard]] bool IsTimedFeedEnable() const noexcept;
    [[nodiscard]] bool UseSystemTime() const noexcept;

    void SetTimedFeedPeriod(uint8_t aPeriod) noexcept;
    void SetManualFeedEnabled(bool aIsEnabled) noexcept;
    void SetTimedFeedEnabled(bool aIsEnabled) noexcept;
    void SetUseSystemTime(bool aUseSystemTime) noexcept;

private:
    // DBRec interface.
    [[nodiscard]] auto GetRecSize() const noexcept -> size_t override;
    void Serialize(uint8_t * aDataPtr) const override;
    void Deserialize(uint8_t const * aDataPtr) override;
    void UpdateCRC() noexcept override;

    static constexpr DBRec::Magic sMagic{ 'C', 'F', 'G' };
    static constexpr auto sDfltManualFeedWaitPeriod{2};
    static constexpr auto sDfltManualFeedMaxFeedPeriod{5};
    static constexpr auto sDfltTimedFeedPeriod{4};

    struct RecData {
        struct BaseRec mBase{{}, {sMagic}};
        uint8_t mManualFeedWaitPeriod{sDfltManualFeedWaitPeriod};
        uint8_t mManualFeedMaxFeedPeriod{sDfltManualFeedMaxFeedPeriod};
        uint8_t mTimedFeedPeriod{sDfltTimedFeedPeriod};
        bool mIsManualFeedEnable{true};
        bool mIsTimedFeedEnable{true};
        bool mUseSystemTime{true};
    };

    struct RecData mRec;
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
