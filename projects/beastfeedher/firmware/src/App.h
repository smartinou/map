#pragma once
// *******************************************************************************
//
// Project: Beast Feed'Her.
//
// Module: Application class.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2016-2018, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

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
class RTCC_AO;
class LwIPMgr_AO;


//! \brief Brief description.
//! Details follow...
//! ...here.
class App {
 public:
  App();
  ~App();

  bool Init(void);

 private:
  static void NetCallbackInit(void);

#if LWIP_HTTPD_SSI
  static uint16_t SSIHandler(int   aTagIx,
                             char *aInsertPtr,
                             int   aInsertStrLen);
  static int SSIRadioButtonHandler(int                aTagIx,
                                   char       * const aInsertStr,
                                   int                aInsertStrLen,
                                   char const * const aNameValStr,
                                   bool               aIsChecked);
  static int SSICalendarHandler(int          aTagIx,
                                char * const aInsertStr,
                                int          aInsertStrLen,
                                unsigned int aHour);
  static int SSINetworkHandler(int                aTagIx,
                               char       * const aInsertStr,
                               int                aInsertStrLen,
                               char const * const aTagNameStr);
  static int SSIStatsHandler(int          aTagIx,
                             char * const aInsertStr,
                             int          aInsertStrLen);

  static char const *FindTagVal(char  const *aTagNameStr,
                                int          aParamsQty,
                                char * const aParamsVec[],
                                char * const aValsVec[]);
#endif // LWIP_HTTPD_SSI

#if LWIP_HTTPD_CGI
static char const *DispIndex(int   aIx,
                             int   aParamsQty,
                             char *aParamsVec[],
                             char *aValsVec[]);
static char const *DispCfg(int   aIx,
                           int   aParamsQty,
                           char *aParamsVec[],
                           char *aValsVec[]);
#endif // LWIP_HTTPD_CGI

  // DB records.
  static CalendarRec *sCalendarPtr;
  static NetIFRec    *sNetIFRecPtr;
  static FeedCfgRec  *sFeedCfgRecPtr;

  // QP Event Queues.
  QP::QEvt const *mRTCCEvtQPtr[10];
  QP::QEvt const *mBeastMgrEvtQPtr[5];
  QP::QEvt const *mFileLogSinkEvtQPtr[10];
  QP::QEvt const *mLwIPEvtQPtr[10];
  QP::QEvt const *mDisplayMgrEvtQPtr[5];

  // QP AOs.
  BFHMgr_AO         &mBFHMgr_AO;
  FileLogSink_AO    *mFileLogSink_AO;
  LwIPMgr_AO        *mLwIPMgr_AO;
  DisplayMgr_AO     *mDisplayMgr_AO;
  static RTCC_AO    *sRTCC_AOPtr;

#if LWIP_HTTPD_SSI
  static char const *sSSITags[];
#endif // LWIP_HTTPD_SSI

#if LWIP_HTTPD_CGI
  static tCGI const sCGIEntries[];
#endif // LWIP_HTTPD_CGI
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
