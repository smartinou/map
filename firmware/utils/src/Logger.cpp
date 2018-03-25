// *******************************************************************************
//
// Project: Utilities.
//
// Module: Logger class.
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

// Standard Libraries.
#include <stdlib.h>
#include <string.h>

// QP-port.
#include "qpcpp.h"

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

LogEvt::LogEvt(LogLevel_t   aLevel,
	       char const  *aFileStr,
	       unsigned int aLine,
	       char const  *aFunctionStr,
	       char const  *aCategoryStr,
	       char const  *aFormatStr)
  : mLevel(aLevel)
  , mFileStr(aFileStr)
  , mLine(aLine)
  , mFunctionStr(aFunctionStr)
  , mCategoryStr(aCategoryStr)
  , mFormatStr(aFormatStr) {

  
}


Logger & Logger::Instance() {
  static Logger sLogger;
  return sLogger;
}


Logger::Logger()
  : mLogLevel(PRI_ERR) {

  // Ctor body left intentionally empty.
}


Logger::~Logger() {
  // You would think we want to MutexDestroy(mMutex) but actually not
  // since we override new and delete to do nothing to preserve the
  // singleton nature.
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


bool Logger::AddCategory(unsigned int const  aEvtSignal,
			 char         const *aCategoryStr,
			 LogLevel_t   const  aLevel) {

  if (mCategoryQty >= sMaxLogCategories) {
    return false;
  }

  if (mCategoryQty > 0) {
    LogCategory_t * const lCategoryStr =
      static_cast<LogCategory_t * const>(bsearch(aCategoryStr,
						 mCategories,
						 mCategoryQty,
						 sizeof(LogCategory_t),
						 Logger::CompareStr));
    if (nullptr != lCategoryStr) {
      lCategoryStr->mEvtSignal = aEvtSignal;
      lCategoryStr->mLevel = aLevel;
      return true;
    }
  }

  strncpy(mCategories[mCategoryQty].mName,
	  aCategoryStr,
	  sMaxLogCategoryLen);
  mCategories[mCategoryQty].mLevel = aLevel;
  mCategories[mCategoryQty].mEvtSignal = aEvtSignal;
  mCategoryQty++;

  qsort(mCategories,
	mCategoryQty,
	sizeof(LogCategory_t),
	Logger::CompareStr);
  return true;
}

#if 1
bool Logger::Log(LogLevel_t   aLevel,
		 char const  *aFileStr,
		 unsigned int aLine,
		 char const  *aFunctionStr,
		 char const  *aCategoryStr,
		 char const  *aFormat, ...) {

  // Create the Log event and post it!
  LogLevel_t lCategoryLevel = GetLevel(aCategoryStr);
  if (lCategoryLevel != sInvalidCategory) {
    if (aLevel < lCategoryLevel) {
      return false;
    }
  } else if (aLevel < mLogLevel) {
    return false;
  }  
}
#endif

#define _(enum, str) case enum: return str;
char const *Logger::LogLevelToStr(LogLevel_t const aLevel) {
  switch (aLevel) {
    X_DEBUG_LEVELS
  }
  return "";
}
#undef _
#undef X_DEBUG_LEVELS

// ******************************************************************************
//                              LOCAL FUNCTIONS
// ******************************************************************************

Logger::LogCategory_t *Logger::FindCategory(char const * const aCategoryStr) const {

  if ((0 == mCategoryQty) || (nullptr == aCategoryStr)) {
    return nullptr;
  }

  LogCategory_t * const lCategoryStr =
    static_cast<LogCategory_t * const>(bsearch(aCategoryStr,
					       mCategories,
					       mCategoryQty,
					       sizeof(LogCategory_t),
					       Logger::CompareStr));
  return lCategoryStr;
}


int Logger::CompareStr(void const * const aFirstStr,
		       void const * const aSecondStr) {

  return strcmp(static_cast<LogCategory_t const * const>(aFirstStr)->mName,
		static_cast<LogCategory_t const * const>(aSecondStr)->mName);
}

// ******************************************************************************
//                                END OF FILE
// ******************************************************************************
