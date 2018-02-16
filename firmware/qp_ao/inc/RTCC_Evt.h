#pragma once
// *******************************************************************************
//
// Project: Active Object Library
//
// Module: RTCC QP Events.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2017-2018, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include "Date.h"
#include "Time.h"
#include "SPI.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// Forward declarations.
class DBRec;
class CalendarRec;
class GPIOs;


// Class definitions.
class RTCCInitEvt : public QP::QEvt {
 public:
  RTCCInitEvt(QP::QSignal       aSig,
              CoreLink::SPIDev &aSPIDevRef,
              unsigned long     aIntNbr,
              GPIOs            *aCSn,
              GPIOs            *aInt,
              CalendarRec      *aCalendarPtr):
  mSPIDevRef(aSPIDevRef) {
    sig          = aSig;
    poolId_      = 0U;
    mSPIDevRef   = aSPIDevRef;
    mIntNbr      = aIntNbr;
    mCSn         = aCSn;
    mInt         = aInt;
    mCalendarPtr = aCalendarPtr;
  }

 public:
  CoreLink::SPIDev &mSPIDevRef;
  unsigned long     mIntNbr;
  GPIOs            *mCSn;
  GPIOs            *mInt;
  CalendarRec      *mCalendarPtr;
};


class RTCCTimeDateEvt : public QP::QEvt {
 public:
  RTCCTimeDateEvt(QP::QSignal aSig, Time aTime, Date aDate) {
    sig     = aSig;
    poolId_ = 0U;
    mTime   = aTime;
    mDate   = aDate;
  }

 public:
  Time mTime;
  Date mDate;
};


class RTCCSaveToRAMEvt : public QP::QEvt {
 public:
  RTCCSaveToRAMEvt(QP::QSignal aSig, bool aIsCalendarChanged) {
    sig                = aSig;
    mIsCalendarChanged = aIsCalendarChanged;
  }

 public:
  bool mIsCalendarChanged;
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
