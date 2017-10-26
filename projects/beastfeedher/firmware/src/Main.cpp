// *****************************************************************************
//
// Project: Beast Feed'Her
//
// Module: Main entry point.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// *****************************************************************************
//
//        Copyright (c) 2015-2016, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// QP Library.
#include "qpcpp.h"
#include "qep.h"
#include "qf.h"

// CMSIS Library.
#include "lm3s_cmsis.h"

// TI Library.
#include "hw_types.h"
#include "hw_ints.h"
#include "systick.h"
#include "uartstdio.h"

#include "debug.h"
#include "gpio.h"
#include "interrupt.h"
#include "sysctl.h"

// Common Library.
#include "Button.h"
//#include "Calendar.h"

#include "SSD1329.h"
#include "SPI.h"

// This application.
//#include "BFH_Mgr_AO.h"
#include "BSP.h"
#include "RTCC_AO.h"

Q_DEFINE_THIS_FILE

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

int main(void) {

  // Initialize the Board Support Package.
  // Initialize the framework and the underlying RT kernel.
  QP::QF::init();
  CoreLink::SPIDev *lSPIDevPtr = BSPInit();
  UARTprintf("QF version: %s", QP::QF::getVersion());

  // Initialize event pool.
  //static QF_MPOOL_EL(DPP::ButtonEvt) sSmallPoolSto[20];
  // [MG] VERIFIER LE SIZE MAX D'EVENTS NECESSAIRES.
  static QF_MPOOL_EL(ManualFeedCmdEvt) sSmallPoolSto[20];
  QP::QF::poolInit(sSmallPoolSto,
                   sizeof(sSmallPoolSto),
                   sizeof(sSmallPoolSto[0]));

  static QF_MPOOL_EL(RTCCEvt) sMediumPoolSto[10];
  QP::QF::poolInit(sMediumPoolSto,
                   sizeof(sMediumPoolSto),
                   sizeof(sMediumPoolSto[0]));

  // Init publish-subscribe.
  static QP::QSubscrList lSubsribeSto[SIG_QTY];
  QP::QF::psInit(lSubsribeSto, Q_DIM(lSubsribeSto));
#if 0
  // Instantiate and start the active objects.
  static GPIOInitEvt const sGPIOInitEvt = { SIG_DUMMY, GPIO_PORTA_BASE, GPIO_PIN_6 };
  static QP::QEvt    const *sBeastMgrEvtQPtr[5];

  gMain_BeastFeedHerMgrPtr = new BeastFeedHerMgr(*lSPIDevPtr);
  gMain_BeastFeedHerMgrAOPtr = gMain_BeastFeedHerMgrPtr;
  gMain_BeastFeedHerMgrPtr->start(1U,
                                  sBeastMgrEvtQPtr,
                                  Q_DIM(sBeastMgrEvtQPtr),
                                  static_cast<void *>(0),
                                  0U,
                                  &sGPIOInitEvt);
#else

  // FIXME: find how to move into BSP file.
  static RTCCInitEvt const sRTCCInitEvt = { SIG_DUMMY,
                                            *lSPIDevPtr,
                                            GPIO_PORTA_BASE,
                                            GPIO_PIN_7,
                                            GPIO_PORTA_BASE,
                                            GPIO_PIN_6 };
  static QP::QEvt const *sRTCCEvtQPtr[5];
  RTCC_AO *lRTCC_AOPtr = new RTCC_AO();
  lRTCC_AOPtr->start(1U,
                     sRTCCEvtQPtr,
                     Q_DIM(sRTCCEvtQPtr),
                     static_cast<void *>(0),
                     0U,
                     &sRTCCInitEvt);
#endif
  // Run the QF application.
  return QP::QF::run();
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
