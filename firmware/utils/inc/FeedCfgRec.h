#pragma once
// *******************************************************************************
//
// Project: Beast Feed'Her.
//
// Module: Feeding configuration class.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2016-2018, Martin Garon, All rights reserved.
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

//! \brief Brief description.
//! Details follow...
//! ...here.
class FeedCfgRec : public DBRec {
 public:
  FeedCfgRec();
  ~FeedCfgRec() {}

  uint8_t GetTimedFeedPeriod(void) const;
  bool    IsWebFeedingEnable(void) const;
  bool    IsAutoPetFeedingEnable(void) const;

  void SetTimedFeedPeriod(uint8_t aPeriod);
  void SetIsWebFeedingEnabled(bool aIsEnabled);
  void SetIsAutoPetFeedingEnabled(bool aIsEnabled);

  bool IsSane(void);
  bool IsDirty(void) const;
  void ResetDflt(void);

  // Simple Serialize/Deserialize methods.
  unsigned int GetRecSize(void) const;
  void Serialize(  uint8_t       * const aDataPtr) const;
  void Deserialize(uint8_t const * const aDataPtr);

 private:
  struct RecStructTag {
    uint8_t  mCRC;
    char     mMagic[3];
    uint8_t  mTimedFeedPeriod;
    bool     mIsWebFeedingEnable;
    bool     mIsAutoPetFeedingEnable;
  };

  struct RecStructTag mRec;
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
