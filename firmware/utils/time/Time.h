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


//! \brief Time class, as an aggregate of other classes.
// This is a simple class, meant to represent time snapshots,
// from an RTCC for example.
// It will not properly overflow and keep time if two objects
// are added together.
class Time
  : public ITime {
 public:
  explicit Time(Hour   aHours,
                Minute aMinutes,
                Second aSeconds,
                bool   aIs24H = true,
                bool   aIsPM  = false);
  explicit Time(unsigned int aHours = 0,
                unsigned int aMinutes = 0,
                unsigned int aSeconds = 0,
                bool         aIs24H = true,
                bool         aIsPM  = false);
  ~Time();

  // ITime.
  unsigned int GetHours(void)   const;
  unsigned int GetMinutes(void) const;
  unsigned int GetSeconds(void) const;

  bool Is24H(void) const;
  bool IsPM(void)  const;

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

// Helper functions.
namespace TimeHelper {

char const *ToStr(Time const &aTime, char * const aInStr);

} // namespace TimeHelper

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
