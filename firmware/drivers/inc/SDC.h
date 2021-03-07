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
//        Copyright (c) 2018-2019, Pleora Technologies, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <cstdint>

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
    class ISPIDev;
}


//! \brief Brief description.
//! Details follow...
//! ...here.
class SDC
    : public FatFSDisk {
 public:
    SDC(unsigned int const aDriveIx, CoreLink::ISPIDev &aSPIDev, GPIO const &aCSnPin);

    // FatFSDisk interface.
    DSTATUS GetDiskStatus(void) override;
    DSTATUS InitDisk(void) override;
    DRESULT RdDisk(
        uint8_t * const aBuffer,
        uint32_t aStartSector,
        unsigned int aSectorCount
    ) override;
#if (FF_FS_READONLY == 0)
    DRESULT WrDisk(
        uint8_t const * const aBuffer,
        uint32_t aStartSector,
        unsigned int aSectorCount
    ) override;
#endif

#if (FF_FS_READONLY == 0) || (FF_MAX_SS == FF_MIN_SS)
    DRESULT IOCTL(uint8_t aCmd, void * const aBuffer) override;
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
    void WaitReady(void);
    void PowerOn(void);
    void PowerOff(void);

    bool RxDataBlock(uint8_t *aBuffer, unsigned int aBlockLen);
#if (FF_FS_READONLY == 0)
    bool TxDataBlock(uint8_t const *aBuffer, uint8_t aToken);
#endif // FF_FS_READONLY

    R1_RESPONSE_PKT SendCmd(
        uint8_t aCmd,
        uint32_t aArg,
        uint8_t *aRegPtr = nullptr,
        unsigned int aRegLen = 0
    );
    bool IsExpectedVoltageRange(void);

    CoreLink::ISPIDev &mSPIDev;
    CoreLink::SPISlaveCfg mSPICfg;

    DSTATUS mStatus = STA_NOINIT;
    uint8_t mCardType = 0;

    static unsigned int constexpr mSPIBitRate = 4000000;
    static unsigned int constexpr sSectorSize = 512;
    static uint8_t constexpr sDummyByte = 0xFF;
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
