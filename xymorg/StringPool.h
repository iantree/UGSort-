#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       StringPool.h																						*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.1.0	(Build: 02)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the StringPool class. The class provides storage and reference		*
//* for collections of strings.																						*
//*																													*
//*	USAGE:																											*
//*																													*
//*																													*
//*	NOTES:																											*
//*																													*
//*	1.																												*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 -		02/12/2017	-	Initial Release																		*
//*	1.1.0 -		30/10/2022	-	added searchString() and variants													*
//*							-	addes addUniqueString() and variants												*
//*																													*
//*******************************************************************************************************************/

//
//  Include core xymorg headers
//

#include	"LPBHdrs.h"																		//  Language and Platform base headers
#include	"types.h"																		//  xymorg type definitions
#include	"consts.h"																		//  xymorg constant definitions

//
//  All components are defined within the xymorg namespace
//
namespace xymorg {

	//
	//  StringPool Class Definition
	//

	class StringPool {

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Constants		                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

	private:

		static const int	DefaultNumStrings = 100;												//  Default number of strings
		static const int	DefaultPoolSize = 4096;													//  Default string pool size
		static const STRREF EmptySlot = 0xFFFFFFFF;													//  Empty slot value in the SRT

	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors			                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Default Constructor 
		//
		//  Constructs a new StringPool with the default size settings
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		StringPool() : SRT(nullptr), SP(nullptr), EmptyString(0) {

			//  Initialise the pool for the default capacity
			if (!initialisePool(DefaultNumStrings, DefaultPoolSize)) {
				//  Clear the pool to the default state
				clearPool();
			}

			//  Return to caller
			return;
		}

		//  Constructor 
		//
		//  Constructs a new StringPool with the specified size settings
		//
		//  PARAMETERS:
		//
		//		size_t			-		Capacity of the pool (number of strings)
		//		size_t			-		Capacity of the pool (string storage space)
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		StringPool(size_t RNS, size_t RSPS) : SRT(nullptr), SP(nullptr), EmptyString(0) {

			//  Safety checks on capacities
			if (RNS == 0) RNS = DefaultNumStrings;
			if (RSPS == 0) RSPS = DefaultPoolSize;

			//  Initialise the pool for the spacified capacity
			if (!initialisePool(RNS, RSPS)) {
				//  Clear the pool to the default state
				clearPool();
			}

			//  Return to caller
			return;
		}

		//  Copy Constructor 
		//
		//  Constructs a new StringPool with a copy of the underlying storage from the source
		//
		//  PARAMETERS:
		//
		//		StringPool&			-		Const reference to the source StringPool
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		StringPool(const StringPool& Src) : SRT(nullptr), SP(nullptr), EmptyString(0) {

			//  Initialise the pool using the capacity from the source
			if (!initialisePool(Src.SRTCap, Src.SPCap)) {
				clearPool();
				return;
			}

			//  Copy the content of the SRT & SP from the source
			if (SRT != nullptr) memcpy(SRT, Src.SRT, Src.SRTCap * sizeof(size_t));
			if (SP != nullptr) memcpy(SP, Src.SP, Src.SPCap);

			//  Copy the pool information from the source
			SRTCap = Src.SRTCap;
			SRTEnts = Src.SRTEnts;
			SRTHiWater = Src.SRTHiWater;
			SPCap = Src.SPCap;
			SPUsed = Src.SPUsed;

			//  Return to caller
			return;
		}

