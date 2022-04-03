#pragma once
// *******************************************************************************
//
// Project: PFPP.
//
// Module: BSP.
//
// *******************************************************************************

//! \file
//! \brief BSP Factory class.
//! \ingroup application_bsp

// ******************************************************************************
//
//        Copyright (c) 2019-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <memory>

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// Forward declaration.
namespace QP {
    class QActive;
}


// Consider namespace App or BSP.

class IBSPFactory {
public:

    virtual std::shared_ptr<QP::QActive> CreateBLEAO(void) = 0;

    virtual std::shared_ptr<RTCC::AO::RTCC_AO> StartRTCCAO(
        const std::shared_ptr<CalendarRec> &aCalendarRec,
        uint8_t const aPrio,
        QP::QEvt const * aQSto[],
        uint32_t const aQLen
    ) = 0;

    virtual bool MountFS(void) = 0;
    virtual bool StartFileSinkAO(
        uint8_t const aPrio,
        QP::QEvt const * aQSto[],
        uint32_t const aQLen
    ) = 0;

    virtual bool StartPFPPAO(
        const std::shared_ptr<FeedCfgRec> &aFeedCfgRec,
        uint8_t const aPrio,
        QP::QEvt const * aQSto[],
        uint32_t const aQLen
    ) = 0;

    virtual bool StartLwIPMgrAO(
        uint8_t const aPrio,
        QP::QEvt const * aQSto[],
        uint32_t const aQLen,
        QP::QEvt const * const aInitEvt
    ) = 0;

    virtual bool StartDisplayMgrAO(
        uint8_t const aPrio,
        QP::QEvt const * aQSto[],
        uint32_t const aQLen
    ) = 0;
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
