#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       Cache.h																							*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.0.0	(Build: 01)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2018 Hadleigh Marshall Netherlands b.v.														*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the Cache class. The class is the base for all Cache types			*
//* it provides the cache management functions for cacheing arbitrary memory allocations (objects).					*
//*																													*
//*	USAGE:																											*
//*																													*
//*																													*
//*	NOTES:																											*
//*																													*
//*	1.		This is the single threaded implementation. It is NOT thread safe.										*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 -		02/12/2017	-	Initial Release																		*
//*																													*
//*******************************************************************************************************************/

//
//  Include core xymorg headers
//

#include	"LPBHdrs.h"																		//  Language and Platform base headers
#include	"types.h"																		//  xymorg type definitions
#include	"consts.h"																		//  xymorg constant definitions
#include	"StringPool.h"																	//  String Pool
#include	"Logging.h"																		//  Logging message class

//
//  All components are defined within the xymorg namespace
//
namespace xymorg {

	//
	//  Cache Class Definition
	//

	class Cache {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants		                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		static const SWITCHES	EVICTION_STRATEGY_LRU = 0x00000001;									//  Least Recently Used cache eviction strategy
		static const SWITCHES	EVICTION_STRATEGY_LFU = 0x00000002;									//  Least Frequently Used cache eviction strategy
		static const SWITCHES	OBSERVE_EXPIRY = 0x00000004;										//  Cache observes content expiry (TTL)
		static const SWITCHES	OBSERVE_BUDGET = 0x00000008;										//  Cache observes the budget space allocation
		static const SWITCHES	OBSERVE_KEY_CASE = 0x00000010;										//  Keys are case sensitive
		static const SWITCHES	CACHE_NOT_EXIST = 0x00000020;										//  Cache non-existent keys (Negative cacheing)
		static const SWITCHES	WRITE_DEFERRED = 0x00000040;										//  Writes through the cache are deferred

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Structures																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Constants		                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Statistics
		typedef struct Stats {
			size_t		Hits;																		//  Number of cache hits
			size_t		Misses;																		//  Number of cache misses
			size_t		Reads;																		//  Number of cache misses
			size_t		Peeks;																		//  Number of cache peeks
			size_t		Writes;																		//  Number of cache writes
			size_t		DirtyWrites;																//  Number of Write-Throughs
			size_t		Purges;																		//  Number of records purged from the cache
			size_t		NotFound;																	//  Not found (in cache or store)
			size_t		Inspects;																	//  Number of cache line inspections
			size_t		Evictions;																	//  Number of cache evictions
			size_t		Expires;																	//  Number of cache entries that expired
			size_t		MaxEnts;																	//  Maximum count of cache entries
			size_t		MaxSize;																	//  Maximum size of the cache (Kb)
		} Stats;

	private:

		static const int NumLines = 256;															//  Default number of cache lines

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Structures	                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//   Cache Line
		typedef struct CacheLine {
			TIMER		Expiry;																		//  Time this entry expires (added time + TTL)
			TIMER		LastRef;																	//  Time of the last reference
			size_t		RefCount;																	//  Reference Count
			STRREF		RKey;																		//  Key of the record/object
			size_t		RLen;																		//  Length of the record/object
			BYTE*		RPtr;																		//  Pointer to the cached record/object
			bool		DirtyBit;																	//  Cache line represents an updated record/object
		} CacheLine;

	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors			                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Constructor 
		//
		//  Constructs a new Cache (base) with the given attributes
		//
		//  PARAMETERS:
		//
		//		SWITCHES			-		Cache configuration options
		//		size_t				-		Budget size (Kb)
		//
		//  RETURNS:
		//
		//  NOTES:
		//
		//	1.	Extending classes MUST invoke this constructor
		//

		Cache(SWITCHES NewCfg, size_t NewBudget) : COpts(NewCfg)
			, pCL(NULL)
			, Budget(NewBudget)
			, NCL(0)
			, UCL(0)
			, Size(0)
			, Keys()
			, StatRec()
			, Coherent(false) {

			//  Allocate the cache-line pool for the initial number of entries
			pCL = (CacheLine*)malloc(NumLines * sizeof(CacheLine));
			if (pCL == NULL) return;
			memset(pCL, 0, NumLines * sizeof(CacheLine));
			NCL = NumLines;

			//  Clear the statistics
			memset(&StatRec, 0, sizeof(Stats));

			//  Mark the cache as coherent
			Coherent = true;

			//  Return to caller
			return;
		}

