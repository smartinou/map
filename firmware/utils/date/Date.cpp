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
//        Copyright (c) 2015-2018, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include "stdio.h"

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

Date::Date(unsigned int  aYear,
           Month::Name   aMonth,
           unsigned int  aDate,
           Weekday::Name aWeekday)
    : mYear(aYear)
    , mMonth(aMonth)
    , mDate(aDate)
    , mWeekday(aWeekday) {

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


void Date::SetYear( unsigned int aYear) {
  mYear.Set(aYear);
}


void Date::SetMonth(unsigned int aMonth) {
  mMonth.Set(aMonth);
}


void Date::SetDate( unsigned int aDate) {
  mDate.Set(aDate);
}


void Date::SetWeekday(unsigned int aWeekday) {
  mWeekday.Set(aWeekday);
}


void Date::SetMonth(Month::Name aMonthName) {

  unsigned int lMonthUI = Month::NameToUI(aMonthName);
  mMonth.Set(lMonthUI);
}


void Date::SetWeekday(Weekday::Name aWeekday) {

  unsigned int lWeekdayUI = Weekday::NameToUI(aWeekday);
  mWeekday.Set(lWeekdayUI);
}


// TimeHelper functions.
char const *DateHelper::ToStr(Date &aDate, char * const aInStr) {

  snprintf(aInStr,
           10 + 1,
           "%04d-%02d-%02d",
           aDate.GetYear(),
           aDate.GetMonth(),
           aDate.GetDate());

  return aInStr;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
