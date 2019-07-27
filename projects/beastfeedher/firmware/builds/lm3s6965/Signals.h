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

#include <qpcpp.h>

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// [MG] MOVE THIS TO "BEAST FEEDER" HEADER FILE.
enum PFPP_SIGS_ENUM_TAG {
  SIG_DUMMY = QP::Q_USER_SIG,
  TERMINATE_SIG,
  TIME_TICK_SIG,
  LOG_SIG,

  // RTCC signals.
  RTCC_INTERRUPT_SIG,
  RTCC_TIME_TICK_ALARM_SIG,
  RTCC_CALENDAR_EVENT_ALARM_SIG,
  RTCC_SAVE_TO_NV_MEMORY_SIG,
  RTCC_SET_TIME_SIG,
  RTCC_SET_DATE_SIG,
  //RTCC_LOG_SIG,

  // Feed manager signals.
  FEED_MGR_TIMED_FEED_CMD_SIG,
  SIG_FEED_MGR_TIMED_FEED_CMD = FEED_MGR_TIMED_FEED_CMD_SIG,
  FEED_MGR_MANUAL_FEED_CMD_SIG,
  SIG_FEED_MGR_MANUAL_FEED_CMD = FEED_MGR_MANUAL_FEED_CMD_SIG,
  FEED_MGR_TIMEOUT_SIG,
  SIG_FEED_MGR_TIMEOUT = FEED_MGR_TIMEOUT_SIG,
  FEED_MGR_LOG_SIG,
  SIG_FEED_MGR_LOG = FEED_MGR_LOG_SIG,

  // LwIP manager signals.
  SIG_LWIP_SLOW_TICK,
  SIG_LWIP_RX_READY,
  SIG_LWIP_TX_READY,
  SIG_LWIP_RX_OVERRUN,
  SIG_LWIP_MGR_LOG,

  // Display signals.
  DISPLAY_TIMEOUT_SIG,
  DISPLAY_REFRESH_SIG,
  DISPLAY_TEXT_SIG,

  // Log event generation/sinks.
  LOG_EVT_SIG,
  LOG_TIMER_SIG,

  QTY_SIG
};

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

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
