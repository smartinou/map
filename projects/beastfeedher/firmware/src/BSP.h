#ifndef BSP_H_
#define BSP_H_
// *******************************************************************************
//
// Project: Beast Feed'Her!
//
// Module: Board Support Package.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2015-2016, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

//#include "BeastFeedHerMgr.h"

#include "Date.h"
#include "Time.h"
#include "SPI.h"

//using namespace CoreLink;

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// [MG] MOVE THIS TO "BEAST FEEDER" HEADER FILE.
enum BSP_BEAST_MGR_SIGS_ENUM_TAG {
  SIG_DUMMY = QP::Q_USER_SIG,
  SIG_TIME_TICK,

  // RTCC signals.
  SIG_RTCC_INTERRUPT,
  SIG_RTCC_TIME_TICK_ALARM,
  SIG_RTCC_CALENDAR_EVENT_ALARM,
  SIG_RTCC_ADD_CALENDAR_ENTRY,
  SIG_RTCC_DEL_CALENDAR_ENTRY,

  // Feed manager signals.
  SIG_FEED_MGR_TIMED_FEED_CMD,
  SIG_FEED_MGR_MANUAL_FEED_CMD,
  SIG_FEED_MGR_TIMEOUT,

  SIG_BUTTON_EVT,
  SIG_DEBOUNCE_TIMEOUT,

  SIG_TERMINATE,
  SIG_QTY
};


enum BSP_NAV_BUTTON_ENUM_TAG {
  BSP_NAV_BUTTON_UP     = 0,
  BSP_NAV_BUTTON_DOWN   = 1,
  BSP_NAV_BUTTON_LEFT   = 2,
  BSP_NAV_BUTTON_RIGHT  = 3,
  BSP_NAV_BUTTON_SELECT = 4,
  BSP_NAV_BUTTON_QTY    = 5,
};

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// Forward declarations.
//class Time;
//class Date;


// [MG] MOVE THIS TO "BEAST FEEDER" HEADER FILE.

// Event to pass GPIO info to initial transition.
// Avoids ctor with long argument list.
class GPIOInitEvt : public QP::QEvt {
 public:
  GPIOInitEvt(QP::QSignal aSig,
              unsigned long aGPIOPort,
              unsigned int aGPIOPin) {
    sig = aSig;
    poolId_ = 0U;
    mGPIOPort = aGPIOPort;
    mGPIOPin = aGPIOPin;
  }

 public:
  unsigned long mGPIOPort;
  unsigned int  mGPIOPin;
};


class RTCCInitEvt : public QP::QEvt {
 public:
  RTCCInitEvt(QP::QSignal       aSig,
              CoreLink::SPIDev &aSPIDevRef,
              unsigned long     aCSnGPIOPort,
              unsigned int      aCSnGPIOPin,
              unsigned long     aIRQGPIOPort,
              unsigned int      aIRQGPIOPin):
  mSPIDevRef(aSPIDevRef) {
    sig = aSig;
    poolId_ = 0U;
    mSPIDevRef   = aSPIDevRef;
    mCSnGPIOPort = aCSnGPIOPort;
    mCSnGPIOPin  = aCSnGPIOPin;
    mIRQGPIOPort = aIRQGPIOPort;
    mIRQGPIOPin  = aIRQGPIOPin;
  }

 public:
  CoreLink::SPIDev &mSPIDevRef;
  unsigned long     mCSnGPIOPort;
  unsigned int      mCSnGPIOPin;
  unsigned long     mIRQGPIOPort;
  unsigned int      mIRQGPIOPin;
};


class RTCCEvt : public QP::QEvt {
 public:
  RTCCEvt(QP::QSignal aSig, Time aTime, Date aDate) {
    sig     = aSig;
    poolId_ = 0U;
    mTime   = aTime;
    mDate   = aDate;
  }

 public:
  Time mTime;
  Date mDate;
};


class RTCCSetEvt : public QP::QEvt {
 public:
  RTCCSetEvt(QP::QSignal aSig, Weekday aWeekday, Time aTime) {
    sig      = aSig;
    poolId_  = 0U;
    mWeekday = aWeekday;
    mTime    = aTime;
  }

 public:
  Weekday mWeekday;
  Time    mTime;
};


class ManualFeedCmdEvt : public QP::QEvt {
 public:
  ManualFeedCmdEvt(QP::QSignal aSig, bool aIsOn) {
    sig     = aSig;
    poolId_ = 0U;
    mIsOn   = aIsOn;
  }

 public:
  bool mIsOn;
};



// in game.h, simple events are still defined as class.
class FeedCmdEvt : public QP::QEvt {
 public:
  FeedCmdEvt(QP::QSignal aSig, unsigned int aSrc) {
    sig     = aSig;
    poolId_ = 0U;
    mSrc    = aSrc;
  }

 public:
  enum L_SOURCE_ENUM_TAG {
    BUTTON,
    WEB,
    CLI
  };

 public:
  unsigned int mSrc;
};


class CoreLinkPeripheral;
namespace CoreLink {
  class SPIDev;
}

// ******************************************************************************
//                            EXPORTED VARIABLES
// ******************************************************************************

// ******************************************************************************
//                                 EXTERNS
// ******************************************************************************

// ******************************************************************************
//                            EXPORTED FUNCTIONS
// ******************************************************************************

namespace {

  //void BSP_Init(void);

}

CoreLink::SPIDev * BSPInit(void);

unsigned int BSPGPIOPortToInt(unsigned long aGPIOPort);

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
#endif // BSP_H_