		//  Move Constructor 
		//
		//  Constructs a new StringPool acquiring control of the underlying storage from the source
		//
		//  PARAMETERS:
		//
		//		StringPool&&			-		Reference to the source StringPool
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		StringPool(StringPool&& Src) noexcept : SRT(nullptr), SP(nullptr), EmptyString(0) {

			//  Acquire the underlying pool storage from the source
			SRT = Src.SRT;
			SRTCap = Src.SRTCap;
			SRTEnts = Src.SRTEnts;
			SRTHiWater = Src.SRTHiWater;

			SP = Src.SP;
			SPCap = Src.SPCap;
			SPUsed = Src.SPUsed;

			//  Disconnect the underlying storage from the source
			Src.SRT = nullptr;
			Src.SRTCap = 0;
			Src.SRTEnts = 0;
			Src.SRTHiWater = 0;

			Src.SP = nullptr;
			Src.SPCap = 0;
			Src.SPUsed = 0;

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor			                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Destructor
		//
		//  Destroys the StringPool object releasing any underlying storage.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		~StringPool() {

			//  Clear the pool to the default state
			clearPool();

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Members                                                                                                *
		//*                                                                                                                 *
		//*******************************************************************************************************************


		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  dismiss
		//
		//  Releases acquired storage making the pool empty and uninitialised
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		void	dismiss() { return clearPool(); }

		//  addString
		//
		//  Adds the passed null-terminated string to the pool as the next available entry
		//
		//  PARAMETERS:
		//
		//		char *			-		Pointer to the string to be added
		//
		//  RETURNS:
		//
		//		STRREF			-		Reference to the string, NULLSTRREF if the string could not be added
		//
		//  NOTES:
		//

		STRREF		addString(const char* pNewString) {
			//  Convert to an unterminated string addition
			if (pNewString == nullptr) return addString(nullptr, 0);
			return addString(pNewString, strlen(pNewString));
		}

		//  addString
		//
		//  Adds the passed unterminated string to the pool as the next available entry
		//
		//  PARAMETERS:
		//
		//		char *			-		Pointer to the string to be added
		//		size_t			-		String size (bytes)
		//
		//  RETURNS:
		//
		//		STRREF			-		Reference to the string, NULLSTRREF if the string could not be added
		//
		//  NOTES:
		//

		STRREF		addString(const char* pNewString, size_t NewStrLen) {
			STRREF		NewRef = NULLSTRREF;												//  String reference for the new string

			//  Safety Checks
			if (pNewString == nullptr) NewStrLen = 0;

			//  Check (and adjust if necessary) the capacity of the pool
			if (!checkCapacity(NewStrLen)) return NULLSTRREF;

			//  Locate the String Reference to be used
			NewRef = locateFreeStringRef();

			//  If we acquired a valid reference then populate it with the new string
			if (NewRef != NULLSTRREF) {

				//
				//  Set the offset of the new string in the reference table
				//
				SRT[NewRef - 1] = SPUsed;

				//
				//  If we have a non-null string then add it to the pool
				//
				if (pNewString != nullptr && NewStrLen > 0) memcpy(SP + SPUsed, pNewString, NewStrLen);

				//
				//  Add a string terminator
				//
				SP[SPUsed + NewStrLen] = '\0';

				//
				//  Update the pool information
				//
				SRTEnts++;
				if (NewRef > SRTHiWater) SRTHiWater++;
				SPUsed += (NewStrLen + 1);
			}

			//  Return the reference
			return NewRef;
		}

		//  getString
		//
		//  Dereferences a string reference returning the pointer to the string in the pool
		//
		//  PARAMETERS:
		//
		//		STRREF			-		Reference to the string, 
		//
		//  RETURNS:
		//
		//		char*			-		Const Pointer to the null terminated string in the pool
		//
		//  NOTES:
		//
		//		1.	NULLSTRREF returns a pointer to the dummy empty string
		//		2.	String values must be updated using the updateString() functions
		//

		const char* getString(STRREF Ref) {

			//  Any invalid reference returns a pointer to a dummy empty string
			if (Ref == NULLSTRREF) return &EmptyString;
			if (Ref > SRTHiWater + 1) return &EmptyString;

			//  Return the pointer to the string in the pool
			return SP + SRT[Ref - 1];
		}

		//  copyString
		//
		//  Returns a copy (and therefore mutable) of a string from the pool
		//
		//  PARAMETERS:
		//
		//		STRREF			-		Reference to the string, 
		//
		//  RETURNS:
		//
		//		char*			-		Pointer to the null terminated string in a new buffer
		//
		//  NOTES:
		//
		//		1.	NULLSTRREF returns a pointer to a buffer containing an empty string
		//

		char* copyString(STRREF Ref) {
			size_t			StrLen = getLength(Ref);									//  Length of string
			char* pString = nullptr;											//  String buffer

			//  Allocate new buffer
			pString = (char*)malloc(StrLen + 1);
			if (pString == nullptr) return nullptr;

			memcpy(pString, getString(Ref), StrLen);
			pString[StrLen] = '\0';
			return pString;
		}

		//  getLength
		//
		//  Returns the length of a referenced string.
		//
		//  PARAMETERS:
		//
		//		STRREF			-		Reference to the string, 
		//
		//  RETURNS:
		//
		//		size_t			-		Length of the referenced string
		//
		//  NOTES:
		//
		//

		size_t		getLength(STRREF Ref) {
			return strlen(getString(Ref));
		}

		//  deleteString
		//
		//  Removes the referenced string from the pool and frees the reference for reuse.
		//
		//  PARAMETERS:
		//
		//		STRREF			-		Reference to the string to be deleted.
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		void		deleteString(STRREF Ref) {

			//  Any invalid reference returns with no further action
			if (Ref == NULLSTRREF) return;
			if (Ref > SRTHiWater + 1) return;
			if (SRT[Ref - 1] == EmptySlot) return;

			//
			//  Compute the length to be excised from the pool
			//
			size_t		SnipLen = strlen(getString(Ref)) + 1;

			//  Remove the string from the pool
			if (((SPUsed - SRT[Ref - 1]) - SnipLen) > 0) memmove(SP + SRT[Ref - 1], SP + SRT[Ref - 1] + SnipLen, (SPUsed - SRT[Ref - 1]) - SnipLen);

			//  Clear the residual space at the end of the pool
			memset((SP + SPUsed) - SnipLen, 0, SnipLen);

			//  Update the pool information
			SPUsed = SPUsed - SnipLen;

			//
			//  Update all string references in the table that lie above the current reference offset
			//
			for (size_t SRX = 0; SRX < SRTHiWater; SRX++) {
				if (SRT[SRX] != EmptySlot && SRT[SRX] > SRT[Ref - 1]) SRT[SRX] = SRT[SRX] - SnipLen;
			}

			//  Remove the string reference from the table
			SRT[Ref - 1] = EmptySlot;
			SRTEnts--;
			if (Ref == (SRTHiWater + 1)) SRTHiWater--;

			//  Return to caller
			return;
		}

		//  replaceString
		//
		//  Replaces the content of the referenced string with the passed string value
		//
		//  PARAMETERS:
		//
		//		STRREF			-		Reference to the string to be deleted.
		//		char*			-		Pointer to the null-terminated string containing the replacement value
		//
		//  RETURNS:
		//
		//		STRREF			-		Reference to the string, NULLSTRREF if the new string could not be added
		//
		//  NOTES:
		//
		//		1.	The reference will NOT change unless the addition fails
		//

		STRREF		replaceString(STRREF Ref, const char* pNewString) {
			if (pNewString == nullptr) return replaceString(Ref, nullptr, 0);
			return replaceString(Ref, pNewString, strlen(pNewString));
		}

		//  replaceString
		//
		//  Replaces the content of the referenced string with the passed string value
		//
		//  PARAMETERS:
		//
		//		STRREF			-		Reference to the string to be deleted.
		//		char*			-		Pointer to the null-terminated string containing the replacement value
		//		size_t			-		Length of the replacement string value
		//
		//  RETURNS:
		//
		//		STRREF			-		Reference to the string, NULLSTRREF if the new string could not be added
		//
		//  NOTES:
		//
		//		1.	The reference will NOT change unless the addition fails
		//

		STRREF		replaceString(STRREF Ref, const char* pNewString, size_t NewStrLen) {

			//  Safety Checks
			if (pNewString == nullptr) NewStrLen = 0;

			//  If the referenced string is not NULLSTRREF then delete the existing string from the pool
			if (Ref != NULLSTRREF) deleteString(Ref);

			//  Check (and adjust if necessary) the capacity of the pool
			if (!checkCapacity(NewStrLen)) return NULLSTRREF;

			//  Locate the String Reference to be used
			if (Ref == NULLSTRREF) Ref = locateFreeStringRef();

			//  If we acquired a valid reference then populate it with the new string
			if (Ref != NULLSTRREF) {

				//
				//  Set the offset of the new string in the reference table
				//
				SRT[Ref - 1] = SPUsed;

				//
				//  If we have a non-null string then add it to the pool
				//
				if (pNewString != nullptr && NewStrLen > 0) memcpy(SP + SPUsed, pNewString, NewStrLen);

				//
				//  Add a string terminator
				//
				SP[SPUsed + NewStrLen] = '\0';

				//
				//  Update the pool information
				//
				SRTEnts++;
				if (Ref > SRTHiWater) SRTHiWater++;
				SPUsed += (NewStrLen + 1);
			}

			//  Return the reference
			return Ref;
		}

		//  getStringCount
		//
		//  Returns the count of strings in the pool
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		size_t			-		Count of strings in the pool
		//
		//  NOTES:
		//
		//

		size_t		getStringCount() {
			size_t		Count = 0;													//  String counter

			for (size_t SX = 0; SX < SRTHiWater; SX++) {
				if (SRT[SX] != EmptySlot) Count++;
			}
			return Count;
		}

		//  searchString
		//
		//  Returns the reference (STRREF) to the passed string if exists in the pool
		//
		//  PARAMETERS:
		//
		//		char*			-		Pointer to the null-terminated string containing the string
		//
		//  RETURNS:
		//
		//		STRREF			-		Reference to the string, NULLSTRREF if the string could not be found
		//
		//  NOTES:
		//
		//		1.	This function is case sensitive
		//

		STRREF		searchString(const char* String) {
			return searchString(String, strlen(String), false);
		}

		//  searchString
		//
		//  Returns the reference (STRREF) to the passed string if exists in the pool
		//
		//  PARAMETERS:
		//
		//		char*			-		Pointer to the string to be located
		//		size_t			-		Length of the string
		//
		//  RETURNS:
		//
		//		STRREF			-		Reference to the string, NULLSTRREF if the string could not be found
		//
		//  NOTES:
		//
		//		1.	This function is case sensitive
		//

		STRREF		searchString(const char* String, size_t StringLen) {
			return searchString(String, StringLen, false);
		}

		//  searchStringi
		//
		//  Returns the reference (STRREF) to the passed string if exists in the pool
		//
		//  PARAMETERS:
		//
		//		char*			-		Pointer to the null-terminated string containing the string
		//
		//  RETURNS:
		//
		//		STRREF			-		Reference to the string, NULLSTRREF if the string could not be found
		//
		//  NOTES:
		//
		//		1.	This function is case IN-sensitive
		//

		STRREF		searchStringi(const char* String) {
			return searchString(String, strlen(String), true);
		}

		//  searchStringi
		//
		//  Returns the reference (STRREF) to the passed string if exists in the pool
		//
		//  PARAMETERS:
		//
		//		char*			-		Pointer to the string to be located
		//		size_t			-		Length of the string
		//
		//  RETURNS:
		//
		//		STRREF			-		Reference to the string, NULLSTRREF if the string could not be found
		//
		//  NOTES:
		//
		//		1.	This function is case IN-sensitive
		//

		STRREF		searchStringi(const char* String, size_t StringLen) {
			return searchString(String, StringLen, true);
		}

		//  searchString
		//
		//  Returns the reference (STRREF) to the passed string if exists in the pool
		//
		//  PARAMETERS:
		//
		//		char*			-		Pointer to the string to be located
		//		size_t			-		Length of the string
		//		bool			-		if the matching is case insensitive
		//
		//  RETURNS:
		//
		//		STRREF			-		Reference to the string, NULLSTRREF if the string could not be found
		//
		//  NOTES:
		//
		//

		STRREF		searchString(const char* String, size_t StringLen, bool CaseInsensitive) {

			//  Safety
			if (String == nullptr) return NULLSTRREF;
			if (StringLen == 0) return NULLSTRREF;
			if (String[0] == '\0') return NULLSTRREF;

			//  Search the string pool for the passed string
			for (size_t SX = 0; SX < SRTHiWater; SX++) {
				if (SRT[SX] != EmptySlot) {
					if (CaseInsensitive) {
						if (_memicmp(SP + SRT[SX], String, StringLen) == 0) {
							if (strlen(SP + SRT[SX]) == StringLen) return STRREF(SX + 1);
						}
					}
					else {
						if (memcmp(SP + SRT[SX], String, StringLen) == 0) {
							if (strlen(SP + SRT[SX]) == StringLen) return STRREF(SX + 1);
						}
					}
				}
			}

			//  NOT FOUND
			return NULLSTRREF;
		}

		//  addUniqueString
		//
		//  Returns the reference (STRREF) to the passed string if exists in the pool. If not the string is added and the 
		//  reference returned
		//
		//  PARAMETERS:
		//
		//		char*			-		Pointer to the null-terminated string containing the string
		//
		//  RETURNS:
		//
		//		STRREF			-		Reference to the existing or new string
		//
		//  NOTES:
		//
		//		1.	This function is case sensitive
		//

		STRREF		addUniqueString(const char* String) {
			return addUniqueString(String, strlen(String), false);
		}

		//  addUniqueString
		//
		//  Returns the reference (STRREF) to the passed string if exists in the pool. If not the string is added and the 
		//  reference returned
		//
		//  PARAMETERS:
		//
		//		char*			-		Pointer to the string to be added
		//		size_t			-		Length of the string
		//
		//  RETURNS:
		//
		//		STRREF			-		Reference to the existing or new string
		//
		//  NOTES:
		//
		//		1.	This function is case sensitive
		//

		STRREF		addUniqueString(const char* String, size_t StringLen) {
			return addUniqueString(String, StringLen, false);
		}

		//  addUniqueStringi
		//
		//  Returns the reference (STRREF) to the passed string if exists in the pool. If not the string is added and the 
		//  reference returned
		//
		//  PARAMETERS:
		//
		//		char*			-		Pointer to the null-terminated string containing the string
		//
		//  RETURNS:
		//
		//		STRREF			-		Reference to the existing or new string
		//
		//  NOTES:
		//
		//		1.	This function is case IN-sensitive
		//

		STRREF		addUniqueStringi(const char* String) {
			return addUniqueString(String, strlen(String), true);
		}

		//  addUniqueStringi
		//
		//  Returns the reference (STRREF) to the passed string if exists in the pool. If not the string is added and the 
		//  reference returned
		//
		//  PARAMETERS:
		//
		//		char*			-		Pointer to the string to be added
		//		size_t			-		Length of the string
		//
		//  RETURNS:
		//
		//		STRREF			-		Reference to the existing or new string
		//
		//  NOTES:
		//
		//		1.	This function is case IN-sensitive
		//

		STRREF		addUniqueStringi(const char* String, size_t StringLen) {
			return addUniqueString(String, StringLen, true);
		}

		//  addUniqueString
		//
		//  Returns the reference (STRREF) to the passed string if exists in the pool. If not the string is added and the 
		//  reference returned
		//
		//  PARAMETERS:
		//
		//		char*			-		Pointer to the string to be added
		//		size_t			-		Length of the string
		//		bool			-		true if the matching is case insensitive
		//
		//  RETURNS:
		//
		//		STRREF			-		Reference to the existing or new string
		//
		//  NOTES:
		//
		//

		STRREF		addUniqueString(const char* String, size_t StringLen, bool CaseInsensitive) {

			//  Safety
			if (String == nullptr) return NULLSTRREF;
			if (StringLen == 0) return NULLSTRREF;

			STRREF	ExistingString = searchString(String, StringLen, CaseInsensitive);
			if (ExistingString == NULLSTRREF) ExistingString = addString(String, StringLen);

			return ExistingString;
		}

		//  Copy Assignment operator =
		//
		//  Replaces the content of this StringPool with a deep copy of the content of another StringPool
		//
		//  PARAMETERS:
		//
		//		StringPool&		-		Reference to the Source StringPool
		//
		//  RETURNS:
		//
		//		StringPool&		-		Reference to the stringPool
		//
		//  NOTES:
		//

		StringPool& operator = (const StringPool& rhs) {

			//  Dismiss any existing allocations
			if (SRT != nullptr) free(SRT);
			SRT = nullptr;
			if (SP != nullptr) free(SP);
			SP = nullptr;

			//  Initialise the pool using the capacity from the source
			if (!initialisePool(rhs.SRTCap, rhs.SPCap)) {
				clearPool();
				return *this;
			}

			//  Copy the content of the SRT & SP from the source
			if (SRT != nullptr) memcpy(SRT, rhs.SRT, rhs.SRTCap * sizeof(size_t));
			if (SP != nullptr) memcpy(SP, rhs.SP, rhs.SPCap);

			//  Copy the pool information from the source
			SRTCap = rhs.SRTCap;
			SRTEnts = rhs.SRTEnts;
			SRTHiWater = rhs.SRTHiWater;
			SPCap = rhs.SPCap;
			SPUsed = rhs.SPUsed;

			//  Return to caller
			return *this;
		}

		//  Move Assignment operator =
		//
		//  Replaces the content of this StringPool with a shallow copy of the content of another StringPool.
		//  The source StringPool is emptied.
		//
		//  PARAMETERS:
		//
		//		StringPool&		-		Reference to the Source StringPool
		//
		//  RETURNS:
		//
		//		StringPool&		-		Reference to the stringPool
		//
		//  NOTES:
		//

		StringPool& operator = (StringPool&& rhs) noexcept {

			//  Dismiss any existing allocations
			if (SRT != nullptr) free(SRT);
			SRT = nullptr;
			if (SP != nullptr) free(SP);
			SP = nullptr;

			//  Acquire the underlying pool storage from the source
			SRT = rhs.SRT;
			SRTCap = rhs.SRTCap;
			SRTEnts = rhs.SRTEnts;
			SRTHiWater = rhs.SRTHiWater;

			SP = rhs.SP;
			SPCap = rhs.SPCap;
			SPUsed = rhs.SPUsed;

			//  Disconnect the underlying storage from the source
			rhs.SRT = nullptr;
			rhs.SRTCap = 0;
			rhs.SRTEnts = 0;
			rhs.SRTHiWater = 0;

			rhs.SP = nullptr;
			rhs.SPCap = 0;
			rhs.SPUsed = 0;

			//  Return
			return *this;
		}

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  String Feference Table

		size_t			SRTCap;																//  Reference table capacity
		size_t			SRTHiWater;															//  Number of string reference entries (high watermark)
		size_t			SRTEnts;															//  Number of string reference entries (in use)
		size_t*			SRT;																//  Pointer to the String Reference Table

		//  String Pool
		size_t			SPCap;																//  String Pool capacity (bytes)
		size_t			SPUsed;																//  String Pool Used (bytes)
		char*			SP;																	//  String Pool

		//  Empty String
		char			EmptyString;														//  Empty String

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  clearPool 
		//
		//  Clears the content of the pool to the default unallocated state
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		void	clearPool() {

			//  Clear eny existing pool content
			if (SRT != nullptr) free(SRT);
			SRT = nullptr;
			SRTCap = 0;
			SRTHiWater = 0;
			SRTEnts = 0;
			if (SP != nullptr) free(SP);
			SP = nullptr;
			SPCap = 0;
			SPUsed = 0;

			//  Return to caller
			return;
		}

		//  initialisePool 
		//
		//  Constructs a new StringPool with the specified size settings
		//
		//  PARAMETERS:
		//
		//		size_t			-		Capacity of the pool (number of strings)
		//		size_t			-		Capacity of the pool (string storage space)
		//
		//  RETURNS:
		//
		//		bool			-		true if the initialisation succeeded, otherwise false
		//
		//  NOTES:
		//

		bool	initialisePool(size_t RNS, size_t RSPS) {

			//  Clear eny existing pool content
			clearPool();

			//
			//  Allocate and initialise the String Reference Table (SRT)
			//

			SRT = (size_t*)malloc(RNS * sizeof(size_t));
			if (SRT == nullptr) return false;
			SRTCap = RNS;
			SRTHiWater = 0;
			SRTEnts = 0;
			memset(SRT, 0xFF, RNS * sizeof(size_t));

			//
			//  Allocate and initialise the String Pool (SP)
			//

			SP = (char*)malloc(RSPS);
			if (SP == nullptr) {
				free(SRT);
				SRT = nullptr;
				return false;
			}
			SPCap = RSPS;
			SPUsed = 0;
			memset(SP, 0, RSPS);

			//  Return to caller
			return true;
		}

		//  checkCapacity 
		//
		//  Checks and if necessary adjusts the capacity of the pool.
		//
		//  PARAMETERS:
		//
		//		size_t			-		Size if the string to be added to the pool
		//
		//  RETURNS:
		//
		//		bool			-		true if there is capacity for the string, otherwise false
		//
		//  NOTES:
		//

		bool	checkCapacity(size_t NewStrLen) {

			//
			//  1. If the pool is not yet initialised then force initialisation with sufficient capacity
			//

			if (SRTCap == 0 || SPCap == 0) {
				if (NewStrLen < 4096) NewStrLen = 4096;
				else NewStrLen = 2 * NewStrLen;
				return initialisePool(100, NewStrLen);
			}

			//
			//  2. The String Reference Table (SRT) must have capacity for one additional string
			//

			if (SRTEnts == SRTCap) {
				//  Expand the SRT Pool
				size_t* NewSRT = (size_t*)realloc(SRT, (SRTCap + 100) * sizeof(size_t));
				if (NewSRT == nullptr) {
					free(SRT);
					SRT = nullptr;
					return false;
				}
				else SRT = NewSRT;
				memset(SRT + SRTCap, 0xFF, 100 * sizeof(size_t));
				SRTCap += 100;
			}

			//
			//  3. The String Pool must have the capacity for the string plus terminator
			//

			NewStrLen++;
			while (NewStrLen >= (SPCap - SPUsed)) {
				//  Expand the String Pool (SP)
				char* NewSP = (char*)realloc(SP, SPCap + 4096);
				if (NewSP == nullptr) {
					free(SP);
					SP = nullptr;
					return false;
				}
				else SP = NewSP;
				memset(SP + SPCap, 0, 4096);
				SPCap += 4096;
			}

			//  Return to caller
			return true;
		}

		//  locateFreeStringRef 
		//
		//  Locates the first available string reference to use
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		STRREF			-		The first available string reference for a new string
		//
		//  NOTES:
		//

		STRREF	locateFreeStringRef() {

			//  If there are no unused references in the table then return the next at the hi water mark
			if (SRTEnts == SRTHiWater) return STRREF(SRTHiWater + 1);

			//  Search the String Reference Table for the first unused entry
			for (size_t SRX = 0; SRX < SRTHiWater; SRX++) {
				if (SRT[SRX] == EmptySlot) return STRREF(SRX + 1);
			}

			//  SNO - no available slot was located
			return NULLSTRREF;
		}
	};

}
