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

// Corelink driver.
//#include "SPI.h"

// This module.
#include "SDC.h"

using namespace CoreLink;

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

enum L_SPI_SLAVE_CFG_ENUM_TAG {
    L_CLK_FREQ_ID_MODE   = 400000UL,
    L_CLK_FREQ_FAST_MODE = 25000000UL
};


enum {
    L_CMD0_PARAM = 0x00000000UL,
    // [31:12]: Reserved.
    // [11:8]: VHS (Host supplied voltage range). Set to '1'.
    // [7:0]: Check pattern.
    L_CMD8_PARAM  = 0x000001AAUL,

    // [31:0]: Stuff bits.
    L_CMD58_PARAM = 0x00000000UL,

    // [31]: Reserved.
    // [30]: HCS (Host Capacity Support): SDHC/SDXC.
    // [29:0]: Reserved.
    L_ACMD41_PARAM = (0x1 << 30),

    // [31:28]: Command version.
    // [27:12]: Reserved.
    // [11:8]:  Voltage accepted.
    // [7:0]:   Check pattern.
    L_R7_RESPONSE = L_CMD8_PARAM,
};


// R1 response bit masks.
enum L_R1_MASK_ENUM_TAG {
    L_R1_MASK_IN_IDLE_STATE = (0x1 << 0),
    L_R1_MASK_ERASE_RESET   = (0x1 << 1),
    L_R1_MASK_ILLEGAL_CMD   = (0x1 << 2),
    L_R1_MASK_COM_CRC_ERR   = (0x1 << 3),
    L_R1_MASK_ERASE_SEQ_ERR = (0x1 << 4),
    L_R1_MASK_ADD_ERR       = (0x1 << 5),
    L_R1_MASK_PARAM_ERR     = (0x1 << 6),
    L_R1_MASK_BUSY          = (0x1 << 7)
};


// OCR register bit masks.
enum L_OCR_MASK_ENUM_TAG {
    L_OCR_MASK_27_28 = (0x1 << 15),
    L_OCR_MASK_28_29 = (0x1 << 16),
    L_OCR_MASK_29_30 = (0x1 << 17),
    L_OCR_MASK_30_31 = (0x1 << 18),
    L_OCR_MASK_31_32 = (0x1 << 19),
    L_OCR_MASK_32_33 = (0x1 << 20),
    L_OCR_MASK_33_34 = (0x1 << 21),
    L_OCR_MASK_34_35 = (0x1 << 22),
    L_OCR_MASK_35_36 = (0x1 << 23),
    L_OCR_MASK_S18A  = (0x1 << 24),
    L_OCR_MASK_UCS   = (0x1 << 29),
    L_OCR_MASK_CCS   = (0x1 << 30),
    L_OCR_MASK_BUSY  = (0x1 << 31)
};

// MMC/SD command.
#define CMD0    (0)             // GO_IDLE_STATE
#define CMD1    (1)             // SEND_OP_COND (MMC)
#define ACMD41  (0x80+41)       // SEND_OP_COND (SDC)
#define CMD8    (8)             // SEND_IF_COND
#define CMD9    (9)             // SEND_CSD
#define CMD10   (10)            // SEND_CID
#define CMD12   (12)            // STOP_TRANSMISSION
#define ACMD13  (0x80+13)       // SD_STATUS (SDC)
#define CMD16   (16)            // SET_BLOCKLEN
#define CMD17   (17)            // READ_SINGLE_BLOCK
#define CMD18   (18)            // READ_MULTIPLE_BLOCK
#define CMD23   (23)            // SET_BLOCK_COUNT (MMC)
#define ACMD23  (0x80+23)       // SET_WR_BLK_ERASE_COUNT (SDC)
#define CMD24   (24)            // WRITE_BLOCK
#define CMD25   (25)            // WRITE_MULTIPLE_BLOCK
#define CMD32   (32)            // ERASE_ER_BLK_START
#define CMD33   (33)            // ERASE_ER_BLK_END
#define CMD38   (38)            // ERASE
#define CMD48   (48)            // READ_EXTR_SINGLE
#define CMD49   (49)            // WRITE_EXTR_SINGLE
#define CMD55   (55)            // APP_CMD
#define CMD58   (58)            // READ_OCR

#define L_DATA_TOKEN_CMD25 (0xFC)
#define L_STOP_TOKEN_CMD25 (0xFD)
#define L_DATA_TOKEN_OTHER (0xFE)


#define REG32TOH(num)         \
    (((num>>24)&0x000000ff) | \
    ((num<<8)&0x00ff0000)  | \
    ((num>>8)&0x0000ff00)  | \
    ((num<<24)&0xff000000))

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// ******************************************************************************
//                            FUNCTION PROTOTYPES
// ******************************************************************************

