#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       StringThing.h																						*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.3.0	(Build: 04)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the StringThing class. The StringThing class provides the			*
//* namespace encapsulation for the static xymorg string buffer manipulation primitive functions. 					*
//*																													*
//*	USAGE:																											*
//*																													*
//*	NOTES:																											*
//*																													*
//*	1.																												*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 -		03/12/2017	-	Initial Release																		*
//*	1.0.1 -		04/10/2022	-	Fixed a false positive match in _matches											*
//*							-	Allow replacement loops in _replace													*
//* 1.0.2		27/10/2022	-	Fix replacement loops in _replace													*
//* 1.1.0		31/10/2022	-	added st_xtoi and st_xtou hex translation functions.								*
//*																													*
//*******************************************************************************************************************/

//  Platform includes
#include		<cstdlib>									//  Standard Namespaces
#include		<cstring>									//  String manipulation
#include		<cctype>									//  Character type 

#include		"types.h"									//  xymorg primitive types
#include		"consts.h"									//  xymorg constants

namespace xymorg {

	//
	//  Group #1a  -  String Search Functions  -  Global Namespace
	//
#define		st_strstr(h,n)					strstr(h,n)
#define		st_stristr(h,n)					xymorg::StringThing::_search(h,strlen(h),n,strlen(n),true)
#define		st_strcasestr(h,n)				xymorg::StringThing::_search(h,strlen(h),n,strlen(n),true)

//
//  Group #1b  -  Memory Block Search Functions  -  Global Namespace
//
#define		st_memmem(h,l,n,m)				xymorg::StringThing::_search(h,l,n,m,false)
#define		st_memimem(h,l,n,m)				xymorg::StringThing::_search(h,l,n,m,true)
#define		st_memcasemem(h,l,n,m)			xymorg::StringThing::_search(h,l,n,m,true)

//
//  Group #1c  -  Composite Search Functions  -  Global Namespace
//
#define		st_memstr(h,l,n)				xymorg::StringThing::_search(h,l,n,strlen(n),false)
#define		st_memistr(h,l,n)				xymorg::StringThing::_search(h,l,n,strlen(n),true)
#define		st_memcasestr(h,l,n)			xymorg::StringThing::_search(h,l,n,strlen(n),true)
#define		st_strmem(h,n,m)				xymorg::StringThing::_search(h,strlen(h),n,m,false)
#define		st_strimem(h,n,m)				xymorg::StringThing::_search(h,strlen(h),n,m,true)
#define		st_strcasemem(h,n,m)			xymorg::StringThing::_search(h,strlen(h),n,m,true)

//
//  Group #2  -  String Buffer Manipulation  -  Global Namespace
//
#define		st_snip(b,l,p,s)				xymorg::StringThing::_snip(b,l,p,s)
#define		st_inject(b,l,p,i,s)			xymorg::StringThing::_inject(b,l,p,i,s)
#define		st_strcut(b,l,t)				xymorg::StringThing::_remove(b,l,t,strlen(t),false,false)
#define		st_stricut(b,l,t)				xymorg::StringThing::_remove(b,l,t,strlen(t),false,true)
#define		st_strncut(b,l,t,s)				xymorg::StringThing::_remove(b,l,t,s,false,false)
#define		st_strnicut(b,l,t,s)			xymorg::StringThing::_remove(b,l,t,s,false,true)
#define		st_strcutall(b,l,t)				xymorg::StringThing::_remove(b,l,t,strlen(t),true,false)
#define		st_stricutall(b,l,t)			xymorg::StringThing::_remove(b,l,t,strlen(t),true,true)
#define		st_strncutall(b,l,t,s)			xymorg::StringThing::_remove(b,l,t,s,true,false)
#define		st_strnicutall(b,l,t,s)			xymorg::StringThing::_remove(b,l,t,s,true,true)
#define		st_strrep(b,l,t,r)				xymorg::StringThing::_replace(b,l,t,strlen(t),r,strlen(r),false,false)
#define		st_strirep(b,l,t,r)				xymorg::StringThing::_replace(b,l,t,strlen(t),r,strlen(r),false,true)
#define		st_strnrep(b,l,t,m,r,n)			xymorg::StringThing::_replace(b,l,t,m,r,n,false,false)
#define		st_strnirep(b,l,t,m,r,n)		xymorg::StringThing::_replace(b,l,t,m,r,n,false,true)
#define		st_strrepall(b,l,t,r)			xymorg::StringThing::_replace(b,l,t,strlen(t),r,strlen(r),true,false)
#define		st_strirepall(b,l,t,r)			xymorg::StringThing::_replace(b,l,t,strlen(t),r,strlen(r),true,true)
#define		st_strnrepall(b,l,t,m,r,n)		xymorg::StringThing::_replace(b,l,t,m,r,n,true,false)
#define		st_strnirepall(b,l,t,m,r,n)		xymorg::StringThing::_replace(b,l,t,m,r,n,true,true)
#define		st_trim(s)						xymorg::StringThing::_trim(s, strlen(s))
#define		st_ucase(s)						xymorg::StringThing::_ucase(s, strlen(s))
#define		st_lcase(s)						xymorg::StringThing::_lcase(s, strlen(s))
#define		st_pcase(s)						xymorg::StringThing::_propercase(s)
#define		st_integer(s)					xymorg::StringThing::_safeInteger(s)
#define		st_float(s)						xymorg::StringThing::_safeFloat(s, 0)
#define		st_floatp(s,p)					xymorg::StringThing::_safeFloat(s, p)
#define		st_fmtint(s,l)					xymorg::StringThing::_formatInteger(s, l, ',')
#define		st_sigdigs(s,b,a)				xymorg::StringThing::_sigdigs(s,b,a)
#define		st_numtrim(s)					xymorg::StringThing::_numtrim(s)
#define		st_isnumeric(s)					xymorg::StringThing::_isNumeric(s)
#define		st_atori(s,l,h)					xymorg::StringThing::_atori(s,l,h)
#define		st_atorf(s,l,h)					xymorg::StringThing::_atorf(s,l,h)
#define		st_atomsi(s,m,d)				xymorg::StringThing::_atomsi(s,m,d)
#define		st_atomsf(s,m,d)				xymorg::StringThing::_atomsf(s,m,d)

//
//  Group #3  -  String Pattern Matching (wildcards ? and *)
//
#define		st_strmp(t,p)					xymorg::StringThing::_matches(t,strlen(t),p,strlen(p),false)
#define		st_strimp(t,p)					xymorg::StringThing::_matches(t,strlen(t),p,strlen(p),true)
#define		st_strnmp(t,l,p)				xymorg::StringThing::_matches(t,l,p,strlen(p),false)
#define		st_strnimp(t,l,p)				xymorg::StringThing::_matches(t,l,p,strlen(p),true)

//
//  Group #4  -  Special encoding/decoding/formatting functions
//
#define		st_urldecode(s)					xymorg::StringThing::_urldecode(s)
#define		st_urlencode(s)					xymorg::StringThing::_urlencode(s,false)
#define		st_uriencode(s)					xymorg::StringThing::_urlencode(s,true)
#define		st_xmlencode(s)					xymorg::StringThing::_xmlencode(s)
#define		st_xmldecode(s)					xymorg::StringThing::_xmldecode(s)
#define		st_xstrlen(s)					xymorg::StringThing::_xdecode(s,false)
#define		st_xdecode(s)					xymorg::StringThing::_xdecode(s,true)
#define		st_xencode(s,l)					xymorg::StringThing::_xencode(s,l)
#define		st_xtoi(s)						xymorg::StringThing::_xtoi(s)
#define		st_xtou(s)						xymorg::StringThing::_xtou(s)
#define		st_xlate(s,t)					xymorg::StringThing::_xlate(s,t,strlen(s))
#define		st_frecan(s,c)					xymorg::StringThing::_frecan(s,c,strlen(s))
#define		st_frecx(s,c)					xymorg::StringThing::_frecx(s,c,strlen(s))
#define		st_tokenise(s,t)				xymorg::StringThing::_tokenise(s,t)
#define		st_rgb(s,t)						xymorg::StringThing::_getrgb(s,strlen(s),t)
#define		st_rgbn(s,l,t)					xymorg::StringThing::_getrgb(s,l,t)

//
//  Group #3a  -  String Regular Expression Matching - not defined here, they are surfaced in the global header
//

	//
	//  StringThing Class Definition
	//

	class StringThing {
	public:
		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants		                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Standard Translation Tables
		static const BYTE ST_ANPREC_TABLE[256];											//  Alphanumeric string format recognizer table
		static const BYTE ST_XPREC_TABLE[256];											//  Hexadecimal string format recognizer table

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Nested Structures		                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//
		//  Tokens  -  Structure used to capture the output from an _Tokenise() call
		//

		typedef struct Tokens {
			size_t			Num;														//  Number of tokens
			size_t			Max;														//  Max number of tokens
			char* Token[30];													//  Pointers to individual tokens
		} Tokens;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  GROUP #1: String Search Functions

		//  _search
		//
		//	Performs a case insensitive search for the first occurrence of the passed memory block (3) of length (4) in the passed memory block (1) of length (2).
		//
		//  PARAMETERS
		//
		//		const char *			-	Pointer to the memory block to be searched (search space) or haystack.
		//		size_t					-   Length (number of characters) in the sub-string or needle
		//		const char *			-	Pointer to the character sub-string array to be searched for or needle.
		//		size_t					-	Length (number of characters) in the sub-string or needle
		//		bool					-	if true then the search is case insensitive
		//
		//  RETURNS
		//
		//		const char *			-	Const pointer to the first occurrence of the sub-string (2) within the string (1), nullptr if not found
		//
		//  NOTES
		//
		//		1.  If the passed string is nullptr or length 0 then the function returns nullptr
		//		2.  If the passed sub-string is nullptr then the function returns a pointer to the passed string
		//		3.  If the passed sub-string is empty then the function returns a pointer to the passed string
		//		4.  If the passed string is empty then the function returms nullptr
		//		5.  The contract behaviour of the functions mimics the contract for the standard strstr function
		//
		//  ALGORITHM
		//
		//		The algorithm searches the search space (1) for each candidate string that matches the first and last character of the sub-string (2) irrespective of case.
		//		Candidate strings are then checked for a match in their entirety.
		//

		static const char* _search(const char* pString, size_t HLen, const char* pSubString, size_t Len, bool SCI) {
			char			CSL = '\0';																	//  Starting character of the sub-string in lower case
			char			CSU = '\0';																	//  Starting character of the sub-string in upper case
			char			CEL = '\0';																	//  Ending character of the sub-string in lower case
			char			CEU = '\0';																	//  Ending character of the sub-string in upper case
			size_t			Span = 0;																	//  The span betwwen the first and last character characters of the sub-string
			size_t			LtoS = HLen;																//  Length of the haystack that is available to search
			const char*		pCandidate = nullptr;														//  Pointer to a candidate matched string
			const char*		pAltCandidate = nullptr;													//  Alternative candidate matched string
			const char*		pMatch = nullptr;															//  Matching sub-string pointer

			//  Contract envelope checks
			if (pString == nullptr) return nullptr;
			if (HLen == 0) return nullptr;
			if (pSubString == nullptr) return pString;
			if (Len == 0) return pString;
			if (Len > HLen) return nullptr;

			//  Determine the span between the first and last character in the search substring
			Span = Len - 1;

			//  Capture the starting and ending characters in both upper and lower case
			if (SCI) {
				CSL = char(tolower(pSubString[0]));
				CSU = char(toupper(CSL));
				CEL = char(tolower(pSubString[Span]));
				CEU = char(toupper(CEL));
			}
			else {
				CSU = CSL = pSubString[0];
				CEU = CEL = pSubString[Span];
			}

			//  Trivial case - the search sub string is a single character long. Return the first ocurrence of the upper or lower case character
			if (Span == 0) {
				pCandidate = (const char*)memchr(pString, CEL, LtoS);
				pAltCandidate = (const char*)memchr(pString, CEU, LtoS);
				if (pCandidate == nullptr || (pAltCandidate < pCandidate && pAltCandidate != nullptr)) pCandidate = pAltCandidate;
				return pCandidate;
			}

			//  Initialise the search position
			pCandidate = (const char*)memchr(pString + Span, CEL, LtoS - Span);
			pAltCandidate = (const char*)memchr(pString + Span, CEU, LtoS - Span);
			if (pCandidate == nullptr || (pAltCandidate < pCandidate && pAltCandidate != nullptr)) pCandidate = pAltCandidate;

			//  Main search loop - continue detecting candidate strings within the search space until it is exhauseted
			while (pCandidate != nullptr) {
				//  Determine if a real candidate has been detected
				if (*(pCandidate - Span) == CSL || *(pCandidate - Span) == CSU) {
					pAltCandidate = (pCandidate - Span) + 1;
					pMatch = pSubString + 1;
					size_t CComp = 2;
					if (SCI) {
						//  Case insensitive compare
						while (CComp < Len && tolower(*pAltCandidate) == tolower(*pMatch)) {
							pMatch++;
							pAltCandidate++;
							CComp++;
						}
					}
					else {
						//  Case sensitive compare
						while (CComp < Len && (*pAltCandidate == *pMatch)) {
							pMatch++;
							pAltCandidate++;
							CComp++;
						}
					}

					//  If the complete string was matched then return the pointer to the start of the matched string in the search space
					if (CComp == Len) return pCandidate - Span;
				}

				//  The current candidate is not matched - move on to the next candidate
				pAltCandidate = ++pCandidate;
				if (size_t(pCandidate - pString) >= HLen) pCandidate = nullptr;
				else {
					LtoS = HLen - (pCandidate - pString);
					pCandidate = (const char*)memchr(pCandidate, CEL, LtoS);
					pAltCandidate = (const char*)memchr(pAltCandidate, CEU, LtoS);
					if (pCandidate == nullptr || (pAltCandidate < pCandidate && pAltCandidate != nullptr)) pCandidate = pAltCandidate;
				}
			}

			//  Sub-string was not found in the search space - return nullptr
			return nullptr;
		}

