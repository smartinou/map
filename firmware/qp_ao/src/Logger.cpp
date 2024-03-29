//.$file${Logging::../src::Logger.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: PFPP.qm
// File:  ${Logging::../src::Logger.cpp}
//
// This code has been generated by QM 5.1.3 <www.state-machine.com/qm/>.
// DO NOT EDIT SECTIONS BETWEEN THE COMMENTS "$...vvv".."$end...^^^".
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
//.$endhead${Logging::../src::Logger.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// *******************************************************************************
//
// Project: Logging.
//
// Module: Logger.
//
// *******************************************************************************

//! \file
//! \brief Logger class.
//! \ingroup qp_ao

// ******************************************************************************
//
//        Copyright (c) 2018-2019, Pleora Technologies, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

// Standard Libraries.
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// QP-port.
#include "qpcpp.h"

#include "BSP.h"

#include "Logger.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// ******************************************************************************
//                            FUNCTION PROTOTYPES
// ******************************************************************************

// ******************************************************************************
//                             GLOBAL VARIABLES
// ******************************************************************************

Logger::LogCategory_t Logger::mCategories[sMaxLogCategories] = {0};
size_t Logger::mCategoryQty = 0;

// ******************************************************************************
//                            EXPORTED FUNCTIONS
// ******************************************************************************

Logger & Logger::Instance() {
    static Logger sLogger;
    return sLogger;
}


Logger::Logger()
    : mLogLevel(LogLevel::prio::ERROR) {

    // Ctor body left intentionally empty.
}


Logger::~Logger() {
  // Dtor body left intentionally empty.
}


LogLevel Logger::GetLogLevel(void) const {
    return mLogLevel;
}


LogLevel Logger::GetLogLevel(char const * const aCategoryStr) const {

    LogCategory_t const * const lCategory = FindCategory(aCategoryStr);
    if (nullptr != lCategory) {
        return lCategory->mLevel;
    }

    return sInvalidCategory;
}


unsigned int Logger::GetEventSignal(char const * const aCategoryStr) const {

    LogCategory_t const * const lCategoryStr = FindCategory(aCategoryStr);
    if (nullptr != lCategoryStr) {
        return lCategoryStr->mEventSignal;
    }

    return 0;
}


void Logger::SetLogLevel(LogLevel const aLevel) {
    mLogLevel = aLevel;
}


bool Logger::AddCategory(
    unsigned int const aEventSignal,
    char const * const aCategoryStr,
    LogLevel::prio const aLevel
    ) {

    if (mCategoryQty >= sMaxLogCategories) {
        return false;
    }

    if (mCategoryQty > 0) {
        LogCategory_t * const lCategoryStr = static_cast<LogCategory_t * const>(
            bsearch(
                static_cast<void const *>(aCategoryStr),
                static_cast<void const *>(&mCategories[0]),
                mCategoryQty,
                sizeof(LogCategory_t),
                Logger::CompareStr
            )
        );
        if (nullptr != lCategoryStr) {
            //lCategoryStr->mEventSignal = aEventSignal;
            lCategoryStr->mLevel = aLevel;
            return true;
        }
    }

    strncpy(
        mCategories[mCategoryQty].mName,
        aCategoryStr,
        sMaxLogCategoryLen
    );
    mCategories[mCategoryQty].mLevel = aLevel;
    mCategories[mCategoryQty].mEventSignal = aEventSignal;
    mCategoryQty++;

    qsort(
        mCategories,
        mCategoryQty,
        sizeof(LogCategory_t),
        Logger::CompareStr
    );
    return true;
}


bool Logger::Log(
    LogLevel const aLevel,
    Date const &aDate,
    Time const &aTime,
    char const * const aFileStr,
    unsigned int const aLine,
    char const * const aFunctionStr,
    char const * const aCategoryStr,
    char const * const aFormatStr,
    ...
    ) {

    // Check for log level threshold (global and per category).
    LogLevel lCategoryLevel = GetLogLevel(aCategoryStr);
    if (lCategoryLevel != sInvalidCategory) {
        if (aLevel < lCategoryLevel) {
            return false;
        }
    } else if (aLevel < mLogLevel) {
        return false;
    }

    // Create the Log event and publish it to all!
    Logging::Event::LogEntry * const lLogEvent = Q_NEW(
        Logging::Event::LogEntry,
        GetEventSignal(aCategoryStr),
        aLevel,
        aDate,
        aTime,
        aFileStr,
        aLine,
        aFunctionStr,
        aCategoryStr
    );

    if (nullptr != lLogEvent) {
        // Concatenate variadics if any.
        va_list lArgs;
        va_start(lArgs, aFormatStr);
        vsprintf(&lLogEvent->mMsg[0], aFormatStr, lArgs);
        va_end(lArgs);
    }

    QP::QF::PUBLISH(lLogEvent, this);
    return true;
}


void Logger::AddSink(QP::QActive * const aAO, char const * const aCategoryStr) {
    // Make the AO listener subscribe to the category.
    unsigned int lEventSig = GetEventSignal(aCategoryStr);
    aAO->subscribe(lEventSig);
}

// ******************************************************************************
//                              LOCAL FUNCTIONS
// ******************************************************************************

Logger::LogCategory_t *Logger::FindCategory(char const * const aCategoryStr) const {

    if ((0 == mCategoryQty) || (nullptr == aCategoryStr)) {
        return nullptr;
    }

    LogCategory_t * const lCategoryStr = static_cast<LogCategory_t * const>(
        bsearch(
            static_cast<void const *>(aCategoryStr),
            static_cast<void const *>(mCategories),
            mCategoryQty,
            sizeof(LogCategory_t),
            Logger::CompareStr
        )
    );
    return lCategoryStr;
}


int Logger::CompareStr(void const * const aKey, void const * const aElement) {

    return strcmp(
        static_cast<char const * const>(aKey),
        static_cast<LogCategory_t const * const>(aElement)->mName
    );
}

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
