// *****************************************************************************
//
// Project: Utils\Date.
//
// Module: Month class.
//
// *****************************************************************************

//! \file
//! \brief Simple month class.
//! \ingroup utils

// *****************************************************************************
//
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include "Month.h"

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

Month::Month(unsigned int aMonth)
  : Limit(
      Month::NameToUI(Name::Min),
      Month::NameToUI(Name::Max),
      aMonth) {
  // Ctor body left intentionally empty.
}


Month::Month(Month::Name aMonthName)
  : Limit(
      Month::NameToUI(Name::Min),
      Month::NameToUI(Name::Max)) {

  unsigned int lMonthUI = NameToUI(aMonthName);
  Set(lMonthUI);
}


Month::Name Month::ToName(void) const {

  unsigned int lMonthUI = Limit::Get();
  return UIToName(lMonthUI);
}


unsigned int Month::NameToUI(Name aMonthName) {
  unsigned int lMonthUI = static_cast<unsigned int>(aMonthName);
  return lMonthUI;
}


Month::Name Month::UIToName(unsigned int aMonth) {
  return static_cast<Name>(aMonth);
}


char const * MonthHelper::ToStr(Month const &aMonth) {
  switch (aMonth.Get()) {
    case  1: return "Jan";
    case  2: return "Feb";
    case  3: return "Mar";
    case  4: return "Apr";
    case  5: return "May";
    case  6: return "June";
    case  7: return "July";
    case  8: return "Aug";
    case  9: return "Sept";
    case 10: return "Oct";
    case 11: return "Nov";
    case 12: return "Dec";
  }
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
