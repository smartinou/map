#ifndef LWIP_MGR_EVT_H_
#define LWIP_MGR_EVT_H_
// *******************************************************************************
//
// Project: Active Object Library
//
// Module: LwIP manager QP Events.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2017, Martin Garon, All rights reserved.
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

// Forward declarations.

// Class definitions.
class LwIPInitEvt : public QP::QEvt {
 public:
  LwIPInitEvt(QP::QSignal aSig,
              uint32_t    aIPAddr,
              uint32_t    aSubnetMask,
              uint32_t    aGWAddr) {
    sig         = aSig;
    mIPAddr     = aIPAddr;
    mSubnetMask = aSubnetMask;
    mGWAddr     = aGWAddr;
  }

 public:
  uint32_t mIPAddr;
  uint32_t mSubnetMask;
  uint32_t mGWAddr;
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
#endif // LWIP_MGR_EVT_H_
