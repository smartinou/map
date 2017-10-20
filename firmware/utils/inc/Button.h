#ifndef BUTTON_H_
#define BUTTON_H_
// *******************************************************************************
//
// Project: Beast Feed'Her!
//
// Module: Button class.
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
class Button {
 public:
  Button(unsigned long aPort,
	 unsigned int  aPin,
	 unsigned int  aID);

  unsigned long GetGPIOPort(void) { return mGPIOPort; }
  unsigned int  GetGPIOPin(void)  { return mGPIOPin;  }
  unsigned int  GetGPIOPinState(void);

  void GenerateEvt(void);
  void GenerateEvt(QP::QActive &aAORef);

  enum State {
    RELEASED = 0,
    PRESSED  = 1
  };

 private:
  unsigned long mGPIOPort;
  unsigned int  mGPIOPin;
  unsigned int  mID;
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
#endif // BUTTON_H_
