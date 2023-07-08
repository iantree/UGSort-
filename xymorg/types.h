#pragma once
//
//  Suite:  xymorg - Common Application Build
//
//  Common type definitions
//

//  Base includes
#include	<stdint.h>
#include    <atomic>
#include    <thread>
#include    <chrono>

//
//  All components are defined within the xymorg namespace
//
namespace xymorg {

	//
	//  Pseudo intrinsic types
	//

	typedef	unsigned int		UINT;																//  Unsigned integer
	typedef unsigned char		BYTE;																//  Unsigned character
	typedef uint32_t			SWITCHES;															//  Array of 32 bits
	typedef	unsigned short		LZWCODE;															//  Lempel-Ziv-Walsh Dictionary code
	typedef	unsigned short		USHORT;																//  Unsigned short integer
	typedef	unsigned long		ULONG;																//  Unsigned long integer
	typedef	int16_t				HW;																	//  Half-Word
	typedef	uint16_t			UHW;																//  Unsigned Half-Word
	typedef uint32_t			STRREF;																//  String reference
	typedef uint32_t			OBJREF;																//  Object reference

	//
	//  Convenience types
	//

	typedef			uint32_t						THREADID;										//  Thread Identifier
	typedef			std::chrono::system_clock		CLOCK;											//  Clock to use for timing
	typedef			CLOCK::time_point				TIMER;											//  Time point
	typedef			std::chrono::microseconds		MICROSECONDS;									//  Duration in microdeconds
	typedef			std::chrono::milliseconds		MILLISECONDS;									//  Duration in milliseconds
	typedef			std::chrono::duration<float>	SECONDS;										//  Duration in seconds

	//
	//  Macros
	//

#define         sleep(x)        std::this_thread::sleep_for(x)                                      //  Sleep for an interval of time
#define         DURATION(x,y)   std::chrono::duration_cast<x>(y)                                    //  Duration cast

	//
	//  Limits, Maxima, Minima
	//

#define		MAX_LOG_TEXT			255																//  Max size of log text
#ifndef MAX_PATH
#define		MAX_PATH				256																//  Maximum file path length
#endif

	//
	//  GLOBAL Definitions for complex types
	//

	//
	//  Resource structure
	//
	//  Used for referring to in-memory resources
	//

	typedef struct Resource {
		int				Class;																		//  Resource Class
		int				ID;																			//  Resource Identifier
		size_t			Size;																		//  Size of the in memory image
		BYTE* Image;																		//  Pointer to the in-memory image of the resource
	} Resource;

	//
	//  FileRec structure
	//
	//  Used for sorting and handling file references
	//

	typedef struct FileRec {
		int			FType;																			//  File type
		char		FName[MAX_PATH + 1];															//  File name
	} FileRec;

#ifdef  XY_NEEDS_CRYPTO

	//
	//  Key_3DES structure
	//
	//  This is the key structure for triple DES cryptography
	//

	typedef struct Key_DES {
		BYTE		Octet[8];																		//  Single key content
	} Key_DES;

	typedef struct Key_3DES {
		Key_DES		Key[3];																			//  Triple keys
	} Key_3DES;

	//
	//  Key_AES256
	//
	//  This is the key structure for AES-256 cryptography
	//

	typedef struct Key_AES256 {
		BYTE		Octet[32];																		//  Key content
	} Key_AES256;

	//
	//  Key_CHACHA20
	//
	//  This is the key structure for ChaCha20 cryptography
	//

	typedef struct Key_CHACHA20 {
		BYTE		Octet[32];																		//  Key content
	} Key_CHACHA20;

#endif

	//
	//   MD5Digest Structure 
	//
	//		The structure that holds an MD5 digest.
	//

	typedef struct MD5Digest {
		BYTE		Part[16];
	} MD5Digest;

	//
	//   SHA256Digest Structure 
	//
	//		The structure that holds a SHA-256 digest.
	//

	typedef struct SHA256Digest {
		BYTE		Part[32];
	} SHA256Digest;

	//
	//   Poly1305Digest Structure 
	//
	//		The structure that holds a Poly1305 digest.
	//

	typedef struct Poly1305Digest {
		BYTE		Part[16];
	} Poly1305Digest;

	//
	//   IP4A
	//
	//		The structure that holds an IPv4 Address
	//

	typedef struct IP4A {
		uint8_t		Octet1;																			//  Senior octet
		uint8_t		Octet2;
		uint8_t		Octet3;
		uint8_t		Octet4;																			//  Junior octet
	} IP4A;

	//
	//   IP6A
	//
	//		The structure that holds an IPv6 Address
	//

	typedef struct IP6A {
		uint8_t		Octet1;																			//  Senior octet
		uint8_t		Octet2;
		uint8_t		Octet3;
		uint8_t		Octet4;
		uint8_t		Octet5;
		uint8_t		Octet6;
		uint8_t		Octet7;
		uint8_t		Octet8;
		uint8_t		Octet9;
		uint8_t		Octet10;
		uint8_t		Octet11;
		uint8_t		Octet12;
		uint8_t		Octet13;
		uint8_t		Octet14;
		uint8_t		Octet15;
		uint8_t		Octet16;																		//  Junior octet
	} IP6A;

	//
	//  InterfaceInfoV4
	//
	//    The structure containing information pertinent to a specific IPv4 interface address
	//

	typedef struct InterfaceInfoV4 {
		IP4A		NetMask;																		//  NetMast address
		int			CIDRSize;																		//  CIDR Network Bits
		IP4A		BCastAddr;																		//  Broadcast address
		IP4A		ExtAddr;																		//  External Address (if NAT mapped)
		IP4A		GateWay;																		//  Gateway Adress
		IP4A		DNSServer;																		//  DNS server Address
	} InterfaceInfoV4;

	//
	//  InterfaceInfoV6
	//
	//    The structure containing information pertinent to a specific IPv6 interface address
	//

	typedef struct InterfaceInfoV6 {
		int			PrefixLen;																		//  Length of network prefix in bits
		int			IFIndex;																		//  Interface Index
		IP6A		GateWay;																		//  Gateway Adress
		IP6A		DNSServer;																		//  DNS server Address
	} InterfaceInfoV6;

	//
	//  RGB	-	8 bit per channel, 3 channels, (24 bits wide pixel)
	//

	typedef struct RGB {
		uint8_t			R;			//  Red channel
		uint8_t			G;			//  Green channel
		uint8_t			B;			//  Blue channel

		//  Comparators
		bool operator == (const RGB& rhs) {
			if (R != rhs.R) return false;
			if (G != rhs.G) return false;
			if (B != rhs.B) return false;
			return true;
		}

		bool operator != (const RGB rhs) {
			if (R != rhs.R) return true;
			if (G != rhs.G) return true;
			if (B != rhs.B) return true;
			return false;
		}

		//  Documentors
		void document(std::ostream& OS) {
			OS << "[R: " << int(R) << ",G: " << int(G) << ",B: " << int(B) << "]";
			return;
		}

		//  Const Comparators
		bool operator == (const RGB& rhs) const {
			if (R != rhs.R) return false;
			if (G != rhs.G) return false;
			if (B != rhs.B) return false;
			return true;
		}

		bool operator != (const RGB rhs) const {
			if (R != rhs.R) return true;
			if (G != rhs.G) return true;
			if (B != rhs.B) return true;
			return false;
		}

		//  Const Documentors
		void document(std::ostream& OS) const {
			OS << "[R: " << int(R) << ",G: " << int(G) << ",B: " << int(B) << "]";
			return;
		}

	} RGB;

}
