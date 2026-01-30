#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       Splitter.h																						*
//*   Suite:      Experimental Algorithms																			*
//*   Version:    1.17.0	(Build: 20)																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2026 Ian J. Tree																				*
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
//*	1.15.0 -	25/08/2023	-	Binary-Chop search of Store Chain													*
//*	1.16.0 -	16/10/2023	-	Improved PM handling of Worst Case (Tail-Suppression)								*
//*	1.17.0 -	28/01/2026	-	Include instrumentation package														*
//*																													*
//*******************************************************************************************************************/

//  Include xymorg headers
#include	"../xymorg/xymorg.h"															//  xymorg system headers

//  Application headers
#include	"IStats.h"																		//  Instrumentation
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
		T* pRec;														//  Pointer to the current record

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

	};

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Public Nested Structures	                                                                                    *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	typedef struct StoreChain {
		size_t			StoreCount;													//  Count of stores in the chain
		size_t			StoreCap;													//  Store capacity
		SplitStore<T>* Store[4096];												//  Array of pointers to the stores
	} StoreChain;

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
	//		IStats&			-		Reference to the instrumentation object
	//
	//  RETURNS:
	//
	//  NOTES:
	//

	Splitter<T>(T& IRec, size_t KeyLen, IStats& Ins) : KL(KeyLen), Stats(Ins) {

		//  Initialise the splitStore chain
		pStoreChain = (StoreChain*) malloc((2 * sizeof(size_t)) + (4096 * sizeof(void*)));
		if (pStoreChain == nullptr) {
			//  Fatal
			std::cerr << "FATAL: Failed to allocate a new Store Chain structure." << std::endl;
			std::abort();
		}
		else {
			memset(pStoreChain, 0, (2 * sizeof(size_t)) + (4096 * sizeof(void*)));
			pStoreChain->StoreCap = 4096; 
			pStoreChain->Store[0] = new SplitStore<T>(IRec, KeyLen, Stats);
			pStoreChain->StoreCount = 1;
			Stats.newKey();
		}

		//  Initialise the Preemptive Merge controls
		RecNo = 1;
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
	//		T&				-		Reference to the initial record to be stored in the Splitter
	//		size_t			-		Sort Key Length
	//		size_t			-		Keystore Arena Size in KB
	//		IStats&			-		Reference to the instrumentation object
	//
	//  RETURNS:
	//
	//  NOTES:
	//

	Splitter<T>(T& IRec, size_t KeyLen, size_t KSASizeKB, IStats& Ins) : KL(KeyLen), Stats(Ins) {

		//  Initialise the splitStore chain
		pStoreChain = (StoreChain*) malloc((2 * sizeof(size_t)) + (4096 * sizeof(void*)));
		if (pStoreChain == nullptr) {
			//  Fatal
			std::cerr << "FATAL: Failed to allocate a new Store Chain structure." << std::endl;
			std::abort();
		}
		else {
			memset(pStoreChain, 0, (2 * sizeof(size_t)) + (4096 * sizeof(void*)));
			pStoreChain->StoreCap = 4096;
			pStoreChain->Store[0] = new SplitStore<T>(IRec, KeyLen, KSASizeKB, Stats);
			pStoreChain->StoreCount = 1;
			Stats.newKey();
		}

		//  Initialise the Preemptive Merge controls
		RecNo = 1;
		MaxStores = 100;
		MaxSInc = 25;

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

		if (pStoreChain != nullptr) {
			for (size_t sIndex = 0; sIndex < pStoreChain->StoreCount; sIndex++) delete pStoreChain->Store[sIndex];
			delete pStoreChain;
		}
		pStoreChain = nullptr;

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
		int			CurrentStore = 0;														//  Start of the Store chain
		int			Delta = 0;																//  Delta distance for Binary Chop
		bool		Without = false;														//  Within/Without control
		bool		Below = false;															//  Above or below control
		bool		OtherWithout = false;													//  Other Within/Without control

		//  Increment the record number
		RecNo++;
#ifndef INSTRUMENTED
		Stats.newKey();
#endif

		//
		//  First check against the boundaries of the store chain
		//

		//  If the new key is equal or below the low key set a new low key
#ifdef INSTRUMENTED
		Stats.Compares++;
#endif
		if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRALo].pKey, KL) <= 0) {
			pStoreChain->Store[CurrentStore]->addLowKey(NewSR);
#ifdef INSTRUMENTED
			Stats.LoHits++;
			//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
			if (Stats.newKey()) {
				if (Stats.isPileUpInstrumentActive()) {
					Stats.writePileUpLeader();
					for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
						Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
					}
				}
			}
#endif
			return;
		}

		//  If the new key is equal or above the high key then set a new high key
#ifdef INSTRUMENTED
		Stats.Compares++;
