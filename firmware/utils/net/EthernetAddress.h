#pragma once
// *****************************************************************************
//
//     Copyright (c) 2006, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include <cstdint>
#include <cstddef>
#include <string>
#include <string.h>


class EthernetAddress {
public:

    static const EthernetAddress BROADCAST;
	static constexpr size_t SIZE = 6;

    EthernetAddress() : mValue{ 0 } {}
	EthernetAddress(uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4, uint8_t a5);
	EthernetAddress(const char * const aStr);
	EthernetAddress(const void * const aBytes);
    EthernetAddress(const uint64_t &aValue);
    EthernetAddress(const std::string &aStr);

    // NO VIRTUAL DESTRUCTOR, expected to be memcpy'able
    ~EthernetAddress() {}

    bool operator == (const EthernetAddress &aAddress) const { return (memcmp(mValue, aAddress.mValue, sizeof(mValue)) == 0 ); }
    bool operator != (const EthernetAddress &aAddress) const { return (memcmp(mValue, aAddress.mValue, sizeof(mValue)) != 0 ); }

	uint8_t GetByte(unsigned int aIndex) const {
        //PtAssert(aIndex < sizeof(mValue));
        return mValue[aIndex];
    }

	void * GetData(void) { return mValue; }
    const void * GetData(void) const { return mValue; }
    void GetString(char * aOut) const;
    std::string GetString(void) const;
    uint64_t GetValue(void) const;

	bool IsBroadcast(void) const;
	bool IsMulticast(void) const;
	bool IsUnicast(void) const;
	bool IsValid(void) const;

	void Set(const void * const aIn);
    void Set(const uint64_t &aValue);
    void Set(unsigned int aIndex, uint8_t aValue);
	//PtUtilsLib::Result Set(char const * aIn);

    void SetBroadcast(void) { memset(mValue, 0xFF, sizeof(mValue)); }
    void Invalidate(void) { memset(mValue, 0x00, sizeof(mValue)); }

private:
	// Bytes to bytes copy is allowed
	uint8_t mValue[EthernetAddress::SIZE];
};
