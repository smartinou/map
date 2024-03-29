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

CalendarRec *App::sCalendar   = nullptr;
NetIFRec    *App::sNetIFRec   = nullptr;
FeedCfgRec  *App::sFeedCfgRec = nullptr;

std::shared_ptr<RTCC::AO::RTCC_AO> App::sRTCC_AO;
FATFS App::sFatFS{0};

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

App::App() {
    // Ctor body left intentionally empty.
}


App::~App() {
    // Dtor body.
}


bool App::Init(std::shared_ptr<IBSPFactory> aFactory) {

    // Create records and assign them to DB.
    sCalendar = new CalendarRec();
    sNetIFRec = new NetIFRec();
    sFeedCfgRec = new FeedCfgRec();

    // Create all AOs.
    // RTCC AO + FileLogSink.
    App::sRTCC_AO = aFactory->CreateRTCCAO();
    if (App::sRTCC_AO != nullptr) {
        // Create SDC instance to use in FS stubs.
        // WARNING: if SD card disks exist, they should be mounted before calling
        // any other device triggering SPI bus activity.
        // This can be handled in different order when SDCard socket is
        // powered via PIO, but it's not guaranteed the case on all targets.
        // See http://elm-chan.org/docs/mmc/mmc_e.html
        // Section: Consideration on Multi-slave Configuration
        unsigned int lDiskQty = aFactory->CreateDisks();
        if (0 != lDiskQty) {
            // Disks found: mount the default drive.
            static constexpr auto sDefaultDiskIndex = 0;
            FRESULT lResult = FatFSDisk::StaticMountDisk(sDefaultDiskIndex, &sFatFS);
            if (FR_OK == lResult) {
                // FS mounted on default disk: add log sink.
                std::shared_ptr<QP::QActive> mFileLogSink_AO = aFactory->CreateLogFileSinkAO();
                if (mFileLogSink_AO != nullptr) {
                    reinterpret_cast<Logging::AO::FileSink_AO * const>(
                        mFileLogSink_AO.get())->SetSyncLogLevel(LogLevel::prio::INFO);

                    std::unordered_set<std::string> lCategories = {"PFPP"};
                    Logging::Event::Init lFileLogInitEvent(DUMMY_SIG, lCategories);
                    mFileLogSink_AO->start(
                        2U,
                        mFileLogSinkEventQueue,
                        Q_DIM(mFileLogSinkEventQueue),
                        nullptr,
                        0U,
                        &lFileLogInitEvent
                    );
                }
            }
        }

        // Now that disks are mounted, start the RTCC.
        RTCC::Event::Init lRTCCInitEvent(DUMMY_SIG, sCalendar);
        App::sRTCC_AO->start(
            1U,
            mRTCCEventQueue,
            Q_DIM(mRTCCEventQueue),
            nullptr,
            0U,
            &lRTCCInitEvent
        );
    } else {
        // This application has no meaning w/o RTCC.
        return false;
    }

    auto lPFPPMgr_AO = aFactory->CreatePFPPAO(*App::sFeedCfgRec);
    if (lPFPPMgr_AO != nullptr) {
        lPFPPMgr_AO->start(
            3U,
            mPFPPMgrEventQueue,
            Q_DIM(mPFPPMgrEventQueue),
            nullptr,
            0U
        );
    } else {
        // This application has no meaning w/o PFPP Manager.
        return false;
    }


    // Network makes sense in the following cases:
    // -if we use support web pages.
    // -For larger IoT support.
    auto lLwIPMgr_AO = aFactory->CreateLwIPMgrAO();
    if (lLwIPMgr_AO != nullptr) {
        LwIP::Event::Init lLwIPInitEvent(DUMMY_SIG, sNetIFRec, NetInitCallback);
        lLwIPMgr_AO->start(
            4U,
            mLwIPEventQueue,
            Q_DIM(mLwIPEventQueue),
            nullptr,
            0U,
            &lLwIPInitEvent
        );
    }


#if 0
    std::shared_ptr<QP::QActive> lDisplayMgr_AO = aFactory->CreateDisplayMgrAO();
    if (lDisplayMgr_AO != nullptr) {
        lDisplayMgr_AO->start(
            5U,
            mDisplayMgrEventQueue,
            Q_DIM(mDisplayMgrEventQueue),
            nullptr,
            0U
        );
    }


    std::shared_ptr<QP::QActive> lBLEMgr_AO = aFactory->CreateBLEAO();
    if (lBLEMgr_AO != nullptr) {
        PFPP::Event::BLE::Init lBLEInitEvent(
            DUMMY_SIG,
            App::sRTCC_AO,
            sCalendar,
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
    Net::InitCallback(App::sRTCC_AO, App::sCalendar, App::sNetIFRec, App::sFeedCfgRec);
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
