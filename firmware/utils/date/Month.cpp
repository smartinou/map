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
//        Copyright (c) 2015-2018, Martin Garon, All rights reserved.
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
  : Limit(Month::NameToUI(Name::Min),
          Month::NameToUI(Name::Max),
          aMonth) {
  // Ctor body left intentionally empty.
}


Month::Month(Month::Name aMonthName)
  : Limit(Month::NameToUI(Name::Min),
          Month::NameToUI(Name::Max)) {

  unsigned int lMonthUI = NameToUI(aMonthName);
  Set(lMonthUI);
}


unsigned int Month::Get(void) const {
  return Limit::Get();
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

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