		//  _search
		//
		//	Const/non-Const overloads
		//
		//  If the input search space string is non-const then the return string pointer may safely be returned as non-const.
		//

		static char* _search(char* pString, size_t HLen, const char* pSubString, size_t Len, bool SCI) {
			return const_cast<char*>(_search(const_cast<const char*>(pString), HLen, pSubString, Len, SCI));
		}

		static char* _search(char* pString, size_t HLen, char* pSubString, size_t Len, bool SCI) {
			return const_cast<char*>(_search(const_cast<const char*>(pString), HLen, const_cast<const char*>(pSubString), Len, SCI));
		}

		//
		//  Group #2  -  String Buffer Manipulations
		//

		//  _ucase
		//
		//  This function will convert any lowercase alphabetics in the string to uppercase
		//
		//  Parameters
		//
		//  1  -  char *				-  Pointer to the input buffer
		//  2  -  size_t				-  Length of the content in input buffer
		//
		//  Returns
		//
		//  API Use Notes:
		//

		static void		_ucase(char* pBuffer, size_t ContentLen) {
			char* pScan = pBuffer;																		//  Scanning pointer

			//  Contract envelope checks
			if (pBuffer == nullptr) return;
			if (ContentLen == 0) return;

			while (pScan <= (pBuffer + ContentLen)) {
				if (*pScan >= 'a' && *pScan <= 'z') *pScan = char(toupper(*pScan));
				pScan++;
			}

			//  Return to caller
			return;
		}

		//  _lcase
		//
		//  This function will convert any uppercase alphabetics in the string to lowercase
		//
		//  Parameters
		//
		//  1  -  char *				-  Pointer to the input buffer
		//  2  -  size_t				-  Length of the content in input buffer
		//
		//  Returns
		//
		//  API Use Notes:
		//

		static void		_lcase(char* pBuffer, size_t ContentLen) {
			char* pScan = pBuffer;																		//  Scanning pointer

			//  Contract envelope checks
			if (pBuffer == nullptr) return;
			if (ContentLen == 0) return;

			while (pScan <= (pBuffer + ContentLen)) {
				if (*pScan >= 'A' && *pScan <= 'Z') *pScan = char(tolower(*pScan));
				pScan++;
			}

			//  Return to caller
			return;
		}

		//  _trim
		//
		//  This function will trim blank characters from the start and end of the passed string
		//
		//  Parameters
		//
		//  1  -  char *				-  Pointer to the input buffer
		//  2  -  size_t				-  Length of the content in input buffer
		//
		//  Returns
		//
		//		size_t				-  the new length of the buffer contents
		//
		//  API Use Notes:
		//

		static size_t	_trim(char* pBuffer, size_t ContentLen) {
			size_t		NewLen = ContentLen;																		//  Updated size
			size_t		PreTrim = 0;																				//  Size to be trimmed before significance
			size_t		PostTrim = 0;																				//  Size to be trimmed after significance
			char*		pScan = pBuffer;																			//  Scanning pointer
			bool		PreDone = false;																			//  Prescanning complete

			//  Contract envelope checks
			if (pBuffer == nullptr) return 0;
			if (ContentLen == 0) return 0;

			//  Count the characters to be trimed defore and after the significant part of the string
			while (pScan < (pBuffer + ContentLen)) {
				if (*pScan == ' ') {
					if (!PreDone) PreTrim++;
					PostTrim++;
				}
				else {
					PostTrim = 0;
					PreDone = true;
				}
				pScan++;
			}

			//  Check for already trimmed
			if ((PreTrim + PostTrim) == 0) return NewLen;

			//  Chack for a completely blank string
			if (PreTrim == ContentLen) {
				pBuffer[0] = '\0';
				return 0;
			}

			//  Perform the trim
			NewLen = ContentLen - (PreTrim + PostTrim);
			memmove(pBuffer, pBuffer + PreTrim, ContentLen - (PreTrim + PostTrim));
			pBuffer[NewLen] = '\0';

			//  Return the new length to the caller
			return NewLen;
		}

		//  _snip
		//
		//  This function will excise a section from a buffer
		//
		//  Parameters
		//
		//  1  -  char *				-  Pointer to the input buffer
		//  2  -  size_t				-  Length of the content in input buffer
		//  3  -  char *				-  Pointer to the start of the section of the buffer to excise
		//  4  -  size_t				-  Length of the section of the buffer to excise
		//
		//  Returns
		//
		//		size_t				-  the new length of the buffer contents
		//
		//  API Use Notes:
		//

		static size_t _snip(char* pBuffer, size_t ContentLen, char* pSnip, size_t SnipLen) {
			size_t		NewLen = ContentLen;																		//  Updated size

			//  Contract envelope checks
			if (pBuffer == nullptr) return 0;
			if (pSnip == nullptr) return ContentLen;
			if ((pSnip + SnipLen) > (pBuffer + ContentLen)) return ContentLen;

			//  Excise the string from the buffer - remember to retain the trailing string terminator
			memmove(pSnip, pSnip + SnipLen, (NewLen + 1) - (SnipLen + (pSnip - pBuffer)));

			//  Update the length
			NewLen -= SnipLen;

			//  Return the updated content length
			return NewLen;
		}

		//  _inject
		//
		//  This function will inject a new section of content into a buffer
		//
		//  Parameters
		//
		//  1  -  char *				-  Pointer to the input buffer
		//  2  -  size_t				-  Length of content in the input buffer
		//  3  -  char *				-  Pointer to the place in the buffer to inject the new section
		//  4  -  char *				-  Comst Pointer to the start of the section of the buffer to be injected
		//  5  -  size_t				-  Length of the section of the buffer to be injected
		//
		//  Returns
		//
		//		size_t				-  the new length of the buffer contents
		//
		//  API Use Notes:
		//
		//      The buffer is assumed to be large enough to hold the existing content and the injected section.
		//

		static size_t _inject(char* pBuffer, size_t BfrSize, char* pPoint, const char* szText, size_t InjectSize) {
			size_t		NewSize = BfrSize;																		//  Updated size

			//  Make room in the buffer for the to string
			memmove(pPoint + InjectSize, pPoint, (NewSize + 1) - (pPoint - pBuffer));

			//  Update the length
			NewSize += InjectSize;

			//  Copy in the replacement text
			memcpy(pPoint, szText, InjectSize);

			//  Return the updated buffer size
			return NewSize;
		}

		//  _remove
		//
		//  This function will remove all or only the first ocurrences of the passed text string from the passed buffer
		//  The text comparison may be case insensitive, determined by the flags
		//
		//  Parameters
		//
		//  1  -  char *				-  Pointer to the input buffer
		//  2  -  size_t				-  Size of text in the buffer
		//  3  -  char *				-  Const pointer to the text to be removed
		//  4  -  size_t				-  Length of the text to be removes
		//  5  -  bool					-  If true remove all occurrences otherwise only remove the first occurrence
		//  6  -  bool					-  If true then perform the case insensitive string matching otherwise perform an exact match
		//
		//  Returns
		//
		//		size_t				-  the new length of the buffer content
		//
		//  API Use Notes:
		//

		static size_t _remove(char* pBuffer, size_t ContentSize, const char* szText, size_t TextLen, bool RemoveAll, bool CaseInsensitive) {
			size_t		NewSize = ContentSize;																	//  New size of the buffer contents
			char*		pToken = nullptr;																		//  Token to locate

			//  Constract envelope checks
			if (pBuffer == nullptr) return 0;
			if (szText == nullptr) return NewSize;
			if (TextLen == 0) return NewSize;

			//  Locate the first occurrence of the string in the buffer
			if (CaseInsensitive) pToken = st_memimem(pBuffer, NewSize, szText, TextLen);
			else pToken = st_memmem(pBuffer, NewSize, szText, TextLen);

			//  Process all (or only the first of the matches)
			while (pToken != nullptr) {
				//  Snip the text from the buffer
				NewSize = _snip(pBuffer, NewSize, pToken, TextLen);

				//  Move on to the next occurrence unless only doing the first one
				if (RemoveAll) {
					size_t RemainingSize = NewSize;
					RemainingSize = RemainingSize - (pToken - pBuffer);
					if (CaseInsensitive) pToken = st_memimem(pToken, RemainingSize, szText, TextLen);
					else pToken = st_memmem(pToken, RemainingSize, szText, TextLen);
				}
				else pToken = nullptr;
			}

			//  Return the updated size
			return NewSize;
		}

		//  _replace
		//
		//  This function will remove all or only the first ocurrences of the passed text array from the passed buffer and
		//  replace it with the replacement text.
		//  The text comparison may be case insensitive, determined by the flags
		//
		//  Parameters
		//
		//  1  -  char *				-  Pointer to the input buffer
		//  2  -  size_t				-  Size of text in the buffer
		//  3  -  char *				-  Const pointer to the text to be removed
		//  4  -  size_t				-  Length of the text to be removed
		//  5  -  char *				-  Const pointer to the replacement text
		//  6  -  size_t				-  Length of the replacement text
		//  7  -  bool					-  If true replace all occurrences otherwise only replace the first occurrence
		//  8  -  bool					-  If true then perform the case insensitive string matching otherwise perform an exact match
		//
		//  Returns
		//
		//		size_t				-  the new length of the buffer content
		//
		//  API Use Notes:
		//

