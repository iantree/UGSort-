#pragma	once
//
//  (C++) Lanuage and Platform Base Includes
//

//
//  WINDOWS ONLY:
//
//  DEBUGGING EXTENSION
//
//  In debug compilations (_DEBUG) then memory allocation tracing is enabled in the CRT.
//  This allows the detection of memory leaks and identifies the source (module and line) of each memory allocation that has leaked.
//

#if  (defined(_WIN32) || defined(_WIN64))
//    USE the builtin regex for studio version >= 12
#if ((defined(_MSC_VER)) && (_MSC_VER >= 1700))
#define _LPB_USE_BUILTIN_REGEX
#endif
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC																		//  Force tracing of memory allocations
#include	<cstdlib>																			//  Basic c++ definitions and functions
#include	<crtdbg.h>																			//  Debugging runtime
#include	<cstdint>																			//  Standard Integer definitions
#include	<random>																			//  Random Numbers
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)										//  Use the debugging form of "new"
#define new DEBUG_NEW																			//  
#define		CheckForMemoryLeaks() _CrtDumpMemoryLeaks()											//  Runtime checks for memory leaks
#else
#include	<cstdlib>																			//  Basic c++ definitions and functions
#include	<cstdint>																			//  Standard Integer definitions
#include	<random>																			//  Random Numbers
#endif

//
//  UNIX/LINUX ONLY:
//
//  We set __STDC_WANT_LIB_EXT1__ to 1 this sufaces the bounds checked (_s) variants of functions where/if available
//
#else
//    USE the builtin regex for g++ >= 4.9
#if (__GNUC__ > 4)
#define _LPB_USE_BUILTIN_REGEX
#endif
#if (__GNUC__ == 4) && (__GNUC_MINOR__ >= 9)
#define _LPB_USE_BUILTIN_REGE
#endif
#define		__STDC_WANT_LIB_EXT1__		1
#include	<cstddef>																			//  Basic c++ definitions
#include	<cstdlib>																			//  Basic c++ functions
#include	<stdint.h>																			//  Standard Integer definitions
#include	<cstdarg>																			//  Standard Arguments
#include	<cmath>																				//  Mathematics functions
#include	<iomanip>																			//  I/O mauipulators
#include	<random>																			//  Random library
#define		EOK 0
#endif

//
//  Identify the build environment
//
#if  (defined(_WIN32) || defined(_WIN64))
//  Windows
#if (defined(_WIN64))
//  Windows 64 bit
#if (defined(_DEBUG))
#pragma message("Building for: Windows 64bit Debugging.")
#else
#pragma message("Building for: Windows 64bit Release.")
#endif
#else
//  Windows 32 bit
#if ((defined(_MSC_VER)) && (_MSC_VER >= 1400))
#if (defined(_DEBUG))
#pragma message("Building for: Windows 32bit on VS 2005+ Debugging.")
#else
#pragma message("Building for: Windows 32bit on VS 2005+ Release.")
#endif
#else
#if (defined(_DEBUG))
#pragma message("Building for: Windows 32bit on VS < 2005 (OLD) Debugging.")
#else
#pragma message("Building for: Windows 32bit on VS < 2005 (OLD) Release.")
#endif
#endif
#endif
#else
//  UNIX/Linux
#if (INTPTR_MAX == INT64_MAX)
//  UNIX/Linux  64 bit
#if (defined(__STDC_LIB_EXT1__))
//  Bounds checking extensions available
#if (defined(_DEBUG))
#pragma message("Building for: UNIX/Linux 64bit with __STDC_LIB_EXT1__ extensions Debugging.")
#else
#pragma message("Building for: UNIX/Linux 64bit with __STDC_LIB_EXT1__ extensions Release.")
#endif
#else
//  Bounds checking extensions NOT available
#if (defined(_DEBUG))
#pragma message("Building for: UNIX/Linux 64bit Debugging.")
#else
#pragma message("Building for: UNIX/Linux 64bit Release.")
#endif
#endif
#else
//  UNIX/Linux 32 bit
#if (defined(__STDC_LIB_EXT1__))
//  Boiunds checking extensions available
#if (defined(_DEBUG))
#pragma message("Building for: UNIX/Linux 32bit with __STDC_LIB_EXT1__ extensions Debugging.")
#else
#pragma message("Building for: UNIX/Linux 32bit with __STDC_LIB_EXT1__ extensions Release.")
#endif
#else
//  Bounds checking extensions NOT available
#if (defined(_DEBUG))
#pragma message("Building for: UNIX/Linux 32bit Debugging.")
#else
#pragma message("Building for: UNIX/Linux 32bit Release.")
#endif
#endif
#endif
#endif

