// *****************************************************************************
//
// Project: Drivers.
//
// Module: OLED display build.
//
// *****************************************************************************

//! \file
//! \brief SSD1329 OLED controller class.
//! \ingroup ext_peripherals

// *****************************************************************************
//
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include <stdint.h>

// Peripheral Driver Library.
#include <inc/hw_types.h>
#include <driverlib/gpio.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>

#include "SPI.h"

#include "SSD1329.h"

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

// A 5x7 font (in a 6x8 cell, where the sixth column is omitted from this table)
// for displaying text on the OLED display.
// The data is organized as bytes from the left column to the right column,
// with each byte containing the top row in the LSB and the bottom row in the MSB.
//
// Note: This is the same font data that is used in the EK-LM3S811
// osram96x16x1 driver.
// The single bit-per-pixel is expaned in the StringDraw
// function to the appropriate four bit-per-pixel gray scale format.
uint8_t const SSD1329::sFontTbl[96][FONT_WIDTH_MAX] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // " "
    {0x00, 0x00, 0x4f, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7f, 0x14, 0x7f, 0x14}, // #
    {0x24, 0x2a, 0x7f, 0x2a, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1c, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1c, 0x00}, // )
    {0x14, 0x08, 0x3e, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3e, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3e, 0x51, 0x49, 0x45, 0x3e}, // 0
    {0x00, 0x42, 0x7f, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4b, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7f, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3c, 0x4a, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1e}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3e}, // @
    {0x7e, 0x11, 0x11, 0x11, 0x7e}, // A
    {0x7f, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3e, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7f, 0x41, 0x41, 0x22, 0x1c}, // D
    {0x7f, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7f, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3e, 0x41, 0x49, 0x49, 0x7a}, // G
    {0x7f, 0x08, 0x08, 0x08, 0x7f}, // H
    {0x00, 0x41, 0x7f, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3f, 0x01}, // J
    {0x7f, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7f, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7f, 0x02, 0x0c, 0x02, 0x7f}, // M
    {0x7f, 0x04, 0x08, 0x10, 0x7f}, // N
    {0x3e, 0x41, 0x41, 0x41, 0x3e}, // O
    {0x7f, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3e, 0x41, 0x51, 0x21, 0x5e}, // Q
    {0x7f, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7f, 0x01, 0x01}, // T
    {0x3f, 0x40, 0x40, 0x40, 0x3f}, // U
    {0x1f, 0x20, 0x40, 0x20, 0x1f}, // V
    {0x3f, 0x40, 0x38, 0x40, 0x3f}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x7f, 0x41, 0x41, 0x00}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // "\"
    {0x00, 0x41, 0x41, 0x7f, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a
    {0x7f, 0x48, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7f}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x08, 0x7e, 0x09, 0x01, 0x02}, // f
    {0x0c, 0x52, 0x52, 0x52, 0x3e}, // g
    {0x7f, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7d, 0x40, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3d, 0x00}, // j
    {0x7f, 0x10, 0x28, 0x44, 0x00}, // k
    {0x00, 0x41, 0x7f, 0x40, 0x00}, // l
    {0x7c, 0x04, 0x18, 0x04, 0x78}, // m
    {0x7c, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0x7c, 0x14, 0x14, 0x14, 0x08}, // p
    {0x08, 0x14, 0x14, 0x18, 0x7c}, // q
    {0x7c, 0x08, 0x04, 0x04, 0x08}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x3f, 0x44, 0x40, 0x20}, // t
    {0x3c, 0x40, 0x40, 0x20, 0x7c}, // u
    {0x1c, 0x20, 0x40, 0x20, 0x1c}, // v
    {0x3c, 0x40, 0x30, 0x40, 0x3c}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x0c, 0x50, 0x50, 0x50, 0x3c}, // y
    {0x44, 0x64, 0x54, 0x4c, 0x44}, // z
    {0x00, 0x08, 0x36, 0x41, 0x00}, // {
    {0x00, 0x00, 0x7f, 0x00, 0x00}, // |
    {0x00, 0x41, 0x36, 0x08, 0x00}, // }
    {0x02, 0x01, 0x02, 0x04, 0x02}, // ~
    {0x00, 0x00, 0x00, 0x00, 0x00}
};

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

SSD1329::SSD1329(
    std::shared_ptr<CoreLink::ISPIMasterDev> aSPIMasterDev,
    CoreLink::SPISlaveCfg const &aSPISlaveCfg,
    GPIO const &aDCnGPIO,
    GPIO const &aEn15VGPIO,
    unsigned int const aDisplayWidth,
    unsigned int const aDisplayHeight
)
    : mSPIMasterDev(aSPIMasterDev)
    , mSPISlaveCfg(aSPISlaveCfg)
    , mDCnGPIO(aDCnGPIO)
    , mEn15VGPIO(aEn15VGPIO)
    , mDisplayWidth(aDisplayWidth)
    , mDisplayHeight(aDisplayHeight)
{
    mSPISlaveCfg.InitCSnGPIO();
}


