// *****************************************************************************
//
// Project: Network utilities
//
// Module: Ethernet.
//
// *****************************************************************************

//! \file
//! \brief Ethernet class.
//! \ingroup utils_network

// *****************************************************************************
//
//        Copyright (c) 2016-2021, Martin Garon, All rights reserved.
//
// *****************************************************************************

// *****************************************************************************
//                              INCLUDE FILES
// *****************************************************************************

#include <algorithm>

#include <net/EthernetAddress.h>

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

const EthernetAddress EthernetAddress::sBroadcast{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

void EthernetAddress::GetData(uint8_t * aOutByte) const noexcept {
    for (auto const &lByte : mValue) {
        *aOutByte = lByte; aOutByte++;
    }
    return;
}

///
/// \brief Get data as a string.
///

void EthernetAddress::GetString(char *aOut) const noexcept {

    static constexpr auto sDigits{"0123456789abcdef"};
    for (auto const &lByte : mValue) {
        *aOut = sDigits[lByte >> 4]; aOut++;
        *aOut = sDigits[lByte & 0x0F]; aOut++;
        *aOut = ':'; aOut++;
    }

	aOut--;
	*aOut = '\0';
    return;
}


///
/// \brief Returns a PtUtilsLib::String representation of the Ethernet address
///
// #include <sstream>
// #include <iomanip>

std::string EthernetAddress::GetString(void) const noexcept
{
#if 0
    std::stringstream lSS;
    lSS << std::hex;
    lSS << std::setfill( '0' ) << std::setw( 2 ) << (int)mValue[ 0 ] << ":";
    lSS << std::setfill( '0' ) << std::setw( 2 ) << (int)mValue[ 1 ] << ":";
    lSS << std::setfill( '0' ) << std::setw( 2 ) << (int)mValue[ 2 ] << ":";
    lSS << std::setfill( '0' ) << std::setw( 2 ) << (int)mValue[ 3 ] << ":";
    lSS << std::setfill( '0' ) << std::setw( 2 ) << (int)mValue[ 4 ] << ":";
    lSS << std::setfill( '0' ) << std::setw( 2 ) << (int)mValue[ 5 ];

    return lSS.str();
#else
    // Cheaper version.
    char lStr[32]{0};
    GetString(&lStr[0]);
    return std::string(lStr);
#endif
}


///
/// \brief Returns a 64-bits numerical representation of the address
///

uint64_t EthernetAddress::GetValue(void) const noexcept {
    uint64_t lValue = 0;
    lValue |= static_cast<uint64_t>(mValue[0]) << ((6 - 1) * 8);
    lValue |= static_cast<uint64_t>(mValue[1]) << ((5 - 1) * 8);
    lValue |= static_cast<uint64_t>(mValue[2]) << ((4 - 1) * 8);
    lValue |= static_cast<uint64_t>(mValue[3]) << ((3 - 1) * 8);
    lValue |= static_cast<uint64_t>(mValue[4]) << ((2 - 1) * 8);
    lValue |= static_cast<uint64_t>(mValue[5]) << ((1 - 1) * 8);

    return lValue;
}


///
/// \brief Is the address a broadcast address?
///

bool EthernetAddress::IsBroadcast(void) const noexcept {
    bool lFound = std::any_of(
        std::cbegin(mValue),
        std::cend(mValue),
        [](const uint8_t aByte) {return (aByte != 0xFF);}
    );
    if (lFound) {
        return false;
    }

	return true;
}


///
/// \brief Is the ethernet address multicast?
///

bool EthernetAddress::IsMulticast(void) const noexcept {
    return (
        (mValue[ 0 ] == 0x01) && 
        (mValue[ 1 ] == 0x00) && 
        (mValue[ 2 ] == 0x5e) && 
        ((mValue[ 3 ] & 0x80) == 0x00)
    );
}


///
/// \brief Is the ethernet address unicast? (not broadcast)
///

bool EthernetAddress::IsUnicast(void) const noexcept {
    return (!IsBroadcast() && !IsMulticast());
}


///
/// \brief Is the ethernet address valid?
///

bool EthernetAddress::IsValid(void) const noexcept {
    bool lFound = std::any_of(
        std::cbegin(mValue),
        std::cend(mValue),
        [](const uint8_t aByte) {return (aByte != 0x00);}
    );
    if (lFound) {
        return true;
    }

	// If we made it here we're stuck with a bunch of zeroes.
	return false;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
