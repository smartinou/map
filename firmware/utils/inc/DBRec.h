#ifndef DB_REC_H_
#define DB_REC_H_
// *******************************************************************************
//
// Project: Beast Feed'Her!
//
// Module: DB record base abstract class.
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

#include <stddef.h>
#include <stdint.h>

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Brief description.
//! Details follow...
//! ...here.

//! \brief Button component.
class DBRec {
 public:
  DBRec();
  virtual ~DBRec() {}

  unsigned int GetMyIx(void);

  virtual bool IsSane(void) const = 0;
  virtual bool IsDirty(void) const;
  virtual void ResetDflt(void) = 0;

  virtual unsigned int GetRecSize(void) const = 0;
  virtual void         Serialize(uint8_t * const aDataPtr) const = 0;
  virtual void         Deserialize(uint8_t const * const aDataPtr) = 0;

 protected:
  bool mIsDirty;

 private:
  static unsigned int mDBObjCnt;

  unsigned int mDBObjIx;
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
#endif //DB_REC_H_
