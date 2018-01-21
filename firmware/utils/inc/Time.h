#ifndef TIME_H_
#define TIME_H_
// *******************************************************************************
//
// Project: Utilities.
//
// Module: Time class.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


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

//! \brief Brief description.
//! Details follow...
//! ...here.
class Time {
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

  unsigned int GetHours(void)   const;
  unsigned int GetMinutes(void) const;
  unsigned int GetSeconds(void) const;

  bool Is24H(void) const;
  bool IsPM(void)  const;

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
#endif // TIME_H_
