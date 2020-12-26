#pragma once
// *******************************************************************************
//
// Project: PFPP.
//
// Module: BSP.
//
// *******************************************************************************

//! \file
//! \brief BSP Factory class.
//! \ingroup application_bsp

// ******************************************************************************
//
//        Copyright (c) 2019, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <memory>

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// Forward declaration.
namespace QP {
    class QActive;
}


// Consider namespace App or BSP.

class IBSPFactory {
public:
    virtual ~IBSPFactory() {}

    virtual std::shared_ptr<RTCC::AO::RTCC_AO> CreateRTCCAO(void) = 0;
    virtual QP::QActive *GetOpaqueRTCCAO(void) = 0;
    virtual QP::QActive *GetOpaqueBLEAO(void) = 0;
    virtual unsigned int CreateDisks(void) = 0;

    virtual std::shared_ptr<QP::QActive> CreateLogFileSinkAO(void) = 0;
    virtual std::shared_ptr<QP::QActive> CreatePFPPAO(FeedCfgRec &aFeedCfgRec) = 0;
    virtual QP::QActive *GetOpaquePFPPAO(void) = 0;
    virtual std::shared_ptr<QP::QActive> CreateDisplayMgrAO(void) = 0;
    virtual QP::QActive *GetOpaqueDisplayMgrAO(void) = 0;
    virtual std::shared_ptr<QP::QActive> CreateLwIPMgrAO(void) = 0;
    virtual std::shared_ptr<QP::QActive> CreateBLEAO(void) = 0;
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
