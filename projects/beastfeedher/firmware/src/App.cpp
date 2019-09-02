// *****************************************************************************
//
// Project: PFPP
//
// Module: Application class.
//
// *****************************************************************************

//! \file
//! \brief Application class.
//! \ingroup application

// *****************************************************************************
//
//        Copyright (c) 2016-2019, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// QP Library.
#include "qpcpp.h"

// Standard Library.
#include <string.h>

// Common Library.
#include <db/DBRec.h>
#include <db/CalendarRec.h>
#include <db/FeedCfgRec.h>
#include <db/NetIFRec.h>

// LwIP stack.
#include "lwip/apps/httpd.h"
#include "lwip/stats.h"

// This project.
#include "App.h"
#include "BSP.h"
#include "DisplayMgr_AOs.h"
#include "FileLogSink_AO.h"
#include "IBSP.h"
#include "ILCD.h"
#include "IRTCC.h"
#include "LwIPMgr_AO.h"
#include "LwIPMgr_Evt.h"
#include "Net.h"
#include "PFPP_AOs.h"
#include "RTCC_AO.h"
#include "SDC.h"

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

CalendarRec *App::sCalendar   = nullptr;
NetIFRec    *App::sNetIFRec   = nullptr;
FeedCfgRec  *App::sFeedCfgRec = nullptr;

std::shared_ptr<RTCC::AO::RTCC_AO> App::mRTCC_AO;
std::shared_ptr<SDC> App::mSDCDrive0;

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

App::App(){
    // Ctor body left intentionally empty.
}


App::~App() {
    // Dtor body.
}


bool App::Init(void) {

    // Initialize the Board Support Package.
    mFactory = BSP::Init();

    // Create records and assign them to DB.
    sCalendar = new CalendarRec();
    sNetIFRec = new NetIFRec();
    sFeedCfgRec = new FeedCfgRec();

    // Create all AOs.
    // RTCC AO.
    App::mRTCC_AO = mFactory->CreateRTCCAO();
    App::mRTCC_AO->start(
        1U,
        mRTCCEventQueue,
        Q_DIM(mRTCCEventQueue),
        nullptr,
        0U
    );

    // Create SDC instance to use in FS stubs.
    mSDCDrive0 = mFactory->CreateSDC();
    if (mSDCDrive0.get() != nullptr) {
        // If supported, mount FS.
        // No use if there's no flash file storage.
        FRESULT lResult = f_mount(&mFatFS, "", 0);
        if (FR_OK != lResult) {
            return false;
        }
    }

    auto mFileLogSink_AO = mFactory->CreateLogFileSinkAO();
    if (mFileLogSink_AO.get() != nullptr) {
        mFileLogSink_AO->start(
            2U,
            mFileLogSinkEventQueue,
            Q_DIM(mFileLogSinkEventQueue),
            nullptr,
            0U
        );
    }

    auto lPFPPMgr_AO = mFactory->CreatePFPPAO(*App::sFeedCfgRec);
    lPFPPMgr_AO->start(
        3U,
        mPFPPMgrEventQueue,
        Q_DIM(mPFPPMgrEventQueue),
        nullptr,
        0U
    );

#if 0
    // Network makes sense in the following cases:
    // -if we use support web pages.
    // -For larger IoT support.
    // TODO: Check if Init event is required at all.
    LwIPInitEvt const lLwIPInitEvt(
        SIG_DUMMY,
        App::sNetIFRecPtr,
        App::HTTPInitCallback
    );

    mLwIPMgr_AO = new LwIPMgr_AO;
    mLwIPMgr_AO->start(
        4U,
        mLwIPEventQueue,
        Q_DIM(mLwIPEventQueue),
        nullptr,
        0U);// ,
        //&lLwIPInitEvt
    //);
#else
    auto lLwIPMgr_AO = mFactory->CreateLwIPMgrAO();
    if (lLwIPMgr_AO.get() != nullptr) {
        lLwIPMgr_AO->start(
            4U,
            mLwIPEventQueue,
            Q_DIM(mLwIPEventQueue),
            nullptr,
            0U
        );
    }
#endif

    auto lDisplayMgr_AO = mFactory->CreateDisplayMgrAO();
    lDisplayMgr_AO->start(
        5U,
        mDisplayMgrEventQueue,
        Q_DIM(mDisplayMgrEventQueue),
        nullptr,
        0U
    );

    // Send signal dictionaries for globally published events...
    //QS_SIG_DICTIONARY(SIG_TIME_TICK, static_cast<void *>(0));

    // Send object dictionaries for event queues...
    QS_OBJ_DICTIONARY(mRTCCEventQueue);
    QS_OBJ_DICTIONARY(mPFPPMgrEventQueue);
    QS_OBJ_DICTIONARY(mLwIPEventQueue);
    QS_OBJ_DICTIONARY(mDisplayMgrEventQueue);
    QS_OBJ_DICTIONARY(mFileLogSinkEventQueue);

    return true;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

void App::NetInitCallback(void) {
    Net::InitCallback(App::mRTCC_AO, App::sCalendar, App::sFeedCfgRec);
}


extern "C" {

//
// File system stubs.
//

DSTATUS disk_initialize(BYTE pdrv) {
    // Only drive 0 is supported in this application.
    if (0 == pdrv) {
        return App::GetSDCDrive()->DiskInit();
    } else {
        return RES_PARERR;
    }
}


DSTATUS disk_status(BYTE pdrv) {
    // Only drive 0 is supported in this application.
    if (0 == pdrv) {
        return App::GetSDCDrive()->GetDiskStatus();
    } else {
        return RES_PARERR;
    }
}


DRESULT disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count) {
    // Only drive 0 is supported in this application.
    if (0 == pdrv) {
        return App::GetSDCDrive()->DiskRd(buff, sector, count);
    } else {
        return RES_PARERR;
    }
}


#if (FF_FS_READONLY == 0)
DRESULT disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count) {
    // Only drive 0 is supported in this application.
    if (0 == pdrv) {
        return App::GetSDCDrive()->DiskWr(buff, sector, count);
    } else {
        return RES_PARERR;
    }
}
#endif // FF_FS_READONLY


#if (FF_FS_READONLY == 0)
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    return (DRESULT)0;
}
#endif // FF_FS_READONLY


#if !FF_FS_READONLY && !FF_FS_NORTC
DWORD get_fattime(void) {
    return 0;
}
#endif

} // extern C

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
