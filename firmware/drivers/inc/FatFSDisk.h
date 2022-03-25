#pragma once
// *******************************************************************************
//
// Project: Drivers.
//
// Module: FAT FS.
//
// *******************************************************************************

//! \file
//! \brief Media Access Interface base class.
//! \ingroup ext_peripherals

// ******************************************************************************
//
//        Copyright (c) 2018-2022, Pleora Technologies, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <cstdint>
#include <vector>

// FatFS.
#include "diskio.h"
#include "ff.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class FatFSDisk {
public:
    static DSTATUS StaticGetDiskStatus(unsigned int aDriveIndex);
    static DSTATUS StaticInitDisk(unsigned int aDriveIndex);
    static FRESULT StaticMountDisk(unsigned int aDriveIndex, FATFS * const aFatFS);
    static DRESULT StaticRdDisk(
        unsigned int aDriveIndex,
        uint8_t * const aBuffer,
        uint32_t aStartSector,
        unsigned int aSectorCount
    );
#if (FF_FS_READONLY == 0)
    static DRESULT StaticWrDisk(
        unsigned int aDriveIndex,
        uint8_t const * const aBuffer,
        uint32_t aStartSector,
        unsigned int aSectorCount
    );
#endif

#if (FF_FS_READONLY == 0) || (FF_MAX_SS == FF_MIN_SS)
    static DRESULT StaticIOCTL(unsigned int aDriveIndex, uint8_t aCmd, void * const aBufPtr);
#endif

    static unsigned int GetDiskQty(void) { return mVector.size(); }

protected:
     FatFSDisk();

private:
     FatFSDisk(FatFSDisk const &) = delete;
     FatFSDisk &operator=(FatFSDisk const &) = delete;

     virtual DSTATUS GetDiskStatus(void) = 0;
     virtual DSTATUS InitDisk(void) = 0;
     virtual DRESULT RdDisk(
         uint8_t * const aBuffer,
         uint32_t aStartSector,
         unsigned int aSectorCount
     ) = 0;
#if (FF_FS_READONLY == 0)
     virtual DRESULT WrDisk(
         uint8_t const * const aBuffer,
         uint32_t aStartSector,
         unsigned int aSectorCount
     ) = 0;
#endif

#if (FF_FS_READONLY == 0) || (FF_MAX_SS == FF_MIN_SS)
     virtual DRESULT IOCTL(uint8_t aCmd, void * const aBuffer) = 0;
#endif

     unsigned int mDriveIndex = 0;

     static std::vector<FatFSDisk *> mVector;
     static unsigned int mMountedDiskIndex;
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
