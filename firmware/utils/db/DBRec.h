#pragma once
// *******************************************************************************
//
// Project: Utils
//
// Module: DB.
//
// *******************************************************************************

//! \file
//! \brief DB record class.
//! \ingroup utils_db

// ******************************************************************************
//
//        Copyright (c) 2015-2019, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <stddef.h>
#include <stdint.h>
#include <vector>

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Brief description.
//! Details follow...
//! ...here.
class DBRec {
public:
    DBRec();
    ~DBRec();

    bool IsDirty(void) const { return mIsDirty; }

    // DBRec.
    virtual bool IsSane(void) = 0;
    virtual void ResetDflt(void) = 0;

    // DB static methods.
    static bool IsDBSane(void);
    static bool IsDBDirty(void);
    static void ResetDBDflt(void);
    static unsigned int GetDBSize(void);
    static unsigned int GetDBRecCount(void) { return mRecList.size(); }
    static void SerializeDB(uint8_t * aData);
    static void DeserializeDB(uint8_t const * aData);

protected:
    void SetIsDirty(void) { mIsDirty = true; }
    bool IsCRCGood(uint8_t const * const aData, unsigned int aSize);
    uint8_t ComputeCRC(uint8_t const * const aData, unsigned int aSize);

private:
    virtual unsigned int GetRecSize(void) const = 0;
    virtual void Serialize(uint8_t * const aData) const = 0;
    virtual void Deserialize(uint8_t const * const aData) = 0;

    void AddRec(void);

    static std::vector <DBRec *> mRecList;
    bool mIsDirty;
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