// ******************************************************************************
//                             GLOBAL VARIABLES
// ******************************************************************************

// ******************************************************************************
//                            EXPORTED FUNCTIONS
// ******************************************************************************

SDC::SDC(unsigned int const aDriveIx, CoreLink::SPIDev &aSPIDev, GPIOs const aCSnPin)
    : mMyDriveIx(aDriveIx)
    , mSPIDev(aSPIDev)
    , mSPICfg()
    , mStatus(STA_NOINIT)
    , mCardType(0) {

    // Ctor body.
    // Create an SPI slave to operate at maximum device speed.
    mSPICfg.SetProtocol(CoreLink::SPISlaveCfg::MOTO_1);
    mSPICfg.SetBitRate(4000000);
    mSPICfg.SetDataWidth(8);
    mSPICfg.SetCSnGPIO(aCSnPin.GetPort(), aCSnPin.GetPin());
}


DSTATUS SDC::GetDiskStatus(void) {
    return mStatus;
}


DSTATUS SDC::DiskInit(void) {

    PowerOn();
    if (mStatus & STA_NODISK) {
        return mStatus;
    }

    // Set clock to Clock Frequency Identification Mode Max (400KHz).
    mSPICfg.SetBitRate(L_CLK_FREQ_ID_MODE);

    // Send 80 dummy clocks, CSn de-asserted.
    mSPICfg.DeassertCSn();
    for (unsigned int lByteIx = 0; lByteIx < (80/8); lByteIx++) {
        mSPIDev.PushPullByte(0xFF);
    }

    // Put the card in SPI mode.
    R1_RESPONSE_PKT lR1 = SendCmd(CMD0, L_CMD0_PARAM);
    if (L_R1_MASK_IN_IDLE_STATE == lR1) {
        R7_RESPONSE_PKT lR7 = {0};
        lR7.mR1 = SendCmd(CMD8, L_CMD8_PARAM, reinterpret_cast<uint8_t *>(&lR7.mIFCond), sizeof(lR7.mIFCond));
        if (L_R1_MASK_IN_IDLE_STATE == lR7.mR1) {
            // Ver2.00 or later memory card.
            // Valid response?
            if (L_R7_RESPONSE == REG32TOH(lR7.mIFCond)) {
                // Compatible voltage range and check pattern correct.
                // Read OCR.
                R3_RESPONSE_PKT lR3 = {0};
                lR3.mR1 = SendCmd(CMD58, L_CMD58_PARAM, reinterpret_cast<uint8_t *>(&lR3.mOCR), sizeof(lR3.mOCR));

                // Compare with expected voltage range.
                if (IsExpectedVoltageRange()) {

                    do {
                        // Start initialization and wait for it to complete.
                        // TODO: add 1sec timeout.
                        lR1 = SendCmd(ACMD41, L_ACMD41_PARAM);
                    } while (0 != lR1);

                    lR3.mR1 = SendCmd(CMD58, L_CMD58_PARAM, reinterpret_cast<uint8_t *>(&lR3.mOCR), sizeof(lR3.mOCR));
                    if (REG32TOH(lR3.mOCR) & L_OCR_MASK_CCS) {
                        // Ver2.00 or later High Capacity | Extended Capacity SD Memory Card.
                        mCardType = CT_SD2 | CT_BLOCK;
                    } else {
                        // Ver2.00 or later Standard Capacity SD Memory Card.
                        mCardType = CT_SD2;
                        // Force block size to 512 bytes to work with FatFS.
                        lR1 = SendCmd(CMD16, 512);
                    }

                    mStatus &= ~STA_NOINIT;
                } else {
                    // Card with non compatible voltage range.
                    mCardType = 0;
                }

            } else {
                // Non-compatible voltage range or check pattern error.
             mStatus = STA_NOINIT;
            }
        } else {
            // Illegal command:
            // Ver1.X SD memory card, or not SD memory card.

            // Read OCR.
            R3_RESPONSE_PKT lR3 = {0};
            lR3.mR1 = SendCmd(CMD58, L_CMD58_PARAM, reinterpret_cast<uint8_t *>(&lR3.mOCR), sizeof(lR3.mOCR));
            if (L_R1_MASK_IN_IDLE_STATE == lR3.mR1) {
                // Compare with expected voltage range.
                if (IsExpectedVoltageRange()) {

                    uint8_t lCmd = ACMD41;
                    lR1 = SendCmd(ACMD41, L_ACMD41_PARAM);
                    mCardType = CT_SD1;
                    if ((0x00 != lR1) && (L_R1_MASK_IN_IDLE_STATE != lR1)) {
                        lCmd = CMD1;
                        mCardType = CT_MMC;
                    }

                    do {
                        // Start initialization and wait for it to complete.
                        // TODO: add 1sec timeout.
                        lR1 = SendCmd(lCmd, 0x00000000);
                    } while (0 != lR1); // && !Timeout);

                    unsigned int Timeout = 1;
                    if (!Timeout) {
                        // Force block size to 512 bytes to work with FatFS.
                        lR1 = SendCmd(CMD16, 512);
                    } else {
                        mCardType = 0;
                    }

                    // Ver1.x Standard Capacity SD Memory Card.
                    mStatus &= ~STA_NOINIT;
                } else {
                    // Card with non compatible voltage range.
                    mCardType = 0;
                }

            } else {
                // Illegal command: Not SD memory card.
                mStatus = STA_NODISK;
            }
        }
    }

    // Put the card in SPI state.

    return mStatus;
}


