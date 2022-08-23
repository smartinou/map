#pragma once
// *******************************************************************************
//
// Project: Utils.
//
// Module: Generic daily planner DB.
//
// *******************************************************************************

//! \file
//! \brief Daily planner DB class.
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

#include "DBRec.h"

// Standard Libraries.
#include <algorithm>
#include <array>
#include <chrono>
#include <optional>
#include <vector>

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// PRISE 2.
// ON PART D'UN DAILY PLANNER, SUR UN TYPE T.
// UN HELPER DE VECTOR
// ON PEUT INSTANTIER AVEC Time ou std::chrono::duration.
// SHOULD ALSO DERIVE FROM DBRec AT SOME POINT.
template <typename T>
class DailyPlanner
    : public DBRec {
public:
    [[maybe_unused]] auto AddEntry(T const &aEntry) -> bool {
        auto const lFind = IsEntrySet(aEntry);
        if (!lFind) {
            mEntries.push_back(aEntry);
            std::sort(mEntries.begin(), mEntries.end());
            mIsDirty = true;
            return true;
        }
        return false;
    }

    [[maybe_unused]] auto DeleteEntry(T const &aEntry) noexcept -> bool {
        auto const lErased = std::erase(mEntries, aEntry);
        if (lErased > 0) {
            mIsDirty = true;
            return true;
        }
        return false;
    }

    void DeleteAllEntries() noexcept { mEntries.clear(); mIsDirty = true;}

    auto GetFirstEntry() const noexcept -> std::optional<T> {
        if (mEntries.size()) {
            return *mEntries.cbegin();
        }

        return std::nullopt;
    }

    auto GetNextEntry(
        T const &aEntry,
        bool const aWrapAround = true
    ) const noexcept -> std::optional<T> {
        if (mEntries.empty()) {
            return std::nullopt;
        }

        // We must assume a request for an entry that doesn't exist.
        // so we search for the 1st entry greater in value than requested.
        auto const lIt = std::find_if(
            mEntries.cbegin(),
            mEntries.cend(),
            [&](auto const &aVectorEntry) {
                return (aEntry < aVectorEntry);
            }
        );

        if (lIt != mEntries.cend()) {
            // Return found value.
            return *lIt;
        } else if (aWrapAround) {
            // Return 1st entry in the vector.
            // [MG] CONSIDER std::optional<T>(std::in_place{})
            return *mEntries.cbegin();
        }

        return std::nullopt;
    }

    [[nodiscard]] auto IsDirty() const noexcept -> bool {return mIsDirty;}
    void ResetDirty() noexcept {mIsDirty = false;}
    auto Size() const noexcept -> size_t {return mEntries.size();}

    // DBRec Interface.
    [[nodiscard]] virtual auto IsSane() const -> bool;
    virtual void ResetDflt();

private:
    [[nodiscard]] auto IsEntrySet(T const &aEntry) noexcept -> bool {
        auto const lIt = std::find(
            mEntries.cbegin(),
            mEntries.cend(),
            aEntry
        );
        if (lIt != mEntries.cend()) {
            return true;
        }
        return false;
    }

    std::vector<T> mEntries;
    bool mIsDirty{false};
};


// Attempt at using the single-day template as a DB record.
class DailyPlannerRec final
    : public Planner<std::chrono::seconds> // std::chrono::duration
    , public DBRec
{
protected:
    template<class T, typename...Args>
    friend auto DBRec::Create(Args&&... aArgs) -> std::shared_ptr<T>;

public:
    explicit DailyPlannerRec(UseCreateFunc const aDummy) noexcept
        : DBRec{aDummy} {}

    // DBRec Interface.
    [[nodiscard]] auto IsSane() const noexcept -> bool override;
    virtual void ResetDflt() noexcept override;

private:
    [[nodiscard]] auto GetRecSize() const noexcept -> size_t override;
    void Serialize(uint8_t * aData) const override;
    void Deserialize(uint8_t const * aData) override;
    void UpdateCRC() noexcept override;

    static constexpr DBRec::Magic sMagic{ 'P', ' ', 'R' };

    // The remaining of the record is represented by the vector<T>
    // in the base template.
    struct BaseRec mBase{{}, {sMagic}};
}


// Generates a weekly planner of type T.
// Ex.: WeeklyPlanner<std::chrono::seconds> lPlanner;
template <typename T>
class WeeklyPlanner {
public:
    using TimeEntries = DailyPlanner<T>;
    using Weekday = std::chrono::weekday;
    struct DayAndTime {
        Weekday mWeekday{};
        T mTime{};
    };

    [[maybe_unused]] auto AddEntry(Weekday const &aWeekday, T const &aTimeEntry) -> bool {
        auto &lDayEntry = mPlanner.at(aWeekday.c_encoding());
        return lDayEntry.AddEntry(aTimeEntry);
    }

    [[maybe_unused]] auto DeleteEntry(Weekday const &aWeekday, T const &aTimeEntry) -> bool {
        auto &lDayEntry = mPlanner.at(aWeekday.c_encoding());
        return lDayEntry.DeleteEntry(aTimeEntry);
    }

    void DeleteAllEntries() noexcept {
        for (auto &lDayEntry : mPlanner) {lDayEntry.DeleteAllEntries();}
    }

    auto GetNextEntry(
        Weekday const &aWeekday,
        T const &aTimeEntry
    ) const noexcept -> std::optional<struct DayAndTime> {

        // Look for next entry in current weekday.
        auto const &lDayEntry = mPlanner.at(aWeekday.c_encoding());
        auto const lRes = lDayEntry.GetNextEntry(aTimeEntry);
        if (lRes) {
            return DayAndTime{aWeekday, lRes.value()};
        }

        // Catch the first entry starting next following day.
        static constexpr T lTimeEntry{};
        Weekday lWeekday{(aWeekday.c_encoding() + 1) % sDaysPerWeek};
        for (auto lIx{lWeekday.c_encoding()}; lIx < (lWeekday.c_encoding() + mPlanner.size()); ++lIx) {
            auto const &lDayEntry = mPlanner.at(lIx % sDaysPerWeek);
            // Both are working.
            //auto const lRes = lDayEntry.GetNextEntry(lTimeEntry, false);
            auto const lRes = lDayEntry.GetFirstEntry();
            if (lRes) {
                return DayAndTime{lWeekday, lRes.value()};
            }
        }

        return std::nullopt;
    }

    [[nodiscard]] auto IsDirty() const noexcept -> bool {
        return std::any_of(
            mPlanner.cbegin(),
            mPlanner.cend(),
            [](TimeEntries const &aEntry) {return aEntry->IsDirty();}
        );
    }

    void ResetDirty() noexcept {
        for (auto &lDayEntry : mPlanner) {
            lDayEntry.ResetDirty();
        }
    }

    auto Size() const noexcept -> size_t {
        // Accumulate?
        size_t lSize{0};
        for (auto const &lDayEntry : mPlanner) {
            lSize += lDayEntry.Size();
        }
        return lSize;
    }

private:
    static constexpr auto sDaysPerWeek{7};
    // [MG] ALTERNATIVE: INSTEAD OF TREATING THIS LIKE AN ARRAY OF DailyPlanner,
    // [MG] TREAT THIS AS AN ARRAY OR DailyPlannerRec.
    // [MG] THE OVERRIDES OF DBRec WILL LOOP IN THE ARRAY.
    // [MG] TRY TO MAKE THIS A TEMPLATE:
    // [MG] std::array<DailyPlanner<T>, ...>
    // [MG] std::array<DailerPlannerRec<T>, ...>
    std::array<TimeEntries, sDaysPerWeek> mPlanner;
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
