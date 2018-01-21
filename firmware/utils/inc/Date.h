#ifndef DATE_H_
#define DATE_H_
// *******************************************************************************
//
// Project: Utilities.
//
// Module: Date class.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


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

//! \brief Brief description.
//! Details follow...
//! ...here.
class Date {
 public:
  explicit Date(unsigned int  aYear    = 2000,
                Month::Name   aMonth   = Month::Name::January,
                unsigned int  aDate    = 1,
                Weekday::Name aWeekday = Weekday::Name::Saturday);
  ~Date();

  unsigned int GetYear(void)    const;
  unsigned int GetMonth(void)   const;
  unsigned int GetDate(void)    const;
  unsigned int GetWeekday(void) const;

  Month::Name   GetMonthName(void)   const;
  Weekday::Name GetWeekdayName(void) const;

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
#endif // DATE_H_