SSD1329::SSD1329(
    std::shared_ptr<CoreLink::ISPIMasterDev> aSPIMasterDev,
    GPIO const &aCsPin,
    GPIO const &aDCnGPIO,
    GPIO const &aEn15VGPIO,
    unsigned int const aDisplayWidth,
    unsigned int const aDisplayHeight
)
    : mSPIMasterDev(aSPIMasterDev)
    , mSPISlaveCfg(aCsPin, CoreLink::SPISlaveCfg::PROTOCOL::MOTO_3, 4000000, 8)
    , mDCnGPIO(aDCnGPIO)
    , mEn15VGPIO(aEn15VGPIO)
    , mDisplayWidth(aDisplayWidth)
    , mDisplayHeight(aDisplayHeight)
{
    mSPISlaveCfg.InitCSnGPIO();
}


void SSD1329::Init(void) {

    // Configure the GPIO port pin used as D/Cn signal for controller.
    MAP_GPIOPinTypeGPIOOutput(mDCnGPIO.GetPort(), mDCnGPIO.GetPin());
    MAP_GPIOPadConfigSet(
        mDCnGPIO.GetPort(),
        mDCnGPIO.GetPin(),
        GPIO_STRENGTH_8MA,
        GPIO_PIN_TYPE_STD
    );
    MAP_GPIOPinWrite(mDCnGPIO.GetPort(), mDCnGPIO.GetPin(), mDCnGPIO.GetPin());

    // Configure the GPIO port pin used to enable power to the OLED panel.
    MAP_GPIOPinTypeGPIOOutput(mEn15VGPIO.GetPort(), mEn15VGPIO.GetPin());
    MAP_GPIOPadConfigSet(
        mEn15VGPIO.GetPort(),
        mEn15VGPIO.GetPin(),
        GPIO_STRENGTH_8MA,
        GPIO_PIN_TYPE_STD
    );
    MAP_GPIOPinWrite(mEn15VGPIO.GetPort(), mEn15VGPIO.GetPin(), mEn15VGPIO.GetPin());

    // Clear screen.
    Clr();
    return;
}


void SSD1329::DisplayOn(void) {

    static uint8_t const sCmdUnlock[] = { SET_COMMAND_LOCK, 0x12 };
    WrCmd(&sCmdUnlock[0], sizeof(sCmdUnlock));
    DisplayOff();

    static uint8_t const sCmdIconOff[] = { SET_ICON, 0x0 };
    WrCmd(&sCmdIconOff[0], sizeof(sCmdIconOff));

    static uint8_t const sCmdMultiplexRatio[] = { SET_MUX_RATIO, 95 };
    WrCmd(&sCmdMultiplexRatio[0], sizeof(sCmdMultiplexRatio));

    static uint8_t const sCmdContrast[] = { SET_CONSTRAST_CURRENT, 0xB7 };
    WrCmd(&sCmdContrast[0], sizeof(sCmdContrast));

    static uint8_t const sCmdPreChargeCurrent[] = { SET_2ND_PRECHARGE_SPEED, 0x3F };
    WrCmd(&sCmdPreChargeCurrent[0], sizeof(sCmdPreChargeCurrent));

    static uint8_t const sCmdDisplayRemap[] = { SET_REMAP, 0x52 };
    WrCmd(&sCmdDisplayRemap[0], sizeof(sCmdDisplayRemap));

    static uint8_t const sCmdDisplayStartLine[] = { SET_DISPLAY_START_LINE, 0 };
    WrCmd(&sCmdDisplayStartLine[0], sizeof(sCmdDisplayStartLine));

    static uint8_t const sCmdDisplayOffset[] = { SET_DISPLAY_OFFSET, 0 };
    WrCmd(&sCmdDisplayOffset[0], sizeof(sCmdDisplayOffset));

    static uint8_t const sCmdDisplayModeNormal[] = { SET_DISPLAY_MODE_NORMAL };
    WrCmd(&sCmdDisplayModeNormal[0], sizeof(sCmdDisplayModeNormal));

    static uint8_t const sCmdPhaseLen[] = { SET_PHASE_LEN, 0x11 };
    WrCmd(&sCmdPhaseLen[0], sizeof(sCmdPhaseLen));

    static uint8_t const sCmdFrameFreq[] = { SET_FRAME_FREQUENCY, 0x23 };
    WrCmd(&sCmdFrameFreq[0], sizeof(sCmdFrameFreq));

    static uint8_t const sCmdFrontClkDiv[] = { SET_FRONT_CLOCK_DIVIDER, 0xE2 };
    WrCmd(&sCmdFrontClkDiv[0], sizeof(sCmdFrontClkDiv));

    static uint8_t const sGrayScaleTbl[] = {
        LUT_GREYSCALE_PULSE_WIDTH,
        // GS1 GS2 GS3 GS4
        1,  2,   3,  4,
        // GS5 GS6 GS7 GS8
        5,  6,   8, 10,
        // GS9 GS10 GS11 GS12
        12, 14, 16, 19,
        // GS13 GS14 GS15
        22, 26, 30
    };
    WrCmd(&sGrayScaleTbl[0], sizeof(sGrayScaleTbl));

    static uint8_t const sCmd2ndPreChargePeriod[] = { SET_1ST_PRECHARGE_VOLTAGE, 0x01 };
    WrCmd(&sCmd2ndPreChargePeriod[0], sizeof(sCmd2ndPreChargePeriod));

    static uint8_t const sCmdPreChargeVoltage[] = { SET_1ST_PRECHARGE_VOLTAGE, 0x3F };
    WrCmd(&sCmdPreChargeVoltage[0], sizeof(sCmdPreChargeVoltage));

    static uint8_t const sCmdDisplayOn[] = { SET_SLEEP_MODE_OFF };
    WrCmd(&sCmdDisplayOn[0], sizeof(sCmdDisplayOn));
    return;
}


