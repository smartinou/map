// *****************************************************************************
//
// Project: Beast Feed'Her
//
// Module: Feeding calendar class.
//
// *****************************************************************************

//! \file
//! \brief MyClass device class.
//! \ingroup module_group

// *****************************************************************************
//
//        Copyright (c) 2016-2017, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// Standard Library.
#include <string.h>

// Common Library.
#include "DBRec.h"

// This project.
#include "NetIFRec.h"

// *****************************************************************************
//                      DEFINED CONSTANTS AND MACROS
// *****************************************************************************

// *****************************************************************************
//                         TYPEDEFS AND STRUCTURES
// *****************************************************************************

// *****************************************************************************
//                            FUNCTION PROTOTYPES
// *****************************************************************************

// *****************************************************************************
//                             GLOBAL VARIABLES
// *****************************************************************************

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

NetIFRec::NetIFRec()
  : DBRec()
  , mRec{0} {

  // Ctor body left intentionally empty.
}


bool NetIFRec::UseDHCP(void) const {
  return mRec.mUseDHCP;
}


uint32_t NetIFRec::GetIPAddr(void) const {
  return mRec.mIPAddr;
}


uint32_t NetIFRec::GetSubnetMask(void) const {
  return mRec.mSubnetMask;
}


uint32_t NetIFRec::GetGWAddr(void) const {
  return mRec.mGWAddr;
}


void NetIFRec::SetUseDHCP(bool aUseDHCP) {
  mRec.mUseDHCP = aUseDHCP;
}


void NetIFRec::SetIPAddr(uint32_t aIPAddr) {
  mRec.mIPAddr = aIPAddr;
}


void NetIFRec::SetSubnetMask(uint32_t aSubnetMask) {
  mRec.mSubnetMask = aSubnetMask;
}


void NetIFRec::SetGWAddr(uint32_t aGWAddr) {
  mRec.mGWAddr = aGWAddr;
}


bool NetIFRec::IsSane(void) const {

  // Check magic value.
  if (('N' != mRec.mMagic[0])
      || ('E' != mRec.mMagic[1])
      || ('T' != mRec.mMagic[2])
      || ('X' != mRec.mMagic[3])) {
    return false;
  }

  return true;
}


bool NetIFRec::IsDirty(void) const {
  return mIsDirty;
}


void NetIFRec::ResetDflt(void) {

  // Set magic.
  mRec.mMagic[0] = 'N';
  mRec.mMagic[1] = 'E';
  mRec.mMagic[2] = 'T';
  mRec.mMagic[3] = 'X';

  mRec.mUseDHCP = 1;
  mRec.mUseIPv6 = 0;
  mRec.mIPAddr  = 0x00000000;
  mRec.mSubnetMask = 0x00000000;
  mRec.mGWAddr = 0x00000000;

  mIsDirty = true;
}


unsigned int NetIFRec::GetRecSize(void) const {
  return sizeof(struct RecStructTag);
}


// Trivial serialization function.
void NetIFRec::Serialize(uint8_t * const aDataPtr) const {

  memcpy(aDataPtr, &mRec, GetRecSize());
}


// Trivial serialization function.
void NetIFRec::Deserialize(uint8_t const *aDataPtr) {

  memcpy(&mRec, aDataPtr, GetRecSize());
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
