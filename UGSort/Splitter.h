#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       Splitter.h																						*
//*   Suite:      Experimental Algorithms																			*
//*   Version:    1.13.0	(Build: 14)																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the Splitter template class.										*
//* The Splitter class provides the implementation of the UGSort algorithm. 										*
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
//*	1.7.0 -		03/05/2023	-	Head Preemptive Merge 																*
//*	1.8.0 -		03/05/2023	-	Tail Preemptive Merge 																*
//*	1.9.0 -		03/05/2023	-	Revert to Alternate Preemptive Merge												*
//*	1.10.0 -	04/05/2023	-	Autonomous computation of Maximum Splitters											*
//*	1.11.0 -	06/05/2023	-	Tail first final merge																*
//*	1.12.0 -	06/05/2023	-	Revert Tail first final merge														*
//*	1.13.0 -	13/06/2023	-	PM Activity & T_SO sub-phase timing													*
//*	1.14.0 -	08/07/2023	-	Remove T_SO sub-phase timing and clarify timings									*
//*							-	Corrected PM control parameters														*
//*																													*
//*******************************************************************************************************************/

//  Include xymorg headers
#include	"../xymorg/xymorg.h"															//  xymorg system headers

//  Application headers
#include	"SplitStore.h"																	//  Splitter Store

//
//  Splitter Class Template
//

template <typename T>
class Splitter {
public:

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Public Nested Classes		                                                                                    *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//
	//  Output - This class provides an iterator over the content of the splitter
	//

	class Output {
	public:
		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors			                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Constructor 
		//
		//  Constructs the Output iterator positioned at the initial record to be output 
		//
		//  PARAMETERS:
		//
		//		T*				-		Pointer to the record in the store that is the iterator initial position
		//		size_t			-		Index of the initial position record
		//
		//  RETURNS:
		//
		//  NOTES:
		// 
		//   1.		Internally the position index is stored as an ordinal value, to allow 0th element to be accessed
		//

		Output(T* pIRec, size_t IIndex) : CurrPos(IIndex + 1), pRec(pIRec) {

			//  Return to caller
			return;
		}

		//  Copy Constructor 
		//
		//  Constructs the Output iterator as a copy of the supplied iterator, position of the copy is the same as the source
		//
		//  PARAMETERS:
		//
		//		Output&			-		Reference to the source iterator to be copied
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		Output(const Output& Src) : CurrPos(Src.CurrPos), pRec(Src.pRec) {

			//  Return to caller
			return;
		}

