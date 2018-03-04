#pragma once
// *******************************************************************************
//
// Project: Beast Feed'Her!
//
// Module: Beast feeder manager QP Active Object.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2015-2018, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

// Common Library.

// This project.

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// Forward declaration.
class FeedCfgRec;
class TB6612;


//! \brief Brief description.
//! Details follow...
//! ...here.
class BFH_Mgr_AO : public QP::QActive {
 public:
  static BFH_Mgr_AO  &Instance(void);
  static QP::QActive &AOInstance(void);

 protected:
  static QP::QState Initial(BFH_Mgr_AO     * const aMePtr,
                            QP::QEvt const * const aEvtPtr);
  static QP::QState FeedingMgr(BFH_Mgr_AO     * const aMePtr,
                               QP::QEvt const * const aEvtPtr);
  static QP::QState Waiting(BFH_Mgr_AO     * const aMePtr,
                            QP::QEvt const * const aEvtPtr);
  static QP::QState TimedFeed(BFH_Mgr_AO     * const aMePtr,
                              QP::QEvt const * const aEvtPtr);
  static QP::QState ManualFeed(BFH_Mgr_AO     * const aMePtr,
                               QP::QEvt const * const aEvtPtr);
  static QP::QState WaitPeriod(BFH_Mgr_AO     * const aMePtr,
                               QP::QEvt const * const aEvtPtr);
  static QP::QState TimeCappedFeed(BFH_Mgr_AO     * const aMePtr,
                                   QP::QEvt const * const aEvtPtr);

private:
  enum {
    TIME_CAPPED_DEBOUNCE = 2,
    TIME_CAPPED_TIMEOUT  = 10
  };
  BFH_Mgr_AO();
  BFH_Mgr_AO(BFH_Mgr_AO const &);
  void operator=(BFH_Mgr_AO const &) = delete;

  void StartFeeding(void) const;
  void StopFeeding(void)  const;

  QP::QEQueue     mFeedEvtQueue;
  QP::QEvt const *mFeedEvtQueueSto[4];
  QP::QTimeEvt    mFeedTimerEvt;

  FeedCfgRec const *mFeedCfgRecPtr;
  unsigned int      mFeedTime;

  TB6612 *mMotorCtrlPtr;

  static BFH_Mgr_AO *mInstancePtr;
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
