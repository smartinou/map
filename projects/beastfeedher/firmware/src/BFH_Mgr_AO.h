#ifndef BEAST_FEED_HER_MGR_
#define BEAST_FEED_HER_MGR_
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
//        Copyright (c) 2015-2016, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

// Common Library.

// This project.
#include "Calendar.h"
#include "Debouncer.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class SPIDev;
class SPISlaveCfg;

//! \brief Brief description.
//! Details follow...
//! ...here.
class BeastFeedHerMgr : public QP::QActive {
 public:
  BeastFeedHerMgr(CoreLink::SPIDev &aSPIDevRef);

  void GetTime(Time const &aTimeRef) const;
  void GetDate(Date const &aDateRef) const;

  void SetTime(Time const &aTimeRef);
  void SetDate(Date const &aDateRef);

  void ISRCallback(void);

 protected:
  static QP::QState Initial(BeastFeedHerMgr * const aMePtr,
			    QP::QEvt  const * const aEvtPtr);
  static QP::QState FeedingMgr(BeastFeedHerMgr * const aMePtr,
			       QP::QEvt  const * const aEvtPtr);
  static QP::QState Waiting(BeastFeedHerMgr * const aMePtr,
			    QP::QEvt  const * const aEvtPtr);
  static QP::QState FeedingTheBeast(BeastFeedHerMgr * const aMePtr,
				    QP::QEvt  const * const aEvtPtr);
  //static QP::QState Final(BeastFeedHerMgr * const aMePtr,
  //			  QP::QEvt  const * const aEvtPtr);

private:
  //void FeedTheBeast(QP::QEvt const * const aEvtPtr = 0);
  void StartFeeding(void) const;
  void StopFeeding(void)  const;

  static void SetNextFeedingTime(BeastFeedHerMgr * const aMePtr);

  Time  mTime;
  Date  mDate;
  float mTemperature;
  
  DS3234                *mDS3234Ptr;
  CoreLink::SPISlaveCfg *mRTCSPISlaveCfgPtr;

  QP::QEQueue     mFeedEvtQueue;
  QP::QEvt const *mFeedEvtQueueSto[4];
  QP::QTimeEvt    mFeedDelayEvt;

  // Debouncer orthogonal component.
  Debouncer mDebouncer;
  Calendar  mCalendar;
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
#endif // BEAST_FEED_HER_MGR_
