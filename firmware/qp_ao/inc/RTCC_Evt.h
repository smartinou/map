#ifndef RTCC_EVT_H_
#define RTCC_EVT_H_
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
//        Copyright (c) 2017, Martin Garon, All rights reserved.
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
class Calendar;


// Class definitions.
class RTCCInitEvt : public QP::QEvt {
 public:
  RTCCInitEvt(QP::QSignal       aSig,
              CoreLink::SPIDev &aSPIDevRef,
              unsigned long     aCSnGPIOPort,
              unsigned int      aCSnGPIOPin,
              unsigned long     aIRQGPIOPort,
              unsigned int      aIRQGPIOPin,
              Calendar         *aCalendarPtr):
  mSPIDevRef(aSPIDevRef) {
    sig          = aSig;
    poolId_      = 0U;
    mSPIDevRef   = aSPIDevRef;
    mCSnGPIOPort = aCSnGPIOPort;
    mCSnGPIOPin  = aCSnGPIOPin;
    mIRQGPIOPort = aIRQGPIOPort;
    mIRQGPIOPin  = aIRQGPIOPin;
    mCalendarPtr = aCalendarPtr;
  }

 public:
  CoreLink::SPIDev &mSPIDevRef;
  unsigned long     mCSnGPIOPort;
  unsigned int      mCSnGPIOPin;
  unsigned long     mIRQGPIOPort;
  unsigned int      mIRQGPIOPin;
  Calendar         *mCalendarPtr;
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
#endif // RTCC_EVT_H_
