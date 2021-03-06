// *****************************************************************************
//
// Project: PFPP
//
// Module: Main.
//
// *****************************************************************************

//! \file
//! \brief Main entry point.
//! \ingroup application

// *****************************************************************************
//
//        Copyright (c) 2015-2020, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// QP Library.
#include "qpcpp.h"

// This application.
#include "App.h"
#include "BSP.h"
#include "Logging_Events.h"
#include "PFPP_Events.h"
#include "RTCC_Events.h"
#include "Signals.h"

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

    // BSP::Init() has to be called early.
    // QS has to be initialized before creating dictionary items.
    std::shared_ptr<IBSPFactory> lFactory = BSP::Init();

    // Initialize event pool.
    static QF_MPOOL_EL(PFPP::Event::Mgr::ManualFeedCmd) sSmallPoolSto[20];
    QP::QF::poolInit(
        sSmallPoolSto,
        sizeof(sSmallPoolSto),
        sizeof(sSmallPoolSto[0])
    );

    static QF_MPOOL_EL(RTCC::Event::TimeAndDate) sMediumPoolSto[10];
    QP::QF::poolInit(
        sMediumPoolSto,
        sizeof(sMediumPoolSto),
        sizeof(sMediumPoolSto[0])
    );

    static QF_MPOOL_EL(Logging::Event::LogEntry) sLargePoolSto[10];
    QP::QF::poolInit(
        sLargePoolSto,
        sizeof(sLargePoolSto),
        sizeof(sLargePoolSto[0])
    );

    // Init publish-subscribe.
    static QP::QSubscrList lSubsribeSto[QTY_SIG];
    QP::QF::psInit(lSubsribeSto, Q_DIM(lSubsribeSto));

    // Send object dictionaries for event pools...
    QS_OBJ_DICTIONARY(sSmallPoolSto);
    QS_OBJ_DICTIONARY(sMediumPoolSto);
    QS_OBJ_DICTIONARY(sLargePoolSto);

    QS_FUN_DICTIONARY(&QP::QHsm::top);

    // Start master record.
    // Contains all AOs.
    App * const lAppPtr = new App();
    bool lInitGood = lAppPtr->Init(lFactory);

    // Run the QF application.
    if (lInitGood) {
        return QP::QF::run();
    }

    // When lFactory gets out of scope,
    // the Factory is destroyed and all that is responsible for as well.

    // Should never get here, except if Init() went wrong.
    return 1;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