#endif
		if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRAHi].pKey, KL) >= 0) {
			pStoreChain->Store[CurrentStore]->addHighKey(NewSR);
#ifdef INSTRUMENTED
			Stats.HiHits++;
			//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
			if (Stats.newKey()) {
				if (Stats.isPileUpInstrumentActive()) {
					Stats.writePileUpLeader();
					for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
						Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
					}
				}
			}
#endif
			return;
		}

		CurrentStore = int(pStoreChain->StoreCount) - 1;

		//  If the new key is within the key range of the last store in the chain then add a new store to accomodate the key
#ifdef INSTRUMENTED
		Stats.Compares++;
#endif
		if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRALo].pKey, KL) > 0) {
#ifdef INSTRUMENTED
			Stats.Compares++;
#endif
			if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRAHi].pKey, KL) < 0) {
				//  A new store must be added to the array to accomodate the key
#ifdef INSTRUMENTED
				Stats.NewStores++;
				Stats.Stores++;
#endif
				pStoreChain->Store[pStoreChain->StoreCount] = new SplitStore<T>(NewSR, KL, Stats);
				pStoreChain->StoreCount++;

				//  Test for trigger of a preemptive merge if the store count has exceeded the maximum
				if (PMEnabled && (pStoreChain->StoreCount > MaxStores)) {
					//  Perform the preemptive merge
#ifdef INSTRUMENTED
					Stats.PMs++;
#endif
					suppressTail();
					//  Recompute the Maximum number of stores
					MaxStores = computeMaxStores(MaxStores, RecNo, MaxSInc);

					//  Check that there is capacity in the StoreChain structure to accomodate the maximum number of stores
					if (MaxStores > pStoreChain->StoreCap) expandStoreChain();
				}
				else {
					if (!PMEnabled) {
						if ((pStoreChain->StoreCap - pStoreChain->StoreCount) < 10) expandStoreChain();
					}
				}
#ifdef INSTRUMENTED
				//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
				if (Stats.newKey()) {
					if (Stats.isPileUpInstrumentActive()) {
						Stats.writePileUpLeader();
						for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
							Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
						}
					}
				}
#endif
				return;
			}
		}

		//
		//  Setup to perform a Binary Chop search of the Store Chain beginning at the Mid-Point.
		//

		CurrentStore = int(pStoreChain->StoreCount) / 2;									//  Mid-point of the store chain
		Delta = int(pStoreChain->StoreCount) / 4;											//  Initial distance

		while (true) {

			//  Determine if the new key is without or within the range of the current store
			Below = true;
#ifdef INSTRUMENTED
			Stats.Compares++;
#endif
			if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRALo].pKey, KL) > 0) {
				Below = false;
#ifdef INSTRUMENTED
				Stats.Compares++;
#endif
				if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRAHi].pKey, KL) < 0) Without = false;
				else Without = true;
			}
			else Without = true;

			if (Without) {
				if (CurrentStore == 0) OtherWithout = false;
				else {
					//  Moving to the left (lower index) - Test against [CurrentStore - 1]
#ifdef INSTRUMENTED
					Stats.Compares++;
#endif
					if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore - 1]->pSRA[pStoreChain->Store[CurrentStore - 1]->SRALo].pKey, KL) > 0) {
#ifdef INSTRUMENTED
						Stats.Compares++;
#endif
						if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore - 1]->pSRA[pStoreChain->Store[CurrentStore - 1]->SRAHi].pKey, KL) < 0) OtherWithout = false;
						else OtherWithout = true;
					}
					else OtherWithout = true;
				}
				//  If Within range of the other then add the key to the CurrentStore
				if (!OtherWithout) {
					if (Below) pStoreChain->Store[CurrentStore]->addLowKey(NewSR);
					else pStoreChain->Store[CurrentStore]->addHighKey(NewSR);
#ifdef INSTRUMENTED
					if (Below) Stats.LoHits++;
					else Stats.HiHits++;
					//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
					if (Stats.newKey()) {
						if (Stats.isPileUpInstrumentActive()) {
							Stats.writePileUpLeader();
							for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
								Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
							}
						}
					}
#endif
					return;
				}

				//  Move further to the left
				CurrentStore = CurrentStore - Delta;
				if (Delta > 1) Delta = Delta / 2;
			}
			else {
				//  Moving to the right (higher index) - Test against [CurrentStore + 1]
				Below = true;
#ifdef INSTRUMENTED
				Stats.Compares++;
#endif
				if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore + 1]->pSRA[pStoreChain->Store[CurrentStore + 1]->SRALo].pKey, KL) > 0) {
					Below = false;
#ifdef INSTRUMENTED
					Stats.Compares++;
#endif
					if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore + 1]->pSRA[pStoreChain->Store[CurrentStore + 1]->SRAHi].pKey, KL) < 0) OtherWithout = false;
					else OtherWithout = true;
				}
				else OtherWithout = true;

				//  If Without range of the other store then add the key to the other store
				if (OtherWithout) {
					if (Below) pStoreChain->Store[CurrentStore + 1]->addLowKey(NewSR);
					else pStoreChain->Store[CurrentStore + 1]->addHighKey(NewSR);
#ifdef INSTRUMENTED
					if (Below) Stats.LoHits++;
					else Stats.HiHits++;
					//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
					if (Stats.newKey()) {
						if (Stats.isPileUpInstrumentActive()) {
							Stats.writePileUpLeader();
							for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
								Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
							}
						}
					}
