#ifndef TIME_H_
#define TIME_H_
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

#include "Hour.h"
#include "Minute.h"
#include "Second.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Brief description.
//! Details follow...
//! ...here.
// [MG] CONSIDER MOVING IMPLEMENTATION TO CPP FILE TO HIDE DETAILS,
// [MG] ... AND TO PERFORM FORWARD CLASS DECLARATION INSTEAD OF INCLUDES.
class Time {
 public:
  Time()
    : mHours(0),
    mMinutes(0),
    mSeconds(0),
    mIs24H(true),
    mIsPM(false) {}
  explicit Time(Hour aHours, Minute aMinutes, Second aSeconds, bool aIs24H = true, bool aIsPM = false)
    : mHours(aHours),
    mMinutes(aMinutes),
    mSeconds(aSeconds),
    mIs24H(aIs24H),
    mIsPM(aIsPM) {}
  ~Time() {}

  unsigned int GetHours(void)   const { return mHours.Get();   }
  unsigned int GetMinutes(void) const { return mMinutes.Get(); }
  unsigned int GetSeconds(void) const { return mSeconds.Get(); }

  bool Is24H(void) const { return mIs24H; }
  bool IsPM(void)  const { return mIsPM;  }

  void SetHours(  unsigned int aHours)   { mHours.Set(aHours);     }
  void SetMinutes(unsigned int aMinutes) { mMinutes.Set(aMinutes); }
  void SetSeconds(unsigned int aSeconds) { mSeconds.Set(aSeconds); }

  void SetIs24H(bool aIs24H) { mIs24H = aIs24H; }
  void SetIsPM( bool aIsPM)  { mIsPM  = aIsPM;  }
  
 private:
  Hour   mHours;
  Minute mMinutes;
  Second mSeconds;

  bool   mIs24H;
  bool   mIsPM;
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
#endif // TIME_H_
