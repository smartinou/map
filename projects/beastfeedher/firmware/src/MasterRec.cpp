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
  mRecQty = 2;
  mDBRec = new DBRec *[mRecQty];
  CalendarRec *lCalendarPtr = new CalendarRec();
  AddRec(lCalendarPtr);

  NetIFRec *lNetIFRecPtr = new NetIFRec();
  AddRec(lNetIFRecPtr);

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
                                            lCalendarPtr };
  static QP::QEvt const *sRTCCEvtQPtr[10];
  RTCC_AO *lRTCC_AOPtr = new RTCC_AO();
  lRTCC_AOPtr->start(1U,
                     sRTCCEvtQPtr,
                     Q_DIM(sRTCCEvtQPtr),
                     nullptr,
                     0U,
                     &sRTCCInitEvt);

  // DB records are now deserialized, and fixed if required.
  // Create all other AOs.
  static QP::QEvt const *sBeastMgrEvtQPtr[5];
  BFH_Mgr_AO &lBFH_Mgr_AO = BFH_Mgr_AO::Instance();
  lBFH_Mgr_AO.start(2U,
                    sBeastMgrEvtQPtr,
                    Q_DIM(sBeastMgrEvtQPtr),
                    nullptr,
                    0U);

  static LwIPInitEvt const sLwIPInitEvt = { SIG_DUMMY, lNetIFRecPtr, MasterRec::NetCallbackInit };
  static QP::QEvt const *sLwIPEvtQPtr[10];
  LwIPMgr_AO *lLwIPMgr_AOPtr = new LwIPMgr_AO();
  lLwIPMgr_AOPtr->start(3U,
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
#endif // #if LWIP_HTTPD_SSI

#if LWIP_HTTPD_CGI
  http_set_cgi_handlers(MasterRec::sCGIEntries,
                        Q_DIM(MasterRec::sCGIEntries));
#endif //LWIP_HTTPD_CGI
}


#if LWIP_HTTPD_SSI
// HTTPD customizations.
// Server-Side Include (SSI) handler.
uint16_t MasterRec::SSIHandler(int aTagIx, char *aInsertStr, int aInsertStrLen) {

  switch (aTagIx) {
  case SSI_TAG_IX_ZERO:
    return snprintf(aInsertStr, LWIP_HTTPD_MAX_TAG_INSERT_LEN, "%d", 0);
  default:
  case SSI_TAG_IX_EMPTY:
    return snprintf(aInsertStr, LWIP_HTTPD_MAX_TAG_INSERT_LEN, "%s", "");

  // Info.
  case SSI_TAG_IX_INFO_FW_VERSION:
    return snprintf(aInsertStr, LWIP_HTTPD_MAX_TAG_INSERT_LEN, "%s", FWVersionGenerated::VerStr);
  case SSI_TAG_IX_INFO_BUILD_DATE:
    return snprintf(aInsertStr, LWIP_HTTPD_MAX_TAG_INSERT_LEN, "%s", FWVersionGenerated::BuildDate);
  case SSI_TAG_IX_INFO_BUILD_TIME:
    return snprintf(aInsertStr, LWIP_HTTPD_MAX_TAG_INSERT_LEN, "%s", FWVersionGenerated::BuildTime);
  case SSI_TAG_IX_INFO_GIT_HASH:
    return snprintf(aInsertStr, LWIP_HTTPD_MAX_TAG_INSERT_LEN, "%s", FWVersionGenerated::GitHash);
  case SSI_TAG_IX_INFO_DB_STATUS:
    return snprintf(aInsertStr, LWIP_HTTPD_MAX_TAG_INSERT_LEN, "%s", "Passed");

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


int MasterRec::SSIStatsHandler(int aTagIx, char *aInsertStr, int aInsertStrLen) {
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

#endif //LWIP_HTTPD_SSI


#if LWIP_HTTPD_CGI
// HTTPD customizations.
// CGI handlers.
char const *MasterRec::DispIndex(int   aIx,
                                 int   aParamsQty,
                                 char *aParamsPtr[],
                                 char *aValsPtr[]) {

  for (int lIx = 0; lIx < aParamsQty; ++lIx) {
    if (strstr(aParamsPtr[lIx], "timed_feed") != nullptr) {
      // Param found.
      // Send event with value as parameter.
      BFHTimedFeedCmdEvt *lEvtPtr = Q_NEW(BFHTimedFeedCmdEvt, SIG_FEED_MGR_TIMED_FEED_CMD);
      if (0 == strcmp(aValsPtr[aIx], "2s")) {
        lEvtPtr->mTime = 2;
      } else if (0 == strcmp(aValsPtr[aIx], "5s")) {
        lEvtPtr->mTime = 5;
      } else if (0 == strcmp(aValsPtr[aIx], "10s")) {
        lEvtPtr->mTime = 10;
      }

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