#endif
					return;
				}

				//  Move further to the right
				CurrentStore = CurrentStore + Delta;
				if (Delta > 1) Delta = Delta / 2;
			}
		}
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
		int			CurrentStore = 0;														//  Start of the Store chain
		int			Delta = 0;																//  Delta distance for Binary Chop
		bool		Without = false;														//  Within/Without control
		bool		Below = false;															//  Above or below control
		bool		OtherWithout = false;													//  Other Within/Without control

		//  Increment the record number
		RecNo++;
#ifndef INSTRUMENTED
		Stats.newKey();
#endif

		//
		//  First check against the boundaries of the store chain
		//

		//  If the new key is equal or below the low key set a new low key
#ifdef INSTRUMENTED
		Stats.Compares++;
#endif
		if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRALo].pKey, KL) <= 0) {
			pStoreChain->Store[CurrentStore]->addLowExternalKey(NewSR);
#ifdef INSTRUMENTED
			Stats.LoHits++;
			//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
			if (Stats.newKey()) {
				if (Stats.isPileUpInstrumentActive()) {
					Stats.writePileUpLeader();
					for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
						Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
					}
				}
			}
#endif
			return;
		}

		//  If the new key is equal or above the high key then set a new high key
#ifdef INSTRUMENTED
		Stats.Compares++;
#endif
		if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRAHi].pKey, KL) >= 0) {
			pStoreChain->Store[CurrentStore]->addHighExternalKey(NewSR);
#ifdef INSTRUMENTED
			Stats.HiHits++;
			//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
			if (Stats.newKey()) {
				if (Stats.isPileUpInstrumentActive()) {
					Stats.writePileUpLeader();
					for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
						Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
					}
				}
			}
#endif
			return;
		}

		CurrentStore = int(pStoreChain->StoreCount) - 1;

		//  If the new key is within the key range of the last store in the chain then add a new store to accomodate the key
#ifdef INSTRUMENTED
		Stats.Compares++;
#endif
		if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRALo].pKey, KL) > 0) {
#ifdef INSTRUMENTED
			Stats.Compares++;
#endif
			if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRAHi].pKey, KL) < 0) {
				//  A new store must be added to the array to accomodate the key
#ifdef INSTRUMENTED
				Stats.NewStores++;
				Stats.Stores++;
#endif
				pStoreChain->Store[pStoreChain->StoreCount] = new SplitStore<T>(NewSR, KL, Stats);
				pStoreChain->StoreCount++;

				//  Test for trigger of a preemptive merge if the store count has exceeded the maximum
				if (PMEnabled && (pStoreChain->StoreCount > MaxStores)) {
					//  Perform the preemptive merge
#ifdef INSTRUMENTED
					Stats.PMs++;
#endif
					suppressTail();
					//  Recompute the Maximum number of stores
					MaxStores = computeMaxStores(MaxStores, RecNo, MaxSInc);

					//  Check that there is capacity in the StoreChain structure to accomodate the maximum number of stores
					if (MaxStores > pStoreChain->StoreCap) expandStoreChain();
				}
				else {
					if (!PMEnabled) {
						if ((pStoreChain->StoreCap - pStoreChain->StoreCount) < 10) expandStoreChain();
					}
				}
#ifdef INSTRUMENTED
				//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
				if (Stats.newKey()) {
					if (Stats.isPileUpInstrumentActive()) {
						Stats.writePileUpLeader();
						for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
							Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
						}
					}
				}
#endif
				return;
			}
		}

		//
		//  Setup to perform a Binary Chop search of the Store Chain beginning at the Mid-Point.
		//

		CurrentStore = int(pStoreChain->StoreCount) / 2;									//  Mid-point of the store chain
		Delta = int(pStoreChain->StoreCount) / 4;											//  Initial distance

		while (true) {

			//  Determine if the new key is without or within the range of the current store
			Below = true;
#ifdef INSTRUMENTED
			Stats.Compares++;
#endif
			if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRALo].pKey, KL) > 0) {
				Below = false;
#ifdef INSTRUMENTED
				Stats.Compares++;
#endif
				if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRAHi].pKey, KL) < 0) Without = false;
				else Without = true;
			}
			else Without = true;

			if (Without) {
				//  Moving to the left (lower index) - Test against [CurrentStore - 1]
#ifdef INSTRUMENTED
				Stats.Compares++;
#endif
				if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore - 1]->pSRA[pStoreChain->Store[CurrentStore - 1]->SRALo].pKey, KL) > 0) {
#ifdef INSTRUMENTED
					Stats.Compares++;
#endif
					if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore - 1]->pSRA[pStoreChain->Store[CurrentStore - 1]->SRAHi].pKey, KL) < 0) OtherWithout = false;
					else OtherWithout = true;
				}
				else OtherWithout = true;

				//  If Within range of the other then add the key to the CurrentStore
				if (!OtherWithout) {
					if (Below) pStoreChain->Store[CurrentStore]->addLowExternalKey(NewSR);
					else pStoreChain->Store[CurrentStore]->addHighExternalKey(NewSR);
#ifdef INSTRUMENTED
					if (Below) Stats.LoHits++;
					else Stats.HiHits++;
					//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
					if (Stats.newKey()) {
						if (Stats.isPileUpInstrumentActive()) {
							Stats.writePileUpLeader();
							for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
								Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
							}
						}
					}
