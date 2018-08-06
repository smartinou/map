#pragma once
// *******************************************************************************
//
// Project: Active Object Library
//
// Module: LwIP manager QP Events.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2017-2018, Martin Garon, All rights reserved.
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
class NetIFRec;


// Class definitions.
class LwIPInitEvt : public QP::QEvt {
 public:
  LwIPInitEvt(QP::QSignal const aSig,
              NetIFRec  * const aNetIFRecPtr,
              void     (* const aCallbackInit)(void))
    : QP::QEvt(aSig)
    , mNetIFRecPtr(aNetIFRecPtr)
    , mCallbackInit(aCallbackInit) {
    // Ctor body left intentionally empty.
  }

 public:
  NetIFRec  * const mNetIFRecPtr;
  void     (* const mCallbackInit)(void);
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
