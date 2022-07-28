#pragma once
// *******************************************************************************
//
// Project: Utils.
//
// Module: Feeding calendar DB.
//
// *******************************************************************************

//! \file
//! \brief Feeding calendar DB class.
//! \ingroup utils_db

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

// Standard Libraries.
#include <array>
#include <optional>

#include "time/Time.h"
#include "date/Weekday.h"

#include "DBRec.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************


//! \brief Feeding calendar database class.
class CalendarRec
    : public DBRec {
protected:
    template<class T>
    friend auto DBRec::Create() -> std::shared_ptr<T>;

public:
    explicit CalendarRec(Token const aDummy) noexcept
        : DBRec{aDummy} {}

    // DBRec interface.
    [[nodiscard]] auto IsSane() const noexcept -> bool override;
    void ResetDflt() noexcept override;

    // Extended object's interface.

    // Sets/clears the entry for the specified time, rounded to quarter hour.
    void SetEntry(Weekday const &aWeekday, Time const &aTime) noexcept;
    void SetEntry(unsigned int aWeekday, Time const &aTime) noexcept;
    void ClrEntry(Weekday const &aWeekday, Time const &aTime) noexcept;
    void ClrEntry(unsigned int aWeekday, Time const &aTime) noexcept;
    void SetTimeEntry(Time const &aTime) noexcept;
    void ClrAllEntries() noexcept;

    auto IsEntrySet(Time const &aTime) noexcept -> bool;

    // Gets the next set entry from current time.
    struct TimeAndDate_s {
        Time mTime{};
        Weekday mWeekday{};
    };

    using TimeAndDate = struct TimeAndDate_s;
    [[nodiscard]] auto GetNextEntry(
        Weekday const &aWeekday,
        Time const &aTime
    ) const noexcept -> std::optional<std::pair<Time, Weekday>>;

    [[nodiscard]] auto GetNextEntry(TimeAndDate const &aEntry) const noexcept -> std::optional<TimeAndDate>;

private:
    // DBRec interface.
    [[nodiscard]] auto GetRecSize() const noexcept -> size_t override;
    void Serialize(uint8_t * aData) const override;
    void Deserialize(uint8_t const * aData) override;
    void UpdateCRC() noexcept override;

    static auto GetArrayIx(Time const &aTime) noexcept -> unsigned int;
    static auto WeekdayToBitMask(Weekday const &aWeekday) noexcept -> unsigned int;
    static auto BitMaskToWeekday(unsigned int aBitMask) noexcept -> unsigned int ;

    enum BitMaskEnumTag { ALL_WEEK_BIT_MASK = (0x1 << 0) };
    static constexpr auto sHoursQty{24};
    static constexpr auto sSlotsPerHours{4};
    static constexpr auto sTimeEntryQty{sHoursQty * sSlotsPerHours};
    static constexpr auto sWeekdayEntryQty{7};

    static DBRec::Magic constexpr sMagic{ 'C', 'A', 'L' };
    struct RecData {
        BaseRec mBase{{}, {sMagic}};
        std::array<uint8_t, sTimeEntryQty> mCalendarArray{};
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