#endif
					return;
				}

				//  Move further to the left
				CurrentStore = CurrentStore - Delta;
				Delta = Delta / 2;
			}
			else {
				//  Moving to the right (higher index) - Test against [CurrentStore + 1]
				Below = true;
#ifdef INSTRUMENTED
				Stats.Compares++;
#endif
				if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore + 1]->pSRA[pStoreChain->Store[CurrentStore + 1]->SRALo].pKey, KL) > 0) {
					Below = false;
#ifdef INSTRUMENTED
					Stats.Compares++;
#endif
					if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore + 1]->pSRA[pStoreChain->Store[CurrentStore + 1]->SRAHi].pKey, KL) < 0) OtherWithout = false;
					else OtherWithout = true;
				}
				else OtherWithout = true;

				//  If Without range of the other store then add the key to the other store
				if (OtherWithout) {
					if (Below) pStoreChain->Store[CurrentStore + 1]->addLowExternalKey(NewSR);
					else pStoreChain->Store[CurrentStore + 1]->addHighExternalKey(NewSR);
#ifdef INSTRUMENTED
					if (Below) Stats.LoHits++;
					else Stats.HiHits++;
					//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
					if (Stats.newKey()) {
						if (Stats.isPileUpInstrumentActive()) {
							Stats.writePileUpLeader();
							for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
								Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
							}
						}
					}
#endif
					return;
				}

				//  Move further to the right
				CurrentStore = CurrentStore + Delta;
				Delta = Delta / 2;
			}
		}
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
		int			CurrentStore = 0;														//  Start of the Store chain
		int			Delta = 0;																//  Delta distance for Binary Chop
		bool		Without = false;														//  Within/Without control
		bool		Below = false;															//  Above or below control
		bool		OtherWithout = false;													//  Other Within/Without control

		//  Increment the record number
		RecNo++;
#ifndef INSTRUMENTED
		Stats.newKey();
#endif

		//
		//  First check against the boundaries of the store chain
		//

		//  If the new key is below the low key set a new low key
#ifdef INSTRUMENTED
		Stats.Compares++;
#endif
		if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRALo].pKey, KL) < 0) {
			pStoreChain->Store[CurrentStore]->addLowKey(NewSR);
#ifdef INSTRUMENTED
			Stats.LoHits++;
			//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
			if (Stats.newKey()) {
				if (Stats.isPileUpInstrumentActive()) {
					Stats.writePileUpLeader();
					for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
						Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
					}
				}
			}
#endif
			return;
		}

		//  If the new key is above the high key then set a new high key
#ifdef INSTRUMENTED
		Stats.Compares++;
#endif
		if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRAHi].pKey, KL) > 0) {
			pStoreChain->Store[CurrentStore]->addHighKey(NewSR);
#ifdef INSTRUMENTED
			Stats.HiHits++;
			//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
			if (Stats.newKey()) {
				if (Stats.isPileUpInstrumentActive()) {
					Stats.writePileUpLeader();
					for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
						Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
					}
				}
			}
#endif
			return;
		}

		CurrentStore = int(pStoreChain->StoreCount) - 1;

		//  If the new key is within the key range of the last store in the chain then add a new store to accomodate the key
#ifdef INSTRUMENTED
		Stats.Compares++;
