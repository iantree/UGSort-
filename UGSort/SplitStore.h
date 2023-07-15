#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       SplitStore.h																						*
//*   Suite:      Experimental Algorithms																			*
//*   Version:    1.6.0	(Build: 07)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the SplitStore template class.										*
//* The SplitStore class provides the implementation of the storage component for the UGSort algorithm. 			*
//* The template parameter specifies a type for the internal sort records. Internal sort record types MUST contain	*
//* a member 'pKey' that is a pointer to the key on which the records are to be sorted. It would be expected that	*
//* the type also holds a reference to the location of the input sort record to which the key refers.				*
//*																													*
//*	ALGORITHM:																										*
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
//*	1.0.0 -		21/02/2023	-	Initial Release																		*
//*	1.2.0 -		01/03/2023	-	Final merge done with preemptive merges												*
//*	1.4.0 -		10/03/2023	-	Output Iterator added																*
//*	1.5.0 -		13/03/2023	-	SS3 structure changes																*
//*	1.6.0 -		15/03/2023	-	Stable Key Handling																	*
//*																													*
//*******************************************************************************************************************/

//  Include xymorg headers
#include	"../xymorg/xymorg.h"															//  xymorg system headers

//
//  Splitter Class Template
//

template <typename T>
class SplitStore {
private:
	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Private Nested Structures                                                                                     *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Arena (header) structure for keystore implementation
	typedef struct Arena {
		Arena*		pNext;																//  Pointer to the next arena
		size_t		FreeSpace;															//  Size of free space remaining in the arena
		char*		pKey;																//  Pointer to the next key to store
	} Arena;

public:

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Constructors			                                                                                        *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Constructor 
	//
	//  Constructs the SplitStore with an initial record and sort key length, Keystore is NOT used
	//
	//  PARAMETERS:
	//
	//		T&				-		Reference to the initial record to be stored in the Soplitter
	//		size_t			-		Sort Key Length
	//
	//  RETURNS:
	//
	//  NOTES:
	//

	SplitStore<T>(T& IRec, size_t KeyLen) : SRANum(0), SRAHi(0), SRALo(0), KL(KeyLen), SRAInc(256) {

		//  No keystore is used
		pKeyStore = nullptr;
		pLastArena = nullptr;
		ArenaSize = 0;

		//  Initialise the split store chain
		pNext = nullptr;

		//  Initialise the Sort Records Array (SRA)
		SRASize = SRAInc;
		pSRA = (T*)malloc(SRASize * sizeof(T));
		if (pSRA == nullptr) return;

		//  Copy the initial sort record to the mid-point in the array
		SRAHi = SRALo = (SRASize / 2);
		memcpy(&pSRA[SRALo], &IRec, sizeof(T));
		SRANum = 1;

		//  Return to caller
		return;
	}

	//  Constructor 
	//
	//  Constructs the SplitStore with an initial record and sort key length, Keystore is used
	//
	//  PARAMETERS:
	//
	//		T&				-		Reference to the initial record to be stored in the Soplitter
	//		size_t			-		Sort Key Length
	//		size_t			-		Keystore Arena Size in KB
	//
	//  RETURNS:
	//
	//  NOTES:
	//

