// *****************************************************************************
//
//     Copyright (c) 2006, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include <sstream>

#include "EthernetAddress.h"
#include "IPAddress.h"


const IPAddress IPAddress::BROADCAST(0xFF, 0xFF, 0xFF, 0xFF);
constexpr size_t IPAddress::SIZE;


///
/// \brief Constructor.
///

IPAddress::IPAddress(uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3)
	: mValue(0)
    , mAny(false) {

	Set(a0, a1, a2, a3);
}


///
/// \brief Constructor
///

IPAddress::IPAddress(uint32_t aIn)
	: mValue(aIn)
    , mAny(false) {

}


///
/// \brief Constructor
///

IPAddress::IPAddress(const void * const aIn)
	: mValue(*reinterpret_cast<const uint32_t * const>(aIn))
    , mAny(false) {

}


///
/// \brief Constructor
///

IPAddress::IPAddress(const IPAddress &aIn)
	: mValue(aIn.mValue)
    , mAny(false)
{
}


///
/// \brief Constructor.
///

IPAddress::IPAddress(const char * const aIn)
	: mValue(0)
    , mAny(false) {
	
    Set(aIn);
}


///
/// \brief Constructor.
///

IPAddress::IPAddress(const std::string &aIn)
	: mValue(0)
    , mAny(false) {
	
    Set(aIn.c_str());
}


///
/// \brief Data accessor.
///

uint8_t IPAddress::GetByte(uint32_t aIndex) const {

    //PtAssert(aIndex < sizeof(mValue));
	return (reinterpret_cast< const uint8_t * const>(&mValue)[aIndex]);
}


///
/// \brief Data accessor (const)
///

const void *IPAddress::GetData() const {
	return (&mValue);
}


///
/// \brief Data accessor
///

void *IPAddress::GetData() {
	return (&mValue);
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
        *aOut = EthernetAddress::BROADCAST;
    }
}


///
/// \brief Data accessor.
///

unsigned short IPAddress::GetWord(uint32_t aIndex) const {
	//PtAssert(aIndex < (sizeof(mValue) / sizeof(unsigned short)));
	return (reinterpret_cast< const unsigned short * const>(&mValue)[aIndex]);
}


///
/// \brief Convert the address to a string.
///
/// \param aOut The method store the string here.
///

void IPAddress::GetString(char * aOut) const {
	//PtAssert(aOut != nullptr);

	for (unsigned int lIndex = 0; lIndex < SIZE; lIndex++) {
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
/// \brief Data accessor.
///

void IPAddress::Set(const void * const aIn) {
	//PtAssert(aIn != nullptr);
    mAny = false;
	memcpy(&mValue, aIn, sizeof(mValue));
}


///
/// \brief Data accessor.
///

void IPAddress::Set(uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3) {
    mAny = false;
	mValue = a0 | (a1 << 8) | (a2 << 16) | (a3 << 24);
}

///
/// \brief Data accessor.
///
#if 0
Result IPAddress::Set(const char * const aIn) {
	//PtAssert(aIn != nullptr);
    mAny = false;

	uint32_t lValue = 0;

	uint8_t *lPtr = reinterpret_cast< uint8_t *>(&lValue);
	//PtAssert(lPtr != nullptr);

	uint32_t lIndex = 0;

    const char *lIn = aIn;
	while ((*lIn) != '\0') {
		switch (*lIn) {
        case '[' :
            // Skip '[' if first character
            if (lIn != aIn) {
                return Result::INVALID_ARGUMENT;
            }
        break;

        case ']' :
            // Skip ']' if last character
            if (*(lIn + 1) != '\0') {
                return Result::INVALID_ARGUMENT;
            }
        break;

		case '.' :
			lIndex++;
			if (lIndex >= SIZE) {
				return Result::INVALID_ARGUMENT;
			}
		break;

		case '0' :
		case '1' :
		case '2' :
		case '3' :
		case '4' :
		case '5' :
			if (lPtr[lIndex] > 25) {
				return Result::INVALID_ARGUMENT;
			}

			lPtr[lIndex] *= 10;
			lPtr[lIndex] += (*lIn) - '0';
	    break;

		case '6' :
		case '7' :
		case '8' :
		case '9' :
			if (lPtr[lIndex] > 24) {
				return Result::INVALID_ARGUMENT;
			}

			lPtr[lIndex] *= 10;
			lPtr[lIndex] += (*lIn) - '0';
			break;

		default :
			return Result::INVALID_ARGUMENT;
		}

		lIn++;
	}

	if (lIndex != 3) {
		return Result::INVALID_ARGUMENT;
	}

	mValue = lValue;

	return Result::OK;
}
#endif

///
/// \brief Boring uint32_t 'raw' set.
///

void IPAddress::Set(uint32_t aValue) {
    mAny = false;
    mValue = aValue;
}


///
/// \brief Sets as the 'any' address.
///

void IPAddress::SetAny(bool aEnable) {
    mAny = true;
    mValue = 0;
}


///
/// \brief Returns a PtUtilsLib::String representation of the IP address.
///

std::string IPAddress::GetString() const {
    const uint8_t * const lData = reinterpret_cast<const uint8_t *>(&mValue);

    std::stringstream lSS;
    lSS << (int)lData[0] << ".";
    lSS << (int)lData[1] << ".";
    lSS << (int)lData[2] << ".";
    lSS << (int)lData[3];

    //return lSS.str().c_str();
    return lSS.str();
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
