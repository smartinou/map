// *****************************************************************************
//
// Project: Beast Feed'Her
//
// Module: Beast feeder manager QP Active Object.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group
//!
//! This is the main AO of the system.
//! It manages the feeding signals comming from various sources like:
//! RTC alarm, button, web cgi or CLI.
//!
//! The AO uses two different design pattern.
//! 1. Deferred events:
//!   The choice was made to defer feeding signal is the source is from the
//!   RTC alarm (because it's part of the feeding calendar).
//!   Any other feeding signal incoming while in the feeding substate is ignored.

// *****************************************************************************
//
//        Copyright (c) 2015-2017, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************
#define BFH_MGR_DBG
//#undef BFH_MGR_DBG

// Standard Library.
#include <stddef.h>

// TI Library.

// QP Library.
#include "qpcpp.h"

// Common Library.
#include "Date.h"
//#include "DBRec.h"
#include "Time.h"

// This project.
#include "BFH_Mgr_AO.h"
#include "BSP.h"
#include "RTCC_Evt.h"

Q_DEFINE_THIS_FILE

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

BFH_Mgr_AO *BFH_Mgr_AO::mInstancePtr = static_cast<BFH_Mgr_AO *>(0);

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

BFH_Mgr_AO &BFH_Mgr_AO::Instance(void) {

  if (static_cast<BFH_Mgr_AO *>(0) == mInstancePtr) {
    mInstancePtr = new BFH_Mgr_AO();
  }

  return *mInstancePtr;
}


QP::QActive &BFH_Mgr_AO::AOInstance(void) {

  if (static_cast<BFH_Mgr_AO *>(0) == mInstancePtr) {
    mInstancePtr = new BFH_Mgr_AO();
  }

  return static_cast<QP::QActive &>(*mInstancePtr);
}

#if 0
void BFH_Mgr_AO::ISRCallback(void) {

  // Static event.
  static QP::QEvt const sRTCSQWIntEvt = { SIG_RTC_IRQ, 0U, 0U };
  mDS3234Ptr->ISRCallback();

  // Signal that RTC generated an interrupt.
  POST(&sRTCSQWIntEvt, 0);

}
#endif
// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

BFH_Mgr_AO::BFH_Mgr_AO() :
  QActive(Q_STATE_CAST(&BFH_Mgr_AO::Initial)),
  mFeedTimerEvt(this, SIG_FEED_MGR_TIMEOUT, 0U) {

  // Ctor body intentionally left empty.
}


QP::QState BFH_Mgr_AO::Initial(BFH_Mgr_AO     * const me,  //aMePtr,
                               QP::QEvt const * const e) { //aEvtPtr

  // Initialize the QF queue for deferred feed requests.
  me->mFeedEvtQueue.init(me->mFeedEvtQueueSto, Q_DIM(me->mFeedEvtQueueSto));

  // Subscribe to signals if any.
  me->subscribe(SIG_RTCC_CALENDAR_EVENT_ALARM);

  // Store reference to a feeder unit, or a motor controller.

  return Q_TRAN(&BFH_Mgr_AO::FeedingMgr);
}


QP::QState BFH_Mgr_AO::FeedingMgr(BFH_Mgr_AO     * const me,  //aMePtr,
                                  QP::QEvt const * const e) { //aEvtPtr

  switch (e->sig) {
  case Q_ENTRY_SIG:
    // Init Feedeer.
    return Q_HANDLED();

  case SIG_FEED_MGR_MANUAL_FEED_CMD: {
    // Cast event to know the state (on/off).
    ManualFeedCmdEvt const *lEvtPtr = static_cast<ManualFeedCmdEvt const *>(e);
    if (lEvtPtr->mIsOn) {
      return Q_TRAN(&BFH_Mgr_AO::ManualFeed);
    }
    // Off: intentional fallthrough.
  }
  case Q_INIT_SIG:
    // Go into default nested state.
  case SIG_FEED_MGR_TIMEOUT:
    return Q_TRAN(&BFH_Mgr_AO::Waiting);

  case SIG_RTCC_CALENDAR_EVENT_ALARM:
    //RTCCEvt const *lEvtPtr = static_cast<RTCCEvt const *>(e);
    //Log(Time, Date);
  case SIG_FEED_MGR_TIMED_FEED_CMD:
    return Q_TRAN(&BFH_Mgr_AO::TimedFeed);

  case Q_EXIT_SIG:
    return Q_HANDLED();

  case SIG_TERMINATE:
    // Exit application gracefully.
    // Could also transition to a "Final" state.
    // BSP_Exit();
    return Q_HANDLED();
  }

  return Q_SUPER(&QP::QHsm::top);
}


