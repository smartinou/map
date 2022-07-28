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

// This project.
#include "NetIFRec.h"

// Standard Library.
#include <cstring>

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

//
// Start of IDBRec interface.
//

auto NetIFRec::IsSane() const noexcept -> bool {

    bool const lIsMagicGood = IsMagicGood(mRec.mBase, sMagic);
    if (lIsMagicGood) {
        return IsCRCGood(
            {
                reinterpret_cast<uint8_t const * const>(&mRec),
                sizeof(struct RecData)
            }
        );
    }

    return false;
}


void NetIFRec::ResetDflt() noexcept {

    // Set magic.
    mRec.mBase.mMagic = NetIFRec::sMagic;

    mRec.mUseDHCP = 1;
    mRec.mUseIPv6 = 0;
    mRec.mIPAddr  = 0x00000000;
    mRec.mSubnetMask = 0x00000000;
    mRec.mGWAddr = 0x00000000;

    SetIsDirty();
}


auto NetIFRec::GetRecSize() const noexcept -> size_t {
    return sizeof(struct RecData);
}


// Trivial serialization function.
void NetIFRec::Serialize(uint8_t * const aDataPtr) const {

    std::memcpy(aDataPtr, &mRec, GetRecSize());
}


// Trivial serialization function.
void NetIFRec::Deserialize(uint8_t const * const aDataPtr) {

    std::memcpy(&mRec, aDataPtr, GetRecSize());
}


void NetIFRec::UpdateCRC() noexcept {
    mRec.mBase.mCRC = ComputeCRC(
        {
            reinterpret_cast<uint8_t const * const>(mRec.mBase.mMagic.data()),
            //mRec.mBase.mMagic.data(),
            sizeof(struct RecData) - 1
        }
    );
}


//
// Start of child methods.
//

auto NetIFRec::UseDHCP() const noexcept -> bool {
    return static_cast<bool>(mRec.mUseDHCP);
}


auto NetIFRec::GetIPAddr() const noexcept -> uint32_t {
    return mRec.mIPAddr;
}


auto NetIFRec::GetSubnetMask() const noexcept -> uint32_t {
    return mRec.mSubnetMask;
}


auto NetIFRec::GetGWAddr() const noexcept -> uint32_t {
    return mRec.mGWAddr;
}


void NetIFRec::SetUseDHCP(bool const aUseDHCP) noexcept {
    mRec.mUseDHCP = static_cast<uint8_t>(aUseDHCP);
    SetIsDirty();
}


void NetIFRec::SetIPAddr(uint32_t const aIPAddr) noexcept {
    mRec.mIPAddr = aIPAddr;
    SetIsDirty();
}


void NetIFRec::SetSubnetMask(uint32_t const aSubnetMask) noexcept {
    mRec.mSubnetMask = aSubnetMask;
    SetIsDirty();
}


void NetIFRec::SetGWAddr(uint32_t const aGWAddr) noexcept {
    mRec.mGWAddr = aGWAddr;
    SetIsDirty();
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