#endif
		if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRALo].pKey, KL) >= 0) {
#ifdef INSTRUMENTED
			Stats.Compares++;
#endif
			if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRAHi].pKey, KL) <= 0) {
				//  A new store must be added to the array to accomodate the key
#ifdef INSTRUMENTED
				Stats.NewStores++;
				Stats.Stores++;
#endif
				pStoreChain->Store[pStoreChain->StoreCount] = new SplitStore<T>(NewSR, KL, Stats);
				pStoreChain->StoreCount++;

				//  Test for trigger of a preemptive merge if the store count has exceeded the maximum
				if (PMEnabled && (pStoreChain->StoreCount > MaxStores)) {
					//  Perform the preemptive merge
#ifdef INSTRUMENTED
					Stats.PMs++;
#endif
					suppressStableTail(Ascending);
					//  Recompute the Maximum number of stores
					MaxStores = computeMaxStores(MaxStores, RecNo, MaxSInc);

					//  Check that there is capacity in the StoreChain structure to accomodate the maximum number of stores
					if (MaxStores > pStoreChain->StoreCap) expandStoreChain();
				}
				else {
					if (!PMEnabled) {
						if ((pStoreChain->StoreCap - pStoreChain->StoreCount) < 10) expandStoreChain();
					}
				}
#ifdef INSTRUMENTED
				//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
				if (Stats.newKey()) {
					if (Stats.isPileUpInstrumentActive()) {
						Stats.writePileUpLeader();
						for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
							Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
						}
					}
				}
#endif
				return;
			}
		}

		//
		//  Setup to perform a Binary Chop search of the Store Chain beginning at the Mid-Point.
		//

		CurrentStore = int(pStoreChain->StoreCount) / 2;									//  Mid-point of the store chain
		Delta = int(pStoreChain->StoreCount) / 4;											//  Initial distance

		while (true) {

			//  Determine if the new key is without or within the range of the current store
			Below = true;
#ifdef INSTRUMENTED
			Stats.Compares++;
#endif
			if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRALo].pKey, KL) >= 0) {
				Below = false;
#ifdef INSTRUMENTED
				Stats.Compares++;
#endif
				if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRAHi].pKey, KL) <= 0) Without = false;
				else Without = true;
			}
			else Without = true;

			if (Without) {
				//  Moving to the left (lower index) - Test against [CurrentStore - 1]
#ifdef INSTRUMENTED
				Stats.Compares++;
#endif
				if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore - 1]->pSRA[pStoreChain->Store[CurrentStore - 1]->SRALo].pKey, KL) >= 0) {
#ifdef INSTRUMENTED
					Stats.Compares++;
#endif
					if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore - 1]->pSRA[pStoreChain->Store[CurrentStore - 1]->SRAHi].pKey, KL) <= 0) OtherWithout = false;
					else OtherWithout = true;
				}
				else OtherWithout = true;

				//  If Within range of the other then add the key to the CurrentStore
				if (!OtherWithout) {
					if (Below) pStoreChain->Store[CurrentStore]->addLowKey(NewSR);
					else pStoreChain->Store[CurrentStore]->addHighKey(NewSR);
#ifdef INSTRUMENTED
					if (Below) Stats.LoHits++;
					else Stats.HiHits++;
					//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
					if (Stats.newKey()) {
						if (Stats.isPileUpInstrumentActive()) {
							Stats.writePileUpLeader();
							for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
								Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
							}
						}
					}
#endif
					return;
				}

				//  Move further to the left
				CurrentStore = CurrentStore - Delta;
				Delta = Delta / 2;
			}
			else {
				//  Moving to the right (higher index) - Test against [CurrentStore + 1]
				Below = true;
#ifdef INSTRUMENTED
				Stats.Compares++;
#endif
				if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore + 1]->pSRA[pStoreChain->Store[CurrentStore + 1]->SRALo].pKey, KL) >= 0) {
					Below = false;
#ifdef INSTRUMENTED
					Stats.Compares++;
#endif
					if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore + 1]->pSRA[pStoreChain->Store[CurrentStore + 1]->SRAHi].pKey, KL) <= 0) OtherWithout = false;
					else OtherWithout = true;
				}
				else OtherWithout = true;

				//  If Without range of the other store then add the key to the other store
				if (OtherWithout) {
					if (Below) pStoreChain->Store[CurrentStore + 1]->addLowKey(NewSR);
					else pStoreChain->Store[CurrentStore + 1]->addHighKey(NewSR);
#ifdef INSTRUMENTED
					if (Below) Stats.LoHits++;
					else Stats.HiHits++;
					//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
					if (Stats.newKey()) {
						if (Stats.isPileUpInstrumentActive()) {
							Stats.writePileUpLeader();
							for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
								Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
							}
						}
					}
#endif
					return;
				}

				//  Move further to the right
				CurrentStore = CurrentStore + Delta;
				Delta = Delta / 2;
			}
		}
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
		int			CurrentStore = 0;														//  Start of the Store chain
		int			Delta = 0;																//  Delta distance for Binary Chop
		bool		Without = false;														//  Within/Without control
		bool		Below = false;															//  Above or below control
		bool		OtherWithout = false;													//  Other Within/Without control

		//  Increment the record number
		RecNo++;
#ifndef INSTRUMENTED
		Stats.newKey();
#endif

		//
		//  First check against the boundaries of the store chain
		//

		//  If the new key is below the low key set a new low key
#ifdef INSTRUMENTED
		Stats.Compares++;
