#ifndef RTC_H_
#define RTC_H_
// *******************************************************************************
//
// Project: Larger project scope.
//
// Module: Module in the larger project scope.
//
// *******************************************************************************

//! \file
//! \brief RTC base class.
//! \ingroup ext_peripherals


// ******************************************************************************
//
//        Copyright (c) 2015, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <string>

#include "Time.h"
#include "Date.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************


//! \brief Brief description.
//! Details follow...
//! ...here.
class RTC {

 public:
  RTC();
  ~RTC();

  virtual void RdTime(Time &aTimeRef) = 0;
  virtual void RdDate(Date &aDateRef) = 0;
  virtual void RdTimeAndDate(Time &aTimeRef, Date &aDateRef) = 0;

  virtual void WrTime(Time const &aTimeRef) = 0;
  virtual void WrDate(Date const &aDateRef) = 0;
  virtual void WrTimeAndDate(Time const &aTimeRef, Date const &aDateRef) = 0;

  //virtual void GetTime(Time &aTimeRef) = 0;
  //virtual void GetDate(Date &aDateRef) = 0;
  virtual void GetTimeAndDate(Time &aTimeRef, Date &aDateRef) = 0;
  virtual uint8_t GetStatus(void) = 0;
  void ISRCallback(void) { mIsImpure = true; }

 protected:
  bool IsImpure(void) { return mIsImpure; }
  virtual void UpdateCachedVal(void) = 0;

  unsigned int BinaryToBCD(unsigned int aBinVal);
  unsigned int BCDToBinary(unsigned int aBCDVal);

#if 0
  // [MG] ESSAYER DE ME FIER SUR L'API DE DATE ET MONTH SIMPLEMENT.
  static unsigned int WeekdayToUI(Weekday aWeekday);
  static Weekday      UIToWeekday(unsigned int aWeekday);

  static unsigned int MonthToUI(Month aMonth);
  static Month        UIToMonth(unsigned int aMonth);
#endif

  unsigned int mCentury;
  bool         mIsImpure;
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
#endif // RTC_H_
