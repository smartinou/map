#pragma once
// *******************************************************************************
//
// Project: PFPP
//
// Module: Board Support Package.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// ******************************************************************************
//
//        Copyright (c) 2015-2020, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <qpcpp.h>

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

enum PFPP_SIGS_ENUM_TAG {
    DUMMY_SIG = QP::Q_USER_SIG,
    TERMINATE_SIG,
    TIME_TICK_SIG,

    // RTCC signals.
    RTCC_INTERRUPT_SIG,
    RTCC_TIME_TICK_ALARM_SIG,
    RTCC_CALENDAR_EVENT_ALARM_SIG,
    RTCC_SAVE_TO_NV_MEMORY_SIG,
    RTCC_SET_TIME_SIG,
    RTCC_SET_DATE_SIG,
    RTCC_SET_TIME_AND_DATE_SIG,

    // Feed manager signals.
    FEED_MGR_TIMED_FEED_CMD_SIG,
    FEED_MGR_MANUAL_FEED_CMD_SIG,
    FEED_MGR_TIMEOUT_SIG,
    FEED_MGR_LOG_SIG,

    // LwIP manager signals.
    LWIP_SLOW_TICK_SIG,
    LWIP_RX_READY_SIG,
    LWIP_TX_READY_SIG,
    LWIP_RX_OVERRUN_SIG,
    LWIP_MGR_LOG_SIG,
    LWIP_IP_CHANGED_SIG,
    LWIP_NETIF_CHANGED_SIG,
    LWIP_LINK_CHANGED_SIG,
    LWIP_HOST_NAME_FOUND_SIG,
    LWIP_SYSTEM_TIME_UPDATE_SIG,
    LWIP_PHY_INT_SIG,

    // Display signals.
    DISPLAY_TIMEOUT_SIG,
    DISPLAY_REFRESH_SIG,
    DISPLAY_TEXT_SIG,

    // Log event generation/sinks.
    LOG_EVENT_SIG,
    LOG_TIMER_SIG,

    // BLE signals.
    BLE_INTERRUPT_SIG,

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