		//  Move Constructor 
		//
		//  Constructs the Output iterator as a copy of the supplied iterator, position of the copy is the same as the source
		//
		//  PARAMETERS:
		//
		//		Output&			-		Reference to the source iterator to be copied
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		Output(const Output&& Src) noexcept : CurrPos(Src.CurrPos), pRec(Src.pRec) {

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
		//  Destroys the Ouput iterator
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		~Output() {
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

		//  Iterator comparator functions

		bool operator == (const Output& RHS) {
			if (CurrPos == RHS.CurrPos) return true;
			return false;
		}

		bool operator != (const Output& RHS) {
			if (CurrPos == RHS.CurrPos) return false;
			return true;
		}

		bool operator > (const Output& RHS) {
			if (CurrPos > RHS.CurrPos) return true;
			return false;
		}

		bool operator >= (const Output& RHS) {
			if (CurrPos >= RHS.CurrPos) return true;
			return false;
		}

		bool operator < (const Output& RHS) {
			if (CurrPos < RHS.CurrPos) return true;
			return false;
		}

		bool operator <= (const Output& RHS) {
			if (CurrPos <= RHS.CurrPos) return true;
			return false;
		}

		//  Dereference * operator
		//
		//	Returns a non-const reference to record at the current position 
		// 	
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		T&					-	Reference to the record at the current position
		//
		//  NOTES
		//

		T& operator * () { return *pRec; }

		//  Pointer -> operator
		//
		//	Returns a pointer to the current record
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		T*					-	Pointer to the current record
		//
		//  NOTES
		//

		T* operator -> () { return pRec; }

		//  Prefix Increment ++X operator
		//
		//  Increments the index position and pointer to the current record and returns the updated iterautor
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		Output&					-	Reference to the updated iterator
		//
		//  NOTES
		// 
		//   1.	No bounds checking is performed, it is the responsibility of the caller to detect the hibound using comparator functions
		//

		Output& operator ++ () {
			CurrPos++;
			pRec++;
			return *this;
		}

		//  Postfix Increment X++ operator
		//
		//  Increments the index position and pointer to the current record and returns a clone of the pre-modified iterator
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		Output					-	Clone of the pre-modified iterator
		//
		//  NOTES
		// 
		//   1.	No bounds checking is performed, it is the responsibility of the caller to detect the hibound using comparator functions
		//

		Output operator ++ (int) {
			Output		PreMod(*this);
			CurrPos++;
			pRec++;
			return PreMod;
		}

		//  Prefix Decrement --X operator
		//
		//  Decrements the index position and pointer to the current record and returns the updated iterautor
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		Output&					-	Reference to the updated iterator
		//
		//  NOTES
		// 
		//   1.	Minimal bounds checking is performed, it is the responsibility of the caller to detect the hibound using comparator functions
		//

		Output& operator -- () {
			if (CurrPos > 0) {
				CurrPos--;
				pRec--;
			}
			return *this;
		}

		//  Postfix Decrement X-- operator
		//
		//  Decrements the index position and pointer to the current record and returns a clone of the pre-modified iterator
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		Output					-	Clone of the pre-modified iterator
		//
		//  NOTES
		// 
		//   1.	Minimal bounds checking is performed, it is the responsibility of the caller to detect the hibound using comparator functions
		//

		Output operator -- (int) {
			Output		PreMod(*this);
			if (CurrPos > 0) {
				CurrPos--;
				pRec--;
			}
			return PreMod;
		}

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members                                                                                               *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		size_t		CurrPos;													//  Ordinal position index of current record
		T*			pRec;														//  Pointer to the current record

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

	};

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Constructors			                                                                                        *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Constructor 
	//
	//  Constructs the Splitter with an initial record and sort key length, Keystore is NOT used
	//
	//  PARAMETERS:
	//
	//		T&				-		Reference to the initial record to be stored in the Splitter stores
	//		size_t			-		Sort Key Length
	//
	//  RETURNS:
	//
	//  NOTES:
	//

	Splitter<T>(T& IRec, size_t KeyLen) : KL(KeyLen), CumPMTime(0) {

		//  Initialise the splitStore chain
		pStore = new SplitStore<T>(IRec, KeyLen);

		//  Initialise the Preemptive Merge controls
		RecNo = 1;
		StoreCount = 1;
		MaxStores = 100;
		MaxSInc = 25;

		//  Return to caller
		return;
	}

	//  Constructor 
	//
	//  Constructs the Splitter with an initial record and sort key length, Keystore is used
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

	Splitter<T>(T& IRec, size_t KeyLen, size_t KSASizeKB) : KL(KeyLen), CumPMTime(0) {

		//  Initialise the splitstore chain
		pStore = new SplitStore<T>(IRec, KeyLen, KSASizeKB);

		//  Initialise the Preemptive Merge controls
		RecNo = 1;
		StoreCount = 1;
		MaxStores = 400;

		//  Clear statistics
		NumPMs = 0;
		PMStoresMerged = 0;

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

	~Splitter() {

		//  Destroy the SplitStore chain (if it exists)
		if (pStore != nullptr) delete pStore;
		pStore = nullptr;

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

	//  add
	//
	//  Adds the passed record to the Splitter Store chain, preemptive merging is conditionally ENABLED
	//
	//  PARAMETERS:
	//
	//		T&				-		Reference to the Sort Record to be added
	//		bool			-		true if PM is emabled, false if disabled
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 
	//		This is far more elegently implemented as a recursion, however flattening avoids potential stack overflows.
	//  

	void	add(T& NewSR, bool PMEnabled) {
		SplitStore<T>*		pCurrStore = pStore;										//  Current splitter store
		SplitStore<T>*		pLastStore = pStore;										//  Terminal splitter

		//  Increment the record number and if it is a multiple of the increment factor bump the max stores
		RecNo++;
		//if (RecNo % MaxSIncRecs == 0) MaxStores += MaxSInc;

		//  Loop processing each splitter on the chain
		while (pCurrStore != nullptr) {

			//  Determine if the new record should be inserted before the low key record in the current splitter
			if (memcmp(NewSR.pKey, pCurrStore->pSRA[pCurrStore->SRALo].pKey, KL) <= 0) {
				pCurrStore->addLowKey(NewSR);
				return;
			}

			//  Determine if the new record key can be inserted after the high key record in the current splitter
			if (memcmp(NewSR.pKey, pCurrStore->pSRA[pCurrStore->SRAHi].pKey, KL) >= 0) {
				pCurrStore->addHighKey(NewSR);
				return;
			}
			//  Advance to the next store
			if (pCurrStore->pNext == nullptr) pLastStore = pCurrStore;
			pCurrStore = pCurrStore->pNext;
		}

		//  End of the store chain reached without consuming the new record.
		//  A new store must be added to the chain to accomodate the new record

		pLastStore->addNewStore(NewSR);
		StoreCount++;

		//  Test for trigger of a preemptive merge if the store count has exceeded the maximum
		if (PMEnabled && (StoreCount > MaxStores)) {
			//  Perform the preemptive merge
			doPreemptiveMerge();
			//  Reset the store count
			StoreCount = count();
			//  Recompute the Maximum number of stores
			MaxStores = computeMaxStores(MaxStores, RecNo, MaxSInc);
		}

		//  Return to caller
		return;
	}

	//  addExternalKey
	//
	//  Adds the passed record to the Splitter Store chain with an external key, preemptive merging is conditionally ENABLED
	//
	//  PARAMETERS:
	//
	//		T&				-		Reference to the Sort Record to be added
	//		bool			-		true if PM is emabled, false if disabled
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 
	//		This is far more elegently implemented as a recursion, however flattening avoids potential stack overflows.
	//  

	void	addExternalKey(T& NewSR, bool PMEnabled) {
		SplitStore<T>*		pCurrStore = pStore;										//  Current splitter store
		SplitStore<T>*		pLastStore = pStore;										//  Terminal splitter

		//  Increment the record number and if it is a multiple of the increment factor bump the max stores
		RecNo++;

		//  Loop processing each splitter on the chain
		while (pCurrStore != nullptr) {

			//  Determine if the new record should be inserted before the low key record in the current splitter
			if (memcmp(NewSR.pKey, pCurrStore->pSRA[pCurrStore->SRALo].pKey, KL) <= 0) {
				pCurrStore->addLowExternalKey(NewSR);
				return;
			}

			//  Determine if the new record key can be inserted after the high key record in the current splitter
			if (memcmp(NewSR.pKey, pCurrStore->pSRA[pCurrStore->SRAHi].pKey, KL) >= 0) {
				pCurrStore->addHighExternalKey(NewSR);
				return;
			}
			//  Advance to the next store
			if (pCurrStore->pNext == nullptr) pLastStore = pCurrStore;
			pCurrStore = pCurrStore->pNext;
		}

		//  End of the store chain reached without consuming the new record.
		//  A new store must be added to the chain to accomodate the new record

		pLastStore->addNewExternalKeyStore(NewSR);
		StoreCount++;

		//  Test for trigger of a preemptive merge if the store count has exceeded the maximum
		if (PMEnabled && (StoreCount > MaxStores)) {
			//  Perform the preemptive merge
			doPreemptiveMerge();
			//  Reset the store count
			StoreCount = count();
			//  Recompute the Maximum number of stores
			MaxStores = computeMaxStores(MaxStores, RecNo, MaxSInc);
		}

		//  Return to caller
		return;
	}

	//
	//  Add variants for handling Stable Keys - note that ascending/descending sequence must be indicated
	//

	//  addStableKey
	//
	//  Adds the passed record to the Splitter Store chain, preemptive merging is ENABLED
	//  Identical keys have their input sequence preseved
	//
	//  PARAMETERS:
	//
	//		T&				-		Reference to the Sort Record to be added
	//		bool			-		true if the sequence is ascending false if descending
	//		bool			-		true if PM is emabled, false if disabled
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 
	//		This is far more elegently implemented as a recursion, however flattening avoids potential stack overflows.
	//  

	void	addStableKey(T& NewSR, bool Ascending, bool PMEnabled) {
		SplitStore<T>*		pCurrStore = pStore;										//  Current splitter store
		SplitStore<T>*		pLastStore = pStore;										//  Terminal splitter

		//  Increment the record number and if it is a multiple of the increment factor bump the max stores
		RecNo++;

		//  Loop processing each splitter on the chain
		while (pCurrStore != nullptr) {

			//  Determine if the new record should be inserted before the low key record in the current splitter
			if (memcmp(NewSR.pKey, pCurrStore->pSRA[pCurrStore->SRALo].pKey, KL) < 0) {
				pCurrStore->addLowKey(NewSR);
				return;
			}

			//  Determine if the new record key can be inserted after the high key record in the current splitter
			if (memcmp(NewSR.pKey, pCurrStore->pSRA[pCurrStore->SRAHi].pKey, KL) > 0) {
				pCurrStore->addHighKey(NewSR);
				return;
			}
			//  Advance to the next store
			if (pCurrStore->pNext == nullptr) pLastStore = pCurrStore;
			pCurrStore = pCurrStore->pNext;
		}

		//  End of the store chain reached without consuming the new record.
		//  A new store must be added to the chain to accomodate the new record

		pLastStore->addNewStore(NewSR);
		StoreCount++;

		//  Test for trigger of a preemptive merge if the store count has exceeded the maximum
		if (PMEnabled && (StoreCount > MaxStores)) {
			//  Perform the preemptive merge
			doStablePreemptiveMerge(Ascending);
			//  Reset the store count
			StoreCount = count();
			//  Recompute the Maximum number of stores
			MaxStores = computeMaxStores(MaxStores, RecNo, MaxSInc);
		}

		//  Return to caller
		return;
	}

	//  addStableExternalKey
	//
	//  Adds the passed record to the Splitter Store chain with an external key, preemptive merging is ENABLED
	//
	//  PARAMETERS:
	//
	//		T&				-		Reference to the Sort Record to be added
	//		bool			-		true if the sequence is ascending false if descending
	//		bool			-		true if PM is emabled, false if disabled
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 
	//		This is far more elegently implemented as a recursion, however flattening avoids potential stack overflows.
	//  

	void	addStableExternalKey(T& NewSR, bool Ascending, bool PMEnabled) {
		SplitStore<T>*		pCurrStore = pStore;										//  Current splitter store
		SplitStore<T>*		pLastStore = pStore;										//  Terminal splitter

		//  Increment the record number and if it is a multiple of the increment factor bump the max stores
		RecNo++;

		//  Loop processing each splitter on the chain
		while (pCurrStore != nullptr) {

			//  Determine if the new record should be inserted before the low key record in the current splitter
			if (memcmp(NewSR.pKey, pCurrStore->pSRA[pCurrStore->SRALo].pKey, KL) < 0) {
				pCurrStore->addLowExternalKey(NewSR);
				return;
			}

			//  Determine if the new record key can be inserted after the high key record in the current splitter
			if (memcmp(NewSR.pKey, pCurrStore->pSRA[pCurrStore->SRAHi].pKey, KL) > 0) {
				pCurrStore->addHighExternalKey(NewSR);
				return;
			}
			//  Advance to the next store
			if (pCurrStore->pNext == nullptr) pLastStore = pCurrStore;
			pCurrStore = pCurrStore->pNext;
		}

		//  End of the store chain reached without consuming the new record.
		//  A new store must be added to the chain to accomodate the new record

		pLastStore->addNewExternalKeyStore(NewSR);
		StoreCount++;

		//  Test for trigger of a preemptive merge if the store count has exceeded the maximum
		if (PMEnabled && (StoreCount > MaxStores)) {
			//  Perform the preemptive merge
			doStablePreemptiveMerge(Ascending);
			//  Reset the store count
			StoreCount = count();
			//  Recompute the Maximum number of stores
			MaxStores = computeMaxStores(MaxStores, RecNo, MaxSInc);
		}

		//  Return to caller
		return;
	}

	//  signalEndOfSortInput
	//
	//  This function will complete the sort process by merging all splitter stores into a single store
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	// 
	//		size_t		-		Count of records input
	//
	//  NOTES:
	// 

	size_t	signalEndOfSortInput() {

		//  Process merges of last into penultimate until only a single splitter remains
		while (count() > 1) {
			doFinalMergePass();
		}

		//  Return to caller
		return pStore->SRANum;
	}

	//  signalEndOfStableSortInput
	//
	//  This function will complete the sort process by merging all splitter stores into a single store
	//
	//  PARAMETERS:
	//
	//		bool			-		true if the sequence is ascending false if descending
	//
	//  RETURNS:
	// 
	//		size_t		-		Count of records input
	//
	//  NOTES:
	// 

	size_t	signalEndOfStableSortInput(bool Ascending) {

		//  Process merges of last into penultimate until only a single splitter remains
		while (count() > 1) {
			doStableFinalMergePass(Ascending);
		}

		//  Return to caller
		return pStore->SRANum;
	}

	//  isOutputValid
	//
	//  This function will check that the number of records input to the sort is equal to the number of records
	//  in the root store.
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	// 
	//		bool		-		true if the output is valid, otherwise false
	//
	//  NOTES:
	// 

	bool	isOutputValid() {

		if (RecNo != pStore->SRANum) return false;
		return true;
	}

	//
	//  Ouput Iterator creation functions
	//

	//  lowest
	//
	//  Returns an Output iterator positioned at the lowest collating record in the splitter
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	// 
	//		Output&			-		Reference to the iterator positioned to the lowest entry
	//
	//  NOTES:
	//  

	Output lowest() {
		return Output(&pStore->pSRA[pStore->SRALo], pStore->SRALo);
	}

	//  highest
	//
	//  Returns an Output iterator positioned at the highest collating record in the splitter
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	// 
	//		Output&			-		Reference to the iterator positioned to the highest entry
	//
	//  NOTES:
	//  

	Output highest() {
		return Output(&pStore->pSRA[pStore->SRAHi], pStore->SRAHi);
	}

	//  getCumulativePMTime
	//
	//  This function will return the cumulative time spent in preemptive merges
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	// 
	//		MILLISECONDS		-		Time duration of preemptive merges (ms)
	//
	//  NOTES:
	//  

	xymorg::MILLISECONDS		getCumulativePMTime() { return CumPMTime; }

	//  getPMCount
	//
	//  This function will return the count of preemptive merges performed
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	// 
	//		size_t		-		Count of preemptive merges (PM)
	//
	//  NOTES:
	//  

	size_t		getPMCount() { return NumPMs; }

	//  getPMStoresMerged
	//
	//  This function will return the count of stores merged by preemptive merges
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	// 
	//		size_t		-		Count of stores merged by preemptive merges (PM)
	//
	//  NOTES:
	//  

	size_t		getPMStoresMerged() { return PMStoresMerged; }

private:

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Private Members			                                                                                    *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Configuration
	size_t			KL;																		//  Key Length

	//  Preemptive Merge Controls
	size_t			RecNo;																	//  Record counter
	size_t			MaxStores;																//  Maximum number of splitter stores
	size_t			MaxSInc;																//  Percentage of Max Stores
	xymorg::MILLISECONDS	CumPMTime;														//  Cumulative preemptive merge time

	//  SplitStore Chain
	SplitStore<T>*	pStore;																	//  Pointer to the root store
	size_t			StoreCount;																//  Splitter Store counter

	//  Execution Statistics
	size_t			NumPMs;																	//  Count of pre-emptive merges
	size_t			PMStoresMerged;															//  Number of stores merged by PM

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Private Functions                                                                                             *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  doTailPreemptiveMerge
	//
	//  This function will perform a preemptive merge on the current splitter store chain.
	//  The merge pattern is always into the penultimate store on the chain.
	//
	//  PARAMETERS:
	// 
	//		size_t			-		Target number of splitters
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	doTailPreemptiveMerge(size_t TargetStores) {
		SplitStore<T>*		pCurrent = penultimate();										//  Current penultimate splitter
		xymorg::TIMER		StartPM = xymorg::CLOCK::now();									//  Start of preemptive merge
		xymorg::TIMER		EndPM = xymorg::CLOCK::now();									//  End of preemptive merge
		size_t				SCount = count();												//  Current number of splitters

		//  Limit check
		if (SCount <= TargetStores) return;

		//  Process Splitters until end-of-chain or target count is achieved
		while (pCurrent->pNext != nullptr && SCount > TargetStores) {

			//  Merge the following splitter into the current one
			pCurrent->mergeNextStore();

			//  Reduce the store count
			SCount--;

			//  Reposition to the penultimate
			pCurrent = penultimate();
		}

		//  Accumulate the time spent
		EndPM = xymorg::CLOCK::now();
		CumPMTime += DURATION(xymorg::MILLISECONDS, EndPM - StartPM);

		//  Return to caller
		return;
	}

	//  doHeadPreemptiveMerge
	//
	//  This function will perform a preemptive merge on the current splitter store chain.
	//  The merge pattern is always into the first store on the chain.
	//
	//  PARAMETERS:
	// 
	//		size_t			-		Target number of splitters
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	doHeadPreemptiveMerge(size_t TargetStores) {
		SplitStore<T>*		pCurrent = pStore;												//  Current (initially the root) splitter
		xymorg::TIMER		StartPM = xymorg::CLOCK::now();									//  Start of preemptive merge
		xymorg::TIMER		EndPM = xymorg::CLOCK::now();									//  End of preemptive merge
		size_t				SCount = count();												//  Current number of splitters

		//  Limit check
		if (SCount <= TargetStores) return;

		//  Process Splitters until end-of-chain or target count is achieved
		while (pCurrent->pNext != nullptr && SCount > TargetStores) {

			//  Merge the following splitter into the current one
			pCurrent->mergeNextStore();

			//  Reduce the store count
			SCount--;
		}

		//  Accumulate the time spent
		EndPM = xymorg::CLOCK::now();
		CumPMTime += DURATION(xymorg::MILLISECONDS, EndPM - StartPM);

		//  Return to caller
		return;
	}

	//  doPreemptiveMerge
	//
	//  This function will perform a preemptive merge on the current splitter store chain.
	//  The merge pattern is alternate stores are merged into their predecessors on the chain.
	//
	//  PARAMETERS:
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	doPreemptiveMerge() {
		SplitStore<T>*		pCurrent = pStore;												//  Current (initially the root) splitter
		xymorg::TIMER		StartPM = xymorg::CLOCK::now();									//  Start of preemptive merge
		xymorg::TIMER		EndPM = xymorg::CLOCK::now();									//  End of preemptive merge

		NumPMs++;

		//  Process all splitters
		while (pCurrent->pNext != nullptr) {

			// Merge the following splitter into the current one
			pCurrent->mergeNextStore();
			PMStoresMerged++;

			//  Move on to the next pair
			pCurrent = pCurrent->pNext;
			if (pCurrent == nullptr) break;
		}

		//  Accumulate the time spent
		EndPM = xymorg::CLOCK::now();
		CumPMTime += DURATION(xymorg::MILLISECONDS, EndPM - StartPM);

		//  Return to caller
		return;
	}

	//  doTailStablePreemptiveMerge
	//
	//  This function will perform a preemptive merge on the current splitter store chain.
	//  The merge pattern is always intio the penultimate store on the chain.
	//  This variant handles stable keys.
	//
	//  PARAMETERS:
	//
	//		bool			-		true if the sequence is ascending false if descending
	//		size_t			-		Target number of splitters
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	doTailStablePreemptiveMerge(bool Ascending, size_t TargetStores) {
		SplitStore<T>*		pCurrent = penultimate();										//  Current penultimate splitter
		xymorg::TIMER		StartPM = xymorg::CLOCK::now();									//  Start of preemptive merge
		xymorg::TIMER		EndPM = xymorg::CLOCK::now();									//  End of preemptive merge
		size_t				SCount = count();												//  Current number of splitters

		//  Process all splitters until end-of-chain or target reached
		while (pCurrent->pNext != nullptr && SCount > TargetStores) {

			// Merge the following splitter into the current one
			if (Ascending) pCurrent->mergeNextStoreAscending();
			else pCurrent->mergeNextStoreDescending();

			//  Reduce the store count
			SCount--;

			//  Reposition to the penultimate
			pCurrent = penultimate();
		}

		//  Accumulate the time spent
		EndPM = xymorg::CLOCK::now();
		CumPMTime += DURATION(xymorg::MILLISECONDS, EndPM - StartPM);

		//  Return to caller
		return;
	}

	//  doHeadStablePreemptiveMerge
	//
	//  This function will perform a preemptive merge on the current splitter store chain.
	//  The merge pattern is always intio the first store on the chain.
	//  This variant handles stable keys.
	//
	//  PARAMETERS:
	//
	//		bool			-		true if the sequence is ascending false if descending
	//		size_t			-		Target number of splitters
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	doHeadStablePreemptiveMerge(bool Ascending, size_t TargetStores) {
		SplitStore<T>*		pCurrent = pStore;												//  Current (initially the root) splitter
		xymorg::TIMER		StartPM = xymorg::CLOCK::now();									//  Start of preemptive merge
		xymorg::TIMER		EndPM = xymorg::CLOCK::now();									//  End of preemptive merge
		size_t				SCount = count();												//  Current number of splitters

		//  Process all splitters until end-of-chain or target reached
		while (pCurrent->pNext != nullptr && SCount > TargetStores) {

			// Merge the following splitter into the current one
			if (Ascending) pCurrent->mergeNextStoreAscending();
			else pCurrent->mergeNextStoreDescending();

			//  Reduce the store count
			SCount--;
		}

		//  Accumulate the time spent
		EndPM = xymorg::CLOCK::now();
		CumPMTime += DURATION(xymorg::MILLISECONDS, EndPM - StartPM);

		//  Return to caller
		return;
	}

	//  doStablePreemptiveMerge
	//
	//  This function will perform a preemptive merge on the current splitter store chain.
	//  The merge pattern is alternate stores are merged into theire predecessors on the chain.
	//  This variant handles stable keys.
	//
	//  PARAMETERS:
	//
	//		bool			-		true if the sequence is ascending false if descending
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	doStablePreemptiveMerge(bool Ascending) {
		SplitStore<T>*		pCurrent = pStore;												//  Current (initially the root) splitter
		xymorg::TIMER		StartPM = xymorg::CLOCK::now();									//  Start of preemptive merge
		xymorg::TIMER		EndPM = xymorg::CLOCK::now();									//  End of preemptive merge

		//  Process all splitters
		while (pCurrent->pNext != nullptr) {

			// Merge the following splitter into the current one
			if (Ascending) pCurrent->mergeNextStoreAscending();
			else pCurrent->mergeNextStoreDescending();

			//  Move on to the next pair
			pCurrent = pCurrent->pNext;
			if (pCurrent == nullptr) break;
		}

		//  Accumulate the time spent
		EndPM = xymorg::CLOCK::now();
		CumPMTime += DURATION(xymorg::MILLISECONDS, EndPM - StartPM);

		//  Return to caller
		return;
	}

	//  doTailFinalMergePass
	//
	//  This function will perform a final merge pass on the current splitter store chain.
	//  The merge pattern is tail first.
	//
	//  PARAMETERS:
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	doTailFinalMergePass() {
		SplitStore<T>* pCurrent = penultimate();												//  Current Penultimate splitter

		//  Process all splitters
		while (pCurrent->pNext != nullptr) {

			// Merge the following splitter into the current one
			pCurrent->mergeNextStore();

			//  Move on to the next pair
			pCurrent = penultimate();
			if (pCurrent == nullptr) break;
		}

		//  Return to caller
		return;
	}

	//  doFinalMergePass
	//
	//  This function will perform a final merge pass on the current splitter store chain.
	//  The merge pattern is alternate stores are merged into theire predecessors on the chain.
	//
	//  PARAMETERS:
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	doFinalMergePass() {
		SplitStore<T>* pCurrent = pStore;												//  Current (initially the root) splitter

		//  Process all splitters
		while (pCurrent->pNext != nullptr) {

			// Merge the following splitter into the current one
			pCurrent->mergeNextStore();

			//  Move on to the next pair
			pCurrent = pCurrent->pNext;
			if (pCurrent == nullptr) break;
		}

		//  Return to caller
		return;
	}

	//  doTailStableFinalMergePass
	//
	//  This function will perform a final merge pass on the current splitter store chain.
	//  The merge pattern is tail first.
	//
	//  PARAMETERS:
	//
	//		bool			-		true if the sequence is ascending false if descending
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	doTailStableFinalMergePass(bool Ascending) {
		SplitStore<T>*		pCurrent = penultimate();												//  Current penultimate splitter

		//  Process all splitters
		while (pCurrent->pNext != nullptr) {

			// Merge the following splitter into the current one
			if (Ascending) pCurrent->mergeNextStoreAscending();
			else pCurrent->mergeNextStoreDescending();

			//  Move on to the next pair
			pCurrent = penultimate();
			if (pCurrent == nullptr) break;
		}

		//  Return to caller
		return;
	}


	//  doStableFinalMergePass
	//
	//  This function will perform a final merge pass on the current splitter store chain.
	//  The merge pattern is alternate stores are merged into their predecessors on the chain.
	//
	//  PARAMETERS:
	//
	//		bool			-		true if the sequence is ascending false if descending
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	doStableFinalMergePass(bool Ascending) {
		SplitStore<T>* pCurrent = pStore;												//  Current (initially the root) splitter

		//  Process all splitters
		while (pCurrent->pNext != nullptr) {

			// Merge the following splitter into the current one
			if (Ascending) pCurrent->mergeNextStoreAscending();
			else pCurrent->mergeNextStoreDescending();

			//  Move on to the next pair
			pCurrent = pCurrent->pNext;
			if (pCurrent == nullptr) break;
		}

		//  Return to caller
		return;
	}

	//  count
	//
	//  Returns the number of splitter stores in the splitter store chain
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	// 
	//		size_t		-		Count of splitter stores
	//
	//  NOTES:
	//  

	size_t	count() {
		SplitStore<T>* pCurrent = pStore;											//  Current splitter store
		size_t				Count = 1;													//  Splitter store counter

		//  Chase the chain counting the splitters
		while (pCurrent->pNext != nullptr) {
			Count++;
			pCurrent = pCurrent->pNext;
		}

		return Count;
	}

	//  penultimate
	//
	//  Returns a pointer to the penultimate store on the chain
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	// 
	//		SplitStore*		-		Pointer to the penultimate splitter stores
	//
	//  NOTES:
	//  

	SplitStore<T>* penultimate() {
		SplitStore<T>* pCurrent = pStore;											//  Current splitter store

		while (pCurrent->pNext != nullptr) {
			if (pCurrent->pNext->pNext == nullptr) return pCurrent;
			pCurrent = pCurrent->pNext;
		}

		//  SNO
		return pStore;
	}

	//  computeMaxStores
	//
	//  Returns the new value for Maximum number of stores (preemptive merge trigger)
	//
	//  PARAMETERS:
	// 
	//		size_t		-		Current Max Stores
	//		size_t		-		Current Record Count
	//		size_t		-		Percentage of S to use
	//
	//  RETURNS:
	// 
	//		size_t		-		New Max Stores
	//
	//  NOTES:
	//  

	size_t		computeMaxStores(size_t CMS, size_t CRC, size_t POS) {
		size_t		NewMS = 0;															//  New Max Stores
		size_t		CurrentS = 0;														//  Current S

		//  Compute S for the current value of n S = 2*sqrt(n/2)
		CurrentS = size_t(ceil(sqrt(double(CRC / 2)) * 2.0));

		//  Compute new MaxS
		NewMS = (CurrentS * POS) / 100;

		//  Return the computed new max stores providing it is greater than the existing value
		if (NewMS > CMS) return NewMS;
		return CMS;
	}

};
