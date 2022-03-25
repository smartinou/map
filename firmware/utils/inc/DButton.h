#pragma once
// *******************************************************************************
//
// Project: Utilities.
//
// Module: Debounced button class.
//
// *******************************************************************************

//! \file
//! \brief DButton class.
//! \ingroup utils_dbutton

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
// [MG] CECI EST UN ESSAI POUR VOIR S'IL Y A UNE POSSIBILITE DE FAIRE UN OBJET
// [MG] "BUTTON" COMPOSITE, AYANT UN OBJET "DEBOUNCER".
// [MG] PAS SUR QUE CE SOIT MEILLEUR QUE DES OBJETS SEPARES.
// [MG] EN FAIT, JE PENSE QUE NON!!!
//! \brief Button component.
class DButton : public QP::QHsm { //QActive {
 public:
  DButton(QP::QActive * const aContainerPtr,
	  unsigned long aPort,
	  unsigned int  aPin);

  //unsigned long GetGPIOPort(void) { return mGPIOPort; }
  //unsigned int  GetGPIOPin(void)  { return mGPIOPin;  }
#if 1
  void GenerateEvt(void);
  void GenerateEvt(QP::QActive &aAORef);
#endif

  enum State {
    RELEASED = 0,
    PRESSED  = 1
  };

 protected:
  static QP::QState Initial(DButton        * const aMePtr,
			    QP::QEvt const * const aEvtPtr);
  static QP::QState Idle(DButton        * const aMePtr,
			 QP::QEvt const * const aEvtPtr);
  static QP::QState RegisteringPressed(DButton        * const aMePtr,
				       QP::QEvt const * const aEvtPtr);
  static QP::QState RegisteringReleased(DButton        * const aMePtr,
					QP::QEvt const * const aEvtPtr);

 private:
  unsigned int  GetGPIOPinState(void);

  static unsigned int sRegisteringPressedDflt;
  static unsigned int sRegisteringReleasedDflt;

  QP::QActive * const mContainerPtr;
  QP::QTimeEvt        mDebounceTimeEvt;

  unsigned long mGPIOPort;
  unsigned int  mGPIOPin;
  unsigned int  mRegisteringReleased;
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
