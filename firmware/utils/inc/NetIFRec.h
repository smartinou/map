#pragma once
// *******************************************************************************
//
// Project: Beast Feed'Her.
//
// Module: Feeding calendar class.
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

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Brief description.
//! Details follow...
//! ...here.
class NetIFRec : public DBRec {
 public:
  NetIFRec();
  ~NetIFRec() {}

  bool     UseDHCP(void) const;
  uint32_t GetIPAddr(void) const;
  uint32_t GetSubnetMask(void) const;
  uint32_t GetGWAddr(void) const;

  void SetUseDHCP(bool aUseHDCP);
  void SetIPAddr(uint32_t aIPAddr);
  void SetSubnetMask(uint32_t aSubnetMask);
  void SetGWAddr(uint32_t aGWAddr);

  bool IsSane(void) const;
  bool IsDirty(void) const;
  void ResetDflt(void);

  // Simple Serialize/Deserialize methods.
  unsigned int GetRecSize(void) const;
  void Serialize(  uint8_t       * const aDataPtr) const;
  void Deserialize(uint8_t const * const aDataPtr);

 private:
  struct RecStructTag {
    char     mMagic[4];
    uint8_t  mUseDHCP;
    uint8_t  mUseIPv6;
    uint32_t mIPAddr;
    uint8_t  mIPv6Addr[16];
    uint32_t mSubnetMask;
    uint32_t mGWAddr;
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
