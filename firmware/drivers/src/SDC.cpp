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

// Corelink driver.
#include "SPI.h"

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
  L_CMD8_PARAM  = 0x000001A5UL,

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

SDC::SDC(unsigned int           aDriveIx,
         CoreLink::SPIDev      &aSPIDev,
         CoreLink::SPISlaveCfg &aSPISlaveCfg)
  : mMyDriveIx(aDriveIx)
  , mSPIDev(aSPIDev)
  , mSPISlaveCfg(aSPISlaveCfg)
  , mStatus(STA_NOINIT)
  , mCardType(0) {

  // Ctor body left intentionally empty.
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
  mSPISlaveCfg.SetBitRate(400000);

  // Send 80 dummy clocks, CSn de-asserted.
  mSPISlaveCfg.DeassertCSn();
  for (unsigned int lByteIx = 0; lByteIx < 10; lByteIx++) {
    mSPIDev.PushPullByte(0xFF);
  }

  // Put the card in SPI mode.
  uint8_t lR1 = SendCmd(CMD0, L_CMD0_PARAM);
  if (L_R1_MASK_IN_IDLE_STATE == lR1) {
    R7_RESPONSE_PKT lR7 = {0};
    lR1 = SendCmd(CMD8, L_CMD8_PARAM, &lR7, sizeof(R7_RESPONSE_PKT));
    if (L_R1_MASK_IN_IDLE_STATE == lR1) {
      // Ver2.00 or later memory card.
      // Valid response?
      if (L_R7_RESPONSE == lR7.mIFCond) {
        // Compatible voltage range and check pattern correct.
        // Read OCR.
        R3_RESPONSE_PKT lR3 = {0};
        lR1 = SendCmd(CMD58, L_CMD58_PARAM, &lR3, sizeof(R3_RESPONSE_PKT));

        // Compare with expected voltage range.
        if (IsExpectedVoltageRange()) {

          do {
            // Start initialization and wait for it to complete.
            // TODO: add 1sec timeout.
            lR1 = SendCmd(ACMD41, L_ACMD41_PARAM);
          } while (0 != lR1);

          lR1 = SendCmd(CMD58, L_CMD58_PARAM, &lR3, sizeof(R3_RESPONSE_PKT));
          if (lR3.mOCR & L_OCR_MASK_CCS) {
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
      lR1 = SendCmd(CMD58, L_CMD58_PARAM, &lR3, sizeof(R3_RESPONSE_PKT));
      if (L_R1_MASK_IN_IDLE_STATE == lR1) {
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


DRESULT SDC::DiskRd(uint8_t     *aBufPtr,
                    uint32_t     aStartSector,
                    unsigned int aSectorCount) {

  return RES_OK;
}


#if (FF_FS_READONLY == 0)
DRESULT SDC::DiskWr(uint8_t const *aBufPtr,
                    uint32_t       aStartSector,
                    unsigned int   aSectorCount) {

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
  mSPISlaveCfg.AssertCSn();
  mSPIDev.PushPullByte(0xFF);

  if (1) {
    // Leading busy check.
    WaitReady();
    return true;
  }

  mSPISlaveCfg.DeassertCSn();
  return false;
}


void SDC::Deselect(void) {
  // Deassert CSn.
  // Dummy clock: force DO high-Z for multiple slave SPI.
  mSPISlaveCfg.DeassertCSn();
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


int SDC::RxDataBlock(uint8_t *aBufPtr, unsigned int aBlockLen) {
  return 0;
}


#if (FF_FS_READONLY == 0)
int SDC:TxDataBlock(uint8_t const *aBufPtr, uint8_t aToken) {

}
#endif // FF_FS_READONLY


uint8_t SDC::SendCmd(uint8_t aCmd, uint32_t aArg) {

  uint8_t lR1 = {0};
  return SendCmd(aCmd, aArg, &lR1, 1);
}


uint8_t SDC::SendCmd(uint8_t      aCmd,
                     uint32_t     aArg,
                     void        *aRegPtr,
                     unsigned int aRegLen) {

  uint8_t lR1 = 0;

  // Send a CMD55 prior to ACMD<n>.
  if (aCmd & 0x80) {
    aCmd &= 0x7F;
    lR1 = SendCmd(CMD55, 0x00000000);
    // Return if there's an error.
    if ((lR1 & 1) && (lR1 & 1)) {
      return lR1;
    }
  }

  // Select the card and wait for ready,
  // except to stop multiple block read.
  if (CMD12 != aCmd) {
    Select();
  }


  // Send command packet: start + command.
  mSPIDev.PushPullByte(aCmd | 0x40, mSPISlaveCfg);
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
  do {
    lR1 = mSPIDev.PushPullByte(0xFF);
  } while ((lR1 & L_R1_MASK_BUSY) && (lWaitCycles--));

  // Ici, il faut pousser le data dans le ptr de retour.

  // TODO: Deselect the card for certain commands (block reads)?
  mSPISlaveCfg.DeassertCSn();

  return lR1;
}


bool SDC::IsExpectedVoltageRange(void) {
  return true;
}


// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
