// *****************************************************************************
//
//     Copyright (c) 2006, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include <sstream>
#include <iomanip>

#include <net/EthernetAddress.h>

const EthernetAddress EthernetAddress::BROADCAST( 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF );
const size_t EthernetAddress::SIZE;

static const char * DIGITS = "0123456789abcdef";


///
/// \brief Constructor used to explicitly set the ethernet address
/// 

EthernetAddress::EthernetAddress(uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4, uint8_t a5) {
	mValue[0] = a0;
	mValue[1] = a1;
	mValue[2] = a2;
	mValue[3] = a3;
	mValue[4] = a4;
	mValue[5] = a5;
}


///
/// \brief Constructor
/// 

EthernetAddress::EthernetAddress(const char * const aStr) {
    Set(aStr);
}


///
/// \brief Constructor
///

EthernetAddress::EthernetAddress(const void * const aBytes) {
    Set(aBytes);
}


///
/// \brief Constructor
///

EthernetAddress::EthernetAddress(const uint64_t &aValue) {
    Set(aValue);
}


///
/// \brief Constructor
///

EthernetAddress::EthernetAddress(const std::string &aStr) {
    Set(aStr.c_str());
}


///
/// \brief Get data as a string.
///

void EthernetAddress::GetString(char *aOut) const {
	//PtAssert( aOut != NULL );

	for (unsigned int lIndex = 0; lIndex < sizeof(mValue); lIndex++) {
		*aOut = DIGITS[mValue[lIndex] >> 4]; aOut++;
		*aOut = DIGITS[mValue[lIndex] & 0x0f]; aOut++;
		*aOut = ':'; aOut++;
	}

	aOut--;
	*aOut = '\0';
}


///
/// \brief Returns a 64-bits numerical representation of the address
///

uint64_t EthernetAddress::GetValue() const {
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

bool EthernetAddress::IsBroadcast() const {
	for (unsigned int lIndex = 0; lIndex < sizeof(mValue); lIndex++) {
		if (mValue[lIndex] != 0xFF) {
			return false;
		}
	}

	return true;
}


///
/// \brief Is the ethernet address multicast?
///

bool EthernetAddress::IsMulticast() const {
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

bool EthernetAddress::IsUnicast() const {
    return (!IsBroadcast() && !IsMulticast());
}


///
/// \brief Is the ethernet address valid?
///

bool EthernetAddress::IsValid() const {
	for (unsigned int lIndex = 0; lIndex < sizeof(mValue); lIndex++) {
		// First non-zero we hit will indicate that the address
		// is valid - exact logical opposite of IsBroadcast behavior
		if (mValue[lIndex] != 0x00) {
			return true;
		}
	}

	// If we made it here we're stuck with a bunch of zeroes
	return false;
}

///
/// \brief Data acessor.
///

void EthernetAddress::Set(const void * const aIn) {
	//PtAssert(aIn != NULL);
	memcpy(mValue, aIn, sizeof(mValue));
}


///
/// \brief Set taking a 64 bits MAC address (only 48 valid)
///

void EthernetAddress::Set(uint64_t const &aValue) {
    mValue[0] = static_cast<uint8_t>((aValue & 0x0000FF0000000000LL) >> 40);
    mValue[1] = static_cast<uint8_t>((aValue & 0x000000FF00000000LL) >> 32);
    mValue[2] = static_cast<uint8_t>((aValue & 0x00000000FF000000LL) >> 24);
    mValue[3] = static_cast<uint8_t>((aValue & 0x0000000000FF0000LL) >> 16);
    mValue[4] = static_cast<uint8_t>((aValue & 0x000000000000FF00LL) >>  8);
    mValue[5] = static_cast<uint8_t>((aValue & 0x00000000000000FFLL) >>  0);
}


///
/// \brief Byte acessor.
///

void EthernetAddress::Set(unsigned int aIndex, unsigned char aValue) {
	//PtAssert(aIndex < sizeof(mValue));
	mValue[aIndex] = aValue;
}


///
/// \brief Par an ascii representation of address and set the object value.
///

#if 0
Result EthernetAddress::Set( const char * aIn )
{
	PtAssert( aIn != NULL );

    // Drop []-:
    char lBuffer[ 32 ] = { 0 };
    const char *lIn = aIn;
    char *lOut = lBuffer;
    while ( *lIn != 0 )
    {
        char lValue = *( lIn++ );
        if ( ( lValue != '[' ) &&
             ( lValue != ']' ) &&
             ( lValue != '-' ) &&
             ( lValue != ':' ) )
        {
            *( lOut++ ) = lValue;
        }
    }

    lIn = lBuffer;
	for ( unsigned int lI = 0; lI < 6 ; lI ++ )
	{
		mValue[ lI ] = 0;
		unsigned int lJ = 0;
		while ( lJ < 2 )
		{
			mValue[ lI ] <<= 4;
			if ( ( '0' <= ( * lIn ) ) && ( '9' >= ( * lIn ) ) )
			{
				mValue[ lI ] |= ( ( * lIn ) - '0' );
                lJ++;
			}
			else if ( ( 'A' <= ( * lIn ) ) && ( 'F' >= ( * lIn ) ) )
			{
				mValue[ lI ] |= ( ( * lIn ) - 'A' + 10 );
                lJ++;
			}
			else if ( ( 'a' <= ( * lIn ) ) && ( 'f' >= ( * lIn ) ) )
			{
				mValue[ lI ] |= ( ( * lIn ) - 'a' + 10 );
                lJ++;
			}
			else
			{
				return Result::INVALID_ARGUMENT;
			}

			lIn ++;
		}

		if ( ( * lIn ) == '\0' )
		{
			return ( lI == sizeof( mValue ) - 1 ) ? 
				Result::OK : 
				Result::INVALID_ARGUMENT;
		}
	}

	return Result::INVALID_ARGUMENT;
}
#endif

///
/// \brief Returns a PtUtilsLib::String representation of the Ethernet address
///

std::string EthernetAddress::GetString() const
{
    std::stringstream lSS;
    lSS << std::hex;
    lSS << std::setfill( '0' ) << std::setw( 2 ) << (int)mValue[ 0 ] << ":";
    lSS << std::setfill( '0' ) << std::setw( 2 ) << (int)mValue[ 1 ] << ":";
    lSS << std::setfill( '0' ) << std::setw( 2 ) << (int)mValue[ 2 ] << ":";
    lSS << std::setfill( '0' ) << std::setw( 2 ) << (int)mValue[ 3 ] << ":";
    lSS << std::setfill( '0' ) << std::setw( 2 ) << (int)mValue[ 4 ] << ":";
    lSS << std::setfill( '0' ) << std::setw( 2 ) << (int)mValue[ 5 ];

    //return lSS.str().c_str();
    return lSS.str();
}
