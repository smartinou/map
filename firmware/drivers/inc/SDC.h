#pragma once
// *******************************************************************************
//
// Project: Drivers.
//
// Module: SDC disk driver.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2018, Pleora Technologies, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <cstdint>

#include "diskio.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// Forward declaration.
namespace CoreLink {
  class SPIDev;
  class SPISlaveCfg;
}


//! \brief Brief description.
//! Details follow...
//! ...here.
class SDC {
 public:
  SDC(
    unsigned int           aDriveIx,
    CoreLink::SPIDev      &aSPIDev,
    CoreLink::SPISlaveCfg &aSPISlaveCfg);

  //static SDC *GetInstancePtr(unsigned int aDriveIx);

  DSTATUS GetDiskStatus(void);
  DSTATUS DiskInit(void);
  DRESULT DiskRd(
    uint8_t     *aBufPtr,
    uint32_t     aStartSector,
    unsigned int aSectorCount);
#if (FF_FS_READONLY == 0)
  DRESULT DiskWr(
    uint8_t const *aBufPtr,
    uint32_t       aStartSector,
    unsigned int   aSectorCount);
#endif // _DISKIO_WRITE

#if _DISKIO_IOCTL
  DRESULT DiskIOCTL(BYTE aCmd, void *aBufPtr);
#endif // _DISKIO_IOCTL


 private:
  // MMC card type flags.
  enum {
    CT_MMC   = 0x01, // MMC ver 3.
    CT_SD1   = 0x02, // SD  ver 1.
    CT_SD2   = 0x04, // SD  ver 2.
    CT_SDC   = (CT_SD1 | CT_SD2), // SD.
    CT_BLOCK = 0x08  // Block addressing.
  };

  typedef uint8_t R1_RESPONSE_PKT;
  typedef uint8_t R1B_RESPONSE_PKT;

  // CID, CSD register.
  struct R2_RESPONSE_PKT_STRUCT_TAG {
    R1_RESPONSE_PKT mR1;
    uint8_t mR2;
  };

  // OCR register.
  struct R3_RESPONSE_PKT_STRUCT_TAG {
    R1_RESPONSE_PKT mR1;
    uint32_t mOCR;
  };

  // Card interface condition.
  struct R7_RESPONSE_PKT_STRUCT_TAG {
    R1_RESPONSE_PKT mR1;
    uint32_t mIFCond;
  };

  typedef struct R2_RESPONSE_PKT_STRUCT_TAG R2_RESPONSE_PKT;
  typedef struct R3_RESPONSE_PKT_STRUCT_TAG R3_RESPONSE_PKT;
  typedef struct R7_RESPONSE_PKT_STRUCT_TAG R7_RESPONSE_PKT;

  bool Select(void);
  void Deselect(void);
  //int  WaitReady(unsigned int aTimeout);
  void WaitReady(void);
  void PowerOn(void);
  void PowerOff(void);

  bool RxDataBlock(uint8_t *aBufPtr, unsigned int aBlockLen);
#if (FF_FS_READONLY == 0)
  bool TxDataBlock(uint8_t const *aBufPtr, uint8_t aToken);
#endif // FF_FS_READONLY

  //R1_RESPONSE_PKT SendCmd(uint8_t aCmd, uint32_t aArg);
  R1_RESPONSE_PKT SendCmd(
    uint8_t      aCmd,
    uint32_t     aArg,
    uint8_t     *aRegPtr = nullptr,
    unsigned int aRegLen = 0);
  bool IsExpectedVoltageRange(void);

  unsigned int           mMyDriveIx;
  CoreLink::SPIDev      &mSPIDev;
  CoreLink::SPISlaveCfg &mSPISlaveCfg;

  DSTATUS      mStatus;
  uint8_t      mCardType;

  static unsigned int const sSectorSize = 512;
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
