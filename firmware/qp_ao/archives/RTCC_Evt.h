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
  RTCCInitEvt(QP::QSignal      const aSig,
              CoreLink::SPIDev      &aSPIDevRef,
              unsigned long    const aIntNbr,
              GPIOs          * const aCSn,
              GPIOs          * const aInt,
              CalendarRec    * const aCalendarPtr):
  QP::QEvt(aSig)
    , mSPIDevRef(aSPIDevRef)
    , mIntNbr(aIntNbr)
    , mCSn(aCSn)
    , mInt(aInt)
    , mCalendarPtr(aCalendarPtr) {
    // Ctor body left intentionally empty.
  }

 public:
  CoreLink::SPIDev       &mSPIDevRef;
  unsigned long    const  mIntNbr;
  GPIOs          * const  mCSn;
  GPIOs          * const  mInt;
  CalendarRec    * const  mCalendarPtr;
};


class RTCCTimeDateEvt : public QP::QEvt {
 public:
  RTCCTimeDateEvt(QP::QSignal const aSig,
                  Time        const aTime,
                  Date        const aDate)
    : QP::QEvt(aSig)
    , mTime(aTime)
    , mDate(aDate) {
    // Ctor body left intentionally empty.
  }

 public:
  Time const mTime;
  Date const mDate;
};


class RTCCSaveToRAMEvt : public QP::QEvt {
 public:
  RTCCSaveToRAMEvt(QP::QSignal const aSig,
                   bool        const aIsCalendarChanged)
    : QP::QEvt(aSig)
    , mIsCalendarChanged(aIsCalendarChanged) {
    // Ctor body left intentionally empty.
  }

 public:
  bool const mIsCalendarChanged;
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
