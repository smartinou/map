// *****************************************************************************
//
// Project: Beast Feed'Her
//
// Module: Master record class.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// *****************************************************************************
//
//        Copyright (c) 2016-2018, Martin Garon, All rights reserved.
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
#include "DBRec.h"

// LwIP stack.
#include "lwip/apps/httpd.h"
#include "lwip/stats.h"

// This project.
#include "BFH_Mgr_AO.h"
#include "BFH_Mgr_Evt.h"
#include "BSP.h"
#include "CalendarRec.h"
#include "FeedCfgRec.h"
#include "FWVersionGenerated.h"
#include "LwIPMgr_AO.h"
#include "LwIPMgr_Evt.h"
#include "MasterRec.h"
#include "NetIFRec.h"
#include "RTCC_AO.h"
#include "RTCC_Evt.h"

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

enum {
  SSI_TAG_IX_ZERO = 0,
  SSI_TAG_IX_EMPTY,

  // Info.
  SSI_TAG_IX_INFO_FW_VERSION,
  SSI_TAG_IX_INFO_BUILD_DATE,
  SSI_TAG_IX_INFO_BUILD_TIME,
  SSI_TAG_IX_INFO_GIT_HASH,
  SSI_TAG_IX_INFO_DB_STATUS,
  SSI_TAG_IX_INFO_RTCC_TEMP,

  // Global: Time and Date.
  SSI_TAG_IX_GLOBAL_DATE,
  SSI_TAG_IX_GLOBAL_TIME,

  // Configuration.
  SSI_TAG_IX_CFG_PAD_ENABLE,
  SSI_TAG_IX_CFG_PAD_DISABLE,
  SSI_TAG_IX_CFG_MANUAL_ENABLE,
  SSI_TAG_IX_CFG_MANUAL_DISABLE,
  SSI_TAG_IX_CFG_FEED_TIME,

  // Calendar.
  SSI_TAG_IX_CFG_CALENDAR_06_00,
  SSI_TAG_IX_CFG_CALENDAR_07_00,
  SSI_TAG_IX_CFG_CALENDAR_08_00,
  SSI_TAG_IX_CFG_CALENDAR_09_00,
  SSI_TAG_IX_CFG_CALENDAR_10_00,
  SSI_TAG_IX_CFG_CALENDAR_11_00,
  SSI_TAG_IX_CFG_CALENDAR_12_00,
  SSI_TAG_IX_CFG_CALENDAR_13_00,
  SSI_TAG_IX_CFG_CALENDAR_14_00,
  SSI_TAG_IX_CFG_CALENDAR_15_00,
  SSI_TAG_IX_CFG_CALENDAR_16_00,
  SSI_TAG_IX_CFG_CALENDAR_17_00,
  SSI_TAG_IX_CFG_CALENDAR_18_00,
  SSI_TAG_IX_CFG_CALENDAR_19_00,
  SSI_TAG_IX_CFG_CALENDAR_20_00,
  SSI_TAG_IX_CFG_CALENDAR_21_00,

  // Network statistics.
  SSI_TAG_IX_STATS_TX,
  SSI_TAG_IX_STATS_RX,
  SSI_TAG_IX_STATS_FW,
  SSI_TAG_IX_STATS_DROP,
  SSI_TAG_IX_STATS_CHK_ERR,
  SSI_TAG_IX_STATS_LEN_ERR,
  SSI_TAG_IX_STATS_MEM_ERR,
  SSI_TAG_IX_STATS_RT_ERR,
  SSI_TAG_IX_STATS_PRO_ERR,
  SSI_TAG_IX_STATS_OPT_ERR,
  SSI_TAG_IX_STATS_ERR,

  // Network interface.
  SSI_TAG_IX_LAST = SSI_TAG_IX_STATS_ERR,
  SSI_TAG_IX_STATS_QTY
};

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

CalendarRec *MasterRec::sCalendarPtr   = nullptr;
NetIFRec    *MasterRec::sNetIFRecPtr   = nullptr;
FeedCfgRec  *MasterRec::sFeedCfgRecPtr = nullptr;

