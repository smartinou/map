#pragma once
// *******************************************************************************
//
// Project: Beast Feed'Her.
//
// Module: BSP class.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2019, Martin Garon, All rights reserved.
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
class IRTCC;
class IFS;
class ILCD;
class GPIOs;
class SDC;


namespace CoreLink {
  class SSIPinCfg;
  class SPISlaveCfg;
  class SPIDev;
}


// Consider namespace App or BSP.

class IBSPFactory {
public:
  virtual CoreLink::SPIDev * CreateSPIDev(void) = 0;

  virtual IRTCC * CreateRTCC(CoreLink::SPIDev &aSPIDev) = 0;
  virtual ILCD * CreateDisplay(CoreLink::SPIDev &aSPIDev) = 0;
  virtual SDC * CreateSDC(CoreLink::SPIDev &aSPIDev) = 0;

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
