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

// Standard Libraries.
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

//! \brief Planner template class.
// When template parameter is set to true, this produces a weekly planner (7 days).
// When set to off, this produces a daily planner (24H).
template <bool aIsWeeklyPlanner = true>
class Planner {
public:
    Planner() = default;
    ~Planner() = default;

    using Time = std::chrono::hh_mm_ss<std::chrono::seconds>;
    using Weekday = std::chrono::weekday;
    struct Entry_s {
        Time mTime;
        Weekday mWeekday;
        Entry_s(Time const &aTime, Weekday const &aWeekday = std::chrono::Sunday)
            : mTime{aTime}, mWeekday{aWeekday} {}
    };
    using Entry = struct Entry_s;

    // Sets/clears the entry for the specified time (opt weekday).
    bool AddEntry(Entry const &aEntry) {
        auto const lFind = IsEntrySet(aEntry);
        if (!lFind) {
            auto const lDuration = aEntry.mTime.to_duration();
            auto &lHours = mWeekdays.at(aEntry.mWeekday.c_encoding());
            lHours.push_back(lDuration);
            std::sort(lHours.begin(), lHours.end());
            //SetIsDirty();
            return true;
        }
        return false;
    }

    bool DeleteEntry(Entry const &aEntry) {
        auto const lDuration = aEntry.mTime.to_duration();
        auto lHours = mWeekdays.at(aEntry.mWeekday.c_encoding());
        auto const lErased = std::erase(lHours, lDuration);
        if (lErased > 0) {
            //SetIsDirty();
            return true;
        }
    }

    void DeleteAllEntries(void) {
        for (auto const &lWeekday : mWeekdays) lWeekday.clear();
    }

    bool IsEntrySet(Entry const &aEntry) {
        auto const lDuration = aEntry.mTime.to_duration();
        auto const lHours = mWeekdays.at(aEntry.mWeekday.c_encoding());
        auto const lIt = std::find(
            lHours.begin(),
            lHours.end(),
            lDuration
        );
        if (lIt != lHours.end()) {
            return true;
        }
        return false;
    }

    // Gets the next set entry from current time.
    std::optional<Entry> GetNextEntry(Entry const &aEntry) {
        auto const lHours = mWeekdays.at(aEntry.mWeekday);
        if (lHours.empty()) {
            return std::nullopt;
        }

        auto const lDuration = aEntry.mTime.to_duration();
        auto lIt = std::find_if(
            lHours.begin(),
            lHours.end(),
            [&](auto const &aTime) {
                return (lDuration > aTime);
            }
        );

        if (lIt != lHours.end()) {
            // Return found value.
            return Entry{*lIt};
        }
        // Return 1st entry in the vector.
        return Entry{lHours.at(0)};
    }

private:
    // Dynamic vector of entries.
    using TimeEntry = std::chrono::seconds;
    using WeekdayEntry = std::vector<TimeEntry>;
    WeekdayEntry mHours;
    std::array<WeekdayEntry, aIsWeeklyPlanner ? 7 : 1> mWeekdays;
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