RTCC_AO     *MasterRec::sRTCC_AOPtr    = nullptr;
LwIPMgr_AO  *MasterRec::sLwIPMgr_AOPtr = nullptr;


#if LWIP_HTTPD_SSI
// Server-Side Include (SSI) demo.
char const *MasterRec::sSSITags[] = {
  // Common/misc. tags.
  "_zero",    // 0
  "_empty",   // 1

  // Info.
  "i_ver",
  "i_date",
  "i_time",
  "i_hash",
  "i_status",
  "i_temp",

  // Global: Time and Date.
  "g_date",
  "g_time",

  // Configuration.
  "c_pad_en",
  "c_pad_di",
  "c_but_en",
  "c_but_di",
  "c_time",
  "c_cal_06",
  "c_cal_07",
  "c_cal_08",
  "c_cal_09",
  "c_cal_10",
  "c_cal_11",
  "c_cal_12",
  "c_cal_13",
  "c_cal_14",
  "c_cal_15",
  "c_cal_16",
  "c_cal_17",
  "c_cal_18",
  "c_cal_19",
  "c_cal_20",
  "c_cal_21",

  // Network statistics.
  "s_xmit",
  "s_recv",
  "s_fw",
  "s_drop",
  "s_chkerr",
  "s_lenerr",
  "s_memerr",
  "s_rterr",
  "s_proerr",
  "s_opterr",
  "s_err",
};
#endif //LWIP_HTTPD_SSI


#if LWIP_HTTPD_CGI
tCGI const MasterRec::sCGIEntries[] = {
  {"/index.cgi",  DispIndex}
};
#endif // LWIP_HTTPD_CGI

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

MasterRec::MasterRec()
  : DBRec()
  , mMasterRec{0}
  , mRecQty(0)
  , mRecIx(0)
  , mDBRec{nullptr} {

  // Ctor body left intentionally empty.
}


MasterRec::~MasterRec() {

  // Dtor body left intentionally empty.
}


bool MasterRec::Init(void) {

  // Initialize the Board Support Package.
  CoreLink::SPIDev *lSPIDevPtr = BSPInit();

  // Create sub-records and assign them to master record.
  // Deserialize NV memory into it.
  // Sanity of records is checked once deserialized.
  mRecQty = 3;
  mDBRec = new DBRec *[mRecQty];
  sCalendarPtr = new CalendarRec();
  AddRec(sCalendarPtr);

  sNetIFRecPtr = new NetIFRec();
  AddRec(sNetIFRecPtr);

  sFeedCfgRecPtr = new FeedCfgRec();
  AddRec(sFeedCfgRecPtr);

  unsigned long lIRQGPIOPort = IRQGPIOPortGet();
  unsigned long lIntNbr = BSPGPIOPortToInt(lIRQGPIOPort);
  static RTCCInitEvt const sRTCCInitEvt = { SIG_DUMMY,
                                            *lSPIDevPtr,
                                            CSnGPIOPortGet(),
                                            CSnGPIOPinGet(),
                                            lIRQGPIOPort,
                                            IRQGPIOPinGet(),
                                            lIntNbr,
                                            this,
                                            MasterRec::sCalendarPtr };
  static QP::QEvt const *sRTCCEvtQPtr[10];
  MasterRec::sRTCC_AOPtr = new RTCC_AO();
  MasterRec::sRTCC_AOPtr->start(1U,
                                sRTCCEvtQPtr,
                                Q_DIM(sRTCCEvtQPtr),
                                nullptr,
                                0U,
                                &sRTCCInitEvt);

  // DB records are now deserialized, and fixed if required.
  // Create all other AOs.
  static BFHInitEvt const sBFHInitEvt = { SIG_DUMMY,
                                          MasterRec::sFeedCfgRecPtr };
  static QP::QEvt const *sBeastMgrEvtQPtr[5];
  BFH_Mgr_AO &lBFH_Mgr_AO = BFH_Mgr_AO::Instance();
  lBFH_Mgr_AO.start(2U,
                    sBeastMgrEvtQPtr,
                    Q_DIM(sBeastMgrEvtQPtr),
                    nullptr,
                    0U,
                    &sBFHInitEvt);

  static LwIPInitEvt const sLwIPInitEvt = { SIG_DUMMY,
                                            MasterRec::sNetIFRecPtr,
                                            MasterRec::NetCallbackInit };
  static QP::QEvt const *sLwIPEvtQPtr[10];
  MasterRec::sLwIPMgr_AOPtr = new LwIPMgr_AO();
  MasterRec::sLwIPMgr_AOPtr->start(3U,
                                   sLwIPEvtQPtr,
                                   Q_DIM(sLwIPEvtQPtr),
                                   nullptr,
                                   0U,
                                   &sLwIPInitEvt);

  return true;
}


