#ifndef DATE_H_
#define DATE_H_
// *******************************************************************************
//
// Project: Larger project scope.
//
// Module: Module in the larger project scope.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2015-2016, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include "Limit.h"
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
		Weekday::Name aWeekday = Weekday::Name::Saturday)
    : mYear(aYear),
    mMonth(aMonth),
    mDate(aDate),
    mWeekday(aWeekday) {}
  ~Date() {}

  unsigned int GetYear(void)    const { return mYear.Get();    }
  unsigned int GetMonth(void)   const { return mMonth.Get();   }
  unsigned int GetDate(void)    const { return mDate.Get();    }
  unsigned int GetWeekday(void) const { return mWeekday.Get(); }

  Month::Name   GetMonthName(void)   const { return mMonth.GetName();   }
  Weekday::Name GetWeekdayName(void) const { return mWeekday.GetName(); }

  void SetYear( unsigned int aYear)      { mYear.Set(aYear);       }
  void SetMonth(unsigned int aMonth)     { mMonth.Set(aMonth);     }
  void SetDate( unsigned int aDate)      { mDate.Set(aDate);       }
  void SetWeekday(unsigned int aWeekday) { mWeekday.Set(aWeekday); }

  void SetMonth(Month::Name     aMonth);
  void SetWeekday(Weekday::Name aWeekday);

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

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
#endif // DATE_H_
