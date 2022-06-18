#pragma once
// *******************************************************************************
//
// Project: Utilities\Date.
//
// Module: Date class.
//
// *******************************************************************************

//! \file
//! \brief Class to represent date of the day.
//! \ingroup utils

// ******************************************************************************
//
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <string>
#include <time.h>

#include "Day.h"
#include "Month.h"
#include "Year.h"
#include "Weekday.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Date class as aggregate class.
class Date final {
public:
    constexpr explicit Date(
        unsigned int const aYear = 2000,
        Month::Name const aMonth = Month::Name::January,
        unsigned int const aDate = 1,
        Weekday::Name const aWeekday = Weekday::Name::Saturday
    ) noexcept
        : mYear(aYear)
        , mMonth(aMonth)
        , mDate(aDate)
        , mWeekday(aWeekday) {}

    constexpr explicit Date(struct tm const * const aDate) noexcept
        : mYear(aDate->tm_year + 1900) // year since 1900.
        , mMonth(aDate->tm_mon + 1) // 0-11 since January.
        , mDate(aDate->tm_mday)
        , mWeekday((aDate->tm_wday + 1) % 7) {}
    ~Date() = default;

    constexpr unsigned int GetYear(void) const noexcept {return mYear.Get();}
    constexpr unsigned int GetMonth(void) const noexcept {return mMonth.Get();}
    constexpr unsigned int GetDate(void) const noexcept {return mDate.Get();}
    constexpr unsigned int GetWeekday(void) const noexcept {return mWeekday.Get();}

    constexpr Month::Name GetMonthName(void) const noexcept {return mMonth.ToName();}
    constexpr Weekday::Name GetWeekdayName(void) const noexcept {return mWeekday.ToName();}

    // YYYY-MM-DD.
    std::string ToStr(void) const;
    // YYYY-Month-DD
    std::string ToStr2(void) const;

private:
    Year mYear;
    Month mMonth;
    Day mDate;
    Weekday mWeekday;
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
