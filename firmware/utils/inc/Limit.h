#pragma once
// *******************************************************************************
//
// Project: Utils.
//
// Module: Limit base class.
//
// *******************************************************************************

//! \file
//! \brief Limit class.
//! \ingroup utils

// ******************************************************************************
//
//        Copyright (c) 2016-2022, Martin Garon, All rights reserved.
//
// This source code is licensed under the GPL-3.0-style license found in the
// LICENSE file in the root directory of this source tree.
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

//! \brief Limit class.
class Limit {
public:
    constexpr explicit Limit(
        const unsigned int aLower,
        const unsigned int aUpper,
        const unsigned int aVal = 0
    ) noexcept
        : mLowerLimit(aLower)
        , mUpperLimit(aUpper)
        , mVal(
            [&]() -> unsigned int {
                if (aVal < aLower) {
                    return aLower;
                } else if (aUpper < aVal) {
                    return aUpper;
                } else {
                    return aVal;
                }
            }()
        ) {}

    constexpr unsigned int Get(void) const noexcept {return mVal;}

    constexpr Limit& operator++ () {
        if (mVal == mUpperLimit) {
            mVal = mLowerLimit;
        } else {
            ++mVal;
        }
        return *this;
    }

    constexpr Limit operator++ (int) {
        Limit lResult(*this);
        ++(*this);
        return lResult;
    }

    constexpr Limit& operator-- () {
        if (mVal <= mLowerLimit) {
            mVal = mUpperLimit;
        } else {
            --mVal;
        }
        return *this;
    }

    constexpr Limit operator-- (int) {
        // Make a copy for result.
        // Now use the prefix version to do the work.
        // Return the copy (the old) value.
        Limit result(*this);
        --(*this);
        return result;
    }

    constexpr bool operator==(Limit const &rhs) {
        if (this->mVal == rhs.mVal) {
            return true;
        }
        return false;
    }

    constexpr bool operator!=(Limit const &rhs) { return !(*this == rhs); }

private:
    unsigned int mLowerLimit;
    unsigned int mUpperLimit;
    unsigned int mVal;
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
