#ifndef DB_H_
#define DB_H_
// *******************************************************************************
//
// Project: Larger project scope.
//
// Module: Module in the larger project scope.
//
// *******************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group


// ******************************************************************************
//
//        Copyright (c) 2016, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Brief description.
//! Details follow...
//! ...here.
class DB {
 public:
  DB();
  ~DB() {}

  Status RdFromNVMem(void);
  Status WrToNvMem(void);

  unsigned int GetSerializedSize(void) { return mCalendarArray.size(); }
  void   Serialize(uint8_t       * const aSerDataPtr);
  void Deserialize(uint8_t const * const aSerDataPtr);

 private:
  struct RecInfoStructTag {
    uint8_t mType;
    uint8_t mOffset;
    uint8_t mSize;
  };

  typedef struct RecInfoStructTag REC_INFO;

  struct MainRecStructTag {
    char     mMagic[4];
    uint8_t  mVerMajor;
    uint8_t  mVerMinor;
    uint8_t  mVerRev;
    uint8_t  mRecQty;
    REC_INFO mRecInfo[4];
    uint8_t  mRsvd[12];
  };

  struct IPCfgRecStructTag {
    char     mMagic[4];
    //uint32_t mIPAddr;
    //uint32_t mSubnetMask;
    //uint32_t mDfltGateway;
    //uint8_t  mIPCfg;
    uint8_t mData[32 - 4];
  };

  struct FeedRecStrucTag {
    char    mMagic[4];
    //char    mBeastName[32];
    //uint8_t mFeedOnPeriod;
    //uint8_t mIsWebFeedingEnabled;
    //uint8_t mIsButtonFeedingEnabled;
    //uint8_t mIsCLIFeedingEnabled;
    //uint8_t mRsvd[24];
    uint8_t mData[64 - 4];
  };

  struct CalendarRecStructTag {
    char    mMagic[4];
    //uint8_t mSerializedDate[124];
    uint8_t mData[128 - 4];
  };

  struct DBStructTag {
    struct MainRecStructTag     mMainRec;
    struct IPCfgRecStructTag    mIPCfgRec;
    struct FeedRecStrucTag      mFeedRec;
    struct CalendarRecStructTag mCalendarRec;
  };

  typedef struct DBStructTag DB;

 private:
  // Local storage for DB.
  DB mDB;
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
#endif // DB_H_
