#pragma once
// *******************************************************************************
//
// Project: Drivers.
//
// Module: LCD.
//
// *******************************************************************************

//! \file
//! \brief LCD interface class.
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

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Brief description.
//! Details follow...
//! ...here.
class ILCD {
public:

    virtual ~ILCD() {}
    virtual void Init() = 0;
    virtual void DisplayOn(void) = 0;
    virtual void DisplayOff(void) = 0;

    virtual void Clr(void) = 0;
    virtual void DrawStr(
        std::string const &aStr,
        unsigned int aXPos,
        unsigned int aYPos,
        unsigned int aGreyLvl) = 0;
    virtual void DrawImg(
        uint8_t const *aImgBufPtr,
        unsigned int aXPos,
        unsigned int aYPos,
        unsigned int aWidth,
        unsigned int aHeight) = 0;

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
