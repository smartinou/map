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


// Class definitions.
class RTCCInitEvt : public QP::QEvt {
 public:
  RTCCInitEvt(QP::QSignal       aSig,
              CoreLink::SPIDev &aSPIDevRef,
              unsigned long     aCSnGPIOPort,
              unsigned int      aCSnGPIOPin,
              unsigned long     aIRQGPIOPort,
              unsigned int      aIRQGPIOPin,
              unsigned long     aIntNbr,
              CalendarRec      *aCalendarPtr):
  mSPIDevRef(aSPIDevRef) {
    sig          = aSig;
    poolId_      = 0U;
    mSPIDevRef   = aSPIDevRef;
    mCSnGPIOPort = aCSnGPIOPort;
    mCSnGPIOPin  = aCSnGPIOPin;
    mIRQGPIOPort = aIRQGPIOPort;
    mIRQGPIOPin  = aIRQGPIOPin;
    mIntNbr      = aIntNbr;
    mCalendarPtr = aCalendarPtr;
  }

 public:
  CoreLink::SPIDev &mSPIDevRef;
  unsigned long     mCSnGPIOPort;
  unsigned int      mCSnGPIOPin;
  unsigned long     mIRQGPIOPort;
  unsigned int      mIRQGPIOPin;
  unsigned long     mIntNbr;
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
