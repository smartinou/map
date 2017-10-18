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
//        Copyright (c) 2015, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include "RTC.h"

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

// Ctor.
RTC::RTC() :
  mCentury(),
  mIsImpure(true) {

  // Intentional empty ctor body.
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

unsigned int RTC::BinaryToBCD(unsigned int aBinVal) {

  unsigned int lBCDVal = 0;

  if (aBinVal > 10) {
    lBCDVal   = BinaryToBCD(aBinVal / 10);
    lBCDVal <<= 4;
    lBCDVal  |= (aBinVal % 10);
  } else {
    lBCDVal = aBinVal;
  }

  return lBCDVal;
}


unsigned int RTC::BCDToBinary(unsigned int aBCDVal) {

  unsigned int lBinVal = aBCDVal & 0x0F;
  aBCDVal >>= 4;
  if (0 != aBCDVal) {
    lBinVal += (10 * BCDToBinary(aBCDVal));
  }

  return lBinVal;
}

#if 0
unsigned int RTC::WeekdayToUI(Weekday aWeekday) {

  switch (aWeekday) {
  case Weekday::sunday:    return 1; break;
  case Weekday::monday:    return 2; break;
  case Weekday::tuesday:   return 3; break;
  case Weekday::wednesday: return 4; break;
  case Weekday::thursday:  return 5; break;
  case Weekday::friday:    return 6; break;
  case Weekday::saturday:  return 7; break;
  }
}


Weekday RTC::UIToWeekday(unsigned int aWeekday) {

  switch (aWeekday) {
  case 1:  return Weekday::sunday;    break;
  case 2:  return Weekday::monday;    break;
  case 3:  return Weekday::tuesday;   break;
  case 4:  return Weekday::wednesday; break;
  case 5:  return Weekday::thursday;  break;
  case 6:  return Weekday::friday;    break;
  case 7:  return Weekday::saturday;  break;
  default: return Weekday::sunday;    break;
  }
}


Month RTC::UIToMonth(unsigned int aMonth) {

  switch (aMonth) {
  case  1: // Intentional fallthrough.
  default: return Month::january;   break;
  case  2: return Month::february;  break;
  case  3: return Month::march;     break;
  case  4: return Month::april;     break;
  case  5: return Month::may;       break;
  case  6: return Month::june;      break;
  case  7: return Month::july;      break;
  case  8: return Month::august;    break;
  case  9: return Month::september; break;
  case 10: return Month::october;   break;
  case 11: return Month::november;  break;
  case 12: return Month::december;  break;
  }
}


unsigned int MonthToUI(Month aMonth) {

  switch (aMonth) {
  case Month::january:
  default:               return  1; break;
  case Month::february:  return  2; break;
  case Month::march:     return  3; break;
  case Month::april:     return  4; break;
  case Month::may:       return  5; break;
  case Month::june:      return  6; break;
  case Month::july:      return  7; break;
  case Month::august:    return  8; break;
  case Month::september: return  9; break;
  case Month::october:   return 10; break;
  case Month::november:  return 11; break;
  case Month::december:  return 12; break;
  }
}
#endif

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