//   Show if we are using Native REGEX
#ifdef _LPB_USE_BUILTIN_REGEX
#pragma message("Using native C++11 regex.")
#else
#pragma message("Using StringThing emulated regex.")
#endif

//
//  Common CROSS-PLATFORM headers
//

#include	<iostream>																			//  Basic C++ I/O
#include	<fstream>																			//  File stream I/O
#include	<ctime>																				//  Basic resolution time functions

//
//  regex C++11 handling
//
#if defined(_LPB_USE_BUILTIN_REGEX)
#define		matching_expression(s)	std::regex(s)
#define		strregmatch(s,m)		std::regex_match(s,m)
#define		strregsearch(s,m)		std::regex_search(s,m)
#else
#define		matching_expression(s)	s
#define		strregmatch(s,m)		StringThing::_regex_match(s,strlen(s),m)
#define		strregsearch(s,m)		StringThing::_regex_search(s,strlen(s),m)
#endif

//
//  PLATFORM SPECIFIC HEADERS and DEFINITIONS
//

#if  (defined(_WIN32) || defined(_WIN64))
//
//  WINDOWS SPECIFIC headers and definitions
//
#define		WIN32_LEAN_AND_MEAN																	//  Exclude rarely-used parts of windows headers
#include	<windows.h>
#include	<shellapi.h>																		//  Shell API functions
						
//  Include the emulation for the POSIX strptime functions
#include	"WINDOWS/strptime.h"

#else
//
//  UNIX/Linux SPECIFIC headers and definitions
//
#include	<cstring>																			//  String handling functions
#include	<errno.h>																			//  Standar error code symbolics
#ifndef errno_t
#define		errno_t		int
#endif
#include	<sys/types.h>																		//  System types
#include	<sys/stat.h>																		//  Stat
#include	<unistd.h>																			//  Standard platform headers
#endif

//
//  CROSS-PLATFORM portability mappings
//
//  Portability Strategy
//
//  In general application should be coded to C++11 conventions with bounds checked functions used where applicable.
//  In cases where the windows _s and the C++11 variants clash then a shim function (_safe) is used to map
//  to the appropriate call form. Where possible unix forms are mapped to their appropriate windows functions
//  and vice versa.
//
//  SHIMS
//
//     localtime_safe			-		shim for localtime_s

#if  (defined(_WIN32) || defined(_WIN64))
//
//  Mappings from UNIX/Linux constructs to Windows constructs
//
#if ((defined(_MSC_VER)) && (_MSC_VER >= 1400))
//
//  VS2005 and above
//

//  localtime_safe
//
//  Shim for localtime_s 
//
//  PARAMETERS:
//
//		time_t *				-		Const pointer to the input time_t value
//      tm *					-		Pointer to the tm structure to be populated
//
//  RETURNS:
//
//		tm *					-		Pointer to the populated tm structure or nullptr if the call fails
//
struct tm* localtime_safe(const time_t *, struct tm *);
inline struct tm* localtime_safe(const time_t *time, struct tm *result) {
	if (localtime_s(result, time) == 0) return result;
	else return nullptr;
}


#else
//
//  Pre-VS2005
//

//  localtime_safe
//
//  Shim for localtime_s 
//
//  PARAMETERS:
//
//		time_t *				-		Const pointer to the input time_t value
//      tm *					-		Pointer to the tm structure to be populated
//
//  RETURNS:
//
//		tm *					-		Pointer to the populated tm structure or NULL if the call fails
//
struct tm* localtime_safe(const time_t *, struct tm *);
inline struct tm* localtime_safe(const time_t *time, struct tm *result) {
	struct tm *p_tm = nullptr;
	memset(result, 0xFF, sizeof(tm));
	p_tm = localtime(time);
	if (p_tm == nullptr) return nullptr;
	memcpy(result, p_tm, sizeof(tm));
	return result;
}

//  fopen_s
//
//  Shim for fopen_s 
//
//  PARAMETERS:
//
//		FILE **					-		Pointer to the pointer to hold the returned FILE pointer
//      char *					-		Const pointer to the filename to be opened
//		char *					-		Const pointer to the open mode string
//
//  RETURNS:
//
//		errno_t					-		0 on success otherwise error code (always ENOENT)
//
errno_t fopen_s(FILE**, const char*, const char*);
inline errno_t fopen_s(FILE **pRetH, const char *szFN, const char *szMode) {
	*pRetH = nullptr;
	*pRetH = fopen(szFN, szMode);
	if (*pRetH == nullptr) return ENOENT;
	return 0;
}

