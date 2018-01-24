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
  SSI_TAG_IX_INFO_FIRST,
  SSI_TAG_IX_INFO_FW_VERSION = SSI_TAG_IX_INFO_FIRST,
  SSI_TAG_IX_INFO_BUILD_DATE,
  SSI_TAG_IX_INFO_BUILD_TIME,
  SSI_TAG_IX_INFO_GIT_HASH,
  SSI_TAG_IX_INFO_DB_STATUS,
  SSI_TAG_IX_INFO_RTCC_TEMP,
  SSI_TAG_IX_INFO_LAST = SSI_TAG_IX_INFO_RTCC_TEMP,

  // Global: Time and Date.
  SSI_TAG_IX_CFG_GLOBAL_DATE,
  SSI_TAG_IX_CFG_GLOBAL_TIME,

  // Configuration.
  SSI_TAG_IX_CFG_FIRST,
  SSI_TAG_IX_CFG_PAD_ENABLE = SSI_TAG_IX_CFG_FIRST,
  SSI_TAG_IX_CFG_PAD_DISABLE,
  SSI_TAG_IX_CFG_MANUAL_ENABLE,
  SSI_TAG_IX_CFG_MANUAL_DISABLE,
  SSI_TAG_IX_CFG_LAST = SSI_TAG_IX_CFG_MANUAL_DISABLE,

  SSI_TAG_IX_CFG_FEED_TIME,

  // Calendar.
  SSI_TAG_IX_CFG_CALENDAR_FIRST,
  SSI_TAG_IX_CFG_CALENDAR_06_00 = SSI_TAG_IX_CFG_CALENDAR_FIRST,
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
  SSI_TAG_IX_CFG_CALENDAR_LAST = SSI_TAG_IX_CFG_CALENDAR_21_00,

  // Network configuration.
  SSI_TAG_IX_NET_MAC_ADDR,
  SSI_TAG_IX_NET_IPV4_ADD,
  SSI_TAG_IX_NET_SUBNET_MASK,
  SSI_TAG_IX_NET_GW_ADD,

  SSI_TAG_IX_NET_USE_DHCP,
  SSI_TAG_IX_NET_USE_MANUAL,
  SSI_TAG_IX_NET_STATIC_FIRST,
  SSI_TAG_IX_NET_STATIC_IPV4_ADD_0 = SSI_TAG_IX_NET_STATIC_FIRST,
  SSI_TAG_IX_NET_STATIC_IPV4_ADD_1,
  SSI_TAG_IX_NET_STATIC_IPV4_ADD_2,
  SSI_TAG_IX_NET_STATIC_IPV4_ADD_3,
  SSI_TAG_IX_NET_STATIC_SUBNET_MASK_0,
  SSI_TAG_IX_NET_STATIC_SUBNET_MASK_1,
  SSI_TAG_IX_NET_STATIC_SUBNET_MASK_2,
  SSI_TAG_IX_NET_STATIC_SUBNET_MASK_3,
  SSI_TAG_IX_NET_STATIC_GW_ADD_0,
  SSI_TAG_IX_NET_STATIC_GW_ADD_1,
  SSI_TAG_IX_NET_STATIC_GW_ADD_2,
  SSI_TAG_IX_NET_STATIC_GW_ADD_3,
  SSI_TAG_IX_NET_STATIC_LAST = SSI_TAG_IX_NET_STATIC_GW_ADD_3,

  // Network statistics.
  SSI_TAG_IX_STATS_FIRST,
  SSI_TAG_IX_STATS_TX = SSI_TAG_IX_STATS_FIRST,
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
  SSI_TAG_IX_STATS_LAST = SSI_TAG_IX_STATS_ERR,

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
  "_zero",     // SSI_TAG_IX_ZERO
  "_empty",    // SSI_TAG_IX_EMPTY

  // Info.
  "i_ver",     // SSI_TAG_IX_INFO_FW_VERSION
  "i_date",    // SSI_TAG_IX_INFO_BUILD_DATE
  "i_time",    // SSI_TAG_IX_INFO_BUILD_TIME
  "i_hash",    // SSI_TAG_IX_INFO_GIT_HASH
  "i_status",  // SSI_TAG_IX_INFO_DB_STATUS
  "i_temp",    // SSI_TAG_IX_INFO_RTCC_TEMP

  // Global: Time and Date.
  "g_date",    // SSI_TAG_IX_CFG_GLOBAL_DATE
  "g_time",    // SSI_TAG_IX_CFG_GLOBAL_TIME

  // Configuration.
  "c_pad_en",  // SSI_TAG_IX_CFG_PAD_ENABLE
  "c_pad_di",  // SSI_TAG_IX_CFG_PAD_DISABLE
  "c_but_en",  // SSI_TAG_IX_CFG_MANUAL_ENABLE
  "c_but_di",  // SSI_TAG_IX_CFG_MANUAL_DISABLE
  "c_time",    // SSI_TAG_IX_CFG_FEED_TIME
  "c_cal_06",  // SSI_TAG_IX_CFG_CALENDAR_06_00
  "c_cal_07",  // SSI_TAG_IX_CFG_CALENDAR_07_00
  "c_cal_08",  // SSI_TAG_IX_CFG_CALENDAR_09_00
  "c_cal_09",  // SSI_TAG_IX_CFG_CALENDAR_09_00
  "c_cal_10",  // SSI_TAG_IX_CFG_CALENDAR_10_00
  "c_cal_11",  // SSI_TAG_IX_CFG_CALENDAR_11_00
  "c_cal_12",  // SSI_TAG_IX_CFG_CALENDAR_12_00
  "c_cal_13",  // SSI_TAG_IX_CFG_CALENDAR_13_00
  "c_cal_14",  // SSI_TAG_IX_CFG_CALENDAR_14_00
  "c_cal_15",  // SSI_TAG_IX_CFG_CALENDAR_15_00
  "c_cal_16",  // SSI_TAG_IX_CFG_CALENDAR_16_00
  "c_cal_17",  // SSI_TAG_IX_CFG_CALENDAR_17_00
  "c_cal_18",  // SSI_TAG_IX_CFG_CALENDAR_18_00
  "c_cal_19",  // SSI_TAG_IX_CFG_CALENDAR_19_00
  "c_cal_20",  // SSI_TAG_IX_CFG_CALENDAR_20_00
  "c_cal_21",  // SSI_TAG_IX_CFG_CALENDAR_21_00

  // Network configuration.
  "n_mac",     // SSI_TAG_IX_NET_MAC_ADDR
  "n_ipv4",    // SSI_TAG_IX_NET_IPV4_ADD
  "n_subnet",  // SSI_TAG_IX_NET_SUBNET_MASK
  "n_gateway", // SSI_TAG_IX_NET_GW_ADD
  "n_dhcp",    // SSI_TAG_IX_NET_USE_DHCP
  "n_manual",  // SSI_TAG_IX_NET_USE_MANUAL
  "n_sip0",    // SSI_TAG_IX_NET_STATIC_IPV4_ADD_0
  "n_sip1",    // SSI_TAG_IX_NET_STATIC_IPV4_ADD_1
  "n_sip2",    // SSI_TAG_IX_NET_STATIC_IPV4_ADD_2
  "n_sip3",    // SSI_TAG_IX_NET_STATIC_IPV4_ADD_3
  "n_ssn0",    // SSI_TAG_IX_NET_STATIC_SUBNET_MASK_0
  "n_ssn1",    // SSI_TAG_IX_NET_STATIC_SUBNET_MASK_1
  "n_ssn2",    // SSI_TAG_IX_NET_STATIC_SUBNET_MASK_2
  "n_ssn3",    // SSI_TAG_IX_NET_STATIC_SUBNET_MASK_3
  "n_sgw0",    // SSI_TAG_IX_NET_STATIC_GW_ADD_0
  "n_sgw1",    // SSI_TAG_IX_NET_STATIC_GW_ADD_1
  "n_sgw2",    // SSI_TAG_IX_NET_STATIC_GW_ADD_2
  "n_sgw3",    // SSI_TAG_IX_NET_STATIC_GW_ADD_3

  // Network statistics.
  "s_xmit",    // SSI_TAG_IX_STATS_TX
  "s_recv",    // SSI_TAG_IX_STATS_RX
  "s_fw",      // SSI_TAG_IX_STATS_FW
  "s_drop",    // SSI_TAG_IX_STATS_DROP
  "s_chkerr",  // SSI_TAG_IX_STATS_CHK_ERR
  "s_lenerr",  // SSI_TAG_IX_STATS_LEN_ERR
  "s_memerr",  // SSI_TAG_IX_STATS_MEM_ERR
  "s_rterr",   // SSI_TAG_IX_STATS_RT_ERR
  "s_proerr",  // SSI_TAG_IX_STATS_PRO_ERR
  "s_opterr",  // SSI_TAG_IX_STATS_OPT_ERR
  "s_err",     // SSI_TAG_IX_STATS_ERR
};
#endif //LWIP_HTTPD_SSI


