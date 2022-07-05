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

std::vector<DBRec::Ptr> DBRec::mRecList;

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

DBRec::DBRec(Token)
    : mIsDirty(false)
{
    // Ctor body.
}


// DB static methods.
bool DBRec::IsDBSane(void) {
    for (const auto& lRec : mRecList) {
        // Return after 1st corrupted record.
        if (!lRec->IsSane()) {
            return false;
        }
    }

    return true;
}


bool DBRec::IsDBDirty(void) {
    for (const auto& lRec : mRecList) {
        // Return after 1st dirty record.
        if (lRec->IsDirty()) {
            return true;
        }
    }

    return false;
}


void DBRec::ResetDBDflt(void) {
    for (const auto& lRec : mRecList) {
        lRec->ResetDflt();
    }

    return;
}


size_t DBRec::GetDBSize(void) {

    size_t lDBSize = 0;
    for (const auto& lRec : mRecList) {
        lDBSize += lRec->GetRecSize();
    }

    return lDBSize;
}


void DBRec::SerializeDB(uint8_t * aData) {
    for (const auto& lRec : mRecList) {
        lRec->Serialize(aData);
        unsigned int lSize = lRec->GetRecSize();
        aData += lSize;
    }

    return;
}


void DBRec::DeserializeDB(uint8_t const * aData) {
    for (const auto& lRec : mRecList) {
        lRec->Deserialize(aData);
        unsigned int lSize = lRec->GetRecSize();
        aData += lSize;
    }

    return;
}


void DBRec::StaticUpdateCRC(void) {
    for (const auto& lRec : mRecList) {
        lRec->UpdateCRC();
    }

    return;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

void DBRec::AddRec(DBRec::Ptr aDBRec) {
    mRecList.push_back(aDBRec);
}


uint8_t DBRec::ComputeCRC(std::span<uint8_t const> const &aSpan) const {

    uint8_t lCRC = 0;
    for (auto const lByte : aSpan) {
        lCRC += lByte;
    }
    return ~lCRC;
}


bool DBRec::IsMagicGood(
    struct BaseRec const &aBaseRec,
    Magic const &aMagic) const
{
    return (aMagic == aBaseRec.mMagic);
}


bool DBRec::IsCRCGood(std::span<uint8_t const> const &aSpan) const {
    // Span contains full record, including computed CRC.
    // Result should be 0x00.
    auto const lComputedCRC = ComputeCRC(aSpan);
    return (lComputedCRC == 0);
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