		//  Caches are not copyable nor moveable
		Cache(const Cache& Src) = delete;
		Cache(Cache&& Src) = delete;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor			                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Destructor
		//
		//  Destroys the Cache releasing any cached records/objects
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		virtual ~Cache() {

			//  Dismiss the cache content
			dismiss();

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

		//  getCachedRecord
		//
		//  This is the primary function for the cache it returns the required record from the cache or initiates a read
		//  from the store and caches and returns that entry.
		//  This variant of the call is NOT THREAD SAFE.
		//
		//  PARAMETERS:
		//
		//		char*		-		Pointer to the key for the desired record (NULL terminated string)
		//		size_t&		-		Reference to the record variable to hold the returned record length	
		//		size_t&		-		Reference to the record variable to hold the Time-To-Live (TTL) of the returned record
		//
		//  RETURNS:
		//
		//		BYTE*		-		Pointer to the record in the cache
		//
		//  NOTES:
		// 
		//		1.		The pointer to the cached record that is returned is ONLY valid until the next call to get or put a cached record.
		//				It is the resposiblity of the caller to guard against use-after-free bugs using the returned pointer.
		//

		BYTE* getCachedRecord(const char* Key, size_t& RecLen, size_t& TTL) {
			CacheLine*			pCEnt = nullptr;															//  Cache line for the entry
			BYTE*				pNewRec = nullptr;															//  New record to be added to the cache
			size_t				InsertAt = 0;																//  Cache line index for new insertions
			TIMER				NowTime = CLOCK::now();														//  Current time
			SECONDS				TTLSecs = {};																//  TTL in seconds

			//  Safety
			RecLen = 0;
			TTL = 0;
			if (Key == nullptr) return nullptr;
			if (Key[0] == '\0') return nullptr;
			if (!Coherent) return nullptr;

			//  Update Stats
			StatRec.Reads++;

			//  Expire any records that are due
			if (COpts & OBSERVE_EXPIRY) expireRecords();

			//  Search the cache to determine if the passed key is in the cache
			pCEnt = findCacheLine(Key);
			if (pCEnt != nullptr) {
				//  Entry is already in the cache - perform cache management bookeeping and return the record
				pCEnt->RefCount++;
				pCEnt->LastRef = CLOCK::now();

				//  Update the statistics
				StatRec.Hits++;

				//  Promote the entry in the cache
				pCEnt = promote(pCEnt);

				//  Compute the remaining TTL
				NowTime = CLOCK::now();
				TTLSecs = pCEnt->Expiry - NowTime;
				TTL = size_t(TTLSecs.count());
				RecLen = pCEnt->RLen;
				pNewRec = pCEnt->RPtr;

				//  Return the cached record
				return pNewRec;
			}

			//  Record not currently in the cache - attempt retrieve  it from the store
			StatRec.Misses++;

			//  Attempt to read the desired key from the store
			pNewRec = getStoredRecord(Key, RecLen, TTL);

			if (pNewRec == nullptr) {
				StatRec.NotFound++;

				//  If NOT-EXIST records are NOT being cached then return to the caller with NULL
				if (!(COpts & CACHE_NOT_EXIST)) {
					return nullptr;
				}
			}

			//  Safety - if the size of the new entry exceeds the budget then increase the budget
			if (COpts & OBSERVE_BUDGET) {
				if (((RecLen + 511) / 1024) > Budget) Budget += ((RecLen + 511) / 1024);
			}

			//  Evict records from the cache until there is sufficient space
			if (COpts & OBSERVE_BUDGET) evictRecords(RecLen);

			//  Make sure that the cache line pool has capacity
			if (NCL == UCL) {
				CacheLine* pNewPool = (CacheLine*)realloc(pCL, (NCL + NumLines) * sizeof(CacheLine));
				if (pNewPool == nullptr) {
					Coherent = false;
					destroyCachedRecord(pNewRec, RecLen);
					return nullptr;
				}
				pCL = pNewPool;
				NCL += NumLines;
			}

			//
			//  Determine the point at which the insertion will take place and clear that slot
			//
			if (COpts & EVICTION_STRATEGY_LRU) {
				//  LRU - make room at the head of the cache line array
				if (UCL > 0) memmove(&pCL[1], &pCL[0], UCL * sizeof(CacheLine));
				memset(&pCL[0], 0, sizeof(CacheLine));
				InsertAt = 0;
			}
			else {
				//  LFU - new cache lines are appended to the existing
				memset(&pCL[UCL], 0, sizeof(CacheLine));
				InsertAt = UCL;
			}

			//  Insert the new record into the cache
			pCL[InsertAt].DirtyBit = false;
			if (TTL == 0) TTL = 24 * 60 * 60;													//  Default 24 hrs
			pCL[InsertAt].Expiry = CLOCK::now() + MILLISECONDS(TTL * 1000);
			pCL[InsertAt].LastRef = CLOCK::now();
			pCL[InsertAt].RefCount = 1;
			pCL[InsertAt].RKey = Keys.addString(Key);
			pCL[InsertAt].RLen = RecLen;
			pCL[InsertAt].RPtr = pNewRec;

			//  LFU - promote the entry
			if (COpts & EVICTION_STRATEGY_LFU) promote(&pCL[InsertAt]);

			//  Bookeeping
			UCL++;
			Size += RecLen;
			if (UCL > StatRec.MaxEnts) StatRec.MaxEnts = UCL;
			if (((Size + 511) / 1024) > StatRec.MaxSize) StatRec.MaxSize = ((Size + 511) / 1024);

			//  Return the entry
			return pNewRec;
		}

		//  peekCachedRecord
		//
		//  This is the primary function for the cache it returns the required record from the cache or initiates a read
		//  from the store and caches and returns that entry.
		//  Peek does not update the cache by counting hits or recording last accessed times.
		//  This variant of the call is NOT THREAD SAFE.
		//
		//  PARAMETERS:
		//
		//		char*		-		Pointer to the key for the desired record (NULL terminated string)
		//		size_t&		-		Reference to the record variable to hold the returned record length	
		//		size_t&		-		Reference to the record variable to hold the Time-To-Live (TTL) of the returned record
		//
		//  RETURNS:
		//
		//		BYTE*		-		Pointer to the record in the cache
		//
		//  NOTES:
		//

		BYTE* peekCachedRecord(const char* Key, size_t& RecLen, size_t& TTL) {
			CacheLine*			pCEnt = nullptr;															//  Cache line for the entry
			BYTE*				pNewRec = nullptr;															//  New record to be added to the cache
			size_t				InsertAt = 0;																//  Cache line index for new insertions
			TIMER				NowTime = CLOCK::now();														//  Current time
			SECONDS				TTLSecs = {};																//  TTL in seconds

			//  Safety
			RecLen = 0;
			TTL = 0;
			if (Key == nullptr) return nullptr;
			if (Key[0] == '\0') return nullptr;
			if (!Coherent) return nullptr;

			//  Count number of peeks
			StatRec.Peeks++;

			//  Expire any records that are due
			if (COpts & OBSERVE_EXPIRY) expireRecords();

			//  Search the cache to determine if the passed key is in the cache
			pCEnt = findCacheLine(Key);
			if (pCEnt != nullptr) {

				//  Update the statistics
				StatRec.Hits++;

				//  Compute the remaining TTL
				NowTime = CLOCK::now();
				TTLSecs = pCEnt->Expiry - NowTime;
				TTL = size_t(TTLSecs.count());
				RecLen = pCEnt->RLen;
				pNewRec = pCEnt->RPtr;

				//  Return the cached record
				return pNewRec;
			}

			//  Update the statistics
			StatRec.Misses++;

			//  No cache entry was available to return
			return nullptr;
		}

		//  writeRecord
		//
		//  This function will write a record through the cache. 
		//  The passed buffer will be owned by the cache on completion.
		//  This variant of the call is NOT THREAD SAFE.
		//
		//  PARAMETERS:
		//
		//		char*		-		Pointer to the key for the desired record (NULL terminated string)
		//		BYTE*		-		Pointer to the record to be written
		//		size_t		-		Size of the record to be written
		//		size_t		-		TTL (Time-To-Live) for the record
		//
		//  RETURNS:
		//
		//		bool		-		True if the record was written into the cache, otherwise false
		//
		//  NOTES:
		//

		bool		writeRecord(const char* Key, BYTE* Rec, size_t RecLen, size_t TTL) {
			CacheLine*			pCEnt = nullptr;															//  Cache line for the entry
			size_t				InsertAt = 0;																//  Cache line index for new insertions

			//  Safety
			if (Key == nullptr) return false;
			if (Key[0] == '\0') return false;
			if (Rec == nullptr) return false;
			if (RecLen == 0) return false;
			if (!Coherent) return false;

			//  Count number of writes
			StatRec.Writes++;

			//  Expire any records that are due
			if (COpts & OBSERVE_EXPIRY) expireRecords();

			pCEnt = findCacheLine(Key);
			if (pCEnt != nullptr) {
				//  Entry is already in the cache - replace the record
				pCEnt->RefCount++;
				pCEnt->LastRef = CLOCK::now();

				//  Update the statistics
				StatRec.Hits++;

				//  Promote the entry in the cache
				pCEnt = promote(pCEnt);

				//  Destroy the existing cached record
				destroyCachedRecord(pCEnt->RPtr, pCEnt->RLen);

				//  Update the cache entry
				pCEnt->RPtr = Rec;
				pCEnt->RLen = RecLen;
				pCEnt->Expiry = CLOCK::now() + MILLISECONDS(TTL * 1000);

				//  If the cache policy is deferred write through then set the dirty bit, otherwise update the backing store.
				if (COpts & WRITE_DEFERRED) pCEnt->DirtyBit = true;
				else {
					if (!putCachedRecord(Key, Rec, RecLen)) return false;
					pCEnt->DirtyBit = false;
				}

				//  Return the showing success
				return true;
			}

			//
			//  Record was not found in the cache - create a new cache line to hold the record
			//
			StatRec.Misses++;

			//  Safety - if the size of the new entry exceeds the budget then increase the budget
			if (COpts & OBSERVE_BUDGET) {
				if (((RecLen + 511) / 1024) > Budget) Budget += ((RecLen + 511) / 1024);
			}

			//  Evict records from the cache until there is sufficient space
			if (COpts & OBSERVE_BUDGET) evictRecords(RecLen);

			//  Make sure that the cache line pool has capacity
			if (NCL == UCL) {
				CacheLine* pNewPool = (CacheLine*)realloc(pCL, (NCL + NumLines) * sizeof(CacheLine));
				if (pNewPool == nullptr) {
					Coherent = false;
					destroyCachedRecord(Rec, RecLen);
					return false;
				}
				pCL = pNewPool;
				NCL += NumLines;
			}

			//
			//  Determine the point at which the insertion will take place and clear that slot
			//
			if (COpts & EVICTION_STRATEGY_LRU) {
				//  LRU - make room at the head of the cache line array
				if (UCL > 0) memmove(&pCL[1], &pCL[0], UCL * sizeof(CacheLine));
				memset(&pCL[0], 0, sizeof(CacheLine));
				InsertAt = 0;
			}
			else {
				//  LFU - new cache lines are appended to the existing
				memset(&pCL[UCL], 0, sizeof(CacheLine));
				InsertAt = UCL;
			}

			//  Insert the new record into the cache
			pCL[InsertAt].DirtyBit = true;
			if (TTL == 0) TTL = 24 * 60 * 60;													//  Default 24 hrs
			pCL[InsertAt].Expiry = CLOCK::now() + MILLISECONDS(TTL * 1000);
			pCL[InsertAt].LastRef = CLOCK::now();
			pCL[InsertAt].RefCount = 1;
			pCL[InsertAt].RKey = Keys.addString(Key);
			pCL[InsertAt].RLen = RecLen;
			pCL[InsertAt].RPtr = Rec;

			//  If the cache policy is not deferred write then write through the new record
			if ((COpts & WRITE_DEFERRED) == 0) {
				if (!putCachedRecord(Key, Rec, RecLen)) return false;
				pCL[InsertAt].DirtyBit = false;
			}

			//  LFU - promote the entry
			if (COpts & EVICTION_STRATEGY_LFU) promote(&pCL[InsertAt]);

			//  Bookeeping
			UCL++;
			Size += RecLen;
			if (UCL > StatRec.MaxEnts) StatRec.MaxEnts = UCL;
			if (((Size + 511) / 1024) > StatRec.MaxSize) StatRec.MaxSize = ((Size + 511) / 1024);

			//  Return showing success
			return true;
		}

		//  getStats
		//
		//  Returns a pointer to the cache statistics structure
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		Stats*			-		Pointer to the statistics structure
		//
		//  NOTES:
		//  

		Stats* getStats() {
			return &StatRec;
		}

		//  dumpCacheControl
		//
		//  This function will dump the cache control table to the passed logging stream
		//
		//  PARAMETERS:
		//
		//		std::ostream&		-		Reference to the output (log) stream to use
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void	dumpCacheControl(std::ostream& Log) {
			time_t			TPTime = {};
			tm				TPTM = {};
			char			szPTime[64] = {};															//  Printable time

			//  Show the count and size of the cache
			Log << new LOGMSG("TRACE: There are %i entries in the pool with total size: %i Kb.", UCL, ((Size + 511) / 1024));

			//  Show each entry in the pool
			for (size_t CCX = 0; CCX < UCL; CCX++) {
				Log << "TRACE: Entry #" << (CCX + 1) << ": ";
				Log << "ObjID: " << pCL[CCX].RKey;
				Log << ", Refs: " << pCL[CCX].RefCount;
				TPTime = CLOCK::to_time_t(pCL[CCX].LastRef);
				localtime_safe(&TPTime, &TPTM);
				strftime(szPTime, 64, "%F %T", &TPTM);
				Log << ", Last Ref: " << szPTime;
				Log << ", In-mem: " << (void*)pCL[CCX].RPtr;
				Log << ", Size: " << pCL[CCX].RLen;
				if (pCL[CCX].DirtyBit) Log << ", Dirty";
				TPTime = CLOCK::to_time_t(pCL[CCX].Expiry);
				localtime_safe(&TPTime, &TPTM);
				strftime(szPTime, 64, "%F %T", &TPTM);
				Log << ", Expires: " << szPTime;
				Log << ", key: '" << Keys.getString(pCL[CCX].RKey) << "'";
				Log << "." << std::endl;
			}

			//  Return to caller
			return;
		}

		//  dismiss
		//
		//  Empties the cache and releases the content and the pool
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void		dismiss() {

			//  If the cache is already incoherent DO NOTHING
			if (!Coherent) return;

			//  First purge all entries from the pool (writing any dirty entries)
			purge(true);

			//  Now destroy the cache-line pool
			if (pCL != nullptr) free(pCL);
			NCL = UCL = 0;

			//  Flag the cache as incoherent
			Coherent = false;

			//  Return to caller
			return;
		}

		//  purge
		//
		//  Purges all cached entries from the pool
		//
		//  PARAMETERS:
		//
		//		bool		-		If true then all "dirty" entries are written to backing store, otherwise they are simply destroyed
		//
		//  RETURNS:
		//
		//  NOTES:
		//
		//	1.	To destroy a cache without writing dirty entries first call purge(false) then call dismiss()
		//  

		void		purge(bool WriteDirty) {

			//  Safety
			if (!Coherent) return;

			//  Process each entry in the pool in turn
			for (size_t CEIX = 0; CEIX < UCL; CEIX++) {

				//  If the entry is dirty then it will be written to the backing store (if enabled)
				if (WriteDirty && pCL[CEIX].DirtyBit) {
					if (!putCachedRecord(Keys.getString(pCL[CEIX].RKey), pCL[CEIX].RPtr, pCL[CEIX].RLen)) {
						Coherent = false;
						return;
					}
					StatRec.DirtyWrites++;
				}

				//  Purge the current entry
				destroyCachedRecord(pCL[CEIX].RPtr, pCL[CEIX].RLen);
				memset(&pCL[CEIX], 0, sizeof(CacheLine));
				StatRec.Purges++;
			}

			//  Clear the count and size of cached entries
			UCL = 0;
			Size = 0;

			//  Return to caller
			return;
		}

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		bool			Coherent;															//  Cache is coherent
		SWITCHES		COpts;																//  Cache control options
		CacheLine*		pCL;																//  Cache lines
		size_t			Budget;																//  Budget size (Kb)
		size_t			NCL;																//  Number of cache lines (size of pool)
		size_t			UCL;																//  Number of cache lines used
		size_t			Size;																//  Cache size (Kb)
		StringPool		Keys;																//  String pool holding the keys
		Stats			StatRec;															//  Statistics record


		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  findCacheLine
		//
		//  This function will return a pointer to the Cache Line for a given key.
		//
		//  PARAMETERS:
		//
		//		char*		-		Const pointer the Key of the record to be read/written
		//
		//  RETURNS:
		//
		//		CacheLine*	-		Pointer to the cache-line matching the passed key, NULL if not matched
		//
		//  NOTES:
		//
		//		The cache is maintained in MFU/MRU order so must be searched exhaustively
		//  

		CacheLine* findCacheLine(const char* Key) {

			//  Safety
			if (!Coherent) return nullptr;
			if (Key == nullptr) return nullptr;
			if (Key[0] == '\0') return nullptr;

			//  Search for the key
			for (size_t CEIX = 0; CEIX < UCL; CEIX++) {

				//  Update statistics
				StatRec.Inspects++;

				if (COpts & OBSERVE_KEY_CASE) {
					if (strcmp(Key, Keys.getString(pCL[CEIX].RKey)) == 0) return &pCL[CEIX];
				}
				else {
					if (_stricmp(Key, Keys.getString(pCL[CEIX].RKey)) == 0) return &pCL[CEIX];
				}
			}

			//  Return Not Found
			return nullptr;
		}

		//  promote
		//
		//  This function will promote the passed cache line in the pool according to policy.
		//
		//  PARAMETERS:
		//
		//		CacheLine*	-		Pointer to the cache-line matching the passed key.
		//
		//  RETURNS:
		//
		//		CacheLine*	-		Update pointer to the cache-line matching the passed key.
		//
		//  NOTES:
		//
		//		The cache is maintained in MFU/MRU order so must be searched exhaustively
		//  

		CacheLine* promote(CacheLine* pCEnt) {
			CacheLine		CurrEntry = {};													//  Current entry
			CacheLine*		pTarget = nullptr;												//  Target cache entry
			size_t			MoveSize = 0;													//  Size to move

			if (COpts & EVICTION_STRATEGY_LRU) {
				//  Most Recently Used (MRU), the current entry is moved to the head of the pool
				if (pCEnt == pCL) return pCEnt;

				//  Save the current entry
				memcpy(&CurrEntry, pCEnt, sizeof(CacheLine));

				//  Shuffle all entries down by one
				pTarget = pCL + 1;

				//  Compute the size to move
				MoveSize = ((BYTE*)pCEnt) - ((BYTE*)pCL);

				//  Shuffle the entries down by 1 cache line
				memmove(pTarget, pCL, MoveSize);

				//  Insert the entry at the first slot
				memcpy(pCL, &CurrEntry, sizeof(CacheLine));

				//  Return to caller
				return pCL;
			}

			//  Most Frequently Used  -  Promote the current entry one slot at a time until it is in the correct position

			while (pCEnt > pCL) {
				//  Point to the previous slot
				pTarget = pCEnt - 1;

				//  Check for the correct position
				if (pCEnt->RefCount < pTarget->RefCount) return pCEnt;

				//  Swap the two entries
				memcpy(&CurrEntry, pTarget, sizeof(CacheLine));
				memcpy(pTarget, pCEnt, sizeof(CacheLine));
				memcpy(pCEnt, &CurrEntry, sizeof(CacheLine));

				//  Reposition
				pCEnt--;
			}

			//  Return to caller
			return pCEnt;
		}

		//  expireRecords
		//
		//  This function will expire any records that are currently due to exire.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void		expireRecords() {
			TIMER		BaseLine = CLOCK::now();													//  Baseline time
			int			Expired = 0;																//  Lines expired
			size_t		Inspect = 0;																//  Item being inspected

			while (Inspect < UCL) {

				//  See if the current entry has expired
				if (pCL[Inspect].Expiry <= BaseLine) {
					//  Record has expired
					//  If the entry is dirty then it will be written to the backing store (if enabled)
					if (pCL[Inspect].DirtyBit) {
						if (!putCachedRecord(Keys.getString(pCL[Inspect].RKey), pCL[Inspect].RPtr, pCL[Inspect].RLen)) {
							Coherent = false;
							return;
						}
						StatRec.DirtyWrites++;
					}

					//  Purge the entry from the cache
					Keys.deleteString(pCL[Inspect].RKey);
					destroyCachedRecord(pCL[Inspect].RPtr, pCL[Inspect].RLen);
					Size = Size - pCL[Inspect].RLen;
					memset(&pCL[Inspect], 0, sizeof(CacheLine));

					//  Shuffle up any following entries
					if (Inspect < (UCL - 1)) memmove(&pCL[Inspect], &pCL[Inspect + 1], (UCL - (Inspect + 1)) * sizeof(CacheLine));
					UCL--;
					StatRec.Expires++;
				}
				else Inspect++;
			}

			//  Return to caller
			return;
		}

