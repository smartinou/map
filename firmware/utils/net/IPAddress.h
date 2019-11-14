#pragma once
// *****************************************************************************
//
//     Copyright (c) 2006, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

class EthernetAddress;


class IPAddress {
public:

    static const IPAddress BROADCAST;
	static constexpr size_t SIZE = 4 * sizeof(uint8_t);

    IPAddress() : mValue(0) {}
    IPAddress(uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3);
    IPAddress(uint32_t aIn);
	IPAddress(const void * const aIn);
	IPAddress(const IPAddress &aIn);
	IPAddress(const char * const aIn);
	IPAddress(const std::string &aIn);

    // NO VIRTUAL DESTRUCTOR, expected to be memcpy'able
    ~IPAddress() {}

    bool operator == (const IPAddress &aB) const { return (mValue == aB.mValue); }
    bool operator != (const IPAddress &aB) const { return (mValue != aB.mValue); }

	uint8_t GetByte(uint32_t aIndex) const;
    void *GetData(void);
    const void *GetData(void) const;
    void GetEthernetAddress(EthernetAddress * const aOut) const;
    void GetString(char *aOut) const;
    std::string GetString() const;
    
	unsigned short GetWord(uint32_t aIndex) const;
    uint32_t GetLong(void) const { return mValue; }

    bool IsBroadcast(void) const { return ( mValue == 0xFFFFFFFFUL ); }
    bool IsMulticast(void) const;
    bool IsUnicast(void) const { return ((!IsBroadcast()) && (!IsMulticast())); }
    bool IsAny(void) const { return mAny; }
    bool IsValid(void) const { return ((mValue != 0) || mAny); }
    bool IsValidMask(void) const;

	//PtUtilsLib::Result Set(const char *aIn);
	void Set(const void * const aIn);
	void Set(uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3);
    void Set(uint32_t aValue);
    void SetAny(bool aEnable = true);
    void SetBroadcast(void) { mValue = 0xFFFFFFFFUL; }

private:
    uint32_t mValue;
    bool mAny;
};

