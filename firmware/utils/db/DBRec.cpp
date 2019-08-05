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
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
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

std::vector <DBRec *> DBRec::mRecList;

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

// DB static methods.
bool DBRec::IsDBSane(void) {
    for (std::vector<DBRec *>::iterator lDBIt = mRecList.begin(); lDBIt != mRecList.end(); ++lDBIt) {
        // Return after 1st "insane" record.
        if (!(*lDBIt)->IsSane()) {
            return false;
        }
    }

    return true;
}


bool DBRec::IsDBDirty(void) {
    for (std::vector<DBRec *>::const_iterator lDBIt = mRecList.begin(); lDBIt != mRecList.end(); ++lDBIt) {
        // Return after 1st dirty record.
        if ((*lDBIt)->IsDirty()) {
            return true;
        }
    }

    return false;
}


void DBRec::ResetDBDflt(void) {
    for (std::vector<DBRec *>::iterator lDBIt = mRecList.begin(); lDBIt != mRecList.end(); ++lDBIt) {
        (*lDBIt)->ResetDflt();
    }
}


unsigned int DBRec::GetDBSize(void) {

    unsigned int lDBSize = 0;
    for (std::vector<DBRec *>::const_iterator lDBIt = mRecList.begin(); lDBIt != mRecList.end(); ++lDBIt) {
        lDBSize += (*lDBIt)->GetRecSize();
    }

    return lDBSize;
}


void DBRec::SerializeDB(uint8_t * aData) {
    for (std::vector<DBRec *>::const_iterator lDBIt = mRecList.begin(); lDBIt != mRecList.end(); ++lDBIt) {
        (*lDBIt)->Serialize(aData);
        unsigned int lSize = (*lDBIt)->GetRecSize();
        aData += lSize;
    }
}


void DBRec::DeserializeDB(uint8_t const * aData) {
    for (std::vector<DBRec *>::iterator lDBIt = mRecList.begin(); lDBIt != mRecList.end(); ++lDBIt) {
        (*lDBIt)->Deserialize(aData);
        unsigned int lSize = (*lDBIt)->GetRecSize();
        aData += lSize;
    }
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

DBRec::DBRec()
    : mIsDirty(false) {

    // Ctor body.
    AddRec();
}


DBRec::~DBRec() {
    // Dtor body left intentionally empty.
}


void DBRec::AddRec(void) {
    mRecList.push_back(this);
}


uint8_t DBRec::ComputeCRC(uint8_t const * const aData, unsigned int aSize) {
    uint8_t lCRC = 0;
    for (unsigned int lIx = 1; lIx < aSize; lIx++) {
        lCRC += aData[lIx];
    }

    return ~lCRC;
}


bool DBRec::IsCRCGood(uint8_t const * const aData, unsigned int aSize) {
    uint8_t lCRC = 0;
    for (unsigned int lIx = 0; lIx < aSize; lIx++) {
        lCRC += aData[lIx];
    }

    lCRC++;

    // Total CRC (data + CRC) should yield 0.
    if (lCRC) {
        return false;
    }

    return true;
}

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
