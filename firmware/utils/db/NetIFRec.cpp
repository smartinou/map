// *****************************************************************************
//
// Project: Utils.
//
// Module: Network interface DB.
//
// *****************************************************************************

//! \file
//! \brief Network interface DB class.
//! \ingroup utils_db

// *****************************************************************************
//
//        Copyright (c) 2016-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// Standard Library.
#include <string.h>

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

char constexpr NetIFRec::sMagic[3];

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

NetIFRec::NetIFRec()
    : DBRec()
    , mRec{0} {

    // Ctor body left intentionally empty.
}


//
// Start of IDBRec interface.
//

bool NetIFRec::IsSane(void) const {
    // Check magic.
    if (!IsMagicGood(reinterpret_cast<BaseRec const * const>(&mRec.mBase), &NetIFRec::sMagic[0])) {
        return false;
    }

    // Check CRC.
    if (!IsCRCGood(reinterpret_cast<uint8_t const * const>(&mRec), sizeof(struct RecData))) {
        return false;
    }

    return true;
}


void NetIFRec::ResetDflt(void) {

    // Set magic.
    mRec.mBase.mMagic[0] = NetIFRec::sMagic[0];
    mRec.mBase.mMagic[1] = NetIFRec::sMagic[1];
    mRec.mBase.mMagic[2] = NetIFRec::sMagic[2];

    mRec.mUseDHCP = 1;
    mRec.mUseIPv6 = 0;
    mRec.mIPAddr  = 0x00000000;
    mRec.mSubnetMask = 0x00000000;
    mRec.mGWAddr = 0x00000000;

    SetIsDirty();
}


unsigned int NetIFRec::GetRecSize(void) const {
    return sizeof(struct RecData);
}


// Trivial serialization function.
void NetIFRec::Serialize(uint8_t * const aDataPtr) const {

    memcpy(aDataPtr, &mRec, GetRecSize());
}


// Trivial serialization function.
void NetIFRec::Deserialize(uint8_t const *aDataPtr) {

    memcpy(&mRec, aDataPtr, GetRecSize());
}


void NetIFRec::UpdateCRC() {
    mRec.mBase.mCRC = ComputeCRC(
        reinterpret_cast<uint8_t const * const>(&mRec),
        sizeof(struct RecData)
    );
}


//
// Start of child methods.
//

bool NetIFRec::UseDHCP(void) const {
    return static_cast<bool>(mRec.mUseDHCP);
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
    SetIsDirty();
}


void NetIFRec::SetIPAddr(uint32_t aIPAddr) {
    mRec.mIPAddr = aIPAddr;
    SetIsDirty();
}


void NetIFRec::SetSubnetMask(uint32_t aSubnetMask) {
    mRec.mSubnetMask = aSubnetMask;
    SetIsDirty();
}


void NetIFRec::SetGWAddr(uint32_t aGWAddr) {
    mRec.mGWAddr = aGWAddr;
    SetIsDirty();
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
