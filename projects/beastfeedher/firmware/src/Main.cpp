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
//        Copyright (c) 2015-2017, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// QP Library.
#include "qpcpp.h"

// TI Library.
#include "uartstdio.h"

// Common Library.
#include "SPI.h"

// This application.
#include "App.h"
#include "BFHMgr_Evt.h"
#include "BSP.h"
#include "RTCC_Evt.h"

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

  // Initialize the framework and the underlying RT kernel.
  QP::QF::init();

  // Initialize event pool.
  // [MG] VERIFIER LE SIZE MAX D'EVENTS NECESSAIRES.
  static QF_MPOOL_EL(BFHManualFeedCmdEvt) sSmallPoolSto[20];
  QP::QF::poolInit(sSmallPoolSto,
                   sizeof(sSmallPoolSto),
                   sizeof(sSmallPoolSto[0]));

  static QF_MPOOL_EL(RTCCTimeDateEvt) sMediumPoolSto[10];
  QP::QF::poolInit(sMediumPoolSto,
                   sizeof(sMediumPoolSto),
                   sizeof(sMediumPoolSto[0]));

  // Init publish-subscribe.
  static QP::QSubscrList lSubsribeSto[SIG_QTY];
  QP::QF::psInit(lSubsribeSto, Q_DIM(lSubsribeSto));


  // Start master record.
  // Contains all AOs.
  App *lAppPtr = new App();
  bool lInitGood = lAppPtr->Init();

  // Run the QF application.
  if (lInitGood) {
    UARTprintf("QF version: %s", QP::QF::getVersion());
    return QP::QF::run();
  }

  while (1);
  return 1;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