#endif
		if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRALo].pKey, KL) < 0) {
			pStoreChain->Store[CurrentStore]->addLowExternalKey(NewSR);
#ifdef INSTRUMENTED
			Stats.LoHits++;
			//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
			if (Stats.newKey()) {
				if (Stats.isPileUpInstrumentActive()) {
					Stats.writePileUpLeader();
					for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
						Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
					}
				}
			}
#endif
			return;
		}

		//  If the new key is above the high key then set a new high key
#ifdef INSTRUMENTED
		Stats.Compares++;
#endif
		if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRAHi].pKey, KL) > 0) {
			pStoreChain->Store[CurrentStore]->addHighExternalKey(NewSR);
#ifdef INSTRUMENTED
			Stats.HiHits++;
			//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
			if (Stats.newKey()) {
				if (Stats.isPileUpInstrumentActive()) {
					Stats.writePileUpLeader();
					for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
						Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
					}
				}
			}
#endif
			return;
		}

		CurrentStore = int(pStoreChain->StoreCount) - 1;

		//  If the new key is within the key range of the last store in the chain then add a new store to accomodate the key
#ifdef INSTRUMENTED
		Stats.Compares++;
#endif
		if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRALo].pKey, KL) >= 0) {
#ifdef INSTRUMENTED
			Stats.Compares++;
#endif
			if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRAHi].pKey, KL) <= 0) {
				//  A new store must be added to the array to accomodate the key
#ifdef INSTRUMENTED
				Stats.NewStores++;
				Stats.Stores++;
#endif
				pStoreChain->Store[pStoreChain->StoreCount] = new SplitStore<T>(NewSR, KL, Stats);
				pStoreChain->StoreCount++;

				//  Test for trigger of a preemptive merge if the store count has exceeded the maximum
				if (PMEnabled && (pStoreChain->StoreCount > MaxStores)) {
					//  Perform the preemptive merge
#ifdef INSTRUMENTED
					Stats.PMs++;
#endif
					suppressStableTail(Ascending);
					//  Recompute the Maximum number of stores
					MaxStores = computeMaxStores(MaxStores, RecNo, MaxSInc);

					//  Check that there is capacity in the StoreChain structure to accomodate the maximum number of stores
					if (MaxStores > pStoreChain->StoreCap) expandStoreChain();
				}
				else {
					if (!PMEnabled) {
						if ((pStoreChain->StoreCap - pStoreChain->StoreCount) < 10) expandStoreChain();
					}
				}
#ifdef INSTRUMENTED
				//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
				if (Stats.newKey()) {
					if (Stats.isPileUpInstrumentActive()) {
						Stats.writePileUpLeader();
						for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
							Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
						}
					}
				}
#endif
				return;
			}
		}

		//
		//  Setup to perform a Binary Chop search of the Store Chain beginning at the Mid-Point.
		//

		CurrentStore = int(pStoreChain->StoreCount) / 2;											//  Mid-point of the store chain
		Delta = int(pStoreChain->StoreCount) / 4;													//  Next distance

		while (true) {

			//  Determine if the new key is without or within the range of the current store
			Below = true;
#ifdef INSTRUMENTED
			Stats.Compares++;
#endif
			if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRALo].pKey, KL) >= 0) {
				Below = false;
#ifdef INSTRUMENTED
				Stats.Compares++;
#endif
				if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore]->pSRA[pStoreChain->Store[CurrentStore]->SRAHi].pKey, KL) <= 0) Without = false;
				else Without = true;
			}
			else Without = true;

			if (Without) {
				//  Moving to the left (lower index) - Test against [CurrentStore - 1]
#ifdef INSTRUMENTED
				Stats.Compares++;
#endif
				if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore - 1]->pSRA[pStoreChain->Store[CurrentStore - 1]->SRALo].pKey, KL) >= 0) {
#ifdef INSTRUMENTED
					Stats.Compares++;
#endif
					if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore - 1]->pSRA[pStoreChain->Store[CurrentStore - 1]->SRAHi].pKey, KL) <= 0) OtherWithout = false;
					else OtherWithout = true;
				}
				else OtherWithout = true;

				//  If Within range of the other then add the key to the CurrentStore
				if (!OtherWithout) {
					if (Below) pStoreChain->Store[CurrentStore]->addLowExternalKey(NewSR);
					else pStoreChain->Store[CurrentStore]->addHighExternalKey(NewSR);
#ifdef INSTRUMENTED
					if (Below) Stats.LoHits++;
					else Stats.HiHits++;
					//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
					if (Stats.newKey()) {
						if (Stats.isPileUpInstrumentActive()) {
							Stats.writePileUpLeader();
							for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
								Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
							}
						}
					}
