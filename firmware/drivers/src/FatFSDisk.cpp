// *******************************************************************************
//
// Project: Drivers.
//
// Module: SDC disk driver.
//
// *******************************************************************************

//! \file
//! \brief SD card class.
//! \ingroup ext_peripherals

// ******************************************************************************
//
//        Copyright (c) 2018-2019, Pleora Technologies, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

// This module.
#include "FatFSDisk.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// ******************************************************************************
//                            FUNCTION PROTOTYPES
// ******************************************************************************

// ******************************************************************************
//                             GLOBAL VARIABLES
// ******************************************************************************

std::vector<FatFSDisk *> FatFSDisk::mVector;

// ******************************************************************************
//                            EXPORTED FUNCTIONS
// ******************************************************************************

DSTATUS FatFSDisk::StaticGetDiskStatus(unsigned int aDriveIndex) {
    if (aDriveIndex < mVector.size()) {
        return mVector[aDriveIndex]->GetDiskStatus();
    }

    return RES_PARERR;
}


DSTATUS FatFSDisk::StaticInitDisk(unsigned int aDriveIndex) {
    if (aDriveIndex < mVector.size()) {
        return mVector[aDriveIndex]->InitDisk();
    }

    return RES_PARERR;
}


DRESULT FatFSDisk::StaticRdDisk(
    unsigned int aDriveIndex,
    uint8_t * const aBuffer,
    uint32_t aStartSector,
    unsigned int aSectorCount
) {
    if (aDriveIndex < mVector.size()) {
        return mVector[aDriveIndex]->RdDisk(aBuffer, aStartSector, aSectorCount);
    }

    return RES_PARERR;
}

#if (FF_FS_READONLY == 0)
DRESULT FatFSDisk::StaticWrDisk(
    unsigned int aDriveIndex,
    uint8_t const * const aBuffer,
    uint32_t aStartSector,
    unsigned int aSectorCount
) {
    if (aDriveIndex < mVector.size()) {
        return mVector[aDriveIndex]->WrDisk(aBuffer, aStartSector, aSectorCount);
    }

    return RES_PARERR;
}
#endif // FF_FS_READONLY


#if (FF_FS_READONLY == 0) || (FF_MAX_SS == FF_MIN_SS)
DRESULT FatFSDisk::StaticIOCTL(unsigned int aDriveIndex, uint8_t aCmd, void * const aBuffer) {
    if (aDriveIndex < mVector.size()) {
        return mVector[aDriveIndex]->IOCTL(aCmd, aBuffer);
    }

    return RES_PARERR;
}
#endif

// ******************************************************************************
//                              LOCAL FUNCTIONS
// ******************************************************************************

FatFSDisk::FatFSDisk()
    : mDriveIndex(mVector.size()) {

    // Register new object into vector.
    mVector.push_back(this);
}

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
