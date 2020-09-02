#pragma once
// *******************************************************************************
//
// Project: Utilities\Time.
//
// Module: Time class.
//
// *******************************************************************************

//! \file
//! \brief Class used for representation of time.
//! \ingroup utils

// ******************************************************************************
//
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class ITime {
 public:
  virtual unsigned int GetHours(void)   const = 0;
  virtual unsigned int GetMinutes(void) const = 0;
  virtual unsigned int GetSeconds(void) const = 0;
};


//! \brief Time class.
class Time
  : public ITime {
 public:
  explicit Time(unsigned int aSeconds);
  explicit Time(
    unsigned int aHours,
    unsigned int aMinutes,
    unsigned int aSeconds,
    bool         aIsPM  = false);
  ~Time();

  bool IsValid(
    unsigned int aHours,
    unsigned int aMinutes,
    unsigned int aSeconds);

  Time Now(void);

  // ITime.
  unsigned int GetHours(void) const;
  unsigned int GetMinutes(void) const;
  unsigned int GetSeconds(void) const;

  // Operators.
  Time& operator+=(unsigned int rhs);
  Time& operator+=(Time const &rhs);
  
  friend Time operator+(Time lhs, Time const &rhs) {
    lhs += rhs;
    return lhs;
  }

  friend Time operator+(Time lhs, unsigned int rhs) {
    lhs.mSecondsInDay += rhs;
    return lhs;
  }
  
 private:
  Time();
  bool Init(unsigned int aSeconds);

  unsigned int mSecondsInDay;

public:
  static const unsigned int SECONDS_IN_A_MINUTE = 60;
  static const unsigned int MINUTES_IN_AN_HOUR  = 60;
  static const unsigned int SECONDS_IN_AN_HOUR  = MINUTES_IN_AN_HOUR * SECONDS_IN_A_MINUTE;
  static const unsigned int HOURS_IN_HALF_DAY   = 12;
  static const unsigned int HOURS_IN_A_DAY      = 24;
  static const unsigned int SECONDS_IN_A_DAY    = HOURS_IN_A_DAY * SECONDS_IN_AN_HOUR;
};


// Helper functions.
namespace TimeHelper {

char const *ToStr(Time const &aTime, char * const aInStr);
char const *ToStr12H(Time const &aTime, char * const aInStr, bool aIsPM);

} // namespace TimeHelper

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