#endif
					return;
				}

				//  Move further to the left
				CurrentStore = CurrentStore - Delta;
				Delta = Delta / 2;
			}
			else {
				//  Moving to the right (higher index) - Test against [CurrentStore + 1]
				Below = true;
#ifdef INSTRUMENTED
				Stats.Compares++;
#endif
				if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore + 1]->pSRA[pStoreChain->Store[CurrentStore + 1]->SRALo].pKey, KL) >= 0) {
					Below = false;
#ifdef INSTRUMENTED
					Stats.Compares++;
#endif
					if (memcmp(NewSR.pKey, pStoreChain->Store[CurrentStore + 1]->pSRA[pStoreChain->Store[CurrentStore + 1]->SRAHi].pKey, KL) <= 0) OtherWithout = false;
					else OtherWithout = true;
				}
				else OtherWithout = true;

				//  If Without range of the other store then add the key to the other store
				if (OtherWithout) {
					if (Below) pStoreChain->Store[CurrentStore + 1]->addLowExternalKey(NewSR);
					else pStoreChain->Store[CurrentStore + 1]->addHighExternalKey(NewSR);
#ifdef INSTRUMENTED
					if (Below) Stats.LoHits++;
					else Stats.HiHits++;
					//  Increment the new key counter - if this triggered a stats reporting interval then do the pile-up reporting
					if (Stats.newKey()) {
						if (Stats.isPileUpInstrumentActive()) {
							Stats.writePileUpLeader();
							for (CurrentStore = 0; CurrentStore < pStoreChain->StoreCount; CurrentStore++) {
								Stats.writePileUpStore(int(pStoreChain->Store[CurrentStore]->SRANum), CurrentStore == int(pStoreChain->StoreCount) - 1);
							}
						}
					}
#endif
					return;
				}

				//  Move further to the right
				CurrentStore = CurrentStore + Delta;
				Delta = Delta / 2;
			}
		}
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
		size_t			NumStores = pStoreChain->StoreCount;

		//  Perform pre-emptive merges until there is only a single store remaining
		Stats.startFM();
		while (pStoreChain->StoreCount > 1) {
			doAlternateMerge();
		}
		Stats.finishFM(NumStores);

		//  Return to caller
		return pStoreChain->Store[0]->SRANum;
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
		size_t			NumStores = pStoreChain->StoreCount;

		//  Perform pre-emptive merges until there is only a single store remaining
		Stats.startFM();
		while (pStoreChain->StoreCount > 1) {
			doStableAlternateMerge(Ascending);
		}
		Stats.finishFM(NumStores);

		//  Return to caller
		return pStoreChain->Store[0]->SRANum;
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

		if (RecNo != pStoreChain->Store[0]->SRANum) return false;
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
		return Output(&pStoreChain->Store[0]->pSRA[pStoreChain->Store[0]->SRALo], pStoreChain->Store[0]->SRALo);
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
		return Output(&pStoreChain->Store[0]->pSRA[pStoreChain->Store[0]->SRAHi], pStoreChain->Store[0]->SRAHi);
	}