		//  evictRecords
		//
		//  This function will evict sufficient records from the cache until there is room for the new one.
		//
		//  PARAMETERS:
		//
		//		size_t			-		Size of the new record (Bytes)
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void		evictRecords(size_t ReqSize) {
			size_t		Evictee = 0;															//  Eviction candidate

			//  Process until there is sufficient space in the cache
			while ((Size + ReqSize) > (Budget * 1024)) {

				//  Cache entries are ALWAYS evicted from the tail of the Cache Line array
				Evictee = UCL - 1;

				//  If the entry is dirty then it will be written to the backing store (if enabled)
				if (pCL[Evictee].DirtyBit) {
					if (!putCachedRecord(Keys.getString(pCL[Evictee].RKey), pCL[Evictee].RPtr, pCL[Evictee].RLen)) {
						Coherent = false;
						return;
					}
					StatRec.DirtyWrites++;
				}

				//  Evict the selected entry
				Keys.deleteString(pCL[Evictee].RKey);
				destroyCachedRecord(pCL[Evictee].RPtr, pCL[Evictee].RLen);
				Size = Size - pCL[Evictee].RLen;
				memset(&pCL[Evictee], 0, sizeof(CacheLine));

				UCL--;

				StatRec.Evictions++;
			}

			//  Return to caller
			return;
		}

