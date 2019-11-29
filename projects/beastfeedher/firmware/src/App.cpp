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
#include "Display_AOs.h"
#include "FatFSDisk.h"
#include "Logging_AOs.h"
#include "IBSP.h"
#include "ILCD.h"
#include "IRTCC.h"
#include "LwIP_AOs.h"
#include "LwIP_Events.h"
#include "Net.h"
#include "PFPP_AOs.h"
#include "RTCC_AOs.h"
#include "RTCC_Events.h"
#include "SDC.h"
#include "Signals.h"

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
    RTCC::Event::Init lRTCCInitEvent(DUMMY_SIG, sCalendar);
    App::mRTCC_AO->start(
        1U,
        mRTCCEventQueue,
        Q_DIM(mRTCCEventQueue),
        nullptr,
        0U,
        &lRTCCInitEvent
    );

    // Create SDC instance to use in FS stubs.
    // TODO: RETURN BOOL INSTEAD OF ACTUAL POINTER: IT'S OF NO USE.
    // TODO: CHANGE METHOD NAME TO CreateDisks();
    mSDCDrive0 = mFactory->CreateSDC();
    if (mSDCDrive0.get() != nullptr) {
        // If supported, mount the default drive.
        // No use if there's no flash file storage.
        FRESULT lResult = f_mount(&mFatFS, "", 0);
        if (FR_OK == lResult) {
#if 1
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
#endif
        }
    }


    auto lPFPPMgr_AO = mFactory->CreatePFPPAO(*App::sFeedCfgRec);
    lPFPPMgr_AO->start(
        3U,
        mPFPPMgrEventQueue,
        Q_DIM(mPFPPMgrEventQueue),
        nullptr,
        0U
    );


    // Network makes sense in the following cases:
    // -if we use support web pages.
    // -For larger IoT support.
    LwIP::Event::Init lLwIPInitEvent(DUMMY_SIG, sNetIFRec, NetInitCallback);
    auto lLwIPMgr_AO = mFactory->CreateLwIPMgrAO();
    if (lLwIPMgr_AO.get() != nullptr) {
        lLwIPMgr_AO->start(
            4U,
            mLwIPEventQueue,
            Q_DIM(mLwIPEventQueue),
            nullptr,
            0U,
            &lLwIPInitEvent
        );
    }


    std::shared_ptr<QP::QActive> lDisplayMgr_AO = mFactory->CreateDisplayMgrAO();
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
// Media Access Inferface functions are a 1:1 mapping with the static functions 
// from FatFSDisk class.
// Perform any parameter casting if required.
//

DSTATUS disk_initialize(BYTE aDriveIndex) {
    return FatFSDisk::StaticInitDisk(aDriveIndex);
}


DSTATUS disk_status(BYTE aDriveIndex) {
    return FatFSDisk::StaticGetDiskStatus(aDriveIndex);
}


DRESULT disk_read(BYTE aDriveIndex, BYTE* aBuffer, DWORD aSectorStart, UINT aCount) {
    return FatFSDisk::StaticRdDisk(aDriveIndex, aBuffer, aSectorStart, aCount);
}


#if (FF_FS_READONLY == 0)
DRESULT disk_write(BYTE aDriveIndex, const BYTE* aBuffer, DWORD aSectorStart, UINT aCount) {
    return FatFSDisk::StaticWrDisk(aDriveIndex, aBuffer, aSectorStart, aCount);
}
#endif // FF_FS_READONLY


#if (FF_FS_READONLY == 0) || (FF_MAX_SS == FF_MIN_SS)
DRESULT disk_ioctl(BYTE aDriveIndex, BYTE aCmd, void *aBuffer) {
    return FatFSDisk::StaticIOCTL(aDriveIndex, aCmd, aBuffer);
}
#endif // FF_FS_READONLY


// TODO: UPDATE AS PER SPECS.
#if !FF_FS_READONLY && !FF_FS_NORTC
DWORD get_fattime(void) {
    return 0;
}
#endif // FF_FS_READONLY

} // extern C

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
