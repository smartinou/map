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

  // Display signals.
  SIG_DISPLAY_TIMEOUT,
  SIG_DISPLAY_REFRESH,
  SIG_DISPLAY_TEXT,

  SIG_TERMINATE,
  SIG_QTY
};


#define BSP_TICKS_PER_SEC (100)

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// ******************************************************************************
//                            EXPORTED VARIABLES
// ******************************************************************************

class GPIOs;
class SSD1329;


// RTCC GPIOs.
extern GPIOs * const BSP_gRTCCCSnGPIOPtr;
extern GPIOs * const BSP_gRTCCIntGPIOPtr;

// Motor controller GPIOs.
extern GPIOs * const BSP_gIn1GPIOPtr;
extern GPIOs * const BSP_gIn2GPIOPtr;
extern GPIOs * const BSP_gPWMGPIOPtr;

// ******************************************************************************
//                                 EXTERNS
// ******************************************************************************

// ******************************************************************************
//                            EXPORTED FUNCTIONS
// ******************************************************************************

void BSP_Init(void);

CoreLink::SPIDev *BSP_InitSPIDev(void);
SSD1329          *BSP_InitOLEDDisplay(void);

unsigned int BSP_GPIOPortToInt(unsigned long aGPIOPort);

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
