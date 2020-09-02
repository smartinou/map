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
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include <stdio.h>
#include <ctime>
#include <stdexcept>

#include "Time2.h"

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

Time::Time(unsigned int aSeconds) {
  Init(aSeconds);
}


Time::Time(
  unsigned int aHours,
  unsigned int aMinutes,
  unsigned int aSeconds,
  bool         aIsPM)
    : mSecondsInDay(0) {

  // Ctor body.
  unsigned int lHours = aHours;
  if (aIsPM) {
    lHours += HOURS_IN_HALF_DAY;
  }
  if (IsValid(lHours, aMinutes, aSeconds)) {
    unsigned int lSeconds = aSeconds;
    lSeconds += (aMinutes * SECONDS_IN_A_MINUTE);
    lSeconds += (lHours * SECONDS_IN_AN_HOUR);
    Init(lSeconds);
  } else {
    // Throw.
  }
}


Time::~Time() {
  // Dtor intentionally empty.
}


bool Time::IsValid(
  unsigned int aHours,
  unsigned int aMinutes,
  unsigned int aSeconds) {

  bool lHoursValid   = ((aHours   >= 0) && (aHours   < HOURS_IN_A_DAY));
  bool lMinutesValid = ((aMinutes >= 0) && (aMinutes < MINUTES_IN_AN_HOUR));
  bool lSecondsValid = ((aSeconds >= 0) && (aSeconds < SECONDS_IN_A_MINUTE));

  return (lHoursValid && lMinutesValid && lSecondsValid);
}


Time Time::Now(void) {
  // Get time now.
  time_t lTime = time(0);
  struct tm *lNow = localtime(&lTime);
  return Time(lNow->tm_hour, lNow->tm_min, lNow->tm_sec);
}


unsigned int Time::GetHours(void) const {
  return (mSecondsInDay / SECONDS_IN_AN_HOUR);
}


unsigned int Time::GetMinutes(void) const {
  return ((mSecondsInDay % SECONDS_IN_AN_HOUR)) / SECONDS_IN_A_MINUTE;
}


unsigned int Time::GetSeconds(void) const {
  return (mSecondsInDay % SECONDS_IN_A_MINUTE);
}


Time& Time::operator+=(unsigned int rhs) {
  this->mSecondsInDay += rhs;
  return *this;
}


Time& Time::operator+=(Time const &rhs) {
  this->mSecondsInDay += rhs.mSecondsInDay;
  return *this;
}

#if 0
Time Time::operator+(Time lhs, unsigned int rhs) {
  lhs += rhs;
  return lhs;
}


Time Time::operator+(Time lhs, Time const &rhs) {
  lhs += rhs;
  return lhs;
}
#endif

// TimeHelper functions.
char const *TimeHelper::ToStr(Time const &aTime, char * const aInStr) {

  snprintf(aInStr, 5 + 1, "%02d:%02d", aTime.GetHours(), aTime.GetMinutes());
  return aInStr;
}


char const *ToStr12H(Time const &aTime, char * const aInStr, bool aIsPM) {

if (aIsPM) {
    snprintf(aInStr, 8 + 1, "%02d:%02d PM", aTime.GetHours() - Time::HOURS_IN_HALF_DAY, aTime.GetMinutes());
  } else {
    snprintf(aInStr, 8 + 1, "%02d:%02d AM", aTime.GetHours(), aTime.GetMinutes());
  }

  return aInStr;
}


// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

bool Time::Init(unsigned int aSeconds) {
  if ((aSeconds >= 0) && (aSeconds < SECONDS_IN_A_DAY)) {
    mSecondsInDay = aSeconds;
    return true;
  }

  return false;
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
