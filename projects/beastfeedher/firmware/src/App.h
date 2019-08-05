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
class BFHMgr_AO;
class DisplayMgr_AO;
class FileLogSink_AO;
class LwIPMgr_AO;
class ILCD;
class IMotorControl;
class IRTCC;
class RTCC_AO;
class GPIOs;
class SDC;

namespace RTCC {
    namespace AO {
        class RTCC_AO;
    } // namespace AO
} // namespace RTCC


namespace CoreLink {
    class SPIDev;
    class SPISlaveCfg;
}


namespace RTCC {
    namespace AO {
        class RTCC_AO;
    }
}


namespace PFPP {
    namespace AO {
      class Mgr_AO;
    }
}

namespace Display {
    namespace AO {
        class Mgr_AO;
    }
}


//! \brief Brief description.
//! Details follow...
//! ...here.
class App {
public:
    App();
    ~App();

    bool Init(void);

    static SDC *GetSDCDrive(void) { return mSDCDrive0; }

private:
    static void NetCallbackInit(void);

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

    CoreLink::SPIDev *mSPIDev = nullptr;

    static IRTCC *mRTCC;
    RTCC::AO::RTCC_AO *mRTCC_AO = nullptr;

    static SDC *mSDCDrive0;
    CoreLink::SPISlaveCfg *mSDCSlaveCfg = nullptr;
    FileLogSink_AO *mFileLogSink_AO = nullptr;

    IMotorControl *mMotorControl = nullptr;
    PFPP::AO::Mgr_AO *mPFPPMgr_AO = nullptr;

    ILCD *mDisplay = nullptr;
    Display::AO::Mgr_AO *mDisplayMgr_AO = nullptr;

    // QP AOs.
    LwIPMgr_AO     *mLwIPMgr_AO = nullptr;

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
