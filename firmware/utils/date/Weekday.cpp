// *****************************************************************************
//
// Project: Utils\Date.
//
// Module: Weekday class.
//
// *****************************************************************************

//! \file
//! \brief Simple weekday class.
//! \ingroup utils

// *****************************************************************************
//
//        Copyright (c) 2015-2018, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include "Weekday.h"

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

Weekday::Weekday(unsigned int aVal)
  : Limit(Weekday::NameToUI(Name::Min),
          Weekday::NameToUI(Name::Max),
          aVal) {
  // Ctor body left intentionally empty.
}


Weekday::Weekday(Name aWeekdayName)
  : Limit(Weekday::NameToUI(Name::Min),
          Weekday::NameToUI(Name::Max)) {

  unsigned int lWeekdayToUI = NameToUI(aWeekdayName);
  Set(lWeekdayToUI);
}


Weekday::Name Weekday::ToName(void) const {

  unsigned int lWeekdayUI = Get();
  return UIToName(lWeekdayUI);
}


unsigned int Weekday::NameToUI(Name aWeekdayName) {
  unsigned int lWeekdayUI = static_cast<unsigned int>(aWeekdayName);
  return lWeekdayUI;
}


Weekday::Name Weekday::UIToName(unsigned int aWeekday) {
  return static_cast<Name>(aWeekday);
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