QP::QState BFH_Mgr_AO::Waiting(BFH_Mgr_AO     * const me,  //aMePtr,
                               QP::QEvt const * const e) { //aEvtPtr

  switch (e->sig) {
  case Q_ENTRY_SIG:
    me->recall(&me->mFeedEvtQueue);
    return Q_HANDLED();
  }

  return Q_SUPER(&BFH_Mgr_AO::FeedingMgr);
}


QP::QState BFH_Mgr_AO::TimedFeed(BFH_Mgr_AO     * const me,  //aMePtr,
                                 QP::QEvt const * const e) { //aEvtPtr

  switch (e->sig) {
  case Q_ENTRY_SIG:
    me->mFeedTimerEvt.armX(30);
    me->StartFeeding();
    return Q_HANDLED();

  case SIG_RTCC_CALENDAR_EVENT_ALARM:
    // Cast event, log(Time, Date);
    me->defer(&me->mFeedEvtQueue, e);
    return Q_HANDLED();

  case SIG_FEED_MGR_MANUAL_FEED_CMD:
    // DoNothing();
    return Q_HANDLED();

  case Q_EXIT_SIG:
    me->StopFeeding();
    me->mFeedTimerEvt.disarm();
    return Q_HANDLED();
  }

  return Q_SUPER(&BFH_Mgr_AO::FeedingMgr);
}


QP::QState BFH_Mgr_AO::ManualFeed(BFH_Mgr_AO     * const me,  //aMePtr,
                                  QP::QEvt const * const e) { //aEvtPtr

  switch (e->sig) {
  case Q_ENTRY_SIG:
    return Q_HANDLED();

  case Q_INIT_SIG:
    // Go into default nested state.
    return Q_TRAN(&BFH_Mgr_AO::WaitPeriod);

  case SIG_RTCC_CALENDAR_EVENT_ALARM:
    // Cast event, log(Time, Date);
    me->defer(&me->mFeedEvtQueue, e);
    return Q_HANDLED();

  case SIG_FEED_MGR_MANUAL_FEED_CMD: {
    // Cast event to know the state (on/off).
    ManualFeedCmdEvt const *lEvtPtr = static_cast<ManualFeedCmdEvt const *>(e);
    if (lEvtPtr->mIsOn) {
      // On: DoNothing();
      return Q_HANDLED();
    }
    break;
  }

  case Q_EXIT_SIG:
    me->StopFeeding();
    me->mFeedTimerEvt.disarm();
    return Q_HANDLED();
  }

  return Q_SUPER(&BFH_Mgr_AO::FeedingMgr);
}


QP::QState BFH_Mgr_AO::WaitPeriod(BFH_Mgr_AO     * const me,  //aMePtr,
                                  QP::QEvt const * const e) { //aEvtPtr

  switch (e->sig) {
  case Q_ENTRY_SIG:
    me->mFeedTimerEvt.armX(50);
    return Q_HANDLED();

  case SIG_FEED_MGR_TIMEOUT:
    return Q_TRAN(&BFH_Mgr_AO::TimeCappedFeed);

  case Q_EXIT_SIG:
    return Q_HANDLED();
  }

  return Q_SUPER(&BFH_Mgr_AO::ManualFeed);
}


QP::QState BFH_Mgr_AO::TimeCappedFeed(BFH_Mgr_AO     * const me,  //aMePtr,
                                      QP::QEvt const * const e) { //aEvtPtr

  switch (e->sig) {
  case Q_ENTRY_SIG:
    me->mFeedTimerEvt.armX(200);
    return Q_HANDLED();

  case Q_EXIT_SIG:
    return Q_HANDLED();
  }

  return Q_SUPER(&BFH_Mgr_AO::ManualFeed);
}


void BFH_Mgr_AO::StartFeeding(void) const {
  // Turn on feeding mechanism.
  // FeederUnit->Start();
}


void BFH_Mgr_AO::StopFeeding(void) const {
  // Turn off feeding mechanism.
  // FeederUnit->Stop();
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
