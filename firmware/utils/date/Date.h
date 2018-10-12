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
//        Copyright (c) 2015-2018, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

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


class IDateBuilder
  : public IDate {
 public:
  virtual void SetYear( unsigned int aYear) = 0;
  virtual void SetMonth(unsigned int aMonth) = 0;
  virtual void SetDate( unsigned int aDate) = 0;
  virtual void SetWeekday(unsigned int aWeekday) = 0;

  virtual void SetMonth(Month::Name     aMonthName) = 0;
  virtual void SetWeekday(Weekday::Name aWeekday) = 0;
};


//! \brief Date class as aggregate class.
class Date
  : public IDateBuilder {
 public:
  explicit Date(unsigned int  aYear    = 2000,
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

  // IDateBuilder.
  void SetYear( unsigned int aYear);
  void SetMonth(unsigned int aMonth);
  void SetDate( unsigned int aDate);
  void SetWeekday(unsigned int aWeekday);

  void SetMonth(Month::Name     aMonthName);
  void SetWeekday(Weekday::Name aWeekday);

 private:
  Year    mYear;
  Month   mMonth;
  Day     mDate;
  Weekday mWeekday;
};


// Helper functions.
namespace DateHelper {

char const *ToStr(Date &aDate, char * const aInStr);

} // namespace DateHelper

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