		static size_t _replace(char* pBuffer, size_t ContentSize, const char* szText, size_t TextLen, const char* szNewText, size_t NewTextLen, bool RemoveAll, bool CaseInsensitive) {
			size_t		NewSize = ContentSize;																	//  New size of the buffer contents
			char*		pToken = nullptr;																		//  Pointer to the token in the buffer Token
			bool		LoopDefeat = false;																		//  Loop prevention is needed

			//  Contract envelope checks
			if (pBuffer == nullptr) return 0;
			if (szText == nullptr) return NewSize;
			if (TextLen == 0) return NewSize;
			if (szNewText == nullptr) NewTextLen = 0;

			//  Loop detection/protection
			if (CaseInsensitive) {
				if (NewTextLen > 0) {
					if (NewTextLen == TextLen) {
						//  Replacement text must not be equal to the original text
						if (strcmp(szText, szNewText) == 0) return NewSize;
					}
					else if (NewTextLen > TextLen) {
						//  Replacement text must not contain the original text
						if (st_stristr(szNewText, szText) != nullptr) LoopDefeat = true;
					}
				}
			}
			else {
				if (NewTextLen > 0) {
					if (NewTextLen == TextLen) {
						//  Replacement text must not be equal to the original text
						if (strcmp(szText, szNewText) == 0) return NewSize;
					}
					else if (NewTextLen > TextLen) {
						//  Replacement text must not contain the original text
						if (strstr(szNewText, szText) != nullptr) LoopDefeat = true;
					}
				}
			}

			//  Locate the first occurrence of the string in the buffer
			if (CaseInsensitive) pToken = st_memimem(pBuffer, NewSize, szText, TextLen);
			else pToken = st_memmem(pBuffer, NewSize, szText, TextLen);

			//  Process all (or only the first of the matches)
			while (pToken != nullptr) {
				//  Snip the text from the buffer
				NewSize = _snip(pBuffer, NewSize, pToken, TextLen);

				//  If there is replacement text then inject it into the buffer 
				if (szNewText != nullptr && NewTextLen > 0) NewSize = _inject(pBuffer, NewSize, pToken, szNewText, NewTextLen);

				//  Move on to the next occurrence unless only doing the first one.
				//  This must be done safely to avoid recursion on the injected (replacement) sub-string
				if (RemoveAll) {
					size_t	RemainingSize = NewSize;
					if (LoopDefeat) pToken += NewTextLen;
					RemainingSize = RemainingSize - (pToken - pBuffer);
					if (RemainingSize < TextLen) pToken = nullptr;
					else {
						if (CaseInsensitive) pToken = st_memimem(pToken, RemainingSize, szText, TextLen);
						else pToken = st_memmem(pToken, RemainingSize, szText, TextLen);
					}
				}
				else pToken = nullptr;
			}

			//  Return the updated size
			return NewSize;
		}

		//  _matches
		//
		//  This function will test if the passed string matces the pattern mask provided.
		//
		//  Parameters
		//
		//  1  -  char *				-  Const Pointer to the text to be matched
		//  2  -  size_t				-  Size of text in the buffer
		//  3  -  char *				-  Const pointer to the pattern mask
		//  4  -  size_t				-  Length of the pattern mask
		//  5  -  bool					-  If true then perform the case insensitive string matching otherwise perform an exact match
		//
		//  Returns
		//
		//		bool				-  true if the text matches the pattern mask, otherwise false
		//
		//  API Use Notes:
		//

		static bool _matches(const char* pText, size_t TextLen, const char* pMask, size_t MaskLen, bool CaseInsensitive) {
			const char* pScanText = pText;																			//  Scanning pointer for the text
			const char* pScanMask = pMask;																			//  Scanning pointer for the mask
			size_t		ScannedText = 0;																			//  Size of teh text scanned
			size_t		ScannedMask = 0;																			//  Size of the mask scanned

			//  Base comparison scans until a mismatch is encountered or a variable length wildcard '*' or the string or mask is exhausted
			while (ScannedText < TextLen && ScannedMask < MaskLen && *pScanMask != '*') {
				//  Match the next character
				if (CaseInsensitive) {
					if ((tolower(*pScanText) != tolower(*pScanMask)) && (*pScanMask != '?')) return false;
				}
				else {
					if ((*pScanText != *pScanMask) && (*pScanMask != '?')) return false;
				}

				//  Move on to the next character in the text and the mask
				pScanMask++;
				pScanText++;
				ScannedMask++;
				ScannedText++;
			}

			//  If the string is exhausted and the mask is exhausted or only contains a generic wild card '*' then the match has been made
			if (ScannedText == TextLen) {
				if (ScannedMask == MaskLen) return true;
				if ((MaskLen - ScannedMask == 1) && *pScanMask == '*') return true;
				//  There is residue that must be matched in the mask - so no match was made
				return false;
			}

			//  An '*' wildcard has been encountered in the mask - the text and the mask must now be re-aligned
			pScanMask++;
			ScannedMask++;

			if (ScannedMask < MaskLen) {
				//  Eliminate any superfluous wildcards
				while (ScannedMask < MaskLen && *pScanMask == '*') {
					pScanMask++;
					ScannedMask++;
				}

				//  If the mask is exhausted then the pattern matches
				if (ScannedMask == MaskLen) return true;

				//  If the next character in the mask is a substitution then aligment is established
				if (*pScanMask == '?') return _matches(pScanText, TextLen - ScannedText, pScanMask, MaskLen - ScannedMask, CaseInsensitive);

				while (ScannedText < TextLen) {

					//  Scan forward in the text until we find the first eligible aligment (i.e. a match with the mask)
					if (CaseInsensitive) {
						while (ScannedText < TextLen && tolower(*pScanText) != tolower(*pScanMask)) {
							pScanText++;
							ScannedText++;
						}
					}
					else {
						while (ScannedText < TextLen && *pScanText != *pScanMask) {
							pScanText++;
							ScannedText++;
						}
					}

					//  If the text is exhausted then no re-aligment was possible and the match fails
					if (ScannedText == TextLen) return false;

					//  See if the residue in the text and the mask now match - if so the match has succeeded
					if (_matches(pScanText, TextLen - ScannedText, pScanMask, MaskLen - ScannedMask, CaseInsensitive)) return true;

					//  Advance the string text pointer and try the next candidate
					pScanText++;
					ScannedText++;
				}

				//  We have run out of text and therefore could not match the mask
				return false;
			}

			//  Mask is exhausted - the pattern matches if we have also run out of text
			if (ScannedText == TextLen) return true;
			return false;
		}

		//  LOCAL DEFINITIONS
		//
		//  Encoded tests for regex evaluation
		//

#define		RTEST_NUMERICS				10																		//  Test for numerics
#define		RTEST_NON_NUMERICS			20																		//  Test for non-numerics
#define		RTEST_CHAR					30																		//  Test for a specific character
#define		RTEST_LIST					40																		//  Test against a list of characters

#define		RTEST_COND_NONEORMORE		100																		//  0 or more occurrences
#define		RTEST_COND_ONEORMORE		110																		//  1 or more occurrences
#define		RTEST_COND_NONEORONE		120																		//  0 or 1 occurrences
#define		RTEST_COND_N				130																		//  n occurrences
#define		RTEST_COND_NORMORE			140																		//  n or more occurrences
#define		RTEST_COND_NTOM				150																		//  n to m occurrences

	//  _regex_match
	//
	//  This function will test if the passed string matces the regular expression pattern provided.
	//
	//  Parameters
	//
	//  1  -  char *				-  Const Pointer to the text to be matched
	//  2  -  size_t				-  Size of text in the buffer
	//  3  -  char *				-  Const pointer to the pattern mask
	//
	//  Returns
	//
	//		bool				-  true if the text matches the pattern expression otherwise false
	//
	//  API Use Notes:
	//
	//  THIS IS AN EXTREMELY LIMITED IMPLEMENTATION OF REGEX EVALUATION
	//  IT SHOULD BE NOTED THAT THIS IMPLEMENTATION DOES NOT PERFORM "GIVE BACK" FROM GREEDY MATCHES
	//

