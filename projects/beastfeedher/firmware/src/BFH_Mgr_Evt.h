#pragma once
// *******************************************************************************
//
// Project: Beast Feed'Her!
//
// Module: Beast feeder manager QP Events.
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

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// Forward declarations.
class DBRec;
class FeedCfgRec;
class GPIOs;

// Class definitions.
class BFHInitEvt : public QP::QEvt {
 public:
  BFHInitEvt(QP::QSignal  aSig,
             FeedCfgRec  *aFeedCfgRecPtr,
             GPIOs       *aMotorCtrlIn1Ptr,
             GPIOs       *aMotorCtrlIn2Ptr,
             GPIOs       *aMotorCtrlPWMPtr) {
    sig              = aSig;
    mFeedCfgRecPtr   = aFeedCfgRecPtr;
    mMotorCtrlIn1Ptr = aMotorCtrlIn1Ptr;
    mMotorCtrlIn2Ptr = aMotorCtrlIn2Ptr;
    mMotorCtrlPWMPtr = aMotorCtrlPWMPtr;
  }

 public:
  FeedCfgRec *mFeedCfgRecPtr;
  GPIOs      *mMotorCtrlIn1Ptr;
  GPIOs      *mMotorCtrlIn2Ptr;
  GPIOs      *mMotorCtrlPWMPtr;
};


class BFHManualFeedCmdEvt : public QP::QEvt {
 public:
  BFHManualFeedCmdEvt(QP::QSignal aSig, bool aIsOn) {
    sig     = aSig;
    poolId_ = 0U;
    mIsOn   = aIsOn;
  }

 public:
  bool mIsOn;
};


class BFHTimedFeedCmdEvt : public QP::QEvt {
 public:
  BFHTimedFeedCmdEvt(QP::QSignal aSig, unsigned int aTime) {
    sig     = aSig;
    poolId_ = 0U;
    mTime   = aTime;
  }

 public:
  unsigned int mTime;
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
