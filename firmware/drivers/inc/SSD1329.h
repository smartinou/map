#pragma once
// *******************************************************************************
//
// Project: Drivers.
//
// Module: OLED display build.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup ext_peripherals


// ******************************************************************************
//
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

// Standard Libraries.
#include <string>

#include "GPIOs.h"
#include "ILCD.h"
#include "SPI.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Brief description.
//! Details follow...
//! ...here.
class SSD1329
    : public ILCD {
 
public:
   SSD1329(
       CoreLink::SPIDev &aSPIDev,
       CoreLink::SPISlaveCfg aSPICfg,
       GPIOs const aDCnGPIO,
       GPIOs const aEn15VGPIO,
       unsigned int const aDisplayWidth  = 128,
       unsigned int const aDisplayHeight = 128);
   SSD1329(
       CoreLink::SPIDev &aSPIDev,
       GPIOs const &aCsPin,
       GPIOs const aDCnGPIO,
       GPIOs const aEn15VGPIO,
       unsigned int const aDisplayWidth = 128,
       unsigned int const aDisplayHeight = 128);
   virtual ~SSD1329();

    void Init() override;
    void DisplayOn(void) override;
    void DisplayOff(void) override;

    void Clr(void) override;
    void DrawStr(
        std::string aStr,
        unsigned int aXPos,
        unsigned int aYPos,
        unsigned int aGreyLevel) override;
    void DrawImg(
        uint8_t const *aImgBufPtr,
	    unsigned int aXPos,
	    unsigned int aYPos,
	    unsigned int aWidth,
	    unsigned int aHeight) override;

 private:
    enum {
        SET_COLUMN_ADDR = 0x15,
        SET_ROW_ADDR = 0x75,
        SET_CONSTRAST_CURRENT = 0x81,
        SET_2ND_PRECHARGE_SPEED = 0x82,
        SET_MASTER_ICON_CONTROL = 0x90,
        SET_ICON_CURRENT_RANGE = 0x91,
        SET_INDIVIDUAL_ICON_CURRENT = 0x92,
        SET_INDIVIDUAL_ICON = 0x93,
        SET_ICON = 0x94,
        SET_ICON_BLINKING_CYCLE = 0x95,
        SET_ICON_DUTY = 0x96,
        SET_REMAP = 0xA0,
        SET_DISPLAY_START_LINE = 0xA1,
        SET_DISPLAY_OFFSET = 0xA2,
        SET_DISPLAY_MODE_NORMAL = 0xA4,
        SET_DISPLAY_MODE_ALL_ON = 0xA5,
        SET_DISPLAY_MODE_ALL_OFF = 0xA6,
        SET_DISPLAY_MODE_INVERSE = 0xA7,
        SET_MUX_RATIO = 0xA8,
        SET_SLEEP_MODE_ON = 0xAE,
        SET_SLEEP_MODE_OFF = 0xAF,
        SET_PHASE_LEN = 0xB1,
        SET_FRAME_FREQUENCY = 0xB2,
        SET_FRONT_CLOCK_DIVIDER = 0xB3,
        SET_DEFAULT_GREYSCAL_TABLE = 0xB7,
        LUT_GREYSCALE_PULSE_WIDTH = 0xB8,
        SET_2ND_PRECHARGE_PERIOD = 0xBB,
        SET_1ST_PRECHARGE_VOLTAGE = 0xBC,
        SET_V_COMH = 0xBE,
        NOP = 0xE3,
        SET_COMMAND_LOCK = 0xFD,
    };

    void WrCmd(uint8_t const *aCmdBufPtr, unsigned int aLen);
    void WrData(uint8_t const *aDataBufPtr, unsigned int aLen);

    void AssertDataLine(void);
    void AssertCmdLine(void);

    CoreLink::SPIDev      &mSPIDev;
    CoreLink::SPISlaveCfg  mSPICfg;

    GPIOs const mDCnGPIO;
    GPIOs const mEn15VGPIO;

    unsigned int const mDisplayWidth;
    unsigned int const mDisplayHeight;

    // Display limits of the controller.
    static unsigned int constexpr sWidthMax  = (128 / 2) - 1;
    static unsigned int constexpr sHeightMax = 128 - 1;

    enum : unsigned int {
        FONT_WIDTH_MAX = 5,
        FONT_HEIGHT_MAX = 7,
        CELL_WIDTH_MAX = 6,
        CELL_HEIGHT_MAX = 8
    };

    static uint8_t const sFontTbl[96][FONT_WIDTH_MAX];
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