private:

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Private Members			                                                                                    *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Configuration
	size_t			KL;																		//  Key Length
	IStats& Stats;																	//  Reference to the instrumentation object

	//  Preemptive Merge Controls
	size_t			RecNo;																	//  Record counter
	size_t			MaxStores;																//  Maximum number of splitter stores
	size_t			MaxSInc;																//  Percentage of Max Stores

	//  SplitStore Chain
	StoreChain* pStoreChain;															//  Pointer to the store chain structure

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Private Functions                                                                                             *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  suppressTail
	//
	//  This function will perform a preemptive merge on the current splitter store chain.
	//  The merge pattern sweeps up stores from the tail merging 10% of the total store chain.
	//
	//  PARAMETERS:
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	suppressTail() {
		size_t				Stores = pStoreChain->StoreCount;								//  Current number of stores
		size_t				Target = (Stores * 9) / 10;										//  90% of existing stores

		//  Record start time
		Stats.startPM();

		//  Eliminate the ultimate store until the target number of stores is reached
		while (Stores > Target) {
#ifdef INSTRUMENTED
			Stats.startStoreMerge(int(pStoreChain->Store[Stores - 2]->SRANum), int(pStoreChain->Store[Stores - 1]->SRANum));
#endif
			pStoreChain->Store[Stores - 2]->mergeNextStore(pStoreChain->Store[Stores - 1]);
			pStoreChain->Store[Stores - 1] = nullptr;
			Stores--;
		}

		//  Accumulate the time spent
		Stats.finishPM(pStoreChain->StoreCount - Stores);

		//  Update the store count
		pStoreChain->StoreCount = Stores;

		//  Return to caller
		return;
	}

	//  suppressStableTail
	//
	//  This function will perform a preemptive stable merge on the current splitter store chain.
	//  The merge pattern sweeps up stores from the tail merging 10% of the total store chain.
	//
	//  PARAMETERS:
	//
	//		bool		-		true if the sequence is ascending false if descending
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	suppressStableTail(bool Ascending) {
		size_t				Stores = pStoreChain->StoreCount;								//  Current number of stores
		size_t				Target = (Stores * 9) / 10;										//  90% of existing stores

		//  Record start time
		Stats.startPM();

		//  Eliminate the ultimate store until the target number of stores is reached
		while (Stores > Target) {
#ifdef INSTRUMENTED
			Stats.startStoreMerge(int(pStoreChain->Store[Stores - 2]->SRANum), int(pStoreChain->Store[Stores - 1]->SRANum));
#endif
			if (Ascending) pStoreChain->Store[Stores - 1]->mergeNextStoreAscending(pStoreChain->Store[Stores - 1]);
			else pStoreChain->Store[Stores - 2]->mergeNextStoreDescending(pStoreChain->Store[Stores - 1]);
			pStoreChain->Store[Stores - 1] = nullptr;
			Stores--;
		}

		//  Accumulate the time spent
		Stats.finishPM(pStoreChain->StoreCount - Stores);

		//  Update the store count
		pStoreChain->StoreCount = Stores;

		//  Return to caller
		return;
	}

	//  doAlternateMerge
	//
	//  This function will perform a final merge on the current splitter store chain.
	//  The merge pattern is alternate stores are merged into their predecessors on the chain.
	//
	//  PARAMETERS:
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	doAlternateMerge() {
		int					sIndex = 0;														//  Store index
		int					tIndex = 0;														//  Target index
		size_t				Stores = pStoreChain->StoreCount;								//  Current number of stores

		//  Process all stores
		while (size_t(sIndex) < pStoreChain->StoreCount) {

			// Merge the following splitter into the current one
			if (size_t(sIndex + 1) < pStoreChain->StoreCount) {
#ifdef INSTRUMENTED
				Stats.startStoreMerge(int(pStoreChain->Store[sIndex]->SRANum), int(pStoreChain->Store[sIndex + 1]->SRANum));
#endif
				pStoreChain->Store[sIndex]->mergeNextStore(pStoreChain->Store[sIndex + 1]);
				pStoreChain->Store[sIndex + 1] = nullptr;
				Stores--;
			}

			if (tIndex > 0) {
				pStoreChain->Store[tIndex] = pStoreChain->Store[sIndex];
				pStoreChain->Store[sIndex] = nullptr;
			}
			//  Move on to the next pair
			sIndex += 2;
			tIndex++;
		}

		pStoreChain->StoreCount = Stores;

		//  Return to caller
		return;
	}

	//  doStableAlternateMerge
	//
	//  This function will perform a final merge on the current splitter store chain.
	//  The merge pattern is alternate stores are merged into theire predecessors on the chain.
	//  This variant handles stable keys.
	//
	//  PARAMETERS:
	//
	//		bool		-		true if the sequence is ascending false if descending
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	doStableAlternateMerge(bool Ascending) {
		int					sIndex = 0;														//  Store index
		int					tIndex = 0;														//  Target index
		size_t				Stores = pStoreChain->StoreCount;								//  Current number of stores

		//  Process all stores
		while (size_t(sIndex) < pStoreChain->StoreCount) {

#ifdef INSTRUMENTED
			Stats.startStoreMerge(int(pStoreChain->Store[sIndex]->SRANum), int(pStoreChain->Store[sIndex + 1]->SRANum));
#endif
			// Merge the following splitter into the current one
			if (size_t(sIndex + 1) < pStoreChain->StoreCount) {
				if (Ascending) pStoreChain->Store[sIndex]->mergeNextStoreAscending(pStoreChain->Store[sIndex + 1]);
				else pStoreChain->Store[sIndex]->mergeNextStoreDescending(pStoreChain->Store[sIndex + 1]);
				pStoreChain->Store[sIndex + 1] = nullptr;
				Stores--;
			}

			if (tIndex > 0) {
				pStoreChain->Store[tIndex] = pStoreChain->Store[sIndex];
				pStoreChain->Store[sIndex] = nullptr;
			}
			//  Move on to the next pair
			sIndex += 2;
			tIndex++;
		}

		pStoreChain->StoreCount = Stores;

		//  Return to caller
		return;
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

	//  expandStoreChain
	//
	//  This function will add an additional 1024 stores to the store chain.
	//
	//  PARAMETERS:
	// 
	//  RETURNS:
	//
	//  NOTES:
	// 

	void	expandStoreChain() {
		StoreChain* pNewStoreChain = nullptr;

		pStoreChain->StoreCap += 1024;
		pNewStoreChain = (StoreChain*)realloc(pStoreChain, sizeof(StoreChain) + ((pStoreChain->StoreCap - 4096) * sizeof(void*)));
		if (pNewStoreChain != nullptr) pStoreChain = pNewStoreChain;
		else {
			std::cerr << "FATAL: Unable to reallocate the store chain structure for: " << pStoreChain->StoreCap << " stores." << std::endl;
			std::abort();
		}
		return;
	}
};
