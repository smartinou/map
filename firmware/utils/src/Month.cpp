// *****************************************************************************
//
// Project: <Larger project scope.>
//
// Module: <Module in the larger project scope.>
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// *****************************************************************************
//
//        Copyright (c) 2015-2016, Martin Garon, All rights reserved.
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
  : Limit(Month::NameToUI(Name::Min), Month::NameToUI(Name::Max), aMonth) {

  // Ctor intentionally empty.
}


Month::Month(Month::Name aMonthName)
  : Limit(Month::NameToUI(Name::Min), Month::NameToUI(Name::Max)) {

  unsigned int lMonthUI = NameToUI(aMonthName);
  Set(lMonthUI);
}


Month::Name Month::GetName(void) const {

  unsigned int lMonthUI = Get();
  return UIToName(lMonthUI);
}


unsigned int Month::NameToUI(Name aMonthName) {

  unsigned int lMonthUI = 0;
  switch (aMonthName) {
  case Name::January:   lMonthUI =  1; break;
  case Name::February:  lMonthUI =  2; break;
  case Name::March:     lMonthUI =  3; break;
  case Name::April:     lMonthUI =  4; break;
  case Name::May:       lMonthUI =  5; break;
  case Name::June:      lMonthUI =  6; break;
  case Name::July:      lMonthUI =  7; break;
  case Name::August:    lMonthUI =  8; break;
  case Name::September: lMonthUI =  9; break;
  case Name::October:   lMonthUI = 10; break;
  case Name::November:  lMonthUI = 11; break;
  case Name::December:  lMonthUI = 12; break;
  }

  return lMonthUI;
}


Month::Name Month::UIToName(unsigned int aMonth) {

  switch (aMonth) {
  default:
  case  1: return Name::January;   break;
  case  2: return Name::February;  break;
  case  3: return Name::March;     break;
  case  4: return Name::April;     break;
  case  5: return Name::May;       break;
  case  6: return Name::June;      break;
  case  7: return Name::July;      break;
  case  8: return Name::August;    break;
  case  9: return Name::September; break;
  case 10: return Name::October;   break;
  case 11: return Name::November;  break;
  case 12: return Name::December;  break;
  }
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