bool MasterRec::IsDirty(void) const {
  bool lIsDirty = false;

  for (unsigned int lRecIx = 0; lRecIx < mRecQty; lRecIx++) {
    lIsDirty |= mDBRec[lRecIx]->IsDirty();
  }

  return lIsDirty;
}


bool MasterRec::IsSane(void) {
  if (!DBRec::IsCRCGood(reinterpret_cast<uint8_t *>(&mMasterRec), sizeof(mMasterRec))) {
    return false;
  }

  // Check magic value.
  if (('M' != mMasterRec.mMagic[0])
      || ('S' != mMasterRec.mMagic[1])
      || ('T' != mMasterRec.mMagic[2])) {
    return false;
  }

  for (unsigned int lRecIx = 0; lRecIx < mRecQty; lRecIx++) {
    if (!mDBRec[lRecIx]->IsSane()) {
      return false;
    }
  }

  return true;
}


unsigned int MasterRec::AddRec(DBRec * const aDBRecPtr) {
  mDBRec[mRecIx] = aDBRecPtr;
  return mRecIx++;
}


void MasterRec::ResetDflt(void) {

  mMasterRec.mMagic[0] = 'M';
  mMasterRec.mMagic[1] = 'S';
  mMasterRec.mMagic[2] = 'T';

  mMasterRec.mVerMajor = VER_MAJOR;
  mMasterRec.mVerMinor = VER_MINOR;
  mMasterRec.mVerRev   = VER_REV;

  mMasterRec.mRecQty = 4;

  // Set record info.
  for (unsigned int lRecIx= 0; lRecIx < 3; lRecIx++) {
    mMasterRec.mRecInfo[lRecIx].mType   = 0;
    mMasterRec.mRecInfo[lRecIx].mOffset = 0;
    mMasterRec.mRecInfo[lRecIx].mSize   = 0;
  }

  // Nullify reserved fields.
  memset(&mMasterRec.mRsvd[0], 0, 12);

  // Reset all sub-records.
  for (unsigned int lRecIx = 0; lRecIx < mRecQty; lRecIx++) {
    mDBRec[lRecIx]->ResetDflt();
  }

  mMasterRec.mCRC = ComputeCRC(reinterpret_cast<uint8_t *>(&mMasterRec), sizeof(mMasterRec));

  mIsDirty = true;
}


unsigned int MasterRec::GetRecSize(void) const {

  unsigned int lSize = sizeof(mMasterRec);
  for (unsigned int lRecIx = 0; lRecIx < mRecQty; lRecIx++) {
    lSize += mDBRec[lRecIx]->GetRecSize();
  }

  return lSize;
}


// Trivial serialization function.
void MasterRec::Serialize(uint8_t * const aDataPtr) const {

  uint8_t *lDataPtr = aDataPtr;
  memcpy(lDataPtr,
         reinterpret_cast<void const *>(&mMasterRec),
         sizeof(struct RecStructTag));
  lDataPtr += sizeof(struct RecStructTag);

  for (unsigned int lRecIx = 0; lRecIx < mRecQty; lRecIx++) {
    mDBRec[lRecIx]->Serialize(lDataPtr);
    lDataPtr += mDBRec[lRecIx]->GetRecSize();
  }
}


