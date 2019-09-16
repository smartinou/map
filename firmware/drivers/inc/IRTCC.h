#pragma once
// *******************************************************************************
//
// Project: Drivers.
//
// Module: RTCC.
//
// *******************************************************************************

//! \file
//! \brief RTCC interface class.
//! \ingroup ext_peripherals

// ******************************************************************************
//
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <string>

#include <time/Time.h>
#include <date/Date.h>

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************


//! \brief Brief description.
//! Details follow...
//! ...here.
class IRTCC {
public:
    virtual ~IRTCC() {}
    virtual void Init(void) = 0;
    virtual void SetInterrupt(bool aEnable) = 0;
    virtual void AckInterrupt(void) = 0;
    virtual void SetImpure(void) = 0;

    virtual void RdTime(Time &aTimeRef) = 0;
    virtual void RdDate(Date &aDateRef) = 0;
    virtual void RdTimeAndDate(Time &aTimeRef, Date &aDateRef) = 0;

    virtual void WrTime(Time const &aTimeRef) = 0;
    virtual void WrDate(Date const &aDateRef) = 0;
    virtual void WrTimeAndDate(Time const &aTimeRef, Date const &aDateRef) = 0;
    virtual void GetTimeAndDate(Time &aTimeRef, Date &aDateRef) = 0;
    virtual float GetTemperature(void) = 0;

    virtual bool WrAlarm(Time const &aTimeRef, Date const &aDateRef) = 0;
    virtual bool WrAlarm(Time const &aTimeRef, Weekday const &aWeekdayRef) = 0;
    virtual bool IsAlarmOn(void) = 0;
    virtual void DisableAlarm(void) = 0;
    virtual void ClrAlarmFlag(void) = 0;

    virtual unsigned int GetNVMemSize(void) const = 0;
    virtual void RdFromNVMem(
        uint8_t * const aDataPtr,
        unsigned int aOffset,
        unsigned int aSize
    ) = 0;
    virtual void WrToNVMem(
        uint8_t const * const aDataPtr,
        unsigned int aOffset,
        unsigned int aSize
    ) = 0;

protected:
    static unsigned int BinaryToBCD(unsigned int aBinVal);
    static unsigned int BCDToBinary(unsigned int aBCDVal);
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
