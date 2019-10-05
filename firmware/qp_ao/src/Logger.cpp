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
    : mLogLevel(PRI_ERR) {

    // Ctor body left intentionally empty.
}


Logger::~Logger() {
  // Dtor body left intentionally empty.
}


LogLevel_t Logger::GetLogLevel(void) const {
    return mLogLevel;
}


LogLevel_t Logger::GetLogLevel(char const * const aCategoryStr) const {

    LogCategory_t const * const lCategoryStr = FindCategory(aCategoryStr);
    if (nullptr != lCategoryStr) {
        return lCategoryStr->mLevel;
    }

    return sInvalidCategory;
}


unsigned int Logger::GetEvtSignal(char const * const aCategoryStr) const {

    LogCategory_t const * const lCategoryStr = FindCategory(aCategoryStr);
    if (nullptr != lCategoryStr) {
        return lCategoryStr->mEvtSignal;
    }

    return 0;
}


void Logger::SetLogLevel(LogLevel_t const aLevel) {
    mLogLevel = aLevel;
}


bool Logger::AddCategory(
    unsigned int const aEvtSignal,
    char const * const aCategoryStr,
    LogLevel_t   const aLevel
    ) {

    if (mCategoryQty >= sMaxLogCategories) {
        return false;
    }

    if (mCategoryQty > 0) {
        LogCategory_t * const lCategoryStr = static_cast<LogCategory_t * const>(
            bsearch(
                aCategoryStr,
                mCategories,
                mCategoryQty,
                sizeof(LogCategory_t),
                Logger::CompareStr
            )
        );
        if (nullptr != lCategoryStr) {
            //lCategoryStr->mEvtSignal = aEvtSignal;
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
    mCategories[mCategoryQty].mEvtSignal = aEvtSignal;
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
    LogLevel_t   const aLevel,
    char const * const aFileStr,
    unsigned int const aLine,
    char const * const aFunctionStr,
    char const * const aCategoryStr,
    char const * const aFormatStr,
    ...
    ) {

    // Check for log level threshold (global and per category).
    LogLevel_t lCategoryLevel = GetLogLevel(aCategoryStr);
    if (lCategoryLevel != sInvalidCategory) {
        if (aLevel < lCategoryLevel) {
            return false;
        }
    } else if (aLevel < mLogLevel) {
        return false;
    }

    // Concatenate variadics if any.
    va_list lArgs;
    va_start(lArgs, aFormatStr);
    static char lMsgBuf[sMsgBufLen];
    vsprintf(&lMsgBuf[0], aFormatStr, lArgs);
    va_end(lArgs);

    // Create the Log event and publish it to all!
    Logging::Event::LogEvt * const lLogEvt = Q_NEW(
        Logging::Event::LogEvt,
        GetEvtSignal(aCategoryStr),
        aLevel,
        aFileStr,
        aLine,
        aFunctionStr,
        aCategoryStr,
        &lMsgBuf[0]
    );

    QP::QF::PUBLISH(lLogEvt, this);
    return true;
}


void Logger::AddSink(QP::QActive * const aAO, char const * const aCategoryStr) {
    // Make the AO listener subscribe to the category.
    unsigned int lEvtSig = GetEvtSignal(aCategoryStr);
    aAO->subscribe(lEvtSig);
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
            aCategoryStr,
            mCategories,
            mCategoryQty,
            sizeof(LogCategory_t),
            Logger::CompareStr
        )
    );
    return lCategoryStr;
}


int Logger::CompareStr(void const * const aFirstStr, void const * const aSecondStr) {

    return strcmp(
        static_cast<LogCategory_t const * const>(aFirstStr)->mName,
        static_cast<LogCategory_t const * const>(aSecondStr)->mName
    );
}

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
