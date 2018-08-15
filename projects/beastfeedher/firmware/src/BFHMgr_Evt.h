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
  BFHInitEvt(QP::QSignal  const aSig,
             FeedCfgRec * const aFeedCfgRecPtr,
             GPIOs      * const aMotorCtrlIn1Ptr,
             GPIOs      * const aMotorCtrlIn2Ptr,
             GPIOs      * const aMotorCtrlPWMPtr)
    : QP::QEvt(aSig)
    , mFeedCfgRecPtr(aFeedCfgRecPtr)
    , mMotorCtrlIn1Ptr(aMotorCtrlIn1Ptr)
    , mMotorCtrlIn2Ptr(aMotorCtrlIn2Ptr)
    , mMotorCtrlPWMPtr(aMotorCtrlPWMPtr) {
    // Ctor body left intentionally empty.
  }

 public:
  FeedCfgRec * const mFeedCfgRecPtr;
  GPIOs      * const mMotorCtrlIn1Ptr;
  GPIOs      * const mMotorCtrlIn2Ptr;
  GPIOs      * const mMotorCtrlPWMPtr;
};


class BFHManualFeedCmdEvt : public QP::QEvt {
 public:
  BFHManualFeedCmdEvt(QP::QSignal const aSig, bool const aIsOn)
    : QP::QEvt(aSig)
    , mIsOn(aIsOn) {
    // Ctor body left intentionally empty.
  }

 public:
  bool const mIsOn;
};


class BFHTimedFeedCmdEvt : public QP::QEvt {
 public:
  BFHTimedFeedCmdEvt(QP::QSignal const aSig, unsigned int const aTime)
    : QP::QEvt(aSig)
    , mTime(aTime) {
    // Ctor body left intentionally empty.
  }

 public:
  unsigned int const mTime;
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
