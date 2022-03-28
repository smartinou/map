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
//        Copyright (c) 2016-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
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

#ifdef NET_SUPPORT
// LwIP stack.
#include "lwip/apps/httpd.h"
#include "lwip/stats.h"
#endif // NET_SUPPORT

// This project.
#include "App.h"
#include "BLE_Events.h"
#include "BSP.h"
#include "FatFSDisk.h"
#include "Logging_AOs.h"
#include "Logging_Events.h"
#include "IBSP.h"
#include "LwIP_Events.h"
#include "Net.h"
#include "RTCC_AOs.h"
#include "RTCC_Events.h"
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

std::shared_ptr<CalendarRec> App::sCalendarRec = nullptr;
std::shared_ptr<NetIFRec> App::sNetIFRec = nullptr;
std::shared_ptr<FeedCfgRec> App::sFeedCfgRec = nullptr;

std::shared_ptr<RTCC::AO::RTCC_AO> App::sRTCC_AO;

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

bool App::Init(std::shared_ptr<IBSPFactory> aFactory) {

    // Create records and assign them to DB.
    sCalendarRec = CalendarRec::Create();
    sNetIFRec = NetIFRec::Create();
    sFeedCfgRec = FeedCfgRec::Create();

    // Create all AOs.
    // RTCC AO + FileLogSink.
    bool const lIsFSMounted = aFactory->MountFS();
    if (lIsFSMounted) {
        aFactory->StartFileSinkAO(
            2U,
            mFileLogSinkEventQueue,
            Q_DIM(mFileLogSinkEventQueue)
        );
    }

    // Now that disks are mounted, start the RTCC.
    RTCC::Event::Init const lRTCCInitEvent(DUMMY_SIG, sCalendarRec.get());
    App::sRTCC_AO = aFactory->StartRTCCAO(
        1U,
        mRTCCEventQueue,
        Q_DIM(mRTCCEventQueue),
        &lRTCCInitEvent
    );
    if (App::sRTCC_AO == nullptr) {
        return false;
    }

    bool const lRes = aFactory->StartPFPPAO(
        *sFeedCfgRec,
        3U,
        mPFPPMgrEventQueue,
        Q_DIM(mPFPPMgrEventQueue)
    );
    if (lRes == false) {
        return false;
    }

    LwIP::Event::Init const lLwIPInitEvent(DUMMY_SIG, sNetIFRec.get(), NetInitCallback);
    aFactory->StartLwIPMgrAO(
        4U,
        mLwIPEventQueue,
        Q_DIM(mLwIPEventQueue),
        &lLwIPInitEvent
    );


#if 0
    aFactory->StartDisplayMgrAO(
        5U,
        mDisplayMgrEventQueue,
        Q_DIM(mDisplayMgrEventQueue),
    );

    std::shared_ptr<QP::QActive> lBLEMgr_AO = aFactory->CreateBLEAO();
    if (lBLEMgr_AO != nullptr) {
        PFPP::Event::BLE::Init lBLEInitEvent(
            DUMMY_SIG,
            App::sRTCC_AO,
            sCalendarRec,
            sNetIFRec,
            sFeedCfgRec
        );
        lBLEMgr_AO->start(
            6U,
            mBLEMgrEventQueue,
            Q_DIM(mBLEMgrEventQueue),
            nullptr,
            0U,
            &lBLEInitEvent
        );
    }
#endif

    // Send signal dictionaries for globally published events...
    //QS_SIG_DICTIONARY(SIG_TIME_TICK, static_cast<void *>(0));

    // Send object dictionaries for event queues...
    QS_OBJ_DICTIONARY(mRTCCEventQueue);
    QS_OBJ_DICTIONARY(mPFPPMgrEventQueue);
    QS_OBJ_DICTIONARY(mLwIPEventQueue);
    QS_OBJ_DICTIONARY(mDisplayMgrEventQueue);
    QS_OBJ_DICTIONARY(mFileLogSinkEventQueue);
    QS_OBJ_DICTIONARY(mBLEMgrEventQueue);

    return true;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

void App::NetInitCallback(void) {
#if LWIP_HTTPD_SSI || LWIP_HTTPD_CGI
    Net::InitCallback(
        App::sRTCC_AO,
        App::sCalendarRec,
        App::sNetIFRec,
        App::sFeedCfgRec
    );
#endif
}


extern "C" {

//
// File system stubs.
// Media Access Inferface functions are a 1:1 mapping with the static functions 
// from FatFSDisk class.
// Perform any parameter casting if required.
// Consider moving this to BSP.cpp file.
//

DSTATUS disk_initialize(BYTE aDriveIndex) {
    return FatFSDisk::StaticInitDisk(aDriveIndex);
}


DSTATUS disk_status(BYTE aDriveIndex) {
    return FatFSDisk::StaticGetDiskStatus(aDriveIndex);
}


DRESULT disk_read(BYTE aDriveIndex, BYTE *aBuffer, DWORD aSectorStart, UINT aCount) {
    return FatFSDisk::StaticRdDisk(aDriveIndex, aBuffer, aSectorStart, aCount);
}


#if (FF_FS_READONLY == 0)
DRESULT disk_write(BYTE aDriveIndex, const BYTE *aBuffer, DWORD aSectorStart, UINT aCount) {
    return FatFSDisk::StaticWrDisk(aDriveIndex, aBuffer, aSectorStart, aCount);
}
#endif // FF_FS_READONLY


#if (FF_FS_READONLY == 0) || (FF_MAX_SS == FF_MIN_SS)
DRESULT disk_ioctl(BYTE aDriveIndex, BYTE aCmd, void *aBuffer) {
    return FatFSDisk::StaticIOCTL(aDriveIndex, aCmd, aBuffer);
}
#endif // FF_FS_READONLY


#if (FF_FS_READONLY == 0) && (FF_FS_NORTC == 0)
DWORD get_fattime(void) {

    static unsigned int constexpr sBaseYear = 1980;
    static unsigned int constexpr sFieldWidth = 5;
    Date const &lDate = App::GetRTCCAO()->GetDate();
    uint32_t lFatTime = (lDate.GetYear() - sBaseYear) & 0x3F;
    lFatTime <<= sFieldWidth;
    lFatTime |= (lDate.GetMonth() & 0x1F);
    lFatTime <<= sFieldWidth;
    lFatTime |= (lDate.GetDate() & 0x1F);
    lFatTime <<= sFieldWidth;

    Time const &lTime = App::GetRTCCAO()->GetTime();
    lFatTime |= (lTime.GetHours() & 0x1F);
    lFatTime <<= sFieldWidth;
    lFatTime |= (lTime.GetMinutes() & 0x1F);
    lFatTime <<= sFieldWidth;
    lFatTime |= (lTime.GetSeconds() / 2);
    return lFatTime;
}
#endif // FF_FS_READONLY

} // extern C

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
