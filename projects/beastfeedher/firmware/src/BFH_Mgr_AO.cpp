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
//        Copyright (c) 2015-2018, Martin Garon, All rights reserved.
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
#include "DBRec.h"
#include "FeedCfgRec.h"
#include "Time.h"

// This project.
#include "BFH_Mgr_AO.h"
#include "BFH_Mgr_Evt.h"
#include "BSP.h"
#include "RTCC_Evt.h"
#include "TB6612.h"

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

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

BFH_Mgr_AO::BFH_Mgr_AO() :
  QActive(Q_STATE_CAST(&BFH_Mgr_AO::Initial)),
  mFeedEvtQueue(),
  mFeedEvtQueueSto{nullptr},
  mFeedTimerEvt(this, SIG_FEED_MGR_TIMEOUT, 0U),
  mFeedCfgRecPtr(nullptr),
  mFeedTime(0),
  mMotorCtrlPtr(nullptr) {

  // Ctor body intentionally left empty.
}


QP::QState BFH_Mgr_AO::Initial(BFH_Mgr_AO     * const me,  //aMePtr,
                               QP::QEvt const * const e) { //aEvtPtr

  BFHInitEvt const * const lBFHInitEvtPtr = static_cast<BFHInitEvt const * const>(e);
  me->mFeedCfgRecPtr = lBFHInitEvtPtr->mFeedCfgRecPtr;

  // Store reference to a feeder unit, or a motor controller.
  me->mMotorCtrlPtr = new TB6612(*lBFHInitEvtPtr->mMotorCtrlIn1Ptr,
                                 *lBFHInitEvtPtr->mMotorCtrlIn2Ptr,
                                 *lBFHInitEvtPtr->mMotorCtrlPWMPtr);
  me->StopFeeding();

  // Initialize the QF queue for deferred feed requests.
  me->mFeedEvtQueue.init(me->mFeedEvtQueueSto, Q_DIM(me->mFeedEvtQueueSto));

  // Subscribe to signals if any.
  me->subscribe(SIG_RTCC_CALENDAR_EVENT_ALARM);

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
    BFHManualFeedCmdEvt const * const lEvtPtr = static_cast<BFHManualFeedCmdEvt const *>(e);
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
    me->mFeedTime = me->mFeedCfgRecPtr->GetTimedFeedPeriod();
    return Q_TRAN(&BFH_Mgr_AO::TimedFeed);

  case SIG_FEED_MGR_TIMED_FEED_CMD: {
    // FIXME: perform boundary check on value.
    BFHTimedFeedCmdEvt const * const lEvtPtr = reinterpret_cast<BFHTimedFeedCmdEvt const * const>(e);
    me->mFeedTime = lEvtPtr->mTime;
    return Q_TRAN(&BFH_Mgr_AO::TimedFeed);
  }

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
    me->mFeedTimerEvt.armX(me->mFeedTime * BSP_TICKS_PER_SEC);
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
    BFHManualFeedCmdEvt const * const lEvtPtr = static_cast<BFHManualFeedCmdEvt const * const>(e);
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
  mMotorCtrlPtr->TurnOnCW();
}


void BFH_Mgr_AO::StopFeeding(void) const {
  // Turn off feeding mechanism.
  mMotorCtrlPtr->TurnOff();
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