// Trivial serialization function.
void MasterRec::Deserialize(uint8_t const *aDataPtr) {

  memcpy(reinterpret_cast<void *>(&mMasterRec),
         aDataPtr,
         sizeof(struct RecStructTag));
  aDataPtr += sizeof(struct RecStructTag);

  for (unsigned int lRecIx = 0; lRecIx < mRecQty; lRecIx++) {
    mDBRec[lRecIx]->Deserialize(aDataPtr);
    aDataPtr += mDBRec[lRecIx]->GetRecSize();
  }
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

void MasterRec::NetCallbackInit(void) {

#if LWIP_HTTPD_SSI
  http_set_ssi_handler(MasterRec::SSIHandler,
                       MasterRec::sSSITags,
                       Q_DIM(MasterRec::sSSITags));
#endif // LWIP_HTTPD_SSI

#if LWIP_HTTPD_CGI
  http_set_cgi_handlers(MasterRec::sCGIEntries,
                        Q_DIM(MasterRec::sCGIEntries));
#endif // LWIP_HTTPD_CGI
}


#if LWIP_HTTPD_SSI
// HTTPD customizations.
// Server-Side Include (SSI) handler.
uint16_t MasterRec::SSIHandler(int   aTagIx,
                               char *aInsertStr,
                               int   aInsertStrLen) {

  switch (aTagIx) {
  case SSI_TAG_IX_ZERO:
    return snprintf(aInsertStr, LWIP_HTTPD_MAX_TAG_INSERT_LEN, "%d", 0);
  default:
  case SSI_TAG_IX_EMPTY:
    return snprintf(aInsertStr, LWIP_HTTPD_MAX_TAG_INSERT_LEN, "%s", "");

  // Info.
  case SSI_TAG_IX_INFO_FW_VERSION:
    return snprintf(aInsertStr,
                    LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                    "%s",
                    FWVersionGenerated::VerStr);
  case SSI_TAG_IX_INFO_BUILD_DATE:
    return snprintf(aInsertStr,
                    LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                    "%s",
                    FWVersionGenerated::BuildDate);
  case SSI_TAG_IX_INFO_BUILD_TIME:
    return snprintf(aInsertStr,
                    LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                    "%s",
                    FWVersionGenerated::BuildTime);
  case SSI_TAG_IX_INFO_GIT_HASH:
    return snprintf(aInsertStr,
                    LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                    "%s",
                    FWVersionGenerated::GitHash);
  case SSI_TAG_IX_INFO_DB_STATUS:
    if (1) {
      return snprintf(aInsertStr,
                      LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                      "Passed");
    } else {
      return snprintf(aInsertStr,
                      LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                      "Failed");
    }
  case SSI_TAG_IX_INFO_RTCC_TEMP:
    return snprintf(aInsertStr,
                    LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                    "%2.2f",
                    RTCC_AO::GetInstancePtr()->GetTemperature());

  // Global.
  case SSI_TAG_IX_GLOBAL_DATE: {
    static char const * const lDateInputStr =
      "<input type=\"date\" name=\"date\" min=\"2018-01-01\" value=\"";
    char        lDateBuf[16] = {0};
    Date       &lDate    = MasterRec::sRTCC_AOPtr->GetDate();
    char const *lDateStr = DateHelper::ToStr(lDate, &lDateBuf[0]);
    return snprintf(aInsertStr,
                    LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                    "%s%s\">",
                    lDateInputStr,
                    lDateStr);
  }

  case SSI_TAG_IX_GLOBAL_TIME: {
    static char const * const lTimeInputStr =
      "<input type=\"time\" name=\"time\" value=\"";
    char        lTimeBuf[16] = {0};
    Time       &lTime    = MasterRec::sRTCC_AOPtr->GetTime();
    char const *lTimeStr = TimeHelper::ToStr(lTime, &lTimeBuf[0]);
    return snprintf(aInsertStr,
                    LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                    "%s%s\">",
                    lTimeInputStr,
                    lTimeStr);
  }

  // Configuration.
  case SSI_TAG_IX_CFG_PAD_ENABLE: {
    static char const * const sFeedingPadEnabledStr =
      "<input type=\"radio\" name=\"feeding_pad\" value=\"y\"";
    return SSIRadioButtonHandler(aTagIx,
                                 aInsertStr,
                                 aInsertStrLen,
                                 sFeedingPadEnabledStr,
                                 MasterRec::sFeedCfgRecPtr->IsAutoPetFeedingEnable());
  }
  case SSI_TAG_IX_CFG_PAD_DISABLE: {
    static char const * const sFeedingPadDisabledStr =
      "<input type=\"radio\" name=\"feeding_pad\" value=\"n\"";
    return SSIRadioButtonHandler(aTagIx,
                                 aInsertStr,
                                 aInsertStrLen,
                                 sFeedingPadDisabledStr,
                                 !MasterRec::sFeedCfgRecPtr->IsAutoPetFeedingEnable());
  }
  case SSI_TAG_IX_CFG_MANUAL_ENABLE: {
    static char const * const sFeedingButtonEnabledStr =
      "<input type=\"radio\" name=\"feeding_button\" value=\"y\"";
    return SSIRadioButtonHandler(aTagIx,
                                 aInsertStr,
                                 aInsertStrLen,
                                 sFeedingButtonEnabledStr,
                                 MasterRec::sFeedCfgRecPtr->IsManualFeedingEnable());
  }
  case SSI_TAG_IX_CFG_MANUAL_DISABLE: {
    static char const * const sFeedingButtonDisabledStr =
      "<input type=\"radio\" name=\"feeding_button\" value=\"n\"";
    return SSIRadioButtonHandler(aTagIx,
                                 aInsertStr,
                                 aInsertStrLen,
                                 sFeedingButtonDisabledStr,
                                 !MasterRec::sFeedCfgRecPtr->IsManualFeedingEnable());
  }
  case SSI_TAG_IX_CFG_FEED_TIME: {
    static char const * const sFeedingTimeStr =
      "<input type=\"number\" name=\"feeding_time_sec\" "
      "min=\"1\" max=\"10\" value=\"";
    return snprintf(aInsertStr,
                    LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                    "%s%d\">",
                    sFeedingTimeStr,
                    MasterRec::sFeedCfgRecPtr->GetTimedFeedPeriod());
  }

  // Calendar.
  case SSI_TAG_IX_CFG_CALENDAR_06_00:
    return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 6);
  case SSI_TAG_IX_CFG_CALENDAR_07_00:
    return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 7);
  case SSI_TAG_IX_CFG_CALENDAR_08_00:
    return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 8);
  case SSI_TAG_IX_CFG_CALENDAR_09_00:
    return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 9);
  case SSI_TAG_IX_CFG_CALENDAR_10_00:
    return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 10);
  case SSI_TAG_IX_CFG_CALENDAR_11_00:
    return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 11);
  case SSI_TAG_IX_CFG_CALENDAR_12_00:
    return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 12);
  case SSI_TAG_IX_CFG_CALENDAR_13_00:
    return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 13);
  case SSI_TAG_IX_CFG_CALENDAR_14_00:
    return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 14);
  case SSI_TAG_IX_CFG_CALENDAR_15_00:
    return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 15);
  case SSI_TAG_IX_CFG_CALENDAR_16_00:
    return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 16);
  case SSI_TAG_IX_CFG_CALENDAR_17_00:
    return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 17);
  case SSI_TAG_IX_CFG_CALENDAR_18_00:
    return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 18);
  case SSI_TAG_IX_CFG_CALENDAR_19_00:
    return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 19);
  case SSI_TAG_IX_CFG_CALENDAR_20_00:
    return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 20);
  case SSI_TAG_IX_CFG_CALENDAR_21_00:
    return SSICalendarHandler(aTagIx, aInsertStr, aInsertStrLen, 21);

  case SSI_TAG_IX_STATS_TX:
  case SSI_TAG_IX_STATS_RX:
  case SSI_TAG_IX_STATS_FW:
  case SSI_TAG_IX_STATS_DROP:
  case SSI_TAG_IX_STATS_CHK_ERR:
  case SSI_TAG_IX_STATS_LEN_ERR:
  case SSI_TAG_IX_STATS_MEM_ERR:
  case SSI_TAG_IX_STATS_RT_ERR:
  case SSI_TAG_IX_STATS_PRO_ERR:
  case SSI_TAG_IX_STATS_OPT_ERR:
  case SSI_TAG_IX_STATS_ERR: {
    // Sub-handler for network stats.
    STAT_COUNTER lVal = SSIStatsHandler(aTagIx, aInsertStr, aInsertStrLen);
    return snprintf(aInsertStr, LWIP_HTTPD_MAX_TAG_NAME_LEN, "%d", lVal);
  }

  }

  return snprintf(aInsertStr, LWIP_HTTPD_MAX_TAG_NAME_LEN, "%d", 0);
}