DRESULT SDC::DiskRd(
    uint8_t     *aBufPtr,
    uint32_t     aStartSector,
    unsigned int aSectorCount) {

    // Check parameter.
    //if (drv || !aSectorCount) {
    if (!aSectorCount) {
        return RES_PARERR;
    }

    // Check if drive is ready.
    if (mStatus & STA_NOINIT) {
        return RES_NOTRDY;
    }

    if (!(mCardType & CT_BLOCK)) {
        // LBA ot BA conversion (byte addressing cards).
        aStartSector *= SDC::sSectorSize;
    }

    if (aSectorCount == 1) {
        // Single sector read.
        // READ_SINGLE_BLOCK.
        if ((SendCmd(CMD17, aStartSector) == 0)
	        && RxDataBlock(aBufPtr, SDC::sSectorSize)) {
            aSectorCount = 0;
        }
    } else {
        // Multiple sector read.
        if (SendCmd(CMD18, aStartSector) == 0) {
            // READ_MULTIPLE_BLOCK.
            do {
	            if (!RxDataBlock(aBufPtr, SDC::sSectorSize)) {
                    break;
                }
	            aBufPtr += SDC::sSectorSize;
            } while (--aSectorCount);

            // STOP_TRANSMISSION.
            SendCmd(CMD12, 0);
        }
    }

    Deselect();
    if (aSectorCount) {
        return RES_ERROR;
    }

    return RES_OK;
}


#if (FF_FS_READONLY == 0)
DRESULT SDC::DiskWr(
    uint8_t const *aBufPtr,
    uint32_t       aStartSector,
    unsigned int   aSectorCount) {

    if (mStatus & STA_NOINIT) {
        // Check drive status.
        return RES_NOTRDY;
    }

    if (mStatus & STA_PROTECT) {
        // Check write protect.
        return RES_WRPRT;
    }

    if (!(mCardType & CT_BLOCK)) {
        // LBA ==> BA conversion (byte addressing cards).
        aStartSector *= SDC::sSectorSize;
    }

    if (aSectorCount == 1) {
        // Single sector write.
        // WRITE_BLOCK.
        if ((SendCmd(CMD24, aStartSector) == 0)
	        && TxDataBlock(aBufPtr, L_DATA_TOKEN_OTHER)) {
            aSectorCount = 0;
        }
    } else {
        // Multiple sector write.
        if (mCardType & CT_SDC) {
            // Predefine number of sectors.
            SendCmd(ACMD23, aSectorCount);
        }

        // WRITE_MULTIPLE_BLOCK.
        if (SendCmd(CMD25, aStartSector) == 0) {
            do {
	            if (!TxDataBlock(aBufPtr, L_DATA_TOKEN_CMD25)) {
                    break;
                }
	            aBufPtr += SDC::sSectorSize;
            } while (--aSectorCount);

            // STOP_TRAN token.
            if (!TxDataBlock(nullptr, L_STOP_TOKEN_CMD25)) {
                aSectorCount = 1;
            }
        }
    }

    Deselect();
    if (aSectorCount) {
        return RES_ERROR;
    }

    return RES_OK;
}
#endif // FF_FS_READONLY


#if _DISKIO_IOCTL
DRESULT SDC::DiskIOCTL(uint8_t aCmd, void *aBufPtr) {

}
#endif // _DISKIO_IOCTL

// ******************************************************************************
//                              LOCAL FUNCTIONS
// ******************************************************************************

bool SDC::Select(void) {

    // Assert CSn.
    // Dummy clock: force DO enabled.
    mSPICfg.AssertCSn();
    mSPIDev.PushPullByte(0xFF);

    if (1) {
        // Leading busy check.
        WaitReady();
        return true;
    }

    mSPICfg.DeassertCSn();
    return false;
}


