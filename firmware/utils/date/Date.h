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

#include <stdio.h>
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

class IDate {
public:
    virtual unsigned int GetYear(void)    const = 0;
    virtual unsigned int GetMonth(void)   const = 0;
    virtual unsigned int GetDate(void)    const = 0;
    virtual unsigned int GetWeekday(void) const = 0;

    virtual Month::Name   GetMonthName(void)   const = 0;
    virtual Weekday::Name GetWeekdayName(void) const = 0;
};


//! \brief Date class as aggregate class.
class Date final
    : public IDate {
public:
    constexpr explicit Date(
        unsigned int  aYear    = 2000,
        Month::Name   aMonth   = Month::Name::January,
        unsigned int  aDate    = 1,
        Weekday::Name aWeekday = Weekday::Name::Saturday
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

    // IDate.
    constexpr unsigned int GetYear(void) const {return mYear.Get();}
    constexpr unsigned int GetMonth(void) const {return mMonth.Get();}
    constexpr unsigned int GetDate(void) const {return mDate.Get();}
    constexpr unsigned int GetWeekday(void) const {return mWeekday.Get();}

    constexpr Month::Name GetMonthName(void) const {return mMonth.ToName();}
    constexpr Weekday::Name GetWeekdayName(void) const {return mWeekday.ToName();}

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

// Helper functions.
namespace DateHelper {

// YYYY-MM-DD.
char const *ToStr(Date const &aDate, char * const aInStr, size_t const aStrSize = (10 + 1));
// YYYY-Month-DD
char const *ToStr2(Date const &aDate, char * const aInStr, size_t const aStrSize = (12 + 1));

} // namespace DateHelper

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
