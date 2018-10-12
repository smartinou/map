// *****************************************************************************
//
// Project: Utilities.
//
// Module: Time class.
//
// *****************************************************************************

//! \file
//! \brief Class used for representation of time.
//! \ingroup utils

// *****************************************************************************
//
//        Copyright (c) 2015-2018, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include "stdio.h"

#include "Time.h"

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

Time::Time()
  : mHours(0)
  , mMinutes(0)
  , mSeconds(0)
  , mIs24H(true)
  , mIsPM(false) {

  // Ctor intentionally empty.
}


Time::Time(Hour   aHours,
           Minute aMinutes,
           Second aSeconds,
           bool   aIs24H,
           bool   aIsPM)
  : mHours(aHours)
  , mMinutes(aMinutes)
  , mSeconds(aSeconds)
  , mIs24H(aIs24H)
  , mIsPM(aIsPM) {

  // Ctor intentionally empty.
}


Time::Time(unsigned int aHours,
           unsigned int aMinutes,
           unsigned int aSeconds,
           bool         aIs24H,
           bool         aIsPM)
    : mHours(aHours)
    , mMinutes(aMinutes)
    , mSeconds(aSeconds)
    , mIs24H(aIs24H)
    , mIsPM(aIsPM) {

  // Ctor intentionally empty.
}


Time::~Time() {

  // Dtor intentionally empty.
}


unsigned int Time::GetHours(void) const {
  return mHours.Get();
}


unsigned int Time::GetMinutes(void) const {
  return mMinutes.Get();
}


unsigned int Time::GetSeconds(void) const {
  return mSeconds.Get();
}


bool Time::Is24H(void) const {
  return mIs24H;
}


bool Time::IsPM(void) const {
  return mIsPM;
}


void Time::SetHours(unsigned int aHours) {
    mHours.Set(aHours);
}


void Time::SetMinutes(unsigned int aMinutes) {
  mMinutes.Set(aMinutes);
}


void Time::SetSeconds(unsigned int aSeconds) {
  mSeconds.Set(aSeconds);
}


void Time::SetIs24H(bool aIs24H) {
  mIs24H = aIs24H;
}


void Time::SetIsPM(bool aIsPM) {
  mIsPM = aIsPM;
}


// TimeHelper functions.
char const *TimeHelper::ToStr(Time &aTime, char * const aInStr) {

  if (aTime.Is24H()) {
    snprintf(aInStr, 5 + 1, "%02d:%02d", aTime.GetHours(), aTime.GetMinutes());
  } else if (aTime.IsPM()) {
    snprintf(aInStr, 8 + 1, "%02d:%02d PM", aTime.GetHours(), aTime.GetMinutes());
  } else {
    snprintf(aInStr, 8 + 1, "%02d:%02d AM", aTime.GetHours(), aTime.GetMinutes());
  }

  return aInStr;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