void SSD1329::DisplayOff(void) {

    // Put the display to sleep.
    uint8_t const lCmdSleep[] = {SET_SLEEP_MODE_ON};
    WrCmd(&lCmdSleep[0], sizeof(lCmdSleep));
    return;
}


void SSD1329::Clr(void) {

    // Set the window to fill the entire display.
    static uint8_t const sCmdColumnSetAddr[] = { SET_COLUMN_ADDR, 0, sWidthMax };
    WrCmd(&sCmdColumnSetAddr[0], sizeof(sCmdColumnSetAddr));

    static uint8_t const sCmdSetRowAddr[] = { SET_ROW_ADDR, 0, sHeightMax };
    WrCmd(&sCmdSetRowAddr[0], sizeof(sCmdSetRowAddr));

    static uint8_t const sCmdHorizontalInc[] = { SSD1329::SET_REMAP, 0x52 };
    WrCmd(&sCmdHorizontalInc[0], sizeof(sCmdHorizontalInc));

    // Clear the data buffer.
    uint8_t lDataBuf[16] = { 0 };

    // Loop through the rows.
    for (unsigned int lRowIx = 0; lRowIx < mDisplayWidth; lRowIx++) {
        // Loop through the columns.
        // 16 bytes ( * 2 = 32 pixels) are cleared on each write.
        for (unsigned int lColumnIx = 0; lColumnIx < mDisplayHeight; lColumnIx += (sizeof(lDataBuf) * 2)) {
            WrData(&lDataBuf[0], sizeof(lDataBuf));
        }
    }
    return;
}


void SSD1329::DrawStr(
    std::string const &aStr,
    unsigned int const aXPos,
    unsigned int const aYPos,
    unsigned int const aGreyLevel
) {

    // Setup a window starting at the specified column and row,
    // ending at the right edge of the display and 8 rows down
    // (single character row).
    uint8_t lCmdBuf[3];
    lCmdBuf[0] = SET_COLUMN_ADDR;
    lCmdBuf[1] = aXPos / 2;
    lCmdBuf[2] = sWidthMax;
    WrCmd(&lCmdBuf[0], sizeof(lCmdBuf));

    lCmdBuf[0] = SET_ROW_ADDR;
    lCmdBuf[1] = aYPos;
    lCmdBuf[2] = aYPos + FONT_HEIGHT_MAX;
    WrCmd(&lCmdBuf[0], sizeof(lCmdBuf));

    static uint8_t const sCmdVerticalInc[] = {SET_REMAP, 0x56};
    WrCmd(&sCmdVerticalInc[0], sizeof(sCmdVerticalInc));

    for (auto lTmpChr : aStr) {
        // Get a working copy of the current character and convert to an index
        // into the character bit-map array.
        lTmpChr &= 0x7F;
        if (lTmpChr < ' ') {
            lTmpChr = ' ';
        } else {
            lTmpChr -= ' ';
        }

        // Build and display the character buffer.
        for (unsigned int lColumnIx = 0; lColumnIx < CELL_WIDTH_MAX; lColumnIx += 2) {

            // Convert two columns of 1-bit font data into a single data byte column
            // of 4-bit (grayscale) font data.
            uint8_t lDataBuf[8] = {0};
            for (unsigned int lRowIx = 0; lRowIx < FONT_HEIGHT_MAX; lRowIx++) {
                lDataBuf[lRowIx] = 0;
                // Set gray level of 1st of the two columns: MS-Nibble.
                if (sFontTbl[lTmpChr][lColumnIx] & (1 << lRowIx)) {
                    lDataBuf[lRowIx] = ((aGreyLevel << 4) & 0xF0);
                }

                // Set gray level of 2nd of the two columns: LS-Nibble.
                // There are only 5 columns in a 6-column matrix: don't convert column index 5.
                if ((lColumnIx < 4) && (sFontTbl[lTmpChr][lColumnIx + 1] & (1 << lRowIx))) {
                    lDataBuf[lRowIx] |= ((aGreyLevel << 0) & 0x0F);
                }
            }

            // Row index 7 is always 0.
            // Write data buffer.
            lDataBuf[CELL_HEIGHT_MAX] = 0;
            WrData(&lDataBuf[0], 8);
            aXPos += 2;

            // Return if the right side of the display has been reached.
            if (aXPos > sHeightMax) {
                return;
            }
        }
    }
    return;
}


