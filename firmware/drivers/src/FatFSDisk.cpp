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
//        Copyright (c) 2018-2022, Pleora Technologies, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

// Standard libraries.
#include <algorithm>
#include <limits>
#include <string>

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

std::vector<FatFSDisk::Ptr> FatFSDisk::sDrives;
unsigned int FatFSDisk::mMountedDiskIndex{std::numeric_limits<unsigned int>::max()};

// ******************************************************************************
//                            EXPORTED FUNCTIONS
// ******************************************************************************

auto FatFSDisk::StaticGetDiskStatus(unsigned int const aDriveIndex) -> DSTATUS {
    return sDrives.at(aDriveIndex)->GetDiskStatus();
}


auto FatFSDisk::StaticInitDisk(unsigned int const aDriveIndex) -> DSTATUS {
    return sDrives.at(aDriveIndex)->InitDisk();
}


auto FatFSDisk::StaticMountDisk(
    unsigned int const aDriveIndex,
    FATFS * const aFatFS
) -> FRESULT {

    if (aDriveIndex < sDrives.size()) {
        if (aDriveIndex == mMountedDiskIndex) {
            // Disk already mounted.
            return FR_OK;
        }

        static constexpr auto sForceMount{1};
        auto const lResult = f_mount(
            aFatFS,
            [aDriveIndex]() -> auto {
               return (aDriveIndex == 0) ? "" : std::to_string(aDriveIndex).c_str();
            } (),
            sForceMount
        );
        if (lResult == FR_OK) {
            mMountedDiskIndex = aDriveIndex;
        }

        return lResult;
    }

    return FR_INVALID_DRIVE;
}


auto FatFSDisk::StaticRdDisk(
    unsigned int const aDriveIndex,
    uint8_t * const aBuffer,
    uint32_t const aStartSector,
    unsigned int const aSectorCount
) -> DRESULT {
    return sDrives.at(aDriveIndex)->RdDisk(aBuffer, aStartSector, aSectorCount);
}


#if (FF_FS_READONLY == 0)
auto FatFSDisk::StaticWrDisk(
    unsigned int const aDriveIndex,
    uint8_t const * const aBuffer,
    uint32_t const aStartSector,
    unsigned int const aSectorCount
) -> DRESULT {
    return sDrives.at(aDriveIndex)->WrDisk(aBuffer, aStartSector, aSectorCount);
}
#endif // FF_FS_READONLY


#if (FF_FS_READONLY == 0) || (FF_MAX_SS == FF_MIN_SS)
auto FatFSDisk::StaticIOCTL(
    unsigned int const aDriveIndex,
    uint8_t const aCmd,
    void * const aBuffer
) -> DRESULT {
    return sDrives.at(aDriveIndex)->IOCTL(aCmd, aBuffer);
}
#endif

// ******************************************************************************
//                              LOCAL FUNCTIONS
// ******************************************************************************

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
