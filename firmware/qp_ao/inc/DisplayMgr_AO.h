#pragma once
// *******************************************************************************
//
// Project: Beast Feed'Her!
//
// Module: Display manager QP Active Object.
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

// Forward declaration.
class SSD1329;


class DisplayMgrInitEvt : public QP::QEvt {
 public:
  DisplayMgrInitEvt(QP::QSignal  const aSig,
                    unsigned int const aDisplayTime)
    : QP::QEvt(aSig)
    , mDisplayTime(aDisplayTime) {
    // Ctor body left intentionally empty.
  }

 public:
  unsigned int const mDisplayTime;
};


//! \brief Brief description.
//! Details follow...
//! ...here.
class DisplayMgr_AO : public QP::QActive {
 public:
  DisplayMgr_AO(SSD1329 &aSSD1329Ptr);
  static DisplayMgr_AO  &Instance(void);
  static QP::QActive    &AOInstance(void);

 private:
  static QP::QState Initial(DisplayMgr_AO  * const aMePtr,
                            QP::QEvt const * const aEvtPtr);
  static QP::QState Running(DisplayMgr_AO  * const aMePtr,
                            QP::QEvt const * const aEvtPtr);

private:
  DisplayMgr_AO(DisplayMgr_AO const &);
  void operator=(DisplayMgr_AO const &) = delete;

  static void DisplayInit(DisplayMgr_AO * const aMePtr);
  static void DisplayText(DisplayMgr_AO * const aMePtr,
                          QP::QEvt const * const aEvtPtr);

  static void DisplayOn(DisplayMgr_AO  * const aMePtr);
  static void DisplayOff(DisplayMgr_AO * const aMePtr);

  QP::QTimeEvt mDisplayTimerEvt;

  SSD1329     &mDisplay;
  bool         mIsDisplayOn;
  unsigned int mDisplayTime;

  static DisplayMgr_AO *mInstancePtr;
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