		static bool _regex_match(const char* pText, size_t TextLen, const char* szExpr) {
			const char* pEval = pText;																//  Current evaluation point in the text string
			const char* pExpr = szExpr;															//  Current expression being evaluated
			int					TestFor = 0;																//  Test type to apply
			char				TestChar = 0;																//  Character to test for
			char				TList[257] = {};															//  List to test against
			size_t				NList = 0;																	//  Number of characters in the test list
			int					TestCond = 0;																//  Test condition
			int					TestCondMax = 0;															//  Test condition maximum 
			int					TestCondMin = 0;															//  Test condition minimum (or absolute count)
			size_t				CCount = 0;																	//  Number of characters used in the evaluation string

			//  Process each expression until the expression string is exhausted
			while (*pExpr != '\0') {

				//  Decode the current test in the expression
				//  Determine the form of the introducer
				if (*pExpr == '\\') {
					//  Escape sequence
					pExpr++;
					if (*pExpr == 'd') TestFor = RTEST_NUMERICS;
					else {
						if (*pExpr == 'D') TestFor = RTEST_NON_NUMERICS;
						else {
							if (*pExpr == 's') {
								//  Whitespace - ' ' or tab
								TestFor = RTEST_LIST;
								NList = 2;
								TList[0] = ' ';
								TList[1] = 9;
							}
							else {
								if (*pExpr == 'S') {
									//  Non-whitespace
									TestFor = RTEST_LIST;
									for (NList = 0; NList < 256; NList++) {
										if (NList == 9 || NList == ' ') TList[NList] = '?';
										else TList[NList] = char(NList);
									}
									NList = 254;
								}
								else {
									//  This is a character to test for
									TestFor = RTEST_CHAR;
									TestChar = *pExpr;
								}
							}
						}
					}
					pExpr++;
				}
				else {
					if (*pExpr == '[') {
						//  List
						pExpr++;
						TestFor = RTEST_LIST;
						if (*pExpr == '[') {
							//  This is a group name
							//  The only group we support (for the moment is :alpha:
							if (strncmp(pExpr, "[:alpha:]", 9) == 0) {
								strcpy_s(TList, 257, "abcdefghijklmnopqrstuvwxyz");
								strcat_s(TList, 257, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
								NList = strlen(TList);
								while (*pExpr != ']' && *pExpr != '\0') pExpr++;
								while (*pExpr == ']') pExpr++;
							}
							else return false;
						}
						else {
							//  Populate the list
							NList = 0;
							while (*pExpr != ']') {
								TList[NList] = *pExpr;
								NList++;
								pExpr++;
							}
							pExpr++;
						}
					}
					else {
						//  Character test
						TestFor = RTEST_CHAR;
						TestChar = *pExpr;
						pExpr++;
					}
				}

				//
				//  The introducer has been digested now test for a repeat qualifier
				//

				TestCond = RTEST_COND_N;
				TestCondMin = 1;
				TestCondMax = 1;

				if (*pExpr == '?') {
					TestCond = RTEST_COND_NONEORONE;
					pExpr++;
				}
				else {
					if (*pExpr == '+') {
						TestCond = RTEST_COND_ONEORMORE;
						pExpr++;
					}
					else {
						if (*pExpr == '*') {
							TestCond = RTEST_COND_NONEORMORE;
							pExpr++;
						}
						else {
							if (*pExpr == '{') {
								pExpr++;
								TestCondMin = 0;
								TestCondMax = 0;

								//  Get the minimum occurrence count
								while (*pExpr >= '0' && *pExpr <= '9') {
									TestCondMin = (TestCondMin * 10) + (*pExpr - '0');
									pExpr++;
								}

								if (*pExpr == '}') {
									TestCond = RTEST_COND_N;
									pExpr++;
								}
								else {
									if (*pExpr == ',') {
										pExpr++;
										if (*pExpr == '}') {
											TestCond = RTEST_COND_NORMORE;
											pExpr++;
										}
										else {
											TestCond = RTEST_COND_NTOM;
											TestCondMax = 0;

											while (*pExpr >= '0' && *pExpr <= '9') {
												TestCondMax = (TestCondMax * 10) + (*pExpr - '0');
												pExpr++;
											}

											if (*pExpr == '}') pExpr++;
											else return false;										//  Invalid expression
										}
									}
									else return false;												//  Invalid expression
								}
							}
						}
					}
				}

				//
				//  The next atomic test condition has been encoded, test this against the current position in the test string.
				//  The atomic tests are greedy they will consume as musch of the test string as possible
				//

				bool			TestEval = true;
				int				Found = 0;

				switch (TestCond) {

				case RTEST_COND_NONEORONE:
					//
					//  Test for none or one ocurrenses of the specified character or type
					//  This test NEVER fails it only consumes a single character matching the specification
					//
					if (CCount < TextLen) {
						if (TestFor == RTEST_NUMERICS && (*pEval >= '0' && *pEval <= '9')) {
							pEval++;
							CCount++;
						}
						else {
							if (TestFor == RTEST_NON_NUMERICS && (*pEval < '0' || *pEval > '9')) {
								pEval++;
								CCount++;
							}
							else {
								if (TestFor == RTEST_CHAR && *pEval == TestChar) {
									pEval++;
									CCount++;
								}
								else {
									if (memchr(TList, *pEval, NList) != nullptr) {
										pEval++;
										CCount++;
									}
								}
							}
						}
					}
					break;

				case RTEST_COND_NONEORMORE:
					//
					//  Test for none or more occurrebces of the specified character or type
					//  This test NEVER fails it only consumes as many characters of the matching type or value as possible
					//
					if (CCount < TextLen) {
						if (TestFor == RTEST_NUMERICS) {
							while (*pEval >= '0' && *pEval <= '9') {
								pEval++;
								CCount++;
							}
						}
						else {
							if (TestFor == RTEST_NON_NUMERICS) {
								while (*pEval != '\0' && (*pEval < '0' || *pEval > '9')) {
									pEval++;
									CCount++;
								}
							}
							else {
								if (TestFor == RTEST_CHAR) {
									while (*pEval == TestChar) {
										pEval++;
										CCount++;
									}
								}
								else {
									while (memchr(TList, *pEval, NList) != nullptr) {
										pEval++;
										CCount++;
									}
								}
							}
						}
					}
					break;

				case RTEST_COND_ONEORMORE:
					//
					//  Test for one or more occurrences of the specified character or type
					//  This test WILL fail if there is not at least one match
					//
					if (CCount == TextLen) TestEval = false;
					else {
						if (TestFor == RTEST_NUMERICS) {
							while (*(pEval + Found) >= '0' && *(pEval + Found) <= '9') Found++;
						}
						else {
							if (TestFor == RTEST_NON_NUMERICS) {
								while ((CCount + Found) < TextLen && (*(pEval + Found) < '0' || *(pEval + Found) > '9')) Found++;
							}
							else {
								if (TestFor == RTEST_CHAR) {
									while (*(pEval + Found) == TestChar) Found++;
								}
								else {
									while (memchr(TList, *(pEval + Found), NList) != nullptr) Found++;
								}
							}
						}
					}
					if (Found > 0) {
						pEval += Found;
						CCount += Found;
					}
					else TestEval = false;
					break;

				case RTEST_COND_N:
					//
					//  Test for n occurrences of the specified character or type
					//  This test WILL fail if less than n ocuurences were evaluated
					//
					if (CCount == TextLen) TestEval = false;
					else {
						if (TestFor == RTEST_NUMERICS) {
							while (*(pEval + Found) >= '0' && *(pEval + Found) <= '9' && Found < TestCondMin) Found++;
						}
						else {
							if (TestFor == RTEST_NON_NUMERICS) {
								while ((CCount + Found) < TextLen && (*(pEval + Found) < '0' || *(pEval + Found) > '9') && Found < TestCondMin) Found++;
							}
							else {
								if (TestFor == RTEST_CHAR) {
									while (*(pEval + Found) == TestChar && Found < TestCondMin) Found++;
								}
								else {
									while (memchr(TList, *(pEval + Found), NList) != nullptr && Found < TestCondMin) Found++;
								}
							}
						}
					}
					if (Found == TestCondMin) {
						pEval += Found;
						CCount += Found;
					}
					else TestEval = false;
					break;

				case RTEST_COND_NORMORE:
					//  
					//  Test for n or more occurrences of the specified character or type
					//  This test WILL fail if there are less than n occurrences
					//
					if (CCount == TextLen) TestEval = false;
					else {
						if (TestFor == RTEST_NUMERICS) {
							while (*(pEval + Found) >= '0' && *(pEval + Found) <= '9') Found++;
						}
						else {
							if (TestFor == RTEST_NON_NUMERICS) {
								while ((CCount + Found) < TextLen && (*(pEval + Found) < '0' || *(pEval + Found) > '9')) Found++;
							}
							else {
								if (TestFor == RTEST_CHAR) {
									while (*(pEval + Found) == TestChar) Found++;
								}
								else {
									while (memchr(TList, *(pEval + Found), NList) != nullptr) Found++;
								}
							}
						}
					}

					if (Found >= TestCondMin) {
						pEval += Found;
						CCount += Found;
					}
					else TestEval = false;
					break;

				case RTEST_COND_NTOM:
					//
					//  Test for n <= occurrences <= m of the specified character or type
					//  This test WILL fail if there are less than the minimum
					//
					if (CCount == TextLen) TestEval = false;
					else {
						if (TestFor == RTEST_NUMERICS) {
							while (*(pEval + Found) >= '0' && *(pEval + Found) <= '9' && Found < TestCondMax) Found++;
						}
						else {
							if (TestFor == RTEST_NON_NUMERICS) {
								while ((CCount + Found) < TextLen && (*(pEval + Found) < '0' || *(pEval + Found) > '9') && Found < TestCondMax) Found++;
							}
							else {
								if (TestFor == RTEST_CHAR) {
									while (*(pEval + Found) == TestChar && Found < TestCondMax) Found++;
								}
								else {
									while (memchr(TList, *(pEval + Found), NList) != nullptr && Found < TestCondMax) Found++;
								}
							}
						}
					}
					if (Found >= TestCondMin) {
						pEval += Found;
						CCount += Found;
					}
					else TestEval = false;
					break;

				}

				//  If the evaluation of the test failed then no match 
				if (!TestEval) return false;

			}      //  Evaluating the regular expression

			//  For a match both the evaluated string and the expression must be exhausted
			if (*pEval != '\0') return false;

			//  Signal a match
			return true;
		}

		//  _regex_search
		//
		//  This function will test if the passed string contains a sub-string that matches the regular expression pattern provided.
		//
		//  Parameters
		//
		//  1  -  char *				-  Const Pointer to the text to be searched
		//  2  -  size_t				-  Size of text in the buffer
		//  3  -  char *				-  Const pointer to the pattern mask
		//
		//  Returns
		//
		//		bool				-  true if the text matches the pattern expression otherwise false
		//
		//  API Use Notes:
		//
		//  THIS IS AN EXTREMELY LIMITED IMPLEMENTATION OF REGEX EVALUATION
		//  IT SHOULD BE NOTED THAT THIS IMPLEMENTATION DOES NOT PERFORM "GIVE BACK" FROM GREEDY MATCHES
		//

		static bool _regex_search(const char* pText, size_t TextLen, const char* szExpr) {
			const char*			pStartString = pText;														//  Start position in the string to be tested for a match
			const char*			pEval = nullptr;															//  Current evaluation point in the text string
			const char*			pExpr = nullptr;															//  Current expression being evaluated
			int					TestFor = 0;																//  Test type to apply
			char				TestChar = 0;																//  Character to test for
			char				TList[257] = {};															//  List to test against
			size_t				NList = 0;																	//  Number of characters in the test list
			int					TestCond = 0;																//  Test condition
			int					TestCondMax = 0;															//  Test condition maximum 
			int					TestCondMin = 0;															//  Test condition minimum (or absolute count)
			bool				TestEval = true;
			size_t				CCount = 0;																	//  Number of characters used in the evaluation string

			//  Test possible start positions in the string until a match is found
			while (*pStartString != '\0') {

				//  Setup to perform the match
				pEval = pStartString;
				pExpr = szExpr;
				TestEval = true;
				CCount = 0;

				//std::cout << "TRACE: Evaluating string: '" << pEval << "'." << std::endl;

				//  Process each expression until the expression string is exhausted
				while (*pExpr != '\0' && TestEval) {

					//std::cout << "TRACE: Expression: '" << pExpr << "' against string: '" << pEval << "' ";

					//  Decode the current test in the expression
					//  Determine the form of the introducer
					if (*pExpr == '\\') {
						//  Escape sequence
						pExpr++;
						if (*pExpr == 'd') TestFor = RTEST_NUMERICS;
						else {
							if (*pExpr == 'D') TestFor = RTEST_NON_NUMERICS;
							else {
								if (*pExpr == 's') {
									//  Whitespace - ' ' or tab
									TestFor = RTEST_LIST;
									NList = 2;
									TList[0] = ' ';
									TList[1] = 9;
								}
								else {
									if (*pExpr == 'S') {
										//  Non-whitespace
										TestFor = RTEST_LIST;
										for (NList = 0; NList < 256; NList++) {
											if (NList == 9 || NList == ' ') TList[NList] = '?';
											else TList[NList] = char(NList);
										}
										NList = 254;
									}
									else {
										//  This is a character to test for
										TestFor = RTEST_CHAR;
										TestChar = *pExpr;
									}
								}
							}
						}
						pExpr++;
					}
					else {
						if (*pExpr == '[') {
							//  List
							pExpr++;
							TestFor = RTEST_LIST;
							if (*pExpr == '[') {
								//  This is a group name
								//  The only group we support (for the moment is :alpha:
								if (strncmp(pExpr, "[:alpha:]", 9) == 0) {
									strcpy_s(TList, 257, "abcdefghijklmnopqrstuvwxyz");
									strcat_s(TList, 257, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
									NList = strlen(TList);
									while (*pExpr != ']' && *pExpr != '\0') pExpr++;
									while (*pExpr == ']') pExpr++;
								}
								else return false;
							}
							else {
								//  Populate the list
								NList = 0;
								while (*pExpr != ']') {
									TList[NList] = *pExpr;
									NList++;
									pExpr++;
								}
								pExpr++;
							}
						}
						else {
							//  Character test
							TestFor = RTEST_CHAR;
							TestChar = *pExpr;
							pExpr++;
						}
					}

					//
					//  The introducer has been digested now test for a repeat qualifier
					//

					TestCond = RTEST_COND_N;
					TestCondMin = 1;
					TestCondMax = 1;

					if (*pExpr == '?') {
						TestCond = RTEST_COND_NONEORONE;
						pExpr++;
					}
					else {
						if (*pExpr == '+') {
							TestCond = RTEST_COND_ONEORMORE;
							pExpr++;
						}
						else {
							if (*pExpr == '*') {
								TestCond = RTEST_COND_NONEORMORE;
								pExpr++;
							}
							else {
								if (*pExpr == '{') {
									pExpr++;
									TestCondMin = 0;
									TestCondMax = 0;

									//  Get the minimum occurrence count
									while (*pExpr >= '0' && *pExpr <= '9') {
										TestCondMin = (TestCondMin * 10) + (*pExpr - '0');
										pExpr++;
									}

									if (*pExpr == '}') {
										TestCond = RTEST_COND_N;
										pExpr++;
									}
									else {
										if (*pExpr == ',') {
											pExpr++;
											if (*pExpr == '}') {
												TestCond = RTEST_COND_NORMORE;
												pExpr++;
											}
											else {
												TestCond = RTEST_COND_NTOM;
												TestCondMax = 0;

												while (*pExpr >= '0' && *pExpr <= '9') {
													TestCondMax = (TestCondMax * 10) + (*pExpr - '0');
													pExpr++;
												}

												if (*pExpr == '}') pExpr++;
												else return false;										//  Invalid expression
											}
										}
										else return false;												//  Invalid expression
									}
								}
							}
						}
					}

					//
					//  The next atomic test condition has been encoded, test this against the current position in the test string.
					//  The atomic tests are greedy they will consume as musch of the test string as possible
					//

					//std::cout << " (COND: " << TestCond << ") ";

					int				Found = 0;

					switch (TestCond) {

					case RTEST_COND_NONEORONE:
						//
						//  Test for none or one ocurrenses of the specified character or type
						//  This test NEVER fails it only consumes a single character matching the specification
						//
						if (CCount < TextLen) {
							if (TestFor == RTEST_NUMERICS && (*pEval >= '0' && *pEval <= '9')) {
								pEval++;
								CCount++;
							}
							else {
								if (TestFor == RTEST_NON_NUMERICS && (*pEval < '0' || *pEval > '9')) {
									pEval++;
									CCount++;
								}
								else {
									if (TestFor == RTEST_CHAR && *pEval == TestChar) {
										pEval++;
										CCount++;
									}
									else {
										if (memchr(TList, *pEval, NList) != nullptr) {
											pEval++;
											CCount++;
										}
									}
								}
							}
						}
						break;

					case RTEST_COND_NONEORMORE:
						//
						//  Test for none or more occurrebces of the specified character or type
						//  This test NEVER fails it only consumes as many characters of the matching type or value as possible
						//
						if (CCount < TextLen) {
							if (TestFor == RTEST_NUMERICS) {
								while (*pEval >= '0' && *pEval <= '9') {
									pEval++;
									CCount++;
								}
							}
							else {
								if (TestFor == RTEST_NON_NUMERICS) {
									while (CCount < TextLen && (*pEval < '0' || *pEval > '9')) {
										pEval++;
										CCount++;
									}
								}
								else {
									if (TestFor == RTEST_CHAR) {
										while (*pEval == TestChar) {
											pEval++;
											CCount++;
										}
									}
									else {
										while (memchr(TList, *pEval, NList) != nullptr) {
											pEval++;
											CCount++;
										}
									}
								}
							}
						}
						break;

					case RTEST_COND_ONEORMORE:
						//
						//  Test for one or more occurrences of the specified character or type
						//  This test WILL fail if there is not at least one match
						//
						if (CCount == TextLen) TestEval = false;
						else {
							if (TestFor == RTEST_NUMERICS) {
								while (*(pEval + Found) >= '0' && *(pEval + Found) <= '9') Found++;
							}
							else {
								if (TestFor == RTEST_NON_NUMERICS) {
									while ((CCount + Found) < TextLen && (*(pEval + Found) < '0' || *(pEval + Found) > '9')) Found++;
								}
								else {
									if (TestFor == RTEST_CHAR) {
										while (*(pEval + Found) == TestChar) Found++;
									}
									else {
										while (memchr(TList, *(pEval + Found), NList) != nullptr) Found++;
									}
								}
							}
						}
						if (Found > 0) {
							pEval += Found;
							CCount += Found;
						}
						else TestEval = false;
						break;

					case RTEST_COND_N:
						//
						//  Test for n occurrences of the specified character or type
						//  This test WILL fail if less than n ocuurences were evaluated
						//
						if (CCount == TextLen) TestEval = false;
						else {
							if (TestFor == RTEST_NUMERICS) {
								while (*(pEval + Found) >= '0' && *(pEval + Found) <= '9' && Found < TestCondMin) Found++;
							}
							else {
								if (TestFor == RTEST_NON_NUMERICS) {
									while ((CCount + Found) < TextLen && (*(pEval + Found) < '0' || *(pEval + Found) > '9') && Found < TestCondMin) Found++;
								}
								else {
									if (TestFor == RTEST_CHAR) {
										while (*(pEval + Found) == TestChar && Found < TestCondMin) Found++;
									}
									else {
										while (memchr(TList, *(pEval + Found), NList) != nullptr && Found < TestCondMin) Found++;
									}
								}
							}
						}
						if (Found == TestCondMin) {
							pEval += Found;
							CCount += Found;
						}
						else TestEval = false;
						break;

					case RTEST_COND_NORMORE:
						//  
						//  Test for n or more occurrences of the specified character or type
						//  This test WILL fail if there are less than n occurrences
						//
						if (CCount == TextLen) TestEval = false;
						else {
							if (TestFor == RTEST_NUMERICS) {
								while (*(pEval + Found) >= '0' && *(pEval + Found) <= '9') Found++;
							}
							else {
								if (TestFor == RTEST_NON_NUMERICS) {
									while ((CCount + Found) < TextLen && (*(pEval + Found) < '0' || *(pEval + Found) > '9')) Found++;
								}
								else {
									if (TestFor == RTEST_CHAR) {
										while (*(pEval + Found) == TestChar) Found++;
									}
									else {
										while (memchr(TList, *(pEval + Found), NList) != nullptr) Found++;
									}
								}
							}
						}

						if (Found >= TestCondMin) {
							pEval += Found;
							CCount += Found;
						}
						else TestEval = false;
						break;

					case RTEST_COND_NTOM:
						//
						//  Test for n <= occurrences <= m of the specified character or type
						//  This test WILL fail if there are less than the minimum
						//
						if (CCount == TextLen) TestEval = false;
						else {
							if (TestFor == RTEST_NUMERICS) {
								while (*(pEval + Found) >= '0' && *(pEval + Found) <= '9' && Found < TestCondMax) Found++;
							}
							else {
								if (TestFor == RTEST_NON_NUMERICS) {
									while ((CCount + Found) < TextLen && (*(pEval + Found) < '0' || *(pEval + Found) > '9') && Found < TestCondMax) Found++;
								}
								else {
									if (TestFor == RTEST_CHAR) {
										while (*(pEval + Found) == TestChar && Found < TestCondMax) Found++;
									}
									else {
										while (memchr(TList, *(pEval + Found), NList) != nullptr && Found < TestCondMax) Found++;
									}
								}
							}
						}
						if (Found >= TestCondMin) {
							pEval += Found;
							CCount += Found;
						}
						else TestEval = false;
						break;

					}

					//if (TestEval) std::cout << " evaluated as TRUE." << std::endl;
					//else std::cout << " evaluated as FALSE." << std::endl;
				}      //  Evaluating the regular expression

			   //  If we have reached the end of the expression and the last test succeeded then we have found a qualifying sub-string
				if (*pExpr == '\0' && TestEval) return true;

				//  Try the next start position in the string
				pStartString++;
				TextLen--;
			}

			//  No match was found
			return false;
		}

		//  _urlencode
		//
		//  This function will perform url (percent) encoding in-place on the passed string buffer.
		//
		//  Parameters
		//
		//  1  -  char *			-  Pointer to the input buffer
		//  2  -  bool				-  If true then apply the rules for a URI string conversion
		//
		//  Returns
		//
		//		size_t				-  the new length of the buffer content
		//
		//  API Use Notes:
		//

		static size_t _urlencode(char* pString, bool URIStyle) {
			size_t		NewSize = 0;													//  Updated size of the string

			//  Check that there is something to process
			if (pString == nullptr) return 0;
			if (pString[0] == '\0') return 0;

			NewSize = strlen(pString);

			//  If a URI style conversion then allow a single "*" to be passed through unencoded also leave "/" unencodes
			NewSize = _replace(pString, NewSize, "%", 1, "%25", 3, true, false);
			if (URIStyle) {
				if (strcmp(pString, "*") == 0) return NewSize;
			}
			else NewSize = _replace(pString, NewSize, "/", 1, "%2F", 3, true, false);

			//  Convert each unsafe character to it's URL encode (%) form
			NewSize = _replace(pString, NewSize, "!", 1, "%21", 3, true, false);
			NewSize = _replace(pString, NewSize, "#", 1, "%23", 3, true, false);
			NewSize = _replace(pString, NewSize, "$", 1, "%24", 3, true, false);
			NewSize = _replace(pString, NewSize, "&", 1, "%26", 3, true, false);
			NewSize = _replace(pString, NewSize, "'", 1, "%27", 3, true, false);
			NewSize = _replace(pString, NewSize, "(", 1, "%28", 3, true, false);
			NewSize = _replace(pString, NewSize, ")", 1, "%29", 3, true, false);
			NewSize = _replace(pString, NewSize, "*", 1, "%2A", 3, true, false);
			NewSize = _replace(pString, NewSize, "+", 1, "%2B", 3, true, false);
			NewSize = _replace(pString, NewSize, ",", 1, "%2C", 3, true, false);
			NewSize = _replace(pString, NewSize, ":", 1, "%3A", 3, true, false);
			NewSize = _replace(pString, NewSize, ";", 1, "%3B", 3, true, false);
			NewSize = _replace(pString, NewSize, "=", 1, "%3D", 3, true, false);
			NewSize = _replace(pString, NewSize, "?", 1, "%3F", 3, true, false);
			NewSize = _replace(pString, NewSize, "@", 1, "%40", 3, true, false);
			NewSize = _replace(pString, NewSize, "[", 1, "%5B", 3, true, false);
			NewSize = _replace(pString, NewSize, "]", 1, "%5D", 3, true, false);

			//  finally convert all occurrences of space to '+'
			_replace(pString, strlen(pString), " ", 1, "+", 1, true, false);

			//  Return the updated length of the string
			return NewSize;
		}

		//  _urldecode
		//
		//  This function will perform url (percent) decoding in-place on the passed string buffer.
		//
		//  Parameters
		//
		//  1  -  char *			-  Pointer to the input buffer
		//
		//  Returns
		//
		//		size_t				-  the new length of the buffer content
		//
		//  API Use Notes:
		//

		static size_t _urldecode(char* pString) {

			//  Check that there is something to process
			if (pString == nullptr) return 0;
			if (pString[0] == '\0') return 0;

			//  Step #1 convert all occurrences of '+' to spaces
			_replace(pString, strlen(pString), "+", 1, " ", 1, true, false);

			//  Perform decoding of the %HH hexadecimal sequences in the string
			char* pScan = strchr(pString, '%');
			while (pScan != nullptr) {

				//  Build the replacement character and use it to replace the % in the string
				*pScan = _makeChar(pScan + 1);

				//  Shuffle up the buffer to lose the two hex digits
				pScan++;
				memmove(pScan, pScan + 2, strlen(pScan + 1));

				// Next Hex
				pScan = strchr(pScan, '%');
			}

			//  Return the new string length
			return strlen(pString);
		}

		//  _makeChar
		//
		//  This function will construct an ascii character from the two hexedacimal digits pointed to.
		//
		//  Parameters
		//
		//  1  -  char *			-  Pointer to the two hex digits
		//
		//  Returns
		//
		//		char				-  The generated character
		//
		//  API Use Notes:
		//
		//

		static char _makeChar(char* pHex) {
			int			CharValue = 0;

			//  First digit
			if (*pHex >= '0' && *pHex <= '9') CharValue = 16 * (*pHex - '0');
			else if (*pHex >= 'a' && *pHex <= 'f') CharValue = 16 * (*pHex - 'a' + 10);
			else if (*pHex >= 'A' && *pHex <= 'F') CharValue = 16 * (*pHex - 'A' + 10);
			else CharValue = *pHex >> 4;

			pHex++;

			//  Second digit
			if (*pHex >= '0' && *pHex <= '9') CharValue += (*pHex - '0');
			else if (*pHex >= 'a' && *pHex <= 'f') CharValue += (*pHex - 'a' + 10);
			else if (*pHex >= 'A' && *pHex <= 'F') CharValue += (*pHex - 'A' + 10);
			else CharValue += *pHex & 0x0F;

			//  Return the character
			return char(CharValue);
		}

		//  _xmlencode
		//
		//  This function will perform xml encoding on the passed string to render the content XML safe.
		//
		//  Parameters
		//
		//  1  -  char *			-  Pointer to the input buffer
		//
		//  Returns
		//
		//		size_t				-  the new length of the buffer content
		//
		//  API Use Notes:
		//

		static size_t _xmlencode(char* pString) {
			size_t			NewLength = strlen(pString);												//  New length of the string

			//  Check that there is something to process
			if (pString == nullptr) return 0;
			if (pString[0] == '\0') return 0;

			//  Perform the replacements
			NewLength = _replace(pString, NewLength, "<", 1, "&lt;", 4, true, false);
			NewLength = _replace(pString, NewLength, ">", 1, "&gt;", 4, true, false);
			NewLength = _replace(pString, NewLength, "&", 1, "&amp;", 5, true, false);
			NewLength = _replace(pString, NewLength, "\"", 1, "&quot;", 6, true, false);
			NewLength = _replace(pString, NewLength, "'", 1, "&apos;", 6, true, false);

			//  Return the updated length
			return NewLength;
		}

		//  _xmldecode
		//
		//  This function will perform xml decoding on the passed string to render the content XML unsafe.
		//
		//  Parameters
		//
		//  1  -  char *			-  Pointer to the input buffer
		//
		//  Returns
		//
		//		size_t				-  the new length of the buffer content
		//
		//  API Use Notes:
		//

		static size_t _xmldecode(char* pString) {
			size_t			NewLength = strlen(pString);												//  New length of the string

			//  Check that there is something to process
			if (pString == nullptr) return 0;
			if (pString[0] == '\0') return 0;

			//  Perform the replacements
			NewLength = _replace(pString, NewLength, "&lt;", 4, "<", 1, true, false);
			NewLength = _replace(pString, NewLength, "&gt;", 4, ">", 1, true, false);
			NewLength = _replace(pString, NewLength, "&amp;", 5, "&", 1, true, false);
			NewLength = _replace(pString, NewLength, "&quot;", 6, "\"", 1, true, false);
			NewLength = _replace(pString, NewLength, "&apos;", 6, "'", 1, true, false);

			//  Return the updated length
			return NewLength;
		}

		//  _safeInteger
		//
		//  This function will ensure that the string contains a pure/safe integer value
		//
		//  Parameters
		//
		//  1  -  char *			-  Pointer to the input buffer
		//
		//  Returns
		//
		//		size_t				-  the new length of the buffer content
		//
		//  API Use Notes:
		//

		static size_t _safeInteger(char* pString) {
			size_t			NewLength = strlen(pString);												//  New length of the string
			int				IntValue = 0;																//  Integral value

			//  Check that there is something to process
			if (pString == nullptr) return 0;

			//  Obtain the input integral value
			IntValue = atoi(pString);

			//  Convert the value back to string representation
			sprintf_s(pString, NewLength + 10, "%i", IntValue);

			//  Update and return the length
			NewLength = strlen(pString);
			return NewLength;
		}

		//  _formatInteger
		//
		//  This function will format the passed buffer containing an integer value to a right aligned comma separated string buffer.
		//
		//  Parameters
		//
		//  1  -  char *			-  Pointer to the input buffer
		//  2  -  size_t			-  Fixed size of the output field
		//  3  -  char				-  Thousands separator character to use
		//
		//  Returns
		//
		//		size_t				-  the new length of the buffer content
		//
		//  API Use Notes:
		//

		static size_t	_formatInteger(char* pString, size_t FSize, char Sep) {
			size_t			NewLength = strlen(pString);												//  New length of the string
			int				IntValue = 0;																//  Integral value
			char* pScan = nullptr;															//  Scanning pointer
			char			SepStr[2] = { Sep, 0 };														//  Separator as string

			//  Check that there is something to process
			if (pString == nullptr) return 0;

			//  Obtain the input integral value
			IntValue = atoi(pString);

			//  Convert the value back to string representation
			sprintf_s(pString, NewLength + 10, "%i", IntValue);

			//  Insert separators
			NewLength = strlen(pString);
			pScan = pString + (int(NewLength) - 3);
			while (pScan > pString) {
				NewLength = _inject(pString, NewLength, pScan, SepStr, 1);
				pScan -= 3;
			}

			//  Pad
			while (NewLength < FSize) NewLength = _inject(pString, NewLength, pString, " ", 1);

			//  Return to caller
			return NewLength;
		}

		//  _safeFloat
		//
		//  This function will ensure that the string contains a pure/safe floating point value
		//
		//  Parameters
		//
		//  1  -  char *			-  Pointer to the input buffer
		//	2  -  int				-  Number of decimal places of precision to allow (max)
		//
		//  Returns
		//
		//		size_t				-  the new length of the buffer content
		//
		//  API Use Notes:
		//

		static size_t _safeFloat(char* pString, int NDP) {
			size_t			NewLength = strlen(pString);												//  New length of the string
			double			FloatValue = 0.0;															//  Floating point value

			//  Check that there is something to process
			if (pString == nullptr) return 0;

			//  Capture the value
			FloatValue = atof(pString);

			//  Format back to a valid string
			if (NDP == 0) sprintf_s(pString, NewLength + 10, "%f", FloatValue);
			else sprintf_s(pString, NewLength + 10, "%.*f", NDP, FloatValue);

			//  Update and return the length
			NewLength = strlen(pString);
			return NewLength;
		}

		//  _propercase
		//
		//  This function will convert the passed string buffer to a valid proper case format
		//
		//  Parameters
		//
		//  1  -  char *			-  Pointer to the input buffer
		//
		//  Returns
		//
		//		size_t				-  the new length of the buffer content
		//
		//  API Use Notes:
		//

		static size_t _propercase(char* pString) {
			size_t			NewLength = 0;															//  New length of the string
			bool			Capitalise = true;														//  Initial character is always capitalised

			//  Safety
			if (pString == nullptr) return 0;
			NewLength = strlen(pString);
			if (NewLength == 0) return 0;

			//  Loop processing the string content conerting it to propercase
			while (*pString != '\0') {
				if (Capitalise) *pString = char(toupper(*pString));
				else *pString = char(tolower(*pString));

				//  Determine oif the next character should be capitalised
				if (*pString == ' ' || *pString == '-') Capitalise = true;
				else Capitalise = false;

				//  Move on to the next character
				pString++;
			}

			//  Return the length
			return NewLength;
		}

		//  _xdecode
		//
		//  This function will return the size in bytes of the result of Hexadecimal decoding of the passed string.
		//  If the decode switch is set then the buffer is decoded in-place.
		//
		//  Parameters
		//
		//  1  -  char *			-	Pointer to the input buffer
		//  2  -  bool				-	If true then decode the string, otherwise just validate it
		//
		//  Returns
		//
		//		size_t				-  Size of the decoded string, 0 denotes an invalid hex encoded string
		//
		//  API Use Notes:
		//
		//		1.	If the decoding does not succeed (0 returned) then the input buffer will be trashed in the process.
		//		    Therefore call once with the decode switch off to validate and determine the length, only if this succeeds
		//		    call again with the decode switch on.
		//

		static size_t	_xdecode(char* pString, bool DoDecode) {
			size_t		NewLength = 0;																//  Length of decoded string
			char* pDecode = pString;															//  Decoding pointer
			char* pOut = pString;																//  Output pointer

			//  Safety
			if (pString == nullptr) return 0;
			if (pString[0] == '\0') return 0;

			//  Determine if the string is prefixed "0x" or "0X'
			if (pDecode[0] == '0' && (pDecode[1] == 'x' || pDecode[1] == 'X')) pDecode += 2;
			if (pDecode[0] == '\0') return 0;

			//  Loop counting valid pairs of Hex Nibble encodes, any invalid digit causes us to return 0 (String Invalid Hex)
			while (pDecode[0] != '\0') {
				if ((pDecode[0] >= '0' && pDecode[0] <= '9') || (pDecode[0] >= 'a' && pDecode[0] <= 'f') || (pDecode[0] >= 'A' && pDecode[0] <= 'F')) {
					if ((pDecode[1] >= '0' && pDecode[1] <= '9') || (pDecode[1] >= 'a' && pDecode[1] <= 'f') || (pDecode[1] >= 'A' && pDecode[1] <= 'F')) NewLength++;
					else return 0;
				}
				else return 0;

				//  If required perform the decoding
				if (DoDecode) {
					char	NewByte = 0;
					if (pDecode[0] >= '0' && pDecode[0] <= '9') NewByte = (pDecode[0] - '0') << 4;
					else if (pDecode[0] >= 'a' && pDecode[0] <= 'f') NewByte = ((pDecode[0] - 'a') + 10) << 4;
					else if (pDecode[0] >= 'A' && pDecode[0] <= 'F') NewByte = ((pDecode[0] - 'A') + 10) << 4;

					if (pDecode[1] >= '0' && pDecode[1] <= '9') NewByte += (pDecode[1] - '0');
					else if (pDecode[1] >= 'a' && pDecode[1] <= 'f') NewByte += ((pDecode[1] - 'a') + 10);
					else if (pDecode[1] >= 'A' && pDecode[1] <= 'F') NewByte += ((pDecode[1] - 'A') + 10);

					*pOut = NewByte;
					pOut++;
				}

				//  Move on to the next decoded byte
				pDecode += 2;
			}

			//  If decoding then terminate the string
			if (DoDecode) pString[NewLength] = '\0';

			//  Return the decoded string length
			return NewLength;
		}

		//  _xencode
		//
		//  This function will Hex encode the passed buffer in-place and return the length of the
		//  encoded string
		//
		//  Parameters
		//
		//  1  -  char *			-	Pointer to the input buffer
		//  2  -  size_t			-	Number of bytes to encode
		//
		//  Returns
		//
		//		size_t				-  Size of the encoded string (2 * input size)
		//
		//  API Use Notes:
		//

		static size_t	_xencode(char* pString, size_t EncBytes) {
			BYTE		NextByte = 0;													//  Next Byte to Encode
			BYTE* pIn = (BYTE*)pString + (EncBytes - 1);							//  Pointer to the next input
			BYTE* pOut = (BYTE*)pString + (2 * (EncBytes - 1));					//  Pointer to the next output
			int			Nibble = 0;														//  Nibble value

			//  Safety
			if (pString == nullptr) return 0;
			if (EncBytes == 0) return 0;
			pOut[2] = '\0';

			//  Proceed a byte at a time
			while (EncBytes > 0) {
				NextByte = *pIn;
				Nibble = NextByte >> 4;
				if (Nibble > 9) Nibble = (Nibble - 10) + 'a';
				else Nibble = Nibble + '0';
				pOut[0] = BYTE(Nibble);
				Nibble = NextByte & 0x0f;
				if (Nibble > 9) Nibble = (Nibble - 10) + 'a';
				else Nibble = Nibble + '0';
				pOut[1] = BYTE(Nibble);
				pIn--;
				pOut -= 2;
				EncBytes--;
			}

			//  Return the length to the caller
			return strlen(pString);
		}

		//  _xtoi
		//
		//  This function will return an integer (signed) containing the value from the passed hexadecimal string.
		//  The string may be prefixed with 0x or 0X, the decoding terminates at the end-of-string or the first
		//  invalid hex character encountered.
		//
		//  Parameters
		//
		//  1  -  char *			-	Pointer to the input buffer
		//
		//  Returns
		//
		//		int					-  decoded value
		//
		//  API Use Notes:
		//

		static int		_xtoi(const char* pString) {
			int			Accumulator = 0;											//  Decoding accumulator

			//  Safety
			if (pString == nullptr) return 0;
			if (pString[0] == '\0') return 0;

			//  Determine if the string is prefixed "0x" or "0X'
			if (pString[0] == '0' && (pString[1] == 'x' || pString[1] == 'X')) pString += 2;

			//  Loop decoding each pair of nubbles in turn
			while (isValidHexNibblePair(pString)) {
				//  Shift the accumulator to accomodate the next 8 bits
				Accumulator = Accumulator << 8;

				//  Accumulate the next pair of nibbles
				if (pString[0] >= '0' && pString[0] <= '9') Accumulator += (pString[0] - '0') << 4;
				else if (pString[0] >= 'a' && pString[0] <= 'f') Accumulator += ((pString[0] - 'a') + 10) << 4;
				else if (pString[0] >= 'A' && pString[0] <= 'F') Accumulator += ((pString[0] - 'A') + 10) << 4;

				if (pString[1] >= '0' && pString[1] <= '9') Accumulator += (pString[1] - '0');
				else if (pString[1] >= 'a' && pString[1] <= 'f') Accumulator += ((pString[1] - 'a') + 10);
				else if (pString[1] >= 'A' && pString[1] <= 'F') Accumulator += ((pString[1] - 'A') + 10);

				//  Advance to the next pair of nibbles
				pString += 2;
			}

			//  Return the accumulated value
			return Accumulator;
		}

		//  _xtou
		//
		//  This function will return an integer (unsigned) containing the value from the passed hexadecimal string.
		//  The string may be prefixed with 0x or 0X, the decoding terminates at the end-of-string or the first
		//  invalid hex character encountered.
		//
		//  Parameters
		//
		//  1  -  char *			-	Pointer to the input buffer
		//
		//  Returns
		//
		//		unsigned int		-  decoded value
		//
		//  API Use Notes:
		//

		static unsigned int		_xtou(const char* pString) {
			unsigned int		Accumulator = 0;											//  Decoding accumulator

			//  Safety
			if (pString == nullptr) return 0;
			if (pString[0] == '\0') return 0;

			//  Determine if the string is prefixed "0x" or "0X'
			if (pString[0] == '0' && (pString[1] == 'x' || pString[1] == 'X')) pString += 2;

			//  Loop decoding each pair of nubbles in turn
			while (isValidHexNibblePair(pString)) {
				//  Shift the accumulator to accomodate the next 8 bits
				Accumulator = Accumulator << 8;

				//  Accumulate the next pair of nibbles
				if (pString[0] >= '0' && pString[0] <= '9') Accumulator += (pString[0] - '0') << 4;
				else if (pString[0] >= 'a' && pString[0] <= 'f') Accumulator += ((pString[0] - 'a') + 10) << 4;
				else if (pString[0] >= 'A' && pString[0] <= 'F') Accumulator += ((pString[0] - 'A') + 10) << 4;

				if (pString[1] >= '0' && pString[1] <= '9') Accumulator += (pString[1] - '0');
				else if (pString[1] >= 'a' && pString[1] <= 'f') Accumulator += ((pString[1] - 'a') + 10);
				else if (pString[1] >= 'A' && pString[1] <= 'F') Accumulator += ((pString[1] - 'A') + 10);

				//  Advance to the next pair of nibbles
				pString += 2;
			}

			//  Return the accumulated value
			return Accumulator;
		}

		//  _xlate
		//
		//  This function will perform a character by character translation of the passed string against the provided translation table.
		//
		//  Parameters
		//
		//  1  -  char *			-	Pointer to the input buffer
		//  2  -  BYTE *			-	Pointer to the translation table
		//  3  -  size_t			-	Number of bytes to encode
		//
		//  Returns
		//
		//		size_t				-  Size of the translated string i.e. number of bytes translated
		//
		//  API Use Notes:
		//

		static size_t	_xlate(char* pString, const BYTE* pXTab, size_t XBytes) {
			BYTE* pUString = (BYTE*)pString;

			//  Safety
			if (pString == nullptr) return 0;
			if (pString[0] == '\0') return 0;
			if (pXTab == nullptr) return XBytes;
			if (XBytes == 0) return 0;

			//  Perform the translation
			for (size_t CC = 0; CC < XBytes; CC++) pUString[CC] = pXTab[pUString[CC]];

			//  Return the translated size
			return XBytes;
		}

		//  _stfrecan
		//
		//  This function will translate the incoming string into a format recognition string for alphanumeric strings.
		//
		//  Parameters
		//
		//  1  -  char *			-	Pointer to the input buffer
		//  2  -  bool				-	If true then compact the alphanumeric strings
		//  3  -  size_t			-	Number of bytes to encode
		//
		//  Returns
		//
		//		size_t				-  Size of the converted string
		//
		//  API Use Notes:
		//

		static size_t	_frecan(char* pString, bool CompactStrings, size_t InLen) {
			size_t		NewSize = InLen;													//  Size of the output string
			size_t		OldSize = 0;														//  Previous size

			//  Safety
			if (pString == nullptr) return 0;
			if (pString[0] == '\0') return 0;
			if (InLen == 0) return 0;

			//  Translate the buffer to alphanumeric recognition characters
			_xlate(pString, ST_ANPREC_TABLE, InLen);

			//  If the string is to be compacted then do so
			if (CompactStrings) {
				//  Compact alphabetic strings
				OldSize = 0;
				while (NewSize != OldSize) {
					OldSize = NewSize;
					NewSize = _replace(pString, OldSize, "aa", 2, "a", 1, true, false);
				}
				//  Compact numeric strings
				OldSize = 0;
				while (NewSize != OldSize) {
					OldSize = NewSize;
					NewSize = _replace(pString, OldSize, "nn", 2, "n", 1, true, false);
				}
			}

			//  Return the size of the output string
			return NewSize;
		}

		//  _stfrecx
		//
		//  This function will translate the incoming string into a format recognition string for hexadecimal strings.
		//
		//  Parameters
		//
		//  1  -  char *			-	Pointer to the input buffer
		//  2  -  bool				-	If true then compact the alphanumeric strings
		//  3  -  size_t			-	Number of bytes to encode
		//
		//  Returns
		//
		//		size_t				-  Size of the converted string
		//
		//  API Use Notes:
		//

		static size_t	_frecx(char* pString, bool CompactStrings, size_t InLen) {
			size_t		NewSize = InLen;													//  Size of the output string
			size_t		OldSize = 0;														//  Previous size

			//  Safety
			if (pString == nullptr) return 0;
			if (pString[0] == '\0') return 0;
			if (InLen == 0) return 0;

			//  Translate the buffer to alphanumeric recognition characters
			_xlate(pString, ST_XPREC_TABLE, InLen);

			//  If the string is to be compacted then do so
			if (CompactStrings) {
				//  Compact alphabetic strings
				OldSize = 0;
				while (NewSize != OldSize) {
					OldSize = NewSize;
					NewSize = _replace(pString, OldSize, "xx", 2, "x", 1, true, false);
				}
			}

			//  Return the size of the output string
			return NewSize;
		}

		//  _tokenise
		//
		//  This function will tokenise the passed string/record filling in the passed Tokens structure.
		//
		//  Parameters
		//
		//  1  -  char*				-	Pointer to the input buffer (string or record)
		//  2  -  Tokens*			-	Pointer to the Tokens structure to be populated
		//
		//  Returns
		//
		//		size_t				-  Number of tokens discoivered
		//
		//  API Use Notes:
		//
		//		1.	The call is destructive, the input buffer will have all tokens NULL terminated on completion.
		//

		static size_t	_tokenise(char* pBfr, Tokens* pToks) {
			bool		ScanningSpace = true;														//  Scan control

			//  Safety
			if (pToks == nullptr) return 0;
			pToks->Num = 0;
			if (pToks->Max == 0) pToks->Max = 30;													//  Default structure
			memset(&pToks->Token[0], 0, pToks->Max * sizeof(char*));								//  Clear the tokens
			if (pBfr == nullptr) return 0;
			if (pBfr[0] == SCHAR_EOS || pBfr[0] == SCHAR_CR || pBfr[0] == SCHAR_LF) return 0;

			//  Scan the input populating the Tokens structure until end of string or end of record is detected
			while (*pBfr != '\0' && *pBfr != SCHAR_CR && *pBfr != SCHAR_LF) {

				//  If we are scanning whitespace then continue unless we encounter a token
				if (ScanningSpace) {
					if (*pBfr != ' ' && *pBfr != SCHAR_TAB) {
						//  Start of a token
						pToks->Token[pToks->Num++] = pBfr;
						if (pToks->Num == pToks->Max) return pToks->Num + 1;
						ScanningSpace = false;
					}
				}
				else {
					if (*pBfr == ' ' || *pBfr == SCHAR_TAB) {
						*pBfr = SCHAR_EOS;
						ScanningSpace = true;
					}
				}

				//  Advance the buffer
				pBfr++;
			}

			if (*pBfr != SCHAR_EOS) *pBfr = SCHAR_EOS;

			//  Return the count of Tokens discovered
			return pToks->Num;
		}

		//  _sigdigs
		//
		//  This function will return the number of significant digits before and after the decimal point from the passed
		//	floating point number.
		//
		//  Parameters
		//
		//	1  -  double			-	The floatung point number to be evaluated
		//  2  -  size_t&			-	Reference to the variable to hold the significant digits before the point
		//  3  -  size_t&			-	Reference to the variable to hold the significant digits after the point
		//
		//  Returns
		//
		//		size_t			-		Length of the string needed to display this number
		//
		//  API Use Notes:
		//
		//

		static size_t _sigdigs(double FloatVal, size_t& Before, size_t& After) {
			char		Buffer[256] = {};														//  Buffer for evaluating
			char* pPoint = nullptr;														//  Pointer to the point

			//  Clear the significant digits
			Before = 0;
			After = 0;

			//  Lose sign
			FloatVal = abs(FloatVal);

			//  Convert the floating point number to a string
			sprintf_s(Buffer, 256, "%f", FloatVal);

			//  Trim any leading and trailing 0 from the string
			pPoint = Buffer;
			while (*pPoint == '0') memmove(Buffer, Buffer + 1, strlen(Buffer));

			pPoint = Buffer + (strlen(Buffer) - 1);
			while (*pPoint == '0') {
				*pPoint = '\0';
				pPoint--;
			}

			//  Determine if there is a point present
			pPoint = strchr(Buffer, '.');
			if (pPoint == nullptr) {
				//  No decimal point - integral value
				if (strlen(Buffer) > 1) Before = strlen(Buffer);
				else {
					if (Buffer[0] == '.') Before = 0;
					else Before = 1;
				}

				//  Return to caller
				return Before;
			}

			//  Replace the decimal point with a NULL, effectively splitting into two strings
			*pPoint = '\0';
			pPoint++;

			//  Determine the significant digits before the point
			if (strlen(Buffer) > 1) Before = strlen(Buffer);
			else {
				if (Buffer[0] == '.') Before = 0;
				else Before = 1;
			}

			//  Determine the significant digits after the point
			After = strlen(pPoint);

			if (After == 0) return Before;
			if (Before == 0) return After + 2;

			//  Return to caller
			return Before + After + 1;
		}

		//  _numtrim
		//
		//  This function will trim the passed numeric string.
		//
		//  Parameters
		//
		//	1  -  char*				-	Pointer to the string to be trimmed
		//
		//  Returns
		//
		//  API Use Notes:
		//
		//

		static void		_numtrim(char* pString) {
			char* pPoint = pString;

			//  Safety
			if (pString == nullptr) return;
			if (*pString == '\0') return;

			//  Trim any leading zeroes from the string
			while (pString[0] == '0' && pString[1] != '.' && pString[1] != '\0') memmove(pString, pString + 1, strlen(pString));

			//  Trim any trailing zeroes
			pPoint = pString + (strlen(pString) - 1);
			while (*pPoint == '0') {
				*pPoint = '\0';
				pPoint--;
			}
			if (*pPoint == '.') *pPoint = '\0';

			// Return to caller
			return;
		}

		//  _isNumeric
		//
		//  This function will test if the passed string is a valid numeric string
		//
		//  Parameters
		//
		//	1  -  char*				-	Pointer to the string to be validated
		//
		//  Returns
		//
		//		bool		-	true if the string is numeric, otherwise false
		//
		//  API Use Notes:
		//
		//

		static bool		_isNumeric(const char* pString) {

			//  Safety
			if (pString == nullptr) return false;
			if (*pString == '\0') return false;

			//  A leading sign character is valid
			if (*pString == '+' || *pString == '-') pString++;

			//  Loop - digits and , or . separators are valid
			while ((*pString >= '0' && *pString <= '9') || *pString == ',' || *pString == '.') pString++;

			//  Test the termination state
			if (*pString != '\0') return false;
			return true;
		}

		//  _atori
		//
		//  This function will return the two range integers from the string (<low>:<high>).
		//
		//  Parameters
		//
		//	1  -  char*				-	Pointer to the string to be processed
		//	2  -  int&				-	Reference to the integer to hold the lower range limit
		//	3  -  int&				-	Reference to the integer to hold the higher range limit
		//
		//  Returns
		//
		//		bool		-	true if the range was extracted, otherwise false
		//
		//  API Use Notes:
		//
		//

		static bool _atori(const char* pString, int& Lo, int& Hi) {
			size_t		IStrLen = 0;														//  Length of string
			size_t		WStrLen = 0;														//  Working string length
			char* pCopy = nullptr;													//  Copy of string

			//  Safety
			Lo = 0;
			Hi = 0;
			if (pString == nullptr) return false;
			IStrLen = strlen(pString);
			if (IStrLen < 3) return false;

			//  Allocate a buffer to hold a copy of the string
			pCopy = (char*)malloc(IStrLen + 10);
			if (pCopy == nullptr) return false;
			strcpy_s(pCopy, IStrLen + 10, pString);
			WStrLen = _trim(pCopy, IStrLen);
			if (WStrLen < 3) {
				free(pCopy);
				return false;
			}
			WStrLen = _frecan(pCopy, true, IStrLen);
			if (strcmp(pCopy, "n:n") != 0) {
				free(pCopy);
				return false;
			}

			strcpy_s(pCopy, IStrLen + 10, pString);
			IStrLen = _trim(pCopy, IStrLen);

			//  Extract the range
			Lo = atoi(pCopy);
			Hi = atoi(strchr(pCopy, ':') + 1);
			free(pCopy);
			return true;
		}

		//  _atorf
		//
		//  This function will return the two range doubles from the string (<low>:<high>).
		//
		//  Parameters
		//
		//	1  -  char*					-	Pointer to the string to be processed
		//	2  -  double&				-	Reference to the double to hold the lower range limit
		//	3  -  double&				-	Reference to the double to hold the higher range limit
		//
		//  Returns
		//
		//		bool		-	true if the range was extracted, otherwise false
		//
		//  API Use Notes:
		//
		//

		static bool _atorf(const char* pString, double& Lo, double& Hi) {
			size_t		IStrLen = 0;														//  Length of string
			size_t		WStrLen = 0;														//  Working string length
			char* pCopy = nullptr;													//  Copy of string

			//  Safety
			Lo = 0.0;
			Hi = 0.0;

			if (pString == nullptr) return false;
			IStrLen = strlen(pString);
			if (IStrLen < 3) return false;

			//  Allocate a buffer to hold a copy of the string
			pCopy = (char*)malloc(IStrLen + 10);
			if (pCopy == nullptr) return false;
			strcpy_s(pCopy, IStrLen + 10, pString);
			WStrLen = _trim(pCopy, IStrLen);
			if (WStrLen < 3) {
				free(pCopy);
				return false;
			}

			//  Validate format
			WStrLen = _frecan(pCopy, true, IStrLen);
			if (strcmp(pCopy, "n.n:n.n") != 0) {
				if (strcmp(pCopy, "n:n") != 0) {
					if (strcmp(pCopy, "n.n:n") != 0) {
						if (strcmp(pCopy, "n:n.n") != 0) {
							free(pCopy);
							return false;
						}
					}
				}
			}

			strcpy_s(pCopy, IStrLen + 10, pString);
			IStrLen = _trim(pCopy, IStrLen);

			//  Extract the range
			Lo = atof(pCopy);
			Hi = atof(strchr(pCopy, ':') + 1);
			free(pCopy);
			return true;
		}

		//  _atomsi
		//
		//  This function will return the mean and standard deviation (integers) from the string (<mean>/<std. dev.>).
		//
		//  Parameters
		//
		//	1  -  char*				-	Pointer to the string to be processed
		//	2  -  int&				-	Reference to the integer to hold the mean
		//	3  -  int&				-	Reference to the integer to hold the standard deviation
		//
		//  Returns
		//
		//		bool		-	true if the mean and deviation were extracted, otherwise false
		//
		//  API Use Notes:
		//
		//

		static bool _atomsi(const char* pString, int& m, int& sd) {
			size_t		IStrLen = 0;														//  Length of string
			size_t		WStrLen = 0;														//  Working string length
			char* pCopy = nullptr;													//  Copy of string

			//  Safety
			m = 0;
			sd = 0;
			if (pString == nullptr) return false;
			IStrLen = strlen(pString);
			if (IStrLen < 3) return false;

			//  Allocate a buffer to hold a copy of the string
			pCopy = (char*)malloc(IStrLen + 10);
			if (pCopy == nullptr) return false;
			strcpy_s(pCopy, IStrLen + 10, pString);
			WStrLen = _trim(pCopy, IStrLen);
			if (WStrLen < 3) {
				free(pCopy);
				return false;
			}
			WStrLen = _frecan(pCopy, true, IStrLen);
			if (strcmp(pCopy, "n/n") != 0) {
				free(pCopy);
				return false;
			}

			strcpy_s(pCopy, IStrLen + 10, pString);
			IStrLen = _trim(pCopy, IStrLen);

			//  Extract the range
			m = atoi(pCopy);
			sd = atoi(strchr(pCopy, '/') + 1);
			free(pCopy);
			return true;
		}

		//  _atomsf
		//
		//  This function will return the mean and standard deviation (doubles) from the string (<mean>/<std. dev.>).
		//
		//  Parameters
		//
		//	1  -  char*				-	Pointer to the string to be processed
		//	2  -  double&			-	Reference to the double to hold the mean
		//	3  -  double&			-	Reference to the double to hold the standard deviation
		//
		//  Returns
		//
		//		bool		-	true if the mean and deviation were extracted, otherwise false
		//
		//  API Use Notes:
		//
		//

		static bool _atomsf(const char* pString, double& m, double& sd) {
			size_t		IStrLen = 0;														//  Length of string
			size_t		WStrLen = 0;														//  Working string length
			char* pCopy = nullptr;													//  Copy of string

			//  Safety
			m = 0.0;
			sd = 0.0;

			if (pString == nullptr) return false;
			IStrLen = strlen(pString);
			if (IStrLen < 3) return false;

			//  Allocate a buffer to hold a copy of the string
			pCopy = (char*)malloc(IStrLen + 10);
			if (pCopy == nullptr) return false;
			strcpy_s(pCopy, IStrLen + 10, pString);
			WStrLen = _trim(pCopy, IStrLen);
			if (WStrLen < 3) {
				free(pCopy);
				return false;
			}

			//  Validate format
			WStrLen = _frecan(pCopy, true, IStrLen);
			if (strcmp(pCopy, "n.n/n.n") != 0) {
				if (strcmp(pCopy, "n/n") != 0) {
					if (strcmp(pCopy, "n.n/n") != 0) {
						if (strcmp(pCopy, "n/n.n") != 0) {
							free(pCopy);
							return false;
						}
					}
				}
			}

			strcpy_s(pCopy, IStrLen + 10, pString);
			IStrLen = _trim(pCopy, IStrLen);

			//  Extract the range
			m = atof(pCopy);
			sd = atof(strchr(pCopy, '/') + 1);
			free(pCopy);
			return true;
		}

		//  _getrgb
		//
		//  This function will return the RGB value extracted from the passed string.
		//	The string may be "r,g,b", (r,g,b), [r,g,b], {r,g,b} or #rrggbb (hexadecimal)
		//
		//  Parameters
		//
		//	1  -  char*				-	Pointer to the string to be processed
		//	2  -  size_t			-	Length of the string
		//	3  -  RGB&				-	Reference to the RGB to be filled in
		//
		//  Returns
		//
		//		bool		-	true if the RGB value was valid otherwise false
		//
		//  API Use Notes:
		//
		//

		static	bool	_getrgb(const char* In, size_t InLen, RGB& Out) {
			char* pCopy = nullptr;													//  Copy of input string
			int			R = 0, G = 0, B = 0;												//  Intermediates
			const char* pWork = nullptr;													//  Working scan pointer

			//  Safety
			if (In == nullptr) return false;
			if (InLen < 5) return false;
			if (*In == '\0') return false;

			//  Make a copy of the input string
			pCopy = (char*)malloc(InLen + 1);
			memcpy(pCopy, In, InLen);
			pCopy[InLen] = '\0';
			st_trim(pCopy);
			if (strlen(pCopy) < 5) {
				free(pCopy);
				return false;
			}

			//  Build a format recognition string (hexadecimal) for the input
			_frecx(pCopy, true, strlen(pCopy));

			//
			//  Recognise valid formats and process them into the RGB value
			//

			//  Decimal n,n,n
			if (strcmp(pCopy, "x,x,x") == 0) {
				pWork = In;
				while (*pWork < '0') pWork++;
				R = atoi(pWork);
				while (*pWork != ',') pWork++;
				pWork++;
				while (*pWork < '0') pWork++;
				G = atoi(pWork);
				while (*pWork != ',') pWork++;
				pWork++;
				while (*pWork < '0') pWork++;
				B = atoi(pWork);
				if (R <= 255 && R >= 0 && G <= 255 && G >= 0 && B <= 255 && B >= 0) {
					Out.R = uint8_t(R);
					Out.G = uint8_t(G);
					Out.B = uint8_t(B);
					free(pCopy);
					return true;
				}
			}

			//  Decimal (n,n,n)
			if (strcmp(pCopy, "(x,x,x)") == 0) {
				pWork = In;
				while (*pWork != '(') pWork++;
				pWork++;
				while (*pWork < '0') pWork++;
				R = atoi(pWork);
				while (*pWork != ',') pWork++;
				pWork++;
				while (*pWork < '0') pWork++;
				G = atoi(pWork);
				while (*pWork != ',') pWork++;
				pWork++;
				while (*pWork < '0') pWork++;
				B = atoi(pWork);
				if (R <= 255 && R >= 0 && G <= 255 && G >= 0 && B <= 255 && B >= 0) {
					Out.R = uint8_t(R);
					Out.G = uint8_t(G);
					Out.B = uint8_t(B);
					free(pCopy);
					return true;
				}
			}

			//  Decimal [n,n,n]
			if (strcmp(pCopy, "[x,x,x]") == 0) {
				pWork = In;
				while (*pWork != '[') pWork++;
				pWork++;
				while (*pWork < '0') pWork++;
				R = atoi(pWork);
				while (*pWork != ',') pWork++;
				pWork++;
				while (*pWork < '0') pWork++;
				G = atoi(pWork);
				while (*pWork != ',') pWork++;
				pWork++;
				while (*pWork < '0') pWork++;
				B = atoi(pWork);
				if (R <= 255 && R >= 0 && G <= 255 && G >= 0 && B <= 255 && B >= 0) {
					Out.R = uint8_t(R);
					Out.G = uint8_t(G);
					Out.B = uint8_t(B);
					free(pCopy);
					return true;
				}
			}

			//  Decimal {n,n,n}
			if (strcmp(pCopy, "[x,x,x]") == 0) {
				pWork = In;
				while (*pWork != '{') pWork++;
				pWork++;
				while (*pWork < '0') pWork++;
				R = atoi(pWork);
				while (*pWork != ',') pWork++;
				pWork++;
				while (*pWork < '0') pWork++;
				G = atoi(pWork);
				while (*pWork != ',') pWork++;
				pWork++;
				while (*pWork < '0') pWork++;
				B = atoi(pWork);
				if (R <= 255 && R >= 0 && G <= 255 && G >= 0 && B <= 255 && B >= 0) {
					Out.R = uint8_t(R);
					Out.G = uint8_t(G);
					Out.B = uint8_t(B);
					free(pCopy);
					return true;
				}
			}

			//  Hexadecimal #x
			if (strcmp(pCopy, "#x") == 0) {
				pWork = In;
				while (*pWork != '{') pWork++;
				pWork++;
				while (*pWork <= ' ') pWork++;
				R = _xtoi(pWork);
				Out.R = uint8_t(R & 0x000000FF);
				Out.G = uint8_t((R & 0x0000FF00) >> 8);
				Out.B = uint8_t((R & 0x00FF0000) >> 16);
				free(pCopy);
				return true;
			}

			//  Invalid string presented for RGB
			free(pCopy);
			return false;
		}

	private:

		//  isValidHexNibblePair
		//
		//  This function will determine if the passed string points to a valid pair of hex encoded nibbles.
		//
		//  Parameters
		//
		//	1  -  char*				-	Pointer to the string to be processed
		//
		//  Returns
		//
		//		bool		-	true if the string is valid, otherwise false
		//
		//  API Use Notes:
		//
		//

		static bool		isValidHexNibblePair(const char* pString) {

			if ((pString[0] >= '0' && pString[0] <= '9') || (pString[0] >= 'a' && pString[0] <= 'f') || (pString[0] >= 'A' && pString[0] <= 'F')) {
				if ((pString[1] >= '0' && pString[1] <= '9') || (pString[1] >= 'a' && pString[1] <= 'f') || (pString[1] >= 'A' && pString[1] <= 'F')) return true;
			}
			return false;
		}

		//
		//  Undefine local definitions
		//

#undef		RTEST_NUMERICS
#undef		RTEST_NON_NUMERICS
#undef		RTEST_CHAR
#undef		RTEST_LIST
#undef		RTEST_COND_NONEORMORE
#undef		RTEST_COND_ONEORMORE
#undef		RTEST_COND_NONEORONE
#undef		RTEST_COND_N
#undef		RTEST_COND_NORMORE
#undef		RTEST_COND_NTOM

	};

	//  Static Initialisers - Translation Tables

	const BYTE StringThing::ST_ANPREC_TABLE[256] = {
		0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
		0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
		0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
		'n','n','n','n','n','n','n','n','n','n',0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
		0x40,'a','a','a','a','a','a','a','a','a','a','a','a','a','a','a',
		'a','a','a','a','a','a','a','a','a','a','a',0x5b,0x5c,0x5d,0x5e,0x5f,
		0x60,'a','a','a','a','a','a','a','a','a','a','a','a','a','a','a',
		'a','a','a','a','a','a','a','a','a','a','a',0x7b,0x7c,0x7d,0x7e,0x7f,
		0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
		0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
		0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
		0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
		0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
		0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
		0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
		0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
	};

	const BYTE StringThing::ST_XPREC_TABLE[256] = {
		0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
		0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
		0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
		'x','x','x','x','x','x','x','x','x','x',0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
		0x40,'x','x','x','x','x','x',0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
		0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
		0x60,'x','x','x','x','x','x',0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
		0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
		0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
		0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
		0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
		0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
		0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
		0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
		0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
		0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
	};

}
