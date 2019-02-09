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
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <stdio.h>

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
class Date
  : public IDate {
 public:
  explicit Date(
    unsigned int  aYear    = 2000,
    Month::Name   aMonth   = Month::Name::January,
    unsigned int  aDate    = 1,
    Weekday::Name aWeekday = Weekday::Name::Saturday);
  ~Date();

  // IDate.
  unsigned int GetYear(void)    const;
  unsigned int GetMonth(void)   const;
  unsigned int GetDate(void)    const;
  unsigned int GetWeekday(void) const;

  Month::Name   GetMonthName(void)   const;
  Weekday::Name GetWeekdayName(void) const;

 private:
  Year    mYear;
  Month   mMonth;
  Day     mDate;
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

char const *ToStr(Date const &aDate, char * const aInStr, size_t aStrSize = (10 + 1));
char const *ToStr2(Date const &aDate, char * const aInStr, size_t aStrSize = (12 + 1));

} // namespace DateHelper

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
