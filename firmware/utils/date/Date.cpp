// *****************************************************************************
//
// Project: Utilities.
//
// Module: Date class.
//
// *****************************************************************************

//! \file
//! \brief Class to represent date of the day.
//! \ingroup utils

// *****************************************************************************
//
//        Copyright (c) 2015-2021, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include <stdio.h>

#include "Date.h"

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

Date::Date(
    unsigned int  aYear,
    Month::Name   aMonth,
    unsigned int  aDate,
    Weekday::Name aWeekday
)   : mYear(aYear)
    , mMonth(aMonth)
    , mDate(aDate)
    , mWeekday(aWeekday) {

    // Ctor intentionally empty.
}


Date::Date(struct tm const * const aDate)
    : mYear(aDate->tm_year + 1900) // year since 1900.
    , mMonth(aDate->tm_mon + 1) // 0-11 since January.
    , mDate(aDate->tm_mday)
    , mWeekday((aDate->tm_wday + 1) % 7) { // 0-6 since Sunday.

    // Ctor intentionally empty.
}


Date::~Date() {

    // Dtor intentionally empty.
}


unsigned int Date::GetYear(void) const {
    return mYear.Get();
}


unsigned int Date::GetMonth(void) const {
    return mMonth.Get();
}


unsigned int Date::GetDate(void) const {
    return mDate.Get();
}


unsigned int Date::GetWeekday(void) const {
    return mWeekday.Get();
}


Month::Name Date::GetMonthName(void) const {
    return mMonth.ToName();
}


Weekday::Name Date::GetWeekdayName(void) const {
    return mWeekday.ToName();
}


bool Date::operator==(Date const &rhs) {
    if ((this->mYear == rhs.mYear) &&
        (this->mMonth == rhs.mMonth) &&
        (this->mDate == rhs.mDate) &&
        (this->mWeekday == rhs.mWeekday)) {
        return true;
    }

    return false;
}


// TimeHelper functions.
char const *DateHelper::ToStr(Date const &aDate, char * const aInStr, size_t aStrSize) {

    snprintf(
        aInStr,
        aStrSize,
        "%04d-%02d-%02d",
        aDate.GetYear(),
        aDate.GetMonth(),
        aDate.GetDate()
    );

    return aInStr;
}


char const *DateHelper::ToStr2(Date const &aDate, char * const aInStr, size_t aStrSize) {

    snprintf(
        aInStr,
        aStrSize,
        "%04d-%s-%02d",
        aDate.GetYear(),
        MonthHelper::ToStr(aDate.GetMonth()),
        aDate.GetDate()
    );

    return aInStr;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
