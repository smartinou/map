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
//        Copyright (c) 2016-2019, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <memory>

// FatFS.
#include "diskio.h"
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
class LwIPMgr_AO;
class IBSPFactory;
class SDC;

namespace RTCC {
    namespace AO {
        class RTCC_AO;
    }
}


namespace QP {
    class QActive;
}


//! \brief Brief description.
//! Details follow...
//! ...here.
class App {
public:
    App();
    ~App();

    bool Init(void);

    static SDC *GetSDCDrive(void) { return mSDCDrive0.get(); }

private:
    static void NetInitCallback(void);

    // DB records.
    static CalendarRec *sCalendar;
    static NetIFRec    *sNetIFRec;
    static FeedCfgRec  *sFeedCfgRec;

    // QP Event Queues.
    QP::QEvt const *mRTCCEventQueue[10] = {nullptr};
    QP::QEvt const *mPFPPMgrEventQueue[5] = {nullptr};
    QP::QEvt const *mFileLogSinkEventQueue[10] = {nullptr};
    QP::QEvt const *mLwIPEventQueue[10] = {nullptr};
    QP::QEvt const *mDisplayMgrEventQueue[5] = {nullptr};

    // When this object gets out of scope,
    // the Factory is destroyed and all that is responsible as well.
    std::unique_ptr<IBSPFactory> mFactory;

    static std::shared_ptr<RTCC::AO::RTCC_AO> mRTCC_AO;
    static std::shared_ptr<SDC> mSDCDrive0;

    // QP AOs.
    LwIPMgr_AO *mLwIPMgr_AO = nullptr;

    FATFS mFatFS = {0};
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
