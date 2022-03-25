#pragma once
// *******************************************************************************
//
// Project: Utilities.
//
// Module: Debouncer QP Active Object.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// ******************************************************************************
//
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
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

//! \brief Brief description.
//! Details follow...
//! ...here.

//! \brief Button component.
class Debouncer : public QP::QHsm {
 public:
  Debouncer(QP::QActive * const aContainerPtr);

 protected:
  static QP::QState Initial(Debouncer      * const aMePtr,
			    QP::QEvt const * const aEvtPtr);
  static QP::QState Ready(Debouncer      * const aMePtr,
			  QP::QEvt const * const aEvtPtr);
  static QP::QState Pressed(Debouncer      * const aMePtr,
			    QP::QEvt const * const aEvtPtr);

private:
  QP::QActive * const mContainerPtr;
  QP::QTimeEvt        mDebounceTimeEvt;
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
