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
#include "FWVersionGenerated.h"
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

IRTCC *App::mRTCC = nullptr;
SDC *App::mSDCDrive0 = nullptr;

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

App::App(){
    // Ctor body left intentionally empty.
}


App::~App() {
    // Dtor body.
    delete(mDisplayMgr_AO);
    delete(mDisplay);

    delete(mPFPPMgr_AO);
    delete(mMotorControl);

    delete(mSDCDrive0);

    delete(mRTCC_AO);
    delete(mRTCC);

    delete(mSPIDev);
}


bool App::Init(void) {

    // Initialize the Board Support Package.
    IBSPFactory *const lFactory = BSP::Init();

    // Create records and assign them to DB.
    sCalendar = new CalendarRec();
    sNetIFRec = new NetIFRec();
    sFeedCfgRec = new FeedCfgRec();

    // SPI Device.
    mSPIDev = lFactory->CreateSPIDev();

    // Create all other AOs.
    // RTCC & matching AO.
    App::mRTCC = lFactory->CreateRTCC(*mSPIDev);

    mRTCC_AO = new RTCC::AO::RTCC_AO(*App::mRTCC);
    mRTCC_AO->start(
        1U,
        mRTCCEventQueue,
        Q_DIM(mRTCCEventQueue),
        nullptr,
        0U
    );

    // Create SDC instance to use in FS stubs.
    mSDCDrive0 = lFactory->CreateSDC(*mSPIDev);

    // If supported, mount FS.
    FRESULT lResult = f_mount(&mFatFS, "", 0);
    if (FR_OK != lResult) {
        return false;
    }

#if 0
  mFileLogSink_AO = new FileLogSink_AO();
  mFileLogSink_AO->start(
    2U,
    mFileLogSinkEventQueue,
    Q_DIM(mFileLogSinkEventQueue),
    nullptr,
    0U);

#else
    mMotorControl = lFactory->CreateMotorControl();
    mPFPPMgr_AO = new PFPP::AO::Mgr_AO(*App::sFeedCfgRec, *mMotorControl);
    mPFPPMgr_AO->start(
        3U,
        mPFPPMgrEventQueue,
        Q_DIM(mPFPPMgrEventQueue),
        nullptr,
        0U
    );
#endif

#if 0
  // Network makes sense in the following cases:
  // -if we use support web pages.
  // -For larger IoT support.
  LwIPInitEvt const lLwIPInitEvt(
    SIG_DUMMY,
    App::sNetIFRecPtr,
    Net::NetCallbackInit);
  mLwIPMgr_AO = new LwIPMgr_AO;
  mLwIPMgr_AO->start(
    4U,
    mLwIPEventQueue,
    Q_DIM(mLwIPEventQueue),
    nullptr,
    0U,
    &lLwIPInitEvt);


  SSD1329 * const lOLEDDisplayPtr = BSP_InitOLEDDisplay();
  DisplayMgrInitEvt const lDisplayMgrInitEvt(SIG_DUMMY, 5);
  mDisplayMgr_AO = new DisplayMgr_AO(*lOLEDDisplayPtr);
#else
    mDisplay = lFactory->CreateDisplay(*mSPIDev);
    mDisplayMgr_AO = new Display::AO::Mgr_AO(*mDisplay);//, 5);
#endif
    mDisplayMgr_AO->start(
        5U,
        mDisplayMgrEventQueue,
        Q_DIM(mDisplayMgrEventQueue),
        nullptr,
        0U
    );
    //&lDisplayMgrInitEvt);
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
