#pragma once
// *******************************************************************************
//
// Project: Network utilities.
//
// Module: IPAddress.
//
// *******************************************************************************

//! \file
//! \brief IPAddress class.
//! \ingroup utils_network

// ******************************************************************************
//
//        Copyright (c) 2015-2021, Martin Garon, All rights reserved.
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

// Forward declarations.
class EthernetAddress;


// \brief IP Address.
class IPAddress final {
public:

    static const IPAddress BROADCAST;
	static constexpr size_t sSize = 4 * sizeof(uint8_t);

    constexpr IPAddress() noexcept : mValue(0), mIsAny(true) {}
    constexpr explicit IPAddress(uint8_t aB0, uint8_t aB1, uint8_t aB2, uint8_t aB3) noexcept
        : mValue(aB0 | (aB1 << 8) | (aB2 << 16) | (aB3 << 24)), mIsAny(false) {}

    constexpr explicit IPAddress(uint32_t aIn) noexcept : mValue(aIn), mIsAny(false) {}
	constexpr explicit IPAddress(const void * const aIn) noexcept
        : mValue(*reinterpret_cast<const uint32_t * const>(aIn)), mIsAny(false) {}

    bool operator == (const IPAddress &aB) const {return (mValue == aB.mValue);}
    bool operator != (const IPAddress &aB) const {return (mValue != aB.mValue);}

	uint8_t GetByte(size_t aIndex) const noexcept;
    uint32_t GetValue(void) const noexcept {return mValue;}

    void GetEthernetAddress(EthernetAddress * const aOut) const;
    void GetString(char *aOut) const;
    std::string GetString() const;
    
    bool IsBroadcast(void) const {return (mValue == 0xFFFFFFFFUL);}
    bool IsMulticast(void) const;
    bool IsUnicast(void) const {return ((!IsBroadcast()) && (!IsMulticast()));}
    bool IsAny(void) const {return mIsAny;}
    bool IsValid(void) const {return ((mValue != 0) || mIsAny);}
    bool IsValidMask(void) const;

	//PtUtilsLib::Result Set(const char *aIn);
	//void Set(const void * const aIn);
	//void Set(uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3);
    //void Set(uint32_t aValue);
    void SetAny(bool aEnable = true);
    void SetBroadcast(void) {mValue = 0xFFFFFFFFUL;}

private:
    uint32_t mValue;
    bool mIsAny;
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
