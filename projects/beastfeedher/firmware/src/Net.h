#pragma once
// *******************************************************************************
//
// Project: PFPP.
//
// Module: Network functions.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// ******************************************************************************
//
//        Copyright (c) 2016-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
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
class CalendarRec;
class NetIFRec;
class FeedCfgRec;

namespace RTCC {
    namespace AO {
        class RTCC_AO;
    }
}

// ******************************************************************************
//                            EXPORTED VARIABLES
// ******************************************************************************

// ******************************************************************************
//                                 EXTERNS
// ******************************************************************************

// ******************************************************************************
//                            EXPORTED FUNCTIONS
// ******************************************************************************

namespace Net {

#if LWIP_HTTPD_SSI || LWIP_HTTPD_CGI
void InitCallback(
    std::shared_ptr<RTCC::AO::RTCC_AO> const &aRTCC_AO,
    std::shared_ptr<CalendarRec> const &aCalendar,
    std::shared_ptr<NetIFRec> const &aNetIFRec,
    std::shared_ptr<FeedCfgRec> const &aFeedCfgRec
);
#endif

} // namespace Net

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