	SplitStore<T>(T& IRec, size_t KeyLen, size_t KSASizeKB) : SRANum(0), SRAHi(0), SRALo(0), KL(KeyLen), SRAInc(256) {

		//  Initialise keystore 
		ArenaSize = KSASizeKB * 1024;
		if (ArenaSize < KeyLen) ArenaSize = 64 * 1024;
		pLastArena = (Arena*)malloc(ArenaSize);
		if (pLastArena == nullptr) return;
		pKeyStore = pLastArena;

		//  Initialise the arena header
		pLastArena->pNext = nullptr;
		pLastArena->FreeSpace = ArenaSize - sizeof(Arena);
		pLastArena->pKey = (char*)(pLastArena + 1);

		//  Initialise the splitter chain
		pNext = nullptr;

		//  Initialise the Sort Records Array (SRA)
		SRASize = SRAInc;
		pSRA = (T*)malloc(SRASize * sizeof(T));
		if (pSRA == nullptr) return;

		//  Copy the initial sort record to the mid-point in the array
		SRAHi = SRALo = (SRASize / 2);
		memcpy(&pSRA[SRALo], &IRec, sizeof(T));
		SRANum = 1;

		//  Copy the key to the keystore and update the record
		pSRA[SRALo].pKey = addKeyToStore(pSRA[SRALo].pKey);

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
	//  Destroys the Splitter object, dismissing the underlying objects/allocations
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//  NOTES:
	//  

	~SplitStore() {
		//  If there is a next splitter on the chain then delete that first
		if (pNext != nullptr) delete pNext;
		pNext = nullptr;

		//  Free the Sort Records Array (SRA)
		if (pSRA != nullptr) free(pSRA);
		pSRA = nullptr;
		SRASize = 0;
		SRANum = 0;
		SRAHi = 0;
		SRALo = 0;

		//  If the splitter has a keystore then delete each arena in the store
		if (pKeyStore != nullptr) {
			while (pKeyStore->pNext != nullptr) {
				pLastArena = pKeyStore;
				while (pLastArena->pNext->pNext != nullptr) pLastArena = pLastArena->pNext;
				free(pLastArena->pNext);
				pLastArena->pNext = nullptr;
			}

			//  Free the initial arena
			free(pKeyStore);
			pLastArena = pKeyStore = nullptr;
		}

		//  Return to caller
		return;
	}

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Public Members                                                                                                *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Sort Record array
	T*				pSRA;																	//  Array of sort records
	size_t			SRANum;																	//  Number of entries in the SR array
	size_t			SRAHi;																	//  Index of the highest collating entry in the SR array
	size_t			SRALo;																	//  Index of the lowest collating entry in the SR array

	//  SplitStore Chain
	SplitStore<T>*	pNext;																	//  Pointer to the next plitter on the chain

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Public Functions                                                                                              *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  addLowKey
	//
	//  Adds the passed record to the store below the low key position
	//
	//  PARAMETERS:
	//
	//		T&				-		Reference to the Sort Record to be added
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	addLowKey(T& NewRec) {

		//  Ensure that there is capacity for the new record
		if (SRALo == 0) {
			expandArray();
			if (SRALo == 0) return;
		}

		//  Add the record to the array before the lowest entry
		SRALo--;
		memcpy(&pSRA[SRALo], &NewRec, sizeof(T));
		SRANum++;
		return;
	}

	//  addHighKey
	//
	//  Adds the passed record to the store above the high key position
	//
	//  PARAMETERS:
	//
	//		T&				-		Reference to the Sort Record to be added
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	addHighKey(T& NewRec) {

		//  Ensure that there is capacity for the new record
		if (SRAHi == (SRASize - 1)) {
			expandArray();
			if (SRAHi == (SRASize - 1)) return;
		}

		//  Add the record to the array after the highest entry
		SRAHi++;
		memcpy(&pSRA[SRAHi], &NewRec, sizeof(T));
		SRANum++;
		return;
	}

	//  addNewStore
	//
	//  Adds the passed record to a new store oon the chain
	//
	//  PARAMETERS:
	//
	//		T&				-		Reference to the Sort Record to be added
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	addNewStore(T& NewRec) {

		//  The record does not fit the last splitter store extend the chain with a new splitter
		pNext = new SplitStore<T>(NewRec, KL);
		return;
	}

	//  addLowExternalKey
	//
	//  Adds the passed record to the store below the low key position with an external key
	//
	//  PARAMETERS:
	//
	//		T&				-		Reference to the Sort Record to be added
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	addLowExternalKey(T& NewRec) {

		//  Ensure that there is capacity for the new record
		if (SRALo == 0) {
			expandArray();
			if (SRALo == 0) return;
		}

		//  Add the record to the array before the lowest entry
		SRALo--;
		memcpy(&pSRA[SRALo], &NewRec, sizeof(T));
		pSRA[SRALo].pKey = addKeyToStore(pSRA[SRALo].pKey);
		SRANum++;
		return;
	}

	//  addHighExternalKey
	//
	//  Adds the passed record to the store above the high key position
	//
	//  PARAMETERS:
	//
	//		T&				-		Reference to the Sort Record to be added
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	addHighExternalKey(T& NewRec) {

		//  Ensure that there is capacity for the new record
		if (SRAHi == (SRASize - 1)) {
			expandArray();
			if (SRAHi == (SRASize - 1)) return;
		}

		//  Add the record to the array after the highest entry
		SRAHi++;
		memcpy(&pSRA[SRAHi], &NewRec, sizeof(T));
		pSRA[SRAHi].pKey = addKeyToStore(pSRA[SRAHi].pKey);
		SRANum++;
		return;
	}

	//  addNewExternalKeyStore
	//
	//  Adds the passed record to a new store on the chain
	//
	//  PARAMETERS:
	//
	//		T&				-		Reference to the Sort Record to be added
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	addNewExternalKeyStore(T& NewRec) {

		//  The record does not fit the last splitter store extend the chain with a new splitter
		pNext = new SplitStore<T>(NewRec, KL, ArenaSize / 1024);
		return;
	}

	//  mergeNextStore
	//
	//  Merges the next splitter store on the chain into this store
	//
	//  PARAMETERS:
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	mergeNextStore() {
		size_t			NewCapacity = 0;																	//  Capacity of the new merged array
		size_t			NewLo = 128;																		//  New array low entry index
		size_t			NewEnt = NewLo;																		//  Next enttry to be poppulated
		T*				pNewSRA = nullptr;																	//  New Sort Record Array (SRA)
		size_t			OldTEnt = SRALo;																	//  Next candidate (this)
		size_t			OldMEnt = 0;																		//  Next candidate (merge)

		//  Safety
		if (pNext == nullptr) return;
		NewCapacity = SRANum + pNext->SRANum + 256;
		OldMEnt = pNext->SRALo;

		//
		//  If the splitters are a special case for merging then perform the special case merges
		//
		if (mergeSpecialCase()) return;

		//  Allocate a new Sorted Record Array (SRA)
		pNewSRA = (T*)malloc(NewCapacity * sizeof(T));
		if (pNewSRA == nullptr) {
			std::cerr << "ERROR: SplitStore::mergeNextStore() failed to allocate a new SRA buffer (" << (NewCapacity * sizeof(T)) << " bytes)." << std::endl;
			//
			//  The error is catastrophic - delete the Splitter that is to be merged to prevent looping on this error
			//  The error will be picked up by a mismatch between the input and output record count
			//
			removeSplitter(pNext);
			return;
		}

		//
		//  Merge Phase 1 - Copy from the old current array into the new array until the current key > merge key
		//

		while (memcmp(pSRA[OldTEnt].pKey, pNext->pSRA[OldMEnt].pKey, KL) <= 0) {
			memcpy(&pNewSRA[NewEnt], &pSRA[OldTEnt], sizeof(T));
			NewEnt++;
			OldTEnt++;
		}

		//
		//  Merge Phase 2 - Copy from the lowest key of Current or merge until the merge array is exhausted
		//

		while (OldMEnt <= pNext->SRAHi) {
			if (memcmp(pSRA[OldTEnt].pKey, pNext->pSRA[OldMEnt].pKey, KL) <= 0) {
				//  Copy from the current array into the new array
				memcpy(&pNewSRA[NewEnt], &pSRA[OldTEnt], sizeof(T));
				OldTEnt++;
			}
			else {
				//  Copy from the merge array into the new array
				memcpy(&pNewSRA[NewEnt], &pNext->pSRA[OldMEnt], sizeof(T));
				OldMEnt++;
			}
			NewEnt++;
		}

		//
		//  Merge Phase 3 - Copy the residue from the current old array into the new array
		//

		while (OldTEnt <= SRAHi) {
			//  Copy from current into the new array
			memcpy(&pNewSRA[NewEnt], &pSRA[OldTEnt], sizeof(T));
			OldTEnt++;
			NewEnt++;
		}

		//
		//  Update the current splitter with the new array
		//

		free(pSRA);
		pSRA = pNewSRA;
		SRASize = NewCapacity;
		SRANum += pNext->SRANum;
		SRALo = NewLo;
		SRAHi = SRALo + (SRANum - 1);

		//  If the splitters are using a keystore then the mergee keystore chain of arenas is appended to the target chain.
		if (pKeyStore != nullptr) {
			pLastArena->pNext = pNext->pKeyStore;
			pLastArena = pNext->pLastArena;
			pNext->pKeyStore = nullptr;
			pNext->pLastArena = nullptr;
		}

		//  Dispose of the merged splitter
		removeSplitter(pNext);

		//  Return to caller
		return;
	}

	//  mergeNextStoreAscending
	//
	//  Merges the next splitter store on the chain into this store, keys are stable and sort is in ascending sequence
	//
	//  PARAMETERS:
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	mergeNextStoreAscending() {
		size_t			NewCapacity = 0;																	//  Capacity of the new merged array
		size_t			NewLo = 128;																		//  New array low entry index
		size_t			NewEnt = NewLo;																		//  Next enttry to be poppulated
		T*				pNewSRA = nullptr;																	//  New Sort Record Array (SRA)
		size_t			OldTEnt = SRALo;																	//  Next candidate (this)
		size_t			OldMEnt = 0;																		//  Next candidate (merge)

		//  Safety
		if (pNext == nullptr) return;
		NewCapacity = SRANum + pNext->SRANum + 256;
		OldMEnt = pNext->SRALo;

		//
		//  If the splitters are a special case for merging then perform the special case merges
		//
		if (mergeSpecialCase()) return;

		//  Allocate a new Sorted Record Array (SRA)
		pNewSRA = (T*)malloc(NewCapacity * sizeof(T));
		if (pNewSRA == nullptr) {
			std::cerr << "ERROR: SplitStore::mergeNextStore() failed to allocate a new SRA buffer (" << (NewCapacity * sizeof(T)) << " bytes)." << std::endl;
			//
			//  The error is catastrophic - delete the Splitter that is to be merged to prevent looping on this error
			//  The error will be picked up by a mismatch between the input and output record count
			//
			removeSplitter(pNext);
			return;
		}

		//
		//  Merge Phase 1 - Copy from the old current array into the new array until the current key > merge key
		//

		while (memcmp(pSRA[OldTEnt].pKey, pNext->pSRA[OldMEnt].pKey, KL) <= 0) {
			memcpy(&pNewSRA[NewEnt], &pSRA[OldTEnt], sizeof(T));
			NewEnt++;
			OldTEnt++;
		}

		//
		//  Merge Phase 2 - Copy from the lowest key of Current or merge until the merge array is exhausted
		//  Favours the leftmost (target) on identical keys
		//

		while (OldMEnt <= pNext->SRAHi) {
			if (memcmp(pSRA[OldTEnt].pKey, pNext->pSRA[OldMEnt].pKey, KL) <= 0) {
				//  Copy from the current array into the new array
				memcpy(&pNewSRA[NewEnt], &pSRA[OldTEnt], sizeof(T));
				OldTEnt++;
			}
			else {
				//  Copy from the merge array into the new array
				memcpy(&pNewSRA[NewEnt], &pNext->pSRA[OldMEnt], sizeof(T));
				OldMEnt++;
			}
			NewEnt++;
		}

		//
		//  Merge Phase 3 - Copy the residue from the current old array into the new array
		//

		while (OldTEnt <= SRAHi) {
			//  Copy from current into the new array
			memcpy(&pNewSRA[NewEnt], &pSRA[OldTEnt], sizeof(T));
			OldTEnt++;
			NewEnt++;
		}

		//
		//  Update the current splitter with the new array
		//

		free(pSRA);
		pSRA = pNewSRA;
		SRASize = NewCapacity;
		SRANum += pNext->SRANum;
		SRALo = NewLo;
		SRAHi = SRALo + (SRANum - 1);

		//  If the splitters are using a keystore then the mergee keystore chain of arenas is appended to the target chain.
		if (pKeyStore != nullptr) {
			pLastArena->pNext = pNext->pKeyStore;
			pLastArena = pNext->pLastArena;
			pNext->pKeyStore = nullptr;
			pNext->pLastArena = nullptr;
		}

		//  Dispose of the merged splitter
		removeSplitter(pNext);

		//  Return to caller
		return;
	}

	//  mergeNextStoreDescending
	//
	//  Merges the next splitter store on the chain into this store, keys are stable and sort is in ascending sequence
	//
	//  PARAMETERS:
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	mergeNextStoreDescending() {
		size_t			NewCapacity = 0;																	//  Capacity of the new merged array
		size_t			NewLo = 128;																		//  New array low entry index
		size_t			NewEnt = NewLo;																		//  Next enttry to be poppulated
		T*				pNewSRA = nullptr;																	//  New Sort Record Array (SRA)
		size_t			OldTEnt = SRALo;																	//  Next candidate (this)
		size_t			OldMEnt = 0;																		//  Next candidate (merge)

		//  Safety
		if (pNext == nullptr) return;
		NewCapacity = SRANum + pNext->SRANum + 256;
		OldMEnt = pNext->SRALo;

		//
		//  If the splitters are a special case for merging then perform the special case merges
		//
		if (mergeSpecialCase()) return;

		//  Allocate a new Sorted Record Array (SRA)
		pNewSRA = (T*)malloc(NewCapacity * sizeof(T));
		if (pNewSRA == nullptr) {
			std::cerr << "ERROR: SplitStore::mergeNextStore() failed to allocate a new SRA buffer (" << (NewCapacity * sizeof(T)) << " bytes)." << std::endl;
			//
			//  The error is catastrophic - delete the Splitter that is to be merged to prevent looping on this error
			//  The error will be picked up by a mismatch between the input and output record count
			//
			removeSplitter(pNext);
			return;
		}

		//
		//  Merge Phase 1 - Copy from the old current array into the new array until the current key > merge key
		//

		while (memcmp(pSRA[OldTEnt].pKey, pNext->pSRA[OldMEnt].pKey, KL) <= 0) {
			memcpy(&pNewSRA[NewEnt], &pSRA[OldTEnt], sizeof(T));
			NewEnt++;
			OldTEnt++;
		}

		//
		//  Merge Phase 2 - Copy from the lowest key of Current or merge until the merge array is exhausted
		//  Favours the rightmost (target) on identical keys
		//

		while (OldMEnt <= pNext->SRAHi) {
			if (memcmp(pSRA[OldTEnt].pKey, pNext->pSRA[OldMEnt].pKey, KL) < 0) {
				//  Copy from the current array into the new array
				memcpy(&pNewSRA[NewEnt], &pSRA[OldTEnt], sizeof(T));
				OldTEnt++;
			}
			else {
				//  Copy from the merge array into the new array
				memcpy(&pNewSRA[NewEnt], &pNext->pSRA[OldMEnt], sizeof(T));
				OldMEnt++;
			}
			NewEnt++;
		}

		//
		//  Merge Phase 3 - Copy the residue from the current old array into the new array
		//

		while (OldTEnt <= SRAHi) {
			//  Copy from current into the new array
			memcpy(&pNewSRA[NewEnt], &pSRA[OldTEnt], sizeof(T));
			OldTEnt++;
			NewEnt++;
		}

		//
		//  Update the current splitter with the new array
		//

		free(pSRA);
		pSRA = pNewSRA;
		SRASize = NewCapacity;
		SRANum += pNext->SRANum;
		SRALo = NewLo;
		SRAHi = SRALo + (SRANum - 1);

		//  If the splitters are using a keystore then the mergee keystore chain of arenas is appended to the target chain.
		if (pKeyStore != nullptr) {
			pLastArena->pNext = pNext->pKeyStore;
			pLastArena = pNext->pLastArena;
			pNext->pKeyStore = nullptr;
			pNext->pLastArena = nullptr;
		}

		//  Dispose of the merged splitter
		removeSplitter(pNext);

		//  Return to caller
		return;
	}

private:

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Private Members			                                                                                    *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Configuration
	size_t			KL;																		//  Key Length

	//  Sort Record Array
	size_t			SRASize;																//  Size of the sort record array
	size_t			SRAInc;																	//  Sort record array increment size

	//  Keystore
	Arena*			pKeyStore;																//  First arena in the keystore
	Arena*			pLastArena;																//  Last arena in the keystore
	size_t			ArenaSize;																//  Arena size (bytes)

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Private Functions                                                                                             *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  addKeyToStore
	//
	//  Returns a pointer to the copy of the passed key in the keystore
	//
	//  PARAMETERS:
	//
	//		char*			-		Const pointer to the key to be added to the keystore
	//
	//  RETURNS:
	// 
	//		char*			-		Pointer to the copy of the key in the keystore
	//
	//  NOTES:
	//  

	char* addKeyToStore(const char* pKey) {
		char*		pSKey = nullptr;														//  Pointer to key in the store

		//  Check that there is enough free space in the last arena of the keystore
		if (pLastArena->FreeSpace < KL) {
			//  Add an additional arena to the keystore
			pLastArena->pNext = (Arena*)malloc(ArenaSize);
			if (pLastArena->pNext == nullptr) return nullptr;
			pLastArena->pNext->pNext = nullptr;
			pLastArena->pNext->FreeSpace = ArenaSize - sizeof(Arena);
			pLastArena->pNext->pKey = (char*)(pLastArena->pNext + 1);
			pLastArena = pLastArena->pNext;
		}

		//  Copy the key into the keystore and update the arena header
		pSKey = pLastArena->pKey;
		memcpy(pLastArena->pKey, pKey, KL);
		pLastArena->FreeSpace -= KL;
		pLastArena->pKey += KL;

		//  Return pointer to the key
		return pSKey;
	}

	//  expandArray
	//
	//  Expands the Sort Records Array (SRA) to accomodate additional records
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//  NOTES:
	//  

	void	expandArray() {
		size_t		Extra = 0;																//  Additional records
		T*			pNewSRA = nullptr;														//  Pointer to the new Sort Records Array (SRA)
		size_t		NewLo = 0;																//  New Low index

		//  Determine the capacity increment
		if (SRALo == 0) Extra += SRAInc;
		if (SRAHi == (SRASize - 1)) Extra += SRAInc;
		if (Extra == 0) return;

		//  Reallocate the sort records array with the required capacity
		pNewSRA = (T*)realloc(pSRA, (SRASize + Extra) * sizeof(T));
		if (pNewSRA == nullptr) return;

		//  Determine the new index of the low record, if it must be adjusted
		if (SRALo == 0) {
			NewLo = SRAInc;
			memmove(&pNewSRA[NewLo], &pNewSRA[SRALo], SRANum * sizeof(T));
		}
		else NewLo = SRALo;

		//  Update the Hi and Lo indexes
		SRALo = NewLo;
		SRAHi = NewLo + (SRANum - 1);

		//  Update the array
		pSRA = pNewSRA;
		SRASize += Extra;

		//  Update the increment size
		if (SRAInc < (64 * 1024)) SRAInc = SRAInc * 2;

		//  Return to caller
		return;
	}

	//  removeSplitter
	//
	//  Removes the designated Splitter from the splitter chain
	//
	//  PARAMETERS:
	//
	//		Splitter*		-		Pointer to the splitter to be removed
	//
	//  RETURNS:
	//
	//  NOTES:
	//  

	void	removeSplitter(SplitStore<T>* pRS) {
		SplitStore<T>*		pPrevious = this;																		//  Previous splitter on the chain

		//  Not allowed to remove the current splitter nor the root splitter
		if (pRS == nullptr) return;
		if (pRS == this) return;

		//  Scan the splitter chain to find the previous splitter to the target
		while (pPrevious->pNext != pRS) {
			if (pPrevious->pNext == nullptr) {
				std::cerr << "ERROR: SplitStore::removeSplitter() failed to locate the target splitter on the splitter chain." << std::endl;
				return;
			}
			pPrevious = pPrevious->pNext;
		}

		//  pPrevious now points to the splitter immediately before the target splitter

		//  Isolate the target splitter (remove it from the chain
		pPrevious->pNext = pRS->pNext;
		pRS->pNext = nullptr;

		//  Delete the target splitter
		delete pRS;

		//  Return to caller
		return;
	}

	//  mergeSpecialCase
	//
	//  Merges the next splitter into the current one, with special processing to relocate KeyStore entries
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	// 
	//		bool			-		true if the merge was completed by special case processing, otherwise false
	//
	//  NOTES:
	//  

	bool	mergeSpecialCase() {
		size_t			ACSize = 0;																		//  Arena content size

		//  If not using KeyStore then special case merge not required
		if (pKeyStore == nullptr) return false;

		//  If neither the target nor the mergee have a single KeyStore Arena then special case merging not possible
		if ((pKeyStore != pLastArena) && (pNext->pKeyStore != pNext->pLastArena)) return false;

		//  Special processing MAY be possible determine if mergee or target is a candidate for relocation
		if (pNext->pKeyStore == pNext->pLastArena) {
			//  The mergee is a candidate for relocation - determine if the arena content will fit into the
			//  free space in the target last arena.
			ACSize = pNext->pLastArena->pKey - ((char*)pNext->pLastArena + sizeof(Arena));
			if (ACSize > pLastArena->FreeSpace) return false;

			//  Perform a mergee relocation merge
			mergeRelocateMergee();
		}
		else {
			//  The target is a candidate for relocation - determine if the arena content will fit into the
			//  free space in the mergee last arena.
			ACSize = pLastArena->pKey - ((char*)pLastArena + sizeof(Arena));
			if (ACSize > pNext->pLastArena->FreeSpace) return false;

			//  Perform a target relocation merge
			mergeRelocateTarget();
		}

		//  Return showing merge was completed by special processing
		return true;
	}

	//  mergeRelocateTarget
	//
	//  Merges the next splitter store into the current one, the target KeyStore will be relocated during this process.
	//  The function will swap the target and mergee KeyStores and process as a mergee relocation.
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//  NOTES:
	//  

	void	mergeRelocateTarget() {
		Arena* pTemp = nullptr;

		pTemp = pNext->pKeyStore;
		pNext->pKeyStore = pKeyStore;
		pKeyStore = pTemp;

		pTemp = pNext->pLastArena;
		pNext->pLastArena = pLastArena;
		pLastArena = pTemp;

		return mergeRelocateMergee();
	}

	//  mergeRelocateMergee
	//
	//  Merges the next splitter store into the current one, the mergee KeyStore will be relocated during this process.
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//  NOTES:
	//  

	void	mergeRelocateMergee() {
		char*			pRFK = nullptr;																		//  Pointer to the first key to be relocate
		char*			pRelBase = nullptr;																	//  Base address in the target arena
		size_t			RelSize = 0;																		//  Size of keys to be relocated
		size_t			NewCapacity = SRANum + pNext->SRANum + 256;											//  Capacity of the new merged array
		size_t			NewLo = 128;																		//  New array low entry index
		size_t			NewEnt = NewLo;																		//  Next enttry to be poppulated
		T*				pNewSRA = nullptr;																	//  New Sort Record Array (SRA)
		size_t			OldTEnt = SRALo;																	//  Next candidate (this)
		size_t			OldMEnt = pNext->SRALo;																//  Next candidate (merge)

		//  Calculate first key to relocate and relocation size 
		pRFK = (char*)(pNext->pLastArena + 1);
		RelSize = pNext->pLastArena->pKey - pRFK;

		//  Relocate the block of keys into the last arena of the target and update the arena
		pRelBase = pLastArena->pKey;
		memcpy(pRelBase, pRFK, RelSize);
		pLastArena->FreeSpace -= RelSize;
		pLastArena->pKey += RelSize;

		//
		//  The merge process is now performed as usual BUT any keys from the mergee will be updated to point to the key in the target keystore.
		//  pNewKey = (pOldKey - pRFK) + pRelBase - i.e. Offset + Base
		//

		//  Allocate a new Sorted Record Array (SRA)
		pNewSRA = (T*)malloc(NewCapacity * sizeof(T));
		if (pNewSRA == nullptr) {
			std::cerr << "ERROR: SplitStore::mergeRelocateMergee() failed to allocate a new SRA buffer (" << (NewCapacity * sizeof(T)) << " bytes)." << std::endl;
			//
			//  The error is catastrophic - delete the Splitter that is to be merged to prevent looping on this error
			//  The error will be picked up by a mismatch between the input and output record count
			//
			removeSplitter(pNext);
			return;
		}

		//
		//  Merge Phase 1 - Copy from the old current array into the new array until the current key > merge key
		//  No relocation is needed
		//

		while (memcmp(pSRA[OldTEnt].pKey, pNext->pSRA[OldMEnt].pKey, KL) <= 0) {
			memcpy(&pNewSRA[NewEnt], &pSRA[OldTEnt], sizeof(T));
			NewEnt++;
			OldTEnt++;
		}

		//
		//  Merge Phase 2 - Copy from the lowest key of Current or merge until the merge array is exhausted
		//  Relocate keys from the mergee
		//

		while (OldMEnt <= pNext->SRAHi) {
			if (memcmp(pSRA[OldTEnt].pKey, pNext->pSRA[OldMEnt].pKey, KL) <= 0) {
				//  Copy from the current array into the new array
				memcpy(&pNewSRA[NewEnt], &pSRA[OldTEnt], sizeof(T));
				OldTEnt++;
			}
			else {
				//  Copy from the merge array into the new array
				memcpy(&pNewSRA[NewEnt], &pNext->pSRA[OldMEnt], sizeof(T));
				//  Relocate key
				pNewSRA[NewEnt].pKey = (((char*)pNewSRA[NewEnt].pKey) - pRFK) + pRelBase;
				OldMEnt++;
			}
			NewEnt++;
		}

		//
		//  Merge Phase 3 - Copy the residue from the current old array into the new array
		//

		while (OldTEnt <= SRAHi) {
			//  Copy from current into the new array
			memcpy(&pNewSRA[NewEnt], &pSRA[OldTEnt], sizeof(T));
			OldTEnt++;
			NewEnt++;
		}

		//
		//  Update the current splitter with the new array
		//

		free(pSRA);
		pSRA = pNewSRA;
		SRASize = NewCapacity;
		SRANum += pNext->SRANum;
		SRALo = NewLo;
		SRAHi = SRALo + (SRANum - 1);

		//  Dispose of the mereged splitter
		removeSplitter(pNext);

		//  Return to caller
		return;
	}

};
