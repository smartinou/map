#pragma once
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

#include <cstdint>
#include <memory>

#include "FatFSDisk.h"

#include "inc/GPIO.h"
#include "SPISlaveCfg.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// Forward declaration.
namespace CoreLink {
    class ISPIMasterDev;
}


//! \brief MicroSD card driver class.
class SDC final
    : public FatFSDisk {
protected:
    template<class T, typename...Args>
    friend void FatFSDisk::Create(Args&&... aArgs);

public:
    explicit SDC(
        UseCreateFunc const aDummy,
        unsigned int aDriveIx,
        std::shared_ptr<CoreLink::ISPIMasterDev> aSPIMasterDev,
        GPIO const &aCSnPin,
        GPIO const &aDetectPin,
        unsigned int aSPIBitRate = 4000000UL
    );

    // FatFSDisk interface.
    auto GetDiskStatus() -> DSTATUS final;
    auto InitDisk() -> DSTATUS final;
    auto RdDisk(
        uint8_t * aBuffer,
        uint32_t aStartSector,
        unsigned int aSectorCount
    ) -> DRESULT final;
#if (FF_FS_READONLY == 0)
    auto WrDisk(
        uint8_t const * aBuffer,
        uint32_t aStartSector,
        unsigned int aSectorCount
    ) -> DRESULT final;
#endif

#if (FF_FS_READONLY == 0) || (FF_MAX_SS == FF_MIN_SS)
    auto IOCTL(uint8_t aCmd, void * aBuffer) -> DRESULT final;
#endif

 private:
    // MMC card type flags.
    enum {
        CT_MMC   = 0x01, // MMC ver 3.
        CT_SD1   = 0x02, // SD  ver 1.
        CT_SD2   = 0x04, // SD  ver 2.
        CT_SDC   = (CT_SD1 | CT_SD2), // SD.
        CT_BLOCK = 0x08  // Block addressing.
    };

    using R1_RESPONSE_PKT = uint8_t;
    using R1B_RESPONSE_PKT = uint8_t;

    // CID, CSD register.
    struct R2_RESPONSE_PKT_STRUCT_TAG {
        R1_RESPONSE_PKT mR1{};
        uint8_t mR2{};
    };

    // OCR register.
    struct R3_RESPONSE_PKT_STRUCT_TAG {
        R1_RESPONSE_PKT mR1{};
        uint32_t mOCR{};
    };

    // Card interface condition.
    struct R7_RESPONSE_PKT_STRUCT_TAG {
        R1_RESPONSE_PKT mR1{};
        uint32_t mIFCond{};
    };

    using R2_RESPONSE_PKT = struct R2_RESPONSE_PKT_STRUCT_TAG;
    using R3_RESPONSE_PKT = struct R3_RESPONSE_PKT_STRUCT_TAG ;
    using R7_RESPONSE_PKT = struct R7_RESPONSE_PKT_STRUCT_TAG;

    bool Select();
    void Deselect();
    void WaitReady();
    void PowerOn();
    void PowerOff();

    bool RxDataBlock(uint8_t *aBuffer, unsigned int aBlockLen);
#if (FF_FS_READONLY == 0)
    bool TxDataBlock(uint8_t const *aBuffer, uint8_t aToken);
#endif // FF_FS_READONLY

    auto SendCmd(
        uint8_t aCmd,
        uint32_t aArg,
        uint8_t *aRegPtr = nullptr,
        unsigned int aRegLen = 0
    ) -> R1_RESPONSE_PKT;
    bool IsExpectedVoltageRange();

    std::shared_ptr<CoreLink::ISPIMasterDev> mSPIMasterDev;
    CoreLink::SPISlaveCfg mSPISlaveCfg;
    GPIO mDetectPin{ {}, {} };
    unsigned int mSPIBitRate{};

    DSTATUS mStatus{STA_NOINIT};
    uint8_t mCardType{0};

    static constexpr unsigned int sSectorSize{512};
    static constexpr uint8_t sDummyByte{0xFF};
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
