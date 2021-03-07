//$file${Logging::../inc::LogLevel.h} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: PFPP.qm
// File:  ${Logging::../inc::LogLevel.h}
//
// This code has been generated by QM 4.5.1 (https://www.state-machine.com/qm).
// DO NOT EDIT SECTIONS BETWEEN THE COMMENTS "$...###".."$end...###".
// All your changes in these sections will be lost.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//
//$endhead${Logging::../inc::LogLevel.h} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#pragma once
// *******************************************************************************
//
// Project: Logging.
//
// Module: LogLevel.
//
// *******************************************************************************

//! \file
//! \brief LogLevel class.
//! \ingroup qp_ao

// ******************************************************************************
//
//        Copyright (c) 2018-2019, Pleora Technologies, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

class LogLevel {
public:
    enum class prio : unsigned int {
        DEBUG = 0,
        INFO,
        WARNING,
        ERROR,
        CRITICAL,
        DISABLED
    };

    LogLevel() : mLevel(prio::DEBUG) {}
    LogLevel(prio const aLevel) : mLevel(aLevel) {}
    prio GetLevel(void) const { return mLevel; }

    inline bool operator< (const LogLevel& rhs) const { return this->mLevel < rhs.mLevel; }
    inline bool operator> (const LogLevel& rhs) const { return rhs < *this; }
    inline bool operator<=(const LogLevel& rhs) const { return !(*this > rhs); }
    inline bool operator>=(const LogLevel& rhs) const { return !(*this < rhs); }

    inline bool operator==(const LogLevel& rhs) const { return this->mLevel == rhs.mLevel; }
    inline bool operator!=(const LogLevel& rhs) const { return !(*this == rhs); }

    static char const *ToStr(prio const aLevel);

private:
    prio mLevel;
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