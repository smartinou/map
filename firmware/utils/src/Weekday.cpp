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

Weekday::Weekday(Name aWeekdayName)
  : Limit(1, 7) {

  unsigned int lWeekdayToUI = NameToUI(aWeekdayName);
  Set(lWeekdayToUI);
}


Weekday::Name Weekday::GetName(void) const {

  unsigned int lWeekdayUI = Get();
  return UIToName(lWeekdayUI);
}


unsigned int Weekday::NameToUI(Name aWeekdayName) {

  unsigned int lWeekdayUI = 0;
  switch (aWeekdayName) {
  case Name::Sunday:    lWeekdayUI = 1; break;
  case Name::Monday:    lWeekdayUI = 2; break;
  case Name::Tuesday:   lWeekdayUI = 3; break;
  case Name::Wednesday: lWeekdayUI = 4; break;
  case Name::Thursday:  lWeekdayUI = 5; break;
  case Name::Friday:    lWeekdayUI = 6; break;
  case Name::Saturday:  lWeekdayUI = 7; break;
  }

  return lWeekdayUI;
}


Weekday::Name Weekday::UIToName(unsigned int aWeekday) {

  switch (aWeekday) {
  default:
  case 1: return Name::Sunday;    break;
  case 2: return Name::Monday;    break;
  case 3: return Name::Tuesday;   break;
  case 4: return Name::Wednesday; break;
  case 5: return Name::Thursday;  break;
  case 6: return Name::Friday;    break;
  case 7: return Name::Saturday;  break;
  }
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
