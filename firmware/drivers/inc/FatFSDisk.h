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
#include <memory>
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
    virtual ~FatFSDisk() = default;

    template <typename T, typename...Args>
    static void Create(Args&&... aArgs) {
        sDrives.emplace_back(
            std::make_unique<T>(
                typename T::UseCreateFunc{},
                std::forward<Args>(aArgs)...
            )
        );
    }

    static auto StaticGetDiskStatus(unsigned int aDriveIndex) -> DSTATUS;
    static auto StaticInitDisk(unsigned int aDriveIndex) -> DSTATUS;
    static auto StaticMountDisk(unsigned int aDriveIndex, FATFS * aFatFS) -> FRESULT;
    static auto StaticRdDisk(
        unsigned int aDriveIndex,
        uint8_t * aBuffer,
        uint32_t aStartSector,
        unsigned int aSectorCount
    ) -> DRESULT;
#if (FF_FS_READONLY == 0)
    static auto StaticWrDisk(
        unsigned int aDriveIndex,
        uint8_t const * aBuffer,
        uint32_t aStartSector,
        unsigned int aSectorCount
    ) -> DRESULT;
#endif

#if (FF_FS_READONLY == 0) || (FF_MAX_SS == FF_MIN_SS)
    static auto StaticIOCTL(
        unsigned int aDriveIndex,
        uint8_t aCmd,
        void * aBufPtr
    ) -> DRESULT;
#endif

    static auto GetDiskQty() noexcept -> unsigned int { return sDrives.size(); }
    auto GetDiskIndex() const noexcept -> unsigned int { return mDriveIndex; }

protected:
    struct UseCreateFunc {
        explicit UseCreateFunc() = default;
    };
    explicit FatFSDisk([[maybe_unused]] UseCreateFunc /* Dummy */, unsigned int aDriveIndex) noexcept
        : mDriveIndex{aDriveIndex} {}
    FatFSDisk(FatFSDisk const &) = delete;
    FatFSDisk &operator=(FatFSDisk const &) = delete;

private:
    virtual auto GetDiskStatus() -> DSTATUS = 0;
    virtual auto InitDisk() -> DSTATUS = 0;
    virtual auto RdDisk(
        uint8_t * aBuffer,
        uint32_t aStartSector,
        unsigned int aSectorCount
    ) -> DRESULT = 0;
#if (FF_FS_READONLY == 0)
    virtual auto WrDisk(
        uint8_t const * aBuffer,
        uint32_t aStartSector,
        unsigned int aSectorCount
    ) -> DRESULT = 0;
#endif

#if (FF_FS_READONLY == 0) || (FF_MAX_SS == FF_MIN_SS)
    virtual auto IOCTL(uint8_t aCmd, void * aBuffer) -> DRESULT = 0;
#endif

    unsigned int mDriveIndex{};

    using Ptr = std::unique_ptr<FatFSDisk>;
    static std::vector<Ptr> sDrives;
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
