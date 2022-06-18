#pragma once
// *******************************************************************************
//
// Project: PFPP
//
// Module: Application class.
//
// *****************************************************************************

//! \file
//! \brief Application class.
//! \ingroup application

// ******************************************************************************
//
//        Copyright (c) 2016-2022, Martin Garon, All rights reserved.
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
class CalendarRec;
class NetIFRec;
class FeedCfgRec;
class IBSPFactory;

namespace RTCC {
    namespace AO {
        class RTCC_AO;
    }
}


class App {
public:
    App();
    ~App() = default;

    bool Init(std::shared_ptr<IBSPFactory> mFactory);

    static std::shared_ptr<RTCC::AO::RTCC_AO> GetRTCCAO(void) {return sRTCC_AO;}

private:

    static void NetInitCallback(void * const aParam);

    // DB records.
    std::shared_ptr<CalendarRec> const mCalendarRec;
    std::shared_ptr<NetIFRec> const mNetIFRec;
    std::shared_ptr<FeedCfgRec> const mFeedCfgRec;

    // QP Event Queues.
    static size_t constexpr sSmallQueueSize = 5;
    static size_t constexpr sLargeQueueSize = 10;
    QP::QEvt const *mRTCCEventQueue[sLargeQueueSize] = {nullptr};
    QP::QEvt const *mPFPPMgrEventQueue[sSmallQueueSize] = {nullptr};
    QP::QEvt const *mFileLogSinkEventQueue[sLargeQueueSize] = {nullptr};
    QP::QEvt const *mLwIPEventQueue[sLargeQueueSize] = {nullptr};
    QP::QEvt const *mDisplayMgrEventQueue[sSmallQueueSize] = {nullptr};
    QP::QEvt const *mBLEMgrEventQueue[sSmallQueueSize] = {nullptr};

    static std::shared_ptr<RTCC::AO::RTCC_AO> sRTCC_AO;
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
