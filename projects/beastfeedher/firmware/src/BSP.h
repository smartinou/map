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

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// [MG] MOVE THIS TO "BEAST FEEDER" HEADER FILE.
enum BSP_BEAST_MGR_SIGS_ENUM_TAG {
  SIG_DUMMY = QP::Q_USER_SIG,
  SIG_TIME_TICK,
  SIG_RTC_IRQ,
  SIG_BUTTON_EVT,
  SIG_FEED_CMD,
  SIG_DEBOUNCE_TIMEOUT,
  SIG_FEED_DELAY_TIMEOUT,
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

// [MG] MOVE THIS TO "BEAST FEEDER" HEADER FILE.

// Event to pass GPIO info to initial transition.
// Avoids ctor with long argument list.
class GPIOInitEvt : public QP::QEvt {
 public:
  GPIOInitEvt(QP::QSignal aSig, unsigned long aGPIOPort, unsigned int aGPIOPin) {
    sig       = aSig;
    poolId_   = 0U;
    mGPIOPort = aGPIOPort;
    mGPIOPin  = aGPIOPin;
  }

 public:
  unsigned long mGPIOPort;
  unsigned int  mGPIOPin;
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
    RTC_INT,
    BUTTON,
    WEB,
    CLI
  };

 public:
  unsigned int mSrc;
};


// [MG] MOVE THIS TO "BEAST FEEDER" HEADER FILE.
class ButtonEvt : public QP::QEvt {
 public:
  ButtonEvt(QP::QSignal  aSig,
	    unsigned int aID,
	    unsigned int aState,
	    unsigned int aDebounceDelay = 0) {
    sig     = aSig;
    poolId_ = 0U;
    mID     = aID;
    mState  = aState;
    mDebounceDelay = aDebounceDelay;
  }

 public:
  enum L_STATE_ENUM_TAG {
    UNKNOWN,
    RELEASED,
    PRESSED,
    DEBOUNCED
  };

 public:
  unsigned int mID;
  unsigned int mState;
  unsigned int mDebounceDelay;
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
