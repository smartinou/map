#pragma once
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
//        Copyright (c) 2015-2018, Martin Garon, All rights reserved.
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

// [MG] MOVE THIS TO "BEAST FEEDER" HEADER FILE.
enum BSP_BEAST_MGR_SIGS_ENUM_TAG {
  SIG_DUMMY = QP::Q_USER_SIG,
  SIG_TIME_TICK,

  // RTCC signals.
  SIG_RTCC_INTERRUPT,
  SIG_RTCC_TIME_TICK_ALARM,
  SIG_RTCC_CALENDAR_EVENT_ALARM,
  SIG_RTCC_SAVE_TO_NV_MEM,
  SIG_RTCC_SET_TIME,
  SIG_RTCC_SET_DATE,

  // Feed manager signals.
  SIG_FEED_MGR_TIMED_FEED_CMD,
  SIG_FEED_MGR_MANUAL_FEED_CMD,
  SIG_FEED_MGR_TIMEOUT,

  // LwIP manager signals.
  LWIP_SLOW_TICK_SIG,
  LWIP_RX_READY_SIG,
  LWIP_TX_READY_SIG,
  LWIP_RX_OVERRUN_SIG,

  SIG_BUTTON_EVT,
  SIG_DEBOUNCE_TIMEOUT,

  SIG_TERMINATE,
  SIG_QTY
};


#define BSP_TICKS_PER_SEC (100)

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

// ******************************************************************************
//                            EXPORTED VARIABLES
// ******************************************************************************

class GPIOs;


// RTCC GPIOs.
extern GPIOs *gRTCCCSnPtr;
extern GPIOs *gRTCCIntPtr;

// Motor controller GPIOs.
extern GPIOs *gIn1Ptr;
extern GPIOs *gIn2Ptr;
extern GPIOs *gPWMPtr;

// ******************************************************************************
//                                 EXTERNS
// ******************************************************************************

// ******************************************************************************
//                            EXPORTED FUNCTIONS
// ******************************************************************************

CoreLink::SPIDev * BSPInit(void);

unsigned int BSPGPIOPortToInt(unsigned long aGPIOPort);

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
