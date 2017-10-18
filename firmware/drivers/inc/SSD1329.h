#ifndef SSD1329_H_
#define SSD1329_H_
// *******************************************************************************
//
// Project: Larger project scope.
//
// Module: Module in the larger project scope.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2015-2016, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

// Standard Libraries.
#include <string>

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
class SSD1329 {
 public:
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

 SSD1329(CoreLink::SPIDev       &aSPIDevRef,
	 CoreLink::SPISlaveCfg  &aSPICfgRef,
	 unsigned long           aDCnGPIOPort,
	 unsigned int            aDCnGPIOPin,
	 unsigned long           aEn15VGPIOPort,
	 unsigned int            aEn15VGIOPin,
	 unsigned int            aDisplayWidth  = 128,
	 unsigned int            aDisplayHeight = 128);
  ~SSD1329();

  void Init();
  void DisplayOn(void);
  void DisplayOff(void);

  void Clr(void);
  void DrawStr(std::string,
	       unsigned int aXPos,
	       unsigned int aYPos,
	       unsigned int aGreyLvl);
  void DrawImg(uint8_t const *aImgBufPtr,
	       unsigned int aXPos,
	       unsigned int aYPos,
	       unsigned int aWidth,
	       unsigned int aHeight);

 private:
  void WrCmd(uint8_t const *aCmdBufPtr, unsigned int aLen);
  void WrData(uint8_t const *aDataBufPtr, unsigned int aLen);

  void AssertDataLine(void);
  void AssertCmdLine(void);

  CoreLink::SPIDev      &mSPIDevRef;
  CoreLink::SPISlaveCfg &mSPICfgRef;
  
  unsigned long mDCnGPIOPort;
  unsigned int  mDCnGPIOPin;
  unsigned long mEn15VGPIOPort;
  unsigned int  mEn15VGPIOPin;

  unsigned int mDisplayWidth;
  unsigned int mDisplayHeight;

  static unsigned int const mWidthMax  = (128 / 2) - 1;;
  static unsigned int const mHeightMax = 128 - 1;
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
#endif // SSD1329_H_
