// *****************************************************************************
//
// Project: Utils.
//
// Module: Feeding calendar.
//
// *****************************************************************************

//! \file
//! \brief Feeding calendar class class.
//! \ingroup utils_db

// *****************************************************************************
//
//        Copyright (c) 2016-2019, Martin Garon, All rights reserved.
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

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

NetIFRec::NetIFRec()
    : DBRec()
    , mRec{0} {

    // Ctor body left intentionally empty.
}


NetIFRec::~NetIFRec() {
    // Dtor body left intentionally empty.
}


//
// Start of IDBRec interface.
//

bool NetIFRec::IsSane(void) {
    // Check CRC.
    if (!IsCRCGood(reinterpret_cast<uint8_t *>(&mRec), sizeof(mRec))) {
        return false;
    }

    // Check magic value.
    if (('N' != mRec.mMagic[0])
          || ('E' != mRec.mMagic[1])
          || ('T' != mRec.mMagic[2])
    ) {
        return false;
    }

    return true;
}


void NetIFRec::ResetDflt(void) {

    // Set magic.
    mRec.mMagic[0] = 'N';
    mRec.mMagic[1] = 'E';
    mRec.mMagic[2] = 'T';

    mRec.mUseDHCP = 1;
    mRec.mUseIPv6 = 0;
    mRec.mIPAddr  = 0x00000000;
    mRec.mSubnetMask = 0x00000000;
    mRec.mGWAddr = 0x00000000;

    mRec.mCRC = ComputeCRC(reinterpret_cast<uint8_t *>(&mRec), sizeof(mRec));
    SetIsDirty();
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

//
// Start of child methods.
//

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

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
