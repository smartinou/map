#pragma once
// *******************************************************************************
//
// Project: PFPP.
//
// Module: BSP.
//
// *******************************************************************************

//! \file
//! \brief BSP class.
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
class IBSPFactory;


namespace BSP {

// Non-member functions.
std::shared_ptr<IBSPFactory> Init(void);

} // namespace BSP

// ******************************************************************************
//                            EXPORTED VARIABLES
// ******************************************************************************

namespace BSP {
  unsigned int constexpr TICKS_PER_SEC = 100;
  unsigned int constexpr MS_PER_TICK = 1 / 100 * 1000;
} // namespace BSP

// ******************************************************************************
//                                 EXTERNS
// ******************************************************************************

// ******************************************************************************
//                            EXPORTED FUNCTIONS
// ******************************************************************************

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