void SDC::Deselect(void) {
    // Deassert CSn.
    // Dummy clock: force DO high-Z for multiple slave SPI.
    mSPICfg.DeassertCSn();
    mSPIDev.PushPullByte(0xFF);
}


void SDC::WaitReady(void) {

    // Wait until busy to deassert (DO going '1').
    uint8_t lVal = 0;
    do {
        lVal = mSPIDev.PushPullByte(0xFF);
    } while (lVal != 0xFF);
}


void SDC::PowerOn(void) {

}


void SDC::PowerOff(void) {

}


bool SDC::RxDataBlock(uint8_t *aBufPtr, unsigned int aBlockLen) {

    // Wait for DataStart token in timeout of 200ms.
    uint8_t lToken = 0x00;
    unsigned int Timer1 = 200;
    do {
        lToken = mSPIDev.PushPullByte(0xFF);
        // This loop will take a time.
        // Insert rot_rdq() here for multitask envilonment.
    } while ((lToken == 0xFF) && Timer1);

    if (lToken != L_DATA_TOKEN_OTHER) {
        // Function fails if invalid DataStart token or timeout.
        return false;
    }

    // Store trailing data to the buffer.
    mSPIDev.RdData(aBufPtr, aBlockLen, mSPICfg);

    // Discard CRC.
    mSPIDev.PushPullByte(0xFF);
    mSPIDev.PushPullByte(0xFF);

    return true;
}


#if (FF_FS_READONLY == 0)
bool SDC::TxDataBlock(uint8_t const *aBufPtr, uint8_t aToken) {

    // Wait for card ready.
    if (0) {//!wait_ready(500)) {
        return false;
    }

    // Send token byte.
    mSPIDev.PushPullByte(aToken);

    // Send data if token is other than StopTran.
    if (aToken != L_STOP_TOKEN_CMD25) {
        // Data.
        mSPIDev.WrData(aBufPtr, SDC::sSectorSize, mSPICfg);
        // Dummy CRC.
        mSPIDev.PushPullByte(0xFF);
        mSPIDev.PushPullByte(0xFF);

        // Receive data resp.
        uint8_t lVal = mSPIDev.PushPullByte(0xFF);
        if ((lVal & 0x1F) != 0x05) {
            // Function fails if the data packet was not accepted.
            return false;
        }
    }

    return true;
}
#endif // FF_FS_READONLY


SDC::R1_RESPONSE_PKT SDC::SendCmd(
    uint8_t      aCmd,
    uint32_t     aArg,
    uint8_t     *aRegPtr,
    unsigned int aRegLen) {

    // Send a CMD55 prior to ACMD<n>.
    if (aCmd & 0x80) {
        aCmd &= 0x7F;
        R1_RESPONSE_PKT lR1 = SendCmd(CMD55, 0x00000000);
        // Return if there's an error.
        if (lR1 != L_R1_MASK_IN_IDLE_STATE) {
            return lR1;
        }
    }

    // Select the card and wait for ready,
    // except to stop multiple block read.
    if (CMD12 != aCmd) {
        Select();
    }


    // Send command packet: start + command.
    static uint8_t const sTransmitBit = 0x40;
    mSPIDev.PushPullByte(aCmd | sTransmitBit, mSPICfg);
    mSPIDev.PushPullByte(static_cast<uint8_t>(aArg >> 24));
    mSPIDev.PushPullByte(static_cast<uint8_t>(aArg >> 16));
    mSPIDev.PushPullByte(static_cast<uint8_t>(aArg >>  8));
    mSPIDev.PushPullByte(static_cast<uint8_t>(aArg >>  0));

    // Known or dummy CRCs.
    switch (aCmd) {
    case CMD0: mSPIDev.PushPullByte(0x95); break;
    case CMD8: mSPIDev.PushPullByte(0x87); break;
    default:   mSPIDev.PushPullByte(0x1); break;
    }

    if (CMD12 == aCmd) {
        // Discard following one byte when CMD12.
        mSPIDev.PushPullByte(0xFF);
    }

    // Wait for response (10 bytes max).
    unsigned int lWaitCycles = 10;
    R1_RESPONSE_PKT lR1 = 0;
    do {
        lR1 = mSPIDev.PushPullByte(0xFF);
    } while ((lR1 & L_R1_MASK_BUSY) && (lWaitCycles--));

    // Ici, il faut pousser le data dans le ptr de retour.
    while (aRegLen > 0) {
        *(static_cast<uint8_t *>(aRegPtr)) = mSPIDev.PushPullByte(0xFF);
        aRegPtr++;
        aRegLen--;
    }

    // TODO: Deselect the card for certain commands (block reads)?
    mSPICfg.DeassertCSn();

    return lR1;
}


bool SDC::IsExpectedVoltageRange(void) {
    return true;
}

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
