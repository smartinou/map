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
//        Copyright (c) 2016-2017, Martin Garon, All rights reserved.
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

// This project.
#include "BFH_Mgr_AO.h"
#include "BFH_Mgr_Evt.h"
#include "BSP.h"
#include "CalendarRec.h"
#include "LwIPMgr_AO.h"
#include "LwIPMgr_Evt.h"
#include "MasterRec.h"
#include "NetIFRec.h"
#include "RTCC_AO.h"
#include "RTCC_Evt.h"

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

  static LwIPInitEvt const sLwIPInitEvt = { SIG_DUMMY, lNetIFRecPtr };
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


bool MasterRec::IsSane(void) const {

  // Check magic value.
  if (('M' != mMasterRec.mMagic[0])
      || ('S' != mMasterRec.mMagic[1])
      || ('T' != mMasterRec.mMagic[2])
      || ('R' != mMasterRec.mMagic[3])) {
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
  mMasterRec.mMagic[3] = 'R';

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

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
