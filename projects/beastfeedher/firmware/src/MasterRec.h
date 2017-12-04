#ifndef MASTER_REC_H_
#define MASTER_REC_H_
// *******************************************************************************
//
// Project: Beast Feed'Her.
//
// Module: Master record class.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2016-2017, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include "DBRec.h"

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

// Forward declaration.
//class DBRec;
//class Calendar;
//class NetIFRec;
//class BFHMgrRec;

//! \brief Brief description.
//! Details follow...
//! ...here.
class MasterRec : public DBRec {
 public:
  MasterRec(unsigned int aRecQty);
  ~MasterRec();

  //Calendar &GetCalendar(void);

  unsigned int AddRec(DBRec * const aDBRecPtr);

  bool IsSane(void) const;
  bool IsDirty(void) const;
  void ResetDflt(void);

  // Simple Serialize/Deserialize methods.
  unsigned int GetRecSize(void) const;
  void Serialize(  uint8_t       * const aDataPtr) const;
  void Deserialize(uint8_t const * const aDataPtr);

 private:
  enum {
    VER_MAJOR = 1,
    VER_MINOR = 0,
    VER_REV   = 0
  };

  struct RecInfoStructTag {
    uint8_t mType;
    uint8_t mOffset;
    uint8_t mSize;
  };

  typedef struct RecInfoStructTag REC_INFO;

  struct RecStructTag {
    char     mMagic[4];
    uint8_t  mVerMajor;
    uint8_t  mVerMinor;
    uint8_t  mVerRev;
    uint8_t  mRecQty;
    REC_INFO mRecInfo[3];
    uint8_t  mRsvd[12];
  };

  // The actual record storage.
  struct RecStructTag mMasterRec;

  unsigned int mRecQty;
  unsigned int mRecIx;
  DBRec **mDBRec;
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
#endif // MASTER_REC_H_
