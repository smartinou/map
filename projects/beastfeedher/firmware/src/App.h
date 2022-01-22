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
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <memory>

// FatFS.
#include "ff.h"

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
    ~App();

    bool Init(std::shared_ptr<IBSPFactory> mFactory);

    static RTCC::AO::RTCC_AO *GetRTCCAO(void) {return sRTCC_AO.get();}
    static FATFS *GetFatFS(void) {return &sFatFS;}

private:
    static void NetInitCallback(void);

    // DB records.
    static CalendarRec *sCalendar;
    static NetIFRec    *sNetIFRec;
    static FeedCfgRec  *sFeedCfgRec;

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

    static FATFS sFatFS;
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
