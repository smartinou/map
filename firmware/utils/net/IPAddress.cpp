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

//#include <sstream>

#include "EthernetAddress.h"
#include "IPAddress.h"

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

const IPAddress IPAddress::BROADCAST(0xFF, 0xFF, 0xFF, 0xFF);

// *****************************************************************************
//                            EXPORTED FUNCTIONS
// *****************************************************************************

///
/// \brief Data accessor.
///

uint8_t IPAddress::GetByte(size_t aIndex) const {

	return (reinterpret_cast<const uint8_t * const>(&mValue)[aIndex]);
}


void IPAddress::SetByte(size_t aIndex, uint8_t aByte) noexcept {
    reinterpret_cast<uint8_t * const>(&mValue)[aIndex] = aByte;
}


///
/// \brief Get the Ethernet address associated to this IP address.
///

void IPAddress::GetEthernetAddress(EthernetAddress * const aOut) const
{
    //PtAssert(aOut != nullptr);
    //PtAssert(IsValid());
    //PtAssert(IsBroadcast() || IsMulticast());

    if (IsMulticast()) {
        aOut->Set(0, 0x01);
        aOut->Set(1, 0x00);
        aOut->Set(2, 0x5e);
        aOut->Set(3, GetByte(1) & 0x7f);
        aOut->Set(4, GetByte(2));
        aOut->Set(5, GetByte(3));
    } else {
        *aOut = EthernetAddress::sBroadcast;
    }
}


///
/// \brief Convert the address to a string.
///
/// \param aOut The method store the string here.
///

void IPAddress::GetString(char * aOut) const {

	for (unsigned int lIndex = 0; lIndex < sSize; lIndex++) {
		unsigned short lByte = GetByte(lIndex);
		if (lByte >= 100) {
			*aOut = static_cast<char>('0' + (lByte / 100)); aOut++;
			lByte %= 100;
			*aOut = static_cast<char>('0' + (lByte / 10)); aOut++;
			lByte %= 10;
			*aOut = static_cast<char>('0' + lByte); aOut++;
		} else if (lByte >= 10) {
			*aOut = static_cast<char>('0' + (lByte / 10)); aOut++;
			lByte %= 10;
			*aOut = static_cast<char>('0' + lByte); aOut++;
		} else {
			*aOut = static_cast<char>('0' + lByte); aOut++;
		}

		*aOut = '.'; aOut++;
	}

	aOut--;
	*aOut = '\0';
}


///
/// \brief Returns a PtUtilsLib::String representation of the IP address.
///

std::string IPAddress::GetString() const {
#if 0
    const uint8_t * const lData = reinterpret_cast<const uint8_t *>(&mValue);

    std::stringstream lSS;
    lSS << (int)lData[0] << ".";
    lSS << (int)lData[1] << ".";
    lSS << (int)lData[2] << ".";
    lSS << (int)lData[3];

    //return lSS.str().c_str();
    return lSS.str();
#else
    // Cheaper version.
    char lStr[32]{0};
    GetString(&lStr[0]);
    return std::string(lStr);
#endif
}


///
/// \brief Checks if the IP address is in the multicast range.
///

bool IPAddress::IsMulticast() const {
    return false; // ((PtH2LE32(mValue) & 0x000000F0) == 0x000000E0);
}


///
/// \brief Returns true if the IP address contains a valid subnet mask
///

bool IPAddress::IsValidMask() const {
	// Convert mask to BE, if it is not already
    uint32_t lMask = 0xFFFFFFFF; // PtH2N32(mValue);

    // Scan from LSB to MSB
    bool lOne = false;
    bool lZero = false;
    for (int lBitIndex = (sizeof(uint32_t) - 1); lBitIndex >= 0; lBitIndex--) {
        uint32_t lBit = (lMask >> lBitIndex) & 1;
        if (lBit == 0) {
            // Found a Zero
            lZero = true;
        } else {
            // Found a One
            lOne = true;
            if (lZero) {
                // Fail if we already found a Zero
                return false;
            }
        }
    }

    return (lOne && lZero);
}


///
/// \brief Sets as the 'any' address.
///

void IPAddress::SetAny(bool aEnable) {
    mIsAny = true;
    mValue = 0;
}

// *****************************************************************************
//                              LOCAL FUNCTIONS
// *****************************************************************************

// *****************************************************************************
//                                END OF FILE
// *****************************************************************************
