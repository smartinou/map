#pragma once
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

#include <cstddef>
#include <string>

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// Debug levels.
#define X_DEBUG_LEVELS       \
  _(PRI_DBG,  "DEBUG")       \
  _(PRI_INFO, "INFORMATION") \
  _(PRI_WARN, "WARNING")     \
  _(PRI_ERR,  "ERROR")       \
  _(PRI_CRIT, "CRITICAL")    \
  _(PRI_DIS,  "DISABLED")

#define _(enum, str) enum,
typedef enum {
  X_DEBUG_LEVELS
} LogLevel_t;
#undef _


#define LOGGER (Log::Instance())

// TODO: Verify that release builds with gcc define NDEBUG.
#if defined(_DEBUG) || !defined(NDEBUG)
#define LOG_DEBUG(aCategory, ...) \
  LOGGER.Log(PRI_DBG,             \
	     __FILE__,            \
	     __LINE__,            \
	     __FUNCTION__,        \
	     aCategory,           \
	     __VA_ARGS__)
#else
#define LOG_DEBUG(aCategory, ...)
#endif

#define LOG_INFO(aCategory, ...) \
  LOGGER.Log(PRI_INFO,           \
	     __FILE__,           \
	     __LINE__,           \
	     __FUNCTION__,       \
	     aCategory,          \
	     __VA_ARGS__)

#define LOG_WARNING(aCategory, ...) \
  LOGGER.Log(PRI_WARN,              \
	     __FILE__,              \
	     __LINE__,              \
	     __FUNCTION__,          \
	     aCategory,             \
	     __VA_ARGS__)

#define LOG_ERROR(aCategory, ...) \
  LOGGER.Log(PRI_ERR,             \
	     __FILE__,            \
	     __LINE__,            \
	     __FUNCTION__,        \
	     aCategory,           \
	     __VA_ARGS__)

#define LOG_CRITICAL(aCategory, ...) \
  LOGGER.Log(PRI_CRIT,               \
	     __FILE__,               \
	     __LINE__,               \
	     __FUNCTION__,           \
	     aCategory,              \
	     __VA_ARGS__)

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Brief description.
//! Details follow...
//! ...here.
class LogEvt : public QP::QEvt {
 public:
  LogEvt(LogLevel_t   aLevel,
	 char const  *aFileStr,
	 unsigned int aLine,
	 char const  *aFunctionStr,
	 char const  *aCategoryStr,
	 char const  *aFormatStr);

 public:
#if 0
  char const  *mFileStr;
  unsigned int mLine;
  char const  *mFunctionStr;
  char const  *mCategoryStr;
  char const  *mFormatStr;
#else
  LogLevel_t   const  mLevel;
  std::string  const &mFileStr;
  unsigned int const  mLine;
  std::string  const &mFunctionStr;
  std::string  const &mCategoryStr;
  std::string  const &mFormatStr;
#endif
};


class Logger {
 public:
  static Logger &Instance();
  //Logger(char const *aName, char const *aConfiguration = NULL) {}
  virtual ~Logger();
  void *operator new(size_t aSize) { return &(Instance()); }
  void  operator delete(void *aLoggerPtr) {}

  LogLevel_t GetLogLevel(void) const;
  LogLevel_t GetLogLevel(char const * const aCategoryStr) const;
  unsigned int GetEvtSignal(char const * const aCategoryStr) const;

  void SetLogLevel(LogLevel_t const aLevel);
  bool AddCategory(unsigned int const  aEvtSignal,
		   char         const *aCategoryStr,
		   LogLevel_t   const  aLevel = PRI_ERR);

bool Log(LogLevel_t   aLevel,
		 char const  *aFileStr,
		 unsigned int aLine,
		 char const  *aFunctionStr,
		 char const  *aCategoryStr,
	 char const  *aFormat, ...);
  static char const *LogLevelToStr(LogLevel_t const aLevel);

 private:
  // Disable default constructor/copy/assign
  Logger();
  Logger(Logger const&);
  void operator=(Logger const&);

  static int CompareStr(void const * const aFirstStr,
			void const * const aSecondStr);

  static int const sMaxLogCategories  = 32;
  static int const sMaxLogCategoryLen = 32;
  static LogLevel_t const sInvalidCategory = static_cast<LogLevel_t>(-1);

  typedef struct {
    unsigned int mEvtSignal;
    char         mName[sMaxLogCategoryLen];
    LogLevel_t   mLevel;
  } LogCategory_t;

  LogCategory_t *FindCategory(char const * const aCategoryStr) const;

  static LogCategory_t mCategories[sMaxLogCategories];
  static size_t        mCategoryQty;

  LogLevel_t mLogLevel;
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