//
//  Unsafe cast backs
//
#define		strcat_s(t,l,s)		strcat(t,s)
#define		strcpy_s(t,l,s)		strcpy(t,s)
#define		strncpy_s(t,l,s,c)	{memcpy(t,s,c);   \
            t[c] = '\0';}
#define		sprintf_s(t,l,f, ...) sprintf(t, f,  __VA_ARGS__)
#define		vsprintf_s(t,l,f,v) vsprintf(t,f,v)

#endif
#else
//
//  Mappings from Windows constructs to UNIX/Linux constructs
//

//
//  Generic Symbol Mappings
//
#define		_stat		stat
#ifdef __STDC_LIB_EXT1__
//
//  __STDC_LIB_EXT1__ library extensions are available
//

//  SHIM MAPPINGS
#define			localtime_safe(t,r)			localtime_s(t,r)
#else
//
//  __STDC_LIB_EXT1__ library extensions are NOT available
//

//
//  SHIM Functions
//

//  localtime_safe
//
//  Shim for localtime_s 
//
//  PARAMETERS:
//
//		time_t *				-		Const pointer to the input time_t value
//      tm *					-		Pointer to the tm structure to be populated
//
//  RETURNS:
//
//		tm *					-		Pointer to the populated tm structure or NULL if the call failes
//
struct tm* localtime_safe(const time_t *, struct tm *);
inline struct tm* localtime_safe(const time_t *time, struct tm *result) {
	struct tm *p_tm = nullptr;
	memset(result, 0xFF, sizeof(tm));
	p_tm = localtime(time);
	if (p_tm == nullptr) return nullptr;
	memcpy(result, p_tm, sizeof(tm));
	return result;
}

//  fopen_s
//
//  Shim for fopen_s 
//
//  PARAMETERS:
//
//		FILE **					-		Pointer to the pointer to hold the returned FILE pointer
//      char *					-		Const pointer to the filename to be opened
//		char *					-		Const pointer to the open mode string
//
//  RETURNS:
//
//		errno_t					-		0 on success otherwise error code (always ENOENT)
//
errno_t fopen_s(FILE**, const char*, const char*);
inline errno_t fopen_s(FILE **pRetH, const char *szFN, const char *szMode) {
	*pRetH = nullptr;
	*pRetH = fopen(szFN, szMode);
	if (*pRetH == nullptr) return ENOENT;
	return 0;
}

//  _dupenv_s
//
//  Shim for fopen_s 
//
//  PARAMETERS:
//
//		char **					-		Pointer to the pointer to hold the returned copy of the environment variable
//      size_t *				-		Pointer (optonal) to a size_t to hold the size of the returned valiable
//		char *					-		Const pointer to the name of the varianle
//
//  RETURNS:
//
//		errno_t					-		0 on success otherwise error code (always ENOENT)
//
errno_t _depenv_s(char**, size_t*, const char*);
inline errno_t _dupenv_s(char **pRetVal, size_t *pRetLen, const char* VarName) {
	char		*pRV = getenv(VarName);

	*pRetVal = nullptr;
	if (pRetLen != nullptr) *pRetLen = 0;
	if (pRV == nullptr) return ENOENT;
	*pRetVal = (char *) malloc(strlen(pRV) + 1);
	if (*pRetVal == nullptr) return ENOENT;
	strcpy(*pRetVal, pRV);
	if (pRetLen != nullptr) *pRetLen = strlen(pRV);
	return 0;
}

//
//  Unsafe cast backs
//
#define		strcat_s(t,l,s)		strcat(t,s)
#define		strcpy_s(t,l,s)		strcpy(t,s)
#define		strncpy_s(t,l,s,c)	{memcpy(t,s,c);   \
            t[c] = '\0';}
#define		strcat_s(t,l,s)		strcat(t,s)
#define		sprintf_s(t, l, f, ...) sprintf(t, f,  __VA_ARGS__)
#define		vsprintf_s(t,l,f,v) vsprintf(t,f,v)

//
//  Dialect cast backs
//
#define		_stricmp(s,c)		strcasecmp(s,c)
#define		_strnicmp(s,c,l)	strncasecmp(s,c,l)
#define		_memicmp(s,c,l)		strncasecmp(s,c,l)
#endif
#endif

