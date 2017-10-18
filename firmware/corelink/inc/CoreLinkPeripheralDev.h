#ifndef CORELINK_PERIPHERALS_DEV_H_
#define CORELINK_PERIPHERALS_DEV_H_
// *******************************************************************************
//
// Project: ARM Cortex-M.
//
// Module: CoreLink Peripherals.
//
// *******************************************************************************

//! \file
//! \brief Generic CoreLink peripheral device base class.
//! \ingroup corelink_peripherals


// ******************************************************************************
//
//        Copyright (c) 2015-2016, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <stdint.h>

namespace CoreLink {

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

typedef uint32_t volatile reg_t;

struct ID_REG_MAP_STRUCT_TAG {
  reg_t mPeripheralID[4];
  reg_t mPrimeCellID[4];
};

typedef struct ID_REG_MAP_STRUCT_TAG id_reg_map_t;


//! \class PeripheralDev
// \brief CoreLink Peripheral Device base class.
//
// This is the base class from which all CoreLink peripheral device drivers
// are derived.
//
class PeripheralDev {

 public:
  PeripheralDev(uint32_t aBaseAddr);
  virtual ~PeripheralDev() = 0;

  //! \brief Get base address of peripheral device.
  //
  // \pre -
  // \post -
  // \return Base address of the device in CPU address space.
  //
  uint32_t GetBaseAddr(void) { return mBaseAddr; }

  //bool IsExpectedPeripheralID(void) { return (mPeripheralID[0] == mExpectedPeripheralID[1]); }
  //bool IsExpectedPrimeCellID(void)  { return (mPrimeCellID == mExpectedPrimeCellID); }

 private:
  uint32_t mBaseAddr;

  //static uint32_t mExpectedPeripheralID[1];
  //static uint32_t mExpectedPrimeCellID;

  uint32_t mPeripheralID[1];
  uint32_t mPrimeCellID;
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
} // namespace
#endif // CORELINK_PERIPHERALS_H_
