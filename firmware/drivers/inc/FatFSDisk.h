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
//        Copyright (c) 2018-2019, Pleora Technologies, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <cstdint>
#include <vector>

#include "diskio.h"

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
    static DRESULT StaticIOCTL(unsigned int aDriveIndex, BYTE aCmd, void * const aBufPtr);
#endif

    unsigned int GetDiskQty(void) const { return mVector.size(); }

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