int MasterRec::SSIRadioButtonHandler(int                aTagIx,
                                     char              *aInsertStr,
                                     int                aInsertStrLen,
                                     char const * const aHTMLStr,
                                     bool               aIsChecked) {

  if (aIsChecked) {
    return snprintf(aInsertStr,
                    LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                    "%s checked>",
                    aHTMLStr);
  } else {
    return snprintf(aInsertStr,
                    LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                    "%s>",
                    aHTMLStr);
  }
}


int MasterRec::SSICalendarHandler(int          aTagIx,
                                  char        *aInsertStr,
                                  int          aInsertStrLen,
                                  unsigned int aHour) {

  static char const *sFeedingCalStr =
    "<input type=\"checkbox\" name=\"feed_time\" value=\"";

  Time lTime(aHour, 0, 0, true, false);
  if (MasterRec::sCalendarPtr->IsEntrySet(lTime)) {
    return snprintf(aInsertStr,
                    LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                    "%s%02d_00\">%02d:00 checked",
                    sFeedingCalStr,
                    aHour,
                    aHour);
  } else {
    return snprintf(aInsertStr,
                    LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                    "%s%02d_00\">%02d:00",
                    sFeedingCalStr,
                    aHour,
                    aHour);
  }
}


int MasterRec::SSIStatsHandler(int   aTagIx,
                               char *aInsertStr,
                               int   aInsertStrLen) {

  struct stats_proto *lStatsPtr = &lwip_stats.link;

  switch (aTagIx) {
  case SSI_TAG_IX_STATS_TX:      return lStatsPtr->xmit;
  case SSI_TAG_IX_STATS_RX:      return lStatsPtr->recv;
  case SSI_TAG_IX_STATS_FW:      return lStatsPtr->fw;
  case SSI_TAG_IX_STATS_DROP:    return lStatsPtr->drop;
  case SSI_TAG_IX_STATS_CHK_ERR: return lStatsPtr->chkerr;
  case SSI_TAG_IX_STATS_LEN_ERR: return lStatsPtr->lenerr;
  case SSI_TAG_IX_STATS_MEM_ERR: return lStatsPtr->memerr;
  case SSI_TAG_IX_STATS_RT_ERR:  return lStatsPtr->rterr;
  case SSI_TAG_IX_STATS_PRO_ERR: return lStatsPtr->proterr;
  case SSI_TAG_IX_STATS_OPT_ERR: return lStatsPtr->opterr;
  case SSI_TAG_IX_STATS_ERR:     return lStatsPtr->err;
  default:                       return 0;
  }

  return 0;
}

#endif // LWIP_HTTPD_SSI


#if LWIP_HTTPD_CGI
// HTTPD customizations.
// CGI handlers.
char const *MasterRec::DispIndex(int   aIx,
                                 int   aParamsQty,
                                 char *aParamsVec[],
                                 char *aValsVec[]) {

  for (int lIx = 0; lIx < aParamsQty; ++lIx) {
    if (strstr(aParamsVec[lIx], "timed_feed") != nullptr) {
      // Param found.
      // Send event with value as parameter.
      BFHTimedFeedCmdEvt *lEvtPtr = Q_NEW(BFHTimedFeedCmdEvt,
                                          SIG_FEED_MGR_TIMED_FEED_CMD);
      sscanf(aValsVec[aIx], "%d", &lEvtPtr->mTime);

      // Could use QF_Publish() to decouple from active object.
      // Here, there's only this well-known recipient.
      BFH_Mgr_AO::AOInstance().POST(lEvtPtr, 0);

      // Return where we're coming from.
      return "/index.shtml";
    }
  }

  return nullptr;
}
#endif // LWIP_HTTPD_CGI

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