#if LWIP_HTTPD_CGI
tCGI const MasterRec::sCGIEntries[] = {
  {"/index.cgi",  DispIndex},
  {"/config.cgi", DispCfg}
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
  case SSI_TAG_IX_CFG_GLOBAL_DATE: {
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

  case SSI_TAG_IX_CFG_GLOBAL_TIME: {
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
    return SSIRadioButtonHandler(aTagIx,
                                 aInsertStr,
                                 aInsertStrLen,
                                 "feeding_pad\" value=\"y\"",
                                 MasterRec::sFeedCfgRecPtr->IsAutoPetFeedingEnable());
  }
  case SSI_TAG_IX_CFG_PAD_DISABLE: {
    return SSIRadioButtonHandler(aTagIx,
                                 aInsertStr,
                                 aInsertStrLen,
                                 "feeding_pad\" value=\"n\"",
                                 !MasterRec::sFeedCfgRecPtr->IsAutoPetFeedingEnable());
  }
  case SSI_TAG_IX_CFG_MANUAL_ENABLE: {
    return SSIRadioButtonHandler(aTagIx,
                                 aInsertStr,
                                 aInsertStrLen,
                                 "feeding_button\" value=\"y\"",
                                 MasterRec::sFeedCfgRecPtr->IsManualFeedingEnable());
  }
  case SSI_TAG_IX_CFG_MANUAL_DISABLE: {
    return SSIRadioButtonHandler(aTagIx,
                                 aInsertStr,
                                 aInsertStrLen,
                                 "feeding_button\" value=\"n\"",
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


  // Network configuration.
  case SSI_TAG_IX_NET_MAC_ADDR:
  case SSI_TAG_IX_NET_IPV4_ADD:
  case SSI_TAG_IX_NET_SUBNET_MASK:
  case SSI_TAG_IX_NET_GW_ADD:
    return snprintf(aInsertStr,
                    LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                    " ");

  case SSI_TAG_IX_NET_USE_DHCP:
    return SSIRadioButtonHandler(aTagIx,
                                 aInsertStr,
                                 aInsertStrLen,
                                 "use_dhcp\" value=\"y\"",
                                 true);

  case SSI_TAG_IX_NET_USE_MANUAL:
    return SSIRadioButtonHandler(aTagIx,
                                 aInsertStr,
                                 aInsertStrLen,
                                 "use_dhcp\" value=\"n\"",
                                 false);

  case SSI_TAG_IX_NET_STATIC_IPV4_ADD_0:
    return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipaddr_0");
  case SSI_TAG_IX_NET_STATIC_IPV4_ADD_1:
    return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipaddr_1");
  case SSI_TAG_IX_NET_STATIC_IPV4_ADD_2:
    return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipaddr_2");
  case SSI_TAG_IX_NET_STATIC_IPV4_ADD_3:
    return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipaddr_3");

  case SSI_TAG_IX_NET_STATIC_SUBNET_MASK_0:
    return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipmask_0");
  case SSI_TAG_IX_NET_STATIC_SUBNET_MASK_1:
    return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipmask_1");
  case SSI_TAG_IX_NET_STATIC_SUBNET_MASK_2:
    return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipmask_2");
  case SSI_TAG_IX_NET_STATIC_SUBNET_MASK_3:
    return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipmask_3");

  case SSI_TAG_IX_NET_STATIC_GW_ADD_0:
    return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipgw_0");
  case SSI_TAG_IX_NET_STATIC_GW_ADD_1:
    return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipgw_1");
  case SSI_TAG_IX_NET_STATIC_GW_ADD_2:
    return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipgw_2");
  case SSI_TAG_IX_NET_STATIC_GW_ADD_3:
    return SSINetworkHandler(aTagIx, aInsertStr, aInsertStrLen, "ipgw_3");


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
                                     char       * const aInsertStr,
                                     int                aInsertStrLen,
                                     char const * const aNameValStr,
                                     bool               aIsChecked) {

  static char const * const sInputRadioStr = "<input type=\"radio\" name=\"";
  if (aIsChecked) {
    return snprintf(aInsertStr,
                    LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                    "%s%s checked>",
                    sInputRadioStr,
                    aNameValStr);
  } else {
    return snprintf(aInsertStr,
                    LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                    "%s%s>",
                    sInputRadioStr,
                    aNameValStr);
  }
}


int MasterRec::SSICalendarHandler(int          aTagIx,
                                  char * const aInsertStr,
                                  int          aInsertStrLen,
                                  unsigned int aHour) {

  static char const *sFeedingCalStr =
    "<input type=\"checkbox\" name=\"feed_time\" value=\"";

  Time lTime(aHour, 0, 0, true, false);
  if (MasterRec::sCalendarPtr->IsEntrySet(lTime)) {
    return snprintf(aInsertStr,
                    LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                    "%s%02d\" checked>%02d:00",
                    sFeedingCalStr,
                    aHour,
                    aHour);
  } else {
    return snprintf(aInsertStr,
                    LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                    "%s%02d\">%02d:00",
                    sFeedingCalStr,
                    aHour,
                    aHour);
  }
}


int MasterRec::SSINetworkHandler(int                aTagIx,
                                 char       * const aInsertStr,
                                 int                aInsertStrLen,
                                 char const * const aTagNameStr) {

  static char const * const sInputTagStr = "<input name=\"";
  static char const * const sInputValueStr =
    "\" type=\"text\" size=\"2\" maxlength=\"3\" value=\"";
    return snprintf(aInsertStr,
                    LWIP_HTTPD_MAX_TAG_INSERT_LEN,
                    "%s%s%s%d\">",
                    sInputTagStr,
                    aTagNameStr,
                    sInputValueStr,
                    0);
}


int MasterRec::SSIStatsHandler(int          aTagIx,
                               char * const aInsertStr,
                               int          aInsertStrLen) {

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
char const *MasterRec::DispIndex(int   aCGIIx,
                                 int   aParamsQty,
                                 char *aParamsVec[],
                                 char *aValsVec[]) {

  if (0 == strcmp(aParamsVec[0], "timed_feed")) {
    // Param found.
    // Send event with value as parameter.
    BFHTimedFeedCmdEvt *lEvtPtr = Q_NEW(BFHTimedFeedCmdEvt,
                                        SIG_FEED_MGR_TIMED_FEED_CMD);
    sscanf(aValsVec[0], "%d", &lEvtPtr->mTime);

    // Could use QF_Publish() to decouple from active object.
    // Here, there's only this well-known recipient.
    BFH_Mgr_AO::AOInstance().POST(lEvtPtr, 0);

    // Return where we're coming from.
    return "/index.shtml";
  }

  return nullptr;
}


char const *MasterRec::DispCfg(int   aCGIIx,
                               int   aParamsQty,
                               char *aParamsVec[],
                               char *aValsVec[]) {

  // Try to find the Time and Date apply button.
  char const *lSubmitVal = FindTagVal("set_time",
                                      aParamsQty,
                                      aParamsVec,
                                      aValsVec);
  if (0 == strcmp(lSubmitVal, "Apply")) {
    for (int lIx = 0; lIx < aParamsQty; ++lIx) {
      if (0 == strcmp(aParamsVec[lIx], "date")) {
        unsigned int lYear  = 0;
        unsigned int lMonth = 0;
        unsigned int lDayDate  = 0;
        sscanf(aValsVec[lIx], "%d-%d-%d", &lYear, &lMonth, &lDayDate);
        Date lDate(lYear, Month::UIToName(lMonth), lDayDate);

        // Send event to write new date.
        RTCCTimeDateEvt *lEvtPtr = Q_NEW(RTCCTimeDateEvt,
                                         SIG_RTCC_SET_DATE);
        lEvtPtr->mDate = lDate;
        sRTCC_AOPtr->POST(lEvtPtr, 0);

      } else if (0 == strcmp(aParamsVec[lIx], "time")) {
        unsigned int lHours   = 0;
        unsigned int lMinutes = 0;
        sscanf(aValsVec[lIx], "%d%%3A%d", &lHours, &lMinutes);
        Time lTime(lHours, lMinutes, 0);
        Date lDate();

        // Send event to write new time.
        // Send event to write new date.
        RTCCTimeDateEvt *lEvtPtr = Q_NEW(RTCCTimeDateEvt,
                                         SIG_RTCC_SET_TIME);
        lEvtPtr->mTime = lTime;
        sRTCC_AOPtr->POST(lEvtPtr, 0);

      }
    }
    // Return where we're coming from.
    return "/config.shtml";
  }

  // Try to find the Config and Calendar apply button.
  lSubmitVal = FindTagVal("set_cfg",
                          aParamsQty,
                          aParamsVec,
                          aValsVec);
  if (0 == strcmp(lSubmitVal, "Apply")) {
    // Make sure the calendar is cleared before setting new entries.
    // Must be done only once!
    MasterRec::sCalendarPtr->ClrAllEntries();
    for (int lIx = 0; lIx < aParamsQty; ++lIx) {
      if (0 == strcmp(aParamsVec[lIx], "feeding_pad")) {
        if ('y' == *aValsVec[lIx]) {
          MasterRec::sFeedCfgRecPtr->SetIsManualFeedingEnabled(true);
        } else {
          MasterRec::sFeedCfgRecPtr->SetIsManualFeedingEnabled(false);
        }
      } else if (0 == strcmp(aParamsVec[lIx], "feeding_button")) {
        if ('y' == *aValsVec[lIx]) {
          MasterRec::sFeedCfgRecPtr->SetIsAutoPetFeedingEnabled(true);
        } else {
          MasterRec::sFeedCfgRecPtr->SetIsAutoPetFeedingEnabled(false);
        }
      } else if (0 == strcmp(aParamsVec[lIx], "feeding_time_sec")) {
        unsigned int lFeedingTime = 0;
        sscanf(aValsVec[lIx], "%d", &lFeedingTime);
        MasterRec::sFeedCfgRecPtr->SetTimedFeedPeriod(static_cast<uint8_t>(lFeedingTime));
      } else if (0 == strcmp(aParamsVec[lIx], "feed_time")) {
        unsigned int lHour = 0;
        sscanf(aValsVec[lIx], "%d", &lHour);
        Time lTime(lHour, 0, 0);
        MasterRec::sCalendarPtr->SetTimeEntry(lTime);
      }
    }
  }

  // Return where we're coming from.
  return "/config.shtml";
}


char const *MasterRec::FindTagVal(char const  *aTagNameStr,
                                  int          aParamsQty,
                                  char * const aParamsVec[],
                                  char * const aValsVec[]) {

  for (int lIx = 0; lIx < aParamsQty; lIx++) {
    if (0 == strcmp(aParamsVec[lIx], aTagNameStr)) {
      return aValsVec[lIx];
    }
  }

  return nullptr;
}
#endif // LWIP_HTTPD_CGI

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