void SSD1329::DrawImg(
    uint8_t const *aImgBufPtr,
	unsigned int const aXPos,
	unsigned int const aYPos,
	unsigned int const aWidth,
	unsigned int const aHeight
) {

  // [MG] BUNCH OF ASSERTS TO PUT DOWN HERE.

  // Setup a window starting at the specified column and row,
  // and ending at the column + width and row+height.
#if 0
    mCmdBuf[0] = SET_COLUMN_ADDR;
    mCmdBuf[1] = aXPos / 2;
    mCmdBuf[2] = (aXPos + aWidth - 2) / 2;
    WrCmd(&mCmdBuf[0], 3);

    mCmdBuf[0] = SET_ROW_ADDR;
    mCmdBuf[1] = aYPos;
    mCmdBuf[2] = aYPos + aHeight - 1;
    WrCmd(&mCmdBuf[0], 3);
#else
    uint8_t lCmdBuf[3];

    lCmdBuf[0] = SET_COLUMN_ADDR;
    lCmdBuf[1] = aXPos / 2;
    lCmdBuf[2] = (aXPos + aWidth - 2) / 2;
    WrCmd(&lCmdBuf[0], sizeof(lCmdBuf));

    lCmdBuf[0] = SET_ROW_ADDR;
    lCmdBuf[1] = aYPos;
    lCmdBuf[2] = aYPos + aHeight - 1;
    WrCmd(&lCmdBuf[0], sizeof(lCmdBuf));

#endif
    static uint8_t const sCmdHorizontalInc[] = { SSD1329::SET_REMAP, 0x52 };
    WrCmd(&sCmdHorizontalInc[0], sizeof(sCmdHorizontalInc));

    // Loop all the rows to display.
    while (aHeight--) {
        WrData(aImgBufPtr, (aWidth / 2));
        aImgBufPtr += (aWidth / 2);
    }
    return;
}


// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

void SSD1329::WrCmd(uint8_t const *aCmdBufPtr, unsigned int const aLen) {

    // Set the command/control bit to enable command mode.
    // Send all bytes via SPI.
    AssertCmdLine();
    mSPIMasterDev->WrData(aCmdBufPtr, aLen, mSPISlaveCfg);
    return;
}


void SSD1329::WrData(uint8_t const *aDataBufPtr, unsigned int const aLen) {
#if 1
    // Clear the command/control bit to enable command mode.
    // Send all bytes via SPI.
    AssertDataLine();
    mSPIMasterDev->WrData(aDataBufPtr, aLen, mSPISlaveCfg);
#else

    // Clear the command/control bit to enable command mode.
    // Send all bytes via SPI.
    // Should insert a NOP command before de-asserting CSn.
    // Requires manual SPI functions.
    AssertDataLine();
    mSPIMasterDev->AssertCSn(mSPISlaveCfg);
    mSPIMasterDev->WrData(aDataBufPtr, aLen);

    AssertCmdLine();
    uint8_t const lCmdNOP[] = {NOP};
    mSPIMasterDev->WrData(&lCmdNOP[0], 1);
    AssertDataLine();
    mSPIMasterDev->DeasserCSn(mSPISlaveCfg);

#endif
    return;
}


inline void SSD1329::AssertCmdLine(void) {

    MAP_GPIOPinWrite(mDCnGPIO.GetPort(), mDCnGPIO.GetPin(), 0);
    return;
}


inline void SSD1329::AssertDataLine(void) {

    MAP_GPIOPinWrite(mDCnGPIO.GetPort(), mDCnGPIO.GetPin(), mDCnGPIO.GetPin());
    return;
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
