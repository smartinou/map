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


#define LOGGER (Logger::Instance())

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
  bool AddCategory(unsigned int const aEvtSignal,
                   char const * const aCategoryStr,
                   LogLevel_t   const aLevel = PRI_ERR);

  bool Log(LogLevel_t   const         aLevel,
           char         const * const aFileStr,
           unsigned int const         aLine,
           char         const * const aFunctionStr,
           char         const * const aCategoryStr,
           char         const * const aFormatStr,
           ...);
  static char const *LogLevelToStr(LogLevel_t const aLevel);

 private:
  // Disable default constructor/copy/assign
  Logger();
  Logger(Logger const&);
  void operator=(Logger const&);

  static int CompareStr(void const * const aFirstStr,
                        void const * const aSecondStr);

  static unsigned int const sMaxLogCategories  = 32;
  static unsigned int const sMaxLogCategoryLen = 32;
  static LogLevel_t   const sInvalidCategory = static_cast<LogLevel_t>(-1);
  static unsigned int const sMsgBufLen = 1024;

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
