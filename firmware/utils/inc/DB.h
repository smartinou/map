#pragma once
// *******************************************************************************
//
// Project: Beast Feed'Her!
//
// Module: DB holder class.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// ******************************************************************************
//
//        Copyright (c) 2015-2018, Martin Garon, All rights reserved.
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

// Forward declarations.
class DBRec;


//! \brief Brief description.
//! Details follow...
//! ...here.

//! \brief Button component.
class DB {
 public:
  static void AddRec(DBRec * const aDBRecPtr);

  static bool IsSane(void);
  static bool IsDirty(void);
  static void ResetDflt(void);

  static unsigned int GetSize(void);
  static void Serialize(uint8_t *aDataPtr);
  static void Deserialize(uint8_t const * const aDataPtr);

 private:
  static bool IsSane(DBRec * const aDBRecPtr);
  static bool IsDirty(DBRec * const aDBRecPtr);

  static DBRec       *mRootDBRecPtr;
  static unsigned int mDBRecObjCnt;
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
