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
//        Copyright (c) 2015-2018, Martin Garon, All rights reserved.
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

class ITime {
 public:
  virtual unsigned int GetHours(void)   const = 0;
  virtual unsigned int GetMinutes(void) const = 0;
  virtual unsigned int GetSeconds(void) const = 0;

  virtual bool Is24H(void) const = 0;
  virtual bool IsPM(void)  const = 0;
};


class ITimeBuilder
  : public ITime {
 public:
  virtual void SetHours(  unsigned int aHours) = 0;
  virtual void SetMinutes(unsigned int aMinutes) = 0;
  virtual void SetSeconds(unsigned int aSeconds) = 0;

  virtual void SetIs24H(bool aIs24H) = 0;
  virtual void SetIsPM( bool aIsPM) = 0;
};


//! \brief Time class, as an aggregate of other classes.
class Time
  : public ITimeBuilder {
 public:
  Time();
  explicit Time(Hour   aHours,
                Minute aMinutes,
                Second aSeconds,
                bool   aIs24H = true,
                bool   aIsPM  = false);
  explicit Time(unsigned int aHours,
                unsigned int aMinutes,
                unsigned int aSeconds,
                bool         aIs24H = true,
                bool         aIsPM  = false);
  ~Time();

  // ITime.
  unsigned int GetHours(void)   const;
  unsigned int GetMinutes(void) const;
  unsigned int GetSeconds(void) const;

  bool Is24H(void) const;
  bool IsPM(void)  const;

  // ITimeBuilder.
  void SetHours(  unsigned int aHours);
  void SetMinutes(unsigned int aMinutes);
  void SetSeconds(unsigned int aSeconds);

  void SetIs24H(bool aIs24H);
  void SetIsPM( bool aIsPM);

 private:
  Hour   mHours;
  Minute mMinutes;
  Second mSeconds;

  bool   mIs24H;
  bool   mIsPM;
};


// Helper functions.
namespace TimeHelper {

char const *ToStr(Time &aTime, char * const aInStr);

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
