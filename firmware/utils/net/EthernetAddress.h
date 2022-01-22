#pragma once
// *******************************************************************************
//
// Project: Network utilities.
//
// Module: Ethernet.
//
// *******************************************************************************

//! \file
//! \brief Ethernet class.
//! \ingroup utils_network

// ******************************************************************************
//
//        Copyright (c) 2015-2021, Martin Garon, All rights reserved.
//
// ******************************************************************************

// ******************************************************************************
//                              INCLUDE FILES
// ******************************************************************************

#include <cstdint>
#include <string>
#include <string.h>

// ******************************************************************************
//                       DEFINED CONSTANTS AND MACROS
// ******************************************************************************

// ******************************************************************************
//                         TYPEDEFS AND STRUCTURES
// ******************************************************************************

//! \brief Ethernet Address.
class EthernetAddress final {
public:

    static const EthernetAddress sBroadcast;
	static constexpr size_t sSize{6 * sizeof(uint8_t)};

    constexpr EthernetAddress() noexcept : mValue{0} {}
	constexpr explicit EthernetAddress(
        uint8_t aB0, uint8_t aB1, uint8_t aB2, uint8_t aB3, uint8_t aB4, uint8_t aB5
    ) noexcept
        : mValue{aB0, aB1, aB2, aB3, aB4, aB5} {}
	constexpr explicit EthernetAddress(const char * const aStr) noexcept
        : EthernetAddress(aStr[0], aStr[1], aStr[2], aStr[3], aStr[4], aStr[5]) {}
    constexpr explicit EthernetAddress(const uint64_t &aValue) noexcept
        : EthernetAddress(
            static_cast<uint8_t>((aValue & 0x0000FF0000000000LL) >> 40),
            static_cast<uint8_t>((aValue & 0x000000FF00000000LL) >> 32),
            static_cast<uint8_t>((aValue & 0x00000000FF000000LL) >> 24),
            static_cast<uint8_t>((aValue & 0x0000000000FF0000LL) >> 16),
            static_cast<uint8_t>((aValue & 0x000000000000FF00LL) >>  8),
            static_cast<uint8_t>((aValue & 0x00000000000000FFLL) >>  0)
        ) {}
    explicit EthernetAddress(const std::string &aStr)
        : EthernetAddress(aStr.c_str()) {}

    bool operator == (const EthernetAddress &aAddress) const {return (memcmp(mValue, aAddress.mValue, sizeof(mValue)) == 0 );}
    bool operator != (const EthernetAddress &aAddress) const {return (memcmp(mValue, aAddress.mValue, sizeof(mValue)) != 0 );}

	constexpr uint8_t GetByte(size_t aIndex) const {return mValue[aIndex];}

    void GetData(uint8_t * aByte) const noexcept;
    void GetString(char * aOut) const noexcept;
    std::string GetString(void) const noexcept;
    uint64_t GetValue(void) const noexcept;

	bool IsBroadcast(void) const noexcept;
	bool IsMulticast(void) const noexcept;
	bool IsUnicast(void) const noexcept;
	bool IsValid(void) const noexcept;

    constexpr void Set(unsigned int aIndex, uint8_t aValue) noexcept {mValue[aIndex] = aValue;}

private:
	// Bytes to bytes copy is allowed.
	uint8_t mValue[EthernetAddress::sSize];
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