		//
		//  The following functions define the interface that MUST be implemented in extending classes to link
		//  the cache to the underlying storage.
		//

		//  putCachedRecord
		//
		//  This function is used by the cache to "Write Through" writing a dirty (updated) cache entry into the backing store.
		//
		//  PARAMETERS:
		//
		//		char*		-		Const pointer the Key of the record to be written
		//		BYTE*		-		Const pointer to the content of the record to be written
		//		size_t		-		Size of the record to be written
		//
		//  RETURNS:
		//
		//		bool		-		true for a successful write, false will cause the cache to become incoherent (unuseable)
		//
		//  NOTES:
		//  

		virtual bool	putCachedRecord(const char* Key, const BYTE* Rec, size_t RecLen) = 0;

		//  getStoredRecord
		//
		//  This function is used to populate the cache with a record that is not currently in the cache.
		//
		//  PARAMETERS:
		//
		//		char*		-		Pointer to the key for the desired record (NULL terminated string)
		//		size_t&		-		Reference to the record variable to hold the returned record length	
		//		size_t&		-		Reference to the record variable to hold the Time-To-Live (TTL) of the returned record
		//
		//  RETURNS:
		//
		//		BYTE*		-		Pointer to the record in the cache
		//
		//  NOTES:
		//

		virtual BYTE* getStoredRecord(const char* Key, size_t& RecLen, size_t& TTL) = 0;

		//  destroyCachedRecord
		//
		//  This function is used to destroy a record that is being purged/evicted from the cache
		//
		//  PARAMETERS:
		//
		//		BYTE*		-		Pointer to the record in the cache
		//		size_t		-		Size of the record
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		virtual void	destroyCachedRecord(BYTE* Rec, size_t RecLen) = 0;


	};

}
