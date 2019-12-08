//$file${Logging::../inc::Logger.h} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: PFPP.qm
// File:  ${Logging::../inc::Logger.h}
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
//$endhead${Logging::../inc::Logger.h} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#pragma once
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

#include <cstddef>
#include <string>

#include "Logging_Events.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

#define LOGGER (Logger::Instance())

// TODO: Verify that release builds with gcc define NDEBUG.
#if defined(_DEBUG) || !defined(NDEBUG)
#define LOG_DEBUG(aCategory, ...); \
{                                  \
    LOGGER.Log(                    \
        LogLevel::prio::DEBUG,     \
        __FILE__,                  \
        __LINE__,                  \
        __FUNCTION__,              \
        aCategory,                 \
        __VA_ARGS__                \
    );                             \
}
#else
#define LOG_DEBUG(aCategory, ...)
#endif

#define LOG_INFO(aCategory, ...); \
{                                 \
    LOGGER.Log(                   \
        LogLevel::prio::INFO,     \
        __FILE__,                 \
        __LINE__,                 \
        __FUNCTION__,             \
        aCategory,                \
        __VA_ARGS__               \
    );                            \
}

#define LOG_WARNING(aCategory, ...); \
{                                    \
    LOGGER.Log(                      \
        LogLevel::prio::WARNING,     \
        __FILE__,                    \
        __LINE__,                    \
        __FUNCTION__,                \
        aCategory,                   \
        __VA_ARGS__                  \
    );                               \
}

#define LOG_ERROR(aCategory, ...); \
{                                  \
    LOGGER.Log(                    \
        LogLevel::prio::ERR,                   \
        __FILE__,                  \
        __LINE__,                  \
        __FUNCTION__,              \
        aCategory,                 \
        __VA_ARGS__                \
    );                             \
}

#define LOG_CRITICAL(aCategory, ...); \
{                                     \
    LOGGER.Log(                       \
        PRI_CRIT,                     \
        __FILE__,                     \
        __LINE__,                     \
        __FUNCTION__,                 \
        aCategory,                    \
        __VA_ARGS__                   \
    );                                \
}

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Log generation class.
class Logger {
public:
    static Logger &Instance();

    LogLevel GetLogLevel(void) const;
    LogLevel GetLogLevel(char const * const aCategoryStr) const;
    unsigned int GetEventSignal(char const * const aCategoryStr) const;

    void SetLogLevel(LogLevel const aLevel);
    bool AddCategory(
        unsigned int const aEventSignal,
        char const * const aCategoryStr,
        LogLevel::prio const aLevel = LogLevel::prio::ERROR
    );

    bool Log(
        LogLevel     const aLevel,
        char const * const aFileStr,
        unsigned int const aLine,
        char const * const aFunctionStr,
        char const * const aCategoryStr,
        char const * const aFormatStr,
        ...
    );

    void AddSink(QP::QActive * const aAO, char const * const aCategoryStr);

private:
    // Disable default constructor/copy/assign
    Logger();
    Logger(Logger const&) = delete;
    Logger &operator=(Logger const &) = delete;
    ~Logger();
    //void operator=(Logger const&);
    void *operator new(size_t aSize){ return &(Instance()); }
    void  operator delete(void *aLogger) {}

    static int CompareStr(void const * const aFirstStr, void const * const aSecondStr);

    static size_t constexpr sMaxLogCategories  = 32;
    static size_t constexpr sMaxLogCategoryLen = 32;
    static LogLevel::prio constexpr sInvalidCategory = LogLevel::prio::DISABLED;
    static size_t constexpr sMsgBufLen = 1024;

    typedef struct {
        unsigned int mEventSignal;
        char         mName[sMaxLogCategoryLen];
        LogLevel     mLevel;
    } LogCategory_t;

    LogCategory_t *FindCategory(char const * const aCategoryStr) const;

    static LogCategory_t mCategories[sMaxLogCategories];
    static size_t        mCategoryQty;

    LogLevel mLogLevel;
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
