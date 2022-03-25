// *****************************************************************************
//
// Project: Utils
//
// Module: DB.
//
// *******************************************************************************

//! \file
//! \brief DB record class.
//! \ingroup utils_db

// *****************************************************************************
//
//        Copyright (c) 2015-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

// Standard Library.
#include <stddef.h>

// This project.
#include "DBRec.h"

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

std::vector<DBRec *> DBRec::mRecList;

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

// DB static methods.
bool DBRec::IsDBSane(void) {
    for (const auto lRec : mRecList) {
        // Return after 1st corrupted record.
        if (!lRec->IsSane()) {
            return false;
        }
    }

    return true;
}


bool DBRec::IsDBDirty(void) {
    for (const auto lRec : mRecList) {
        // Return after 1st dirty record.
        if (lRec->IsDirty()) {
            return true;
        }
    }

    return false;
}


void DBRec::ResetDBDflt(void) {
    for (const auto lRec : mRecList) {
        lRec->ResetDflt();
    }

    return;
}


size_t DBRec::GetDBSize(void) {

    size_t lDBSize = 0;
    for (const auto lRec : mRecList) {
        lDBSize += lRec->GetRecSize();
    }

    return lDBSize;
}


void DBRec::SerializeDB(uint8_t * aData) {
    for (const auto lRec : mRecList) {
        lRec->Serialize(aData);
        unsigned int lSize = lRec->GetRecSize();
        aData += lSize;
    }

    return;
}


void DBRec::DeserializeDB(uint8_t const * aData) {
    for (const auto lRec : mRecList) {
        lRec->Deserialize(aData);
        unsigned int lSize = lRec->GetRecSize();
        aData += lSize;
    }

    return;
}


void DBRec::StaticUpdateCRC(void) {
    for (auto lRec : mRecList) {
        lRec->UpdateCRC();
    }

    return;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

DBRec::DBRec()
    : mIsDirty(false) {

    // Ctor body.
    AddRec();
}


void DBRec::AddRec(void) {
    mRecList.push_back(this);
    return;
}


uint8_t DBRec::ComputeCRC(uint8_t const * const aData, size_t const aSize) const {

    uint8_t lCRC = 0;
    // Computed CRC is at offset 0 of struct BaseRec that prepends remaining data.
    for (size_t lIx = 1; lIx < aSize; lIx++) {
        lCRC += aData[lIx];
    }
    return ~lCRC;
}


bool DBRec::IsMagicGood(struct BaseRec const * const aBaseRec, char const aMagic[]) const {

    if ((aMagic[0] == aBaseRec->mMagic[0])
        && (aMagic[1] == aBaseRec->mMagic[1])
        && (aMagic[2] == aBaseRec->mMagic[2])
    ) {
        return true;
    }
    return false;
}


bool DBRec::IsCRCGood(uint8_t const * const aData, size_t const aSize) const {
    const uint8_t lRecCRC = reinterpret_cast<DBRec::BaseRec const * const>(aData)->mCRC;
    const uint8_t lComputedCRC = ComputeCRC(aData, aSize);
    if (lComputedCRC == lRecCRC) {
        return true;
    }

    return false;
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
