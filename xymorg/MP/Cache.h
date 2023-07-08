#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       Cache.h																							*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.0.0	(Build: 01)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
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
//*	1.		This is the multi-threaded implementation. It IS thread safe.											*
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

#include	"../LPBHdrs.h"																		//  Language and Platform base headers
#include	"../types.h"																		//  xymorg type definitions
#include	"../consts.h"																		//  xymorg constant definitions
#include	"../StringPool.h"																	//  String Pool
#include	"../Logging.h"																		//  Logging message class
#include	"../MP/Dispatcher.h"																//  MP Dispatcher

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
		//*   Public Nested Classes																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Statistics
		class Stats {
		public:
			Stats() : DirtyWrites(0)
				, Purges(0)
				, PurgeRecs(0)
				, Evictions(0)
				, Expires(0)
				, MaxEnts(0)
				, MaxSize(0)
				, SoftLocks(0)
				, SoftWaits(0)
				, SoftWaitQuanta(0)
				, HardLocks(0)
				, HardWaits(0)
				, HardWaitQuanta(0) {}

			std::atomic<uint64_t>	Hits;																		//  Number of cache hits
			std::atomic<uint64_t>	Misses;																		//  Number of cache misses
			std::atomic<uint64_t>	Reads;																		//  Number of cache reads
			std::atomic<uint64_t>	Peeks;																		//  Number of cache peeks
			std::atomic<uint64_t>	Writes;																		//  Number of cache writes
			uint64_t				DirtyWrites;																//  Number of deferred Write-Throughs
			uint64_t				Purges;																		//  Number of purge requests
			uint64_t				PurgeRecs;																	//  Number of records purged from the cache
			std::atomic<uint64_t>	NotFound;																	//  Not found (in cache or store)
			std::atomic<uint64_t>	Inspects;																	//  Number of cache line inspections
			uint64_t				Evictions;																	//  Number of cache evictions
			uint64_t				Expires;																	//  Number of cache entries that expired
			uint64_t				MaxEnts;																	//  Maximum count of cache entries
			uint64_t				MaxSize;																	//  Maximum size of the cache (Kb)

			//  Cache Lock activity
			uint64_t				SoftLocks;																	//  Number of soft locks
			uint64_t				SoftWaits;																	//  Number of waits to soft lock the cache
			uint64_t				SoftWaitQuanta;																//  Number of wait quanta for soft lock
			uint64_t				HardLocks;																	//  Number of hard locks
			uint64_t				HardWaits;																	//  Number of waits to hard lock the cache
			uint64_t				HardWaitQuanta;																//  Number of wait quanta for hard lock

		};

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Constants		                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		static const int NumLines = 256;																		//  Default number of cache lines

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Structures	                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//   Cache Line
		typedef struct CacheLine {
			MUTEX					CLX;																		//  Cache Line exclusivity mutex
			TIMER					Expiry;																		//  Time this entry expires (added time + TTL)
			TIMER					LastRef;																	//  Time of the last reference
			size_t					RefCount;																	//  Reference Count
			STRREF					RKey;																		//  Key of the record/object
			size_t					RLen;																		//  Length of the record/object
			BYTE*					RPtr;																		//  Pointer to the cached record/object
			bool					DirtyBit;																	//  Cache line represents an updated record/object
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
		//		Dispatcher&			-		Reference to the multi-programming dispatcher
		//
		//  RETURNS:
		//
		//  NOTES:
		//
		//	1.	Extending classes MUST invoke this constructor
		//

		Cache(SWITCHES NewCfg, size_t NewBudget, Dispatcher& MP) : COpts(NewCfg)
			, pCL(NULL)
			, Budget(NewBudget)
			, NCL(0)
			, UCL(0)
			, Size(0)
			, Keys()
			, StatRec()
			, Coherent(false)
			, CacheLock()
			, CMP(MP) {

			//  Clear the statistics
			StatRec.DirtyWrites = 0;
			StatRec.Evictions = 0;
			StatRec.Expires = 0;
			StatRec.HardLocks = 0;
			StatRec.HardWaitQuanta = 0;
			StatRec.HardWaits = 0;
			StatRec.Hits.store(0);
			StatRec.Inspects.store(0);
			StatRec.MaxEnts = 0;
			StatRec.MaxSize = 0;
			StatRec.Misses.store(0);
			StatRec.NotFound.store(0);
			StatRec.Peeks.store(0);
			StatRec.PurgeRecs = 0;
			StatRec.Purges = 0;
			StatRec.Reads.store(0);
			StatRec.SoftLocks = 0;
			StatRec.SoftWaitQuanta = 0;
			StatRec.SoftWaits = 0;
			StatRec.Writes.store(0);

			//  Allocate the cache-line pool for the initial number of entries
			pCL = (CacheLine*)malloc(NumLines * sizeof(CacheLine));
			if (pCL == nullptr) {
				Coherent = false;
				return;
			}
			memset(pCL, 0, NumLines * sizeof(CacheLine));
			NCL = NumLines;

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
			dismiss(CMP);

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
		//  This variant of the call IS THREAD SAFE.
		//
		//  PARAMETERS:
		//
		//		char*		-		Pointer to the key for the desired record (NULL terminated string)
		//		BYTE*		-		Pointer to the record buffer to hold the returned data
		//		size_t&		-		Reference to the record variable to hold the buffer size (in) and returned record length (out)	
		//		size_t&		-		Reference to the record variable to hold the Time-To-Live (TTL) of the returned record
		//		Dispatcher&	-		Reference to the multi-programming dispatcher
		//
		//  RETURNS:
		//
		//		bool		-		true if the record was returned, otherwise false
		//
		//  NOTES:
		// 
		//		1.		In the case where the passed buffer is too small to hold the cached record then the call returns false
		//				and the buffer will hold the truncated record. The length will hold the length of the cached record.
		//				
		//

		bool	getCachedRecord(const char* Key, BYTE* Buffer, size_t& RecLen, size_t& TTL, Dispatcher& MP) {
			THREADID			id = MP.getThreadID();														//  ID of calling thread
			size_t				BfrLen = RecLen;															//  Size of the buffer to return the cached entry
			BYTE*				pNewRec = nullptr;															//  New record to be added to the cache
			size_t				CRLine = 0;																	//  Cache line index
			TIMER				NowTime = CLOCK::now();														//  Current time
			SECONDS				TTLSecs = {};																//  TTL in seconds

			//  Safety
			RecLen = 0;
			TTL = 0;
			if (Key == nullptr) return false;
			if (Key[0] == '\0') return false;
			if (Buffer == nullptr) return false;
			if (BfrLen == 0) return false;
			if (!Coherent) return false;
			if (NCL == 0) return false;
			if (pCL == nullptr) return false;

			//  Update Stats
			StatRec.Reads.fetch_add(1);

			//  Expire any records that are due
			if (COpts & OBSERVE_EXPIRY) expireRecords(MP);

			//  Assert the soft lock on the cache mutex
			CacheLock.softlock(id);

			//  Search the cache to determine if the passed key is in the cache
			CRLine = findCacheLine(Key);
			if (CRLine != UCL) {
				//  Entry is already in the cache - perform cache management bookeeping and return the record
				pCL[CRLine].CLX.lock(id);
				pCL[CRLine].RefCount++;
				pCL[CRLine].LastRef = CLOCK::now();
				pCL[CRLine].CLX.unlock(id);

				//  Update the statistics
				StatRec.Hits.fetch_add(1);

				//  Compute the remaining TTL
				NowTime = CLOCK::now();
				TTLSecs = pCL[CRLine].Expiry - NowTime;
				TTL = size_t(TTLSecs.count());
				RecLen = pCL[CRLine].RLen;
				pNewRec = pCL[CRLine].RPtr;

				//  Entry found in cache (but does not exist)
				if (pNewRec == nullptr) {
					//  Release the soft lock
					CacheLock.softunlock(id);
					return false;
				}

				//  Copy the record to the caller
				if (BfrLen < RecLen) {
					//  Return the truncated record
					memcpy(Buffer, pNewRec, BfrLen);
					return false;
				}

				//  Return the complete record
				memcpy(Buffer, pNewRec, RecLen);
				//MP.Log << "TRACE: Returning record, Cache Line: " << CRLine << ", from: 0x" << (void*)pNewRec << ", size: " << RecLen << "." << std::endl;

				//  Release the soft lock
				CacheLock.softunlock(id);

				//  Return the cached record
				return true;
			}

			//  Release the soft lock on the cache as we will be asserting a hard lock to complete the operation
			CacheLock.softunlock(id);

			//  Record not currently in the cache - attempt to retrieve  it from the store
			StatRec.Misses.fetch_add(1);

			//  Attempt to read the desired key from the store
			pNewRec = getStoredRecord(Key, RecLen, TTL, MP);
			//MP.Log << "TRACE: Read of record not in cache at: 0x" << (void*) pNewRec << ", size: " << RecLen << "." << std::endl;

			if (pNewRec == nullptr) {
				StatRec.NotFound.fetch_add(1);

				//  If NOT-EXIST records are NOT being cached then return to the caller with record unavailable
				if (!(COpts & CACHE_NOT_EXIST)) return false;
			}

			//  Assert a hard lock on the cache
			CacheLock.hardlock(id);

			//  Search the cache again to determine if the passed key is in the cache
			CRLine = findCacheLine(Key);
			if (CRLine != UCL) {

				//  Dispose of the new copy of the reasource
				free(pNewRec);
				RecLen = 0;

				//  Entry is already in the cache - perform cache management bookeeping and return the record
				pCL[CRLine].RefCount++;
				pCL[CRLine].LastRef = CLOCK::now();

				//  Update the statistics
				StatRec.Hits.fetch_add(1);

				//  Compute the remaining TTL
				NowTime = CLOCK::now();
				TTLSecs = pCL[CRLine].Expiry - NowTime;
				TTL = size_t(TTLSecs.count());
				RecLen = pCL[CRLine].RLen;
				pNewRec = pCL[CRLine].RPtr;

				//  Entry found in cache (but does not exist)
				if (pNewRec == nullptr) {
					//  Release the hard lock
					CacheLock.hardunlock(id);
					return false;
				}

				//  Copy the record to the caller
				if (BfrLen < RecLen) {
					//  Return the truncated record
					memcpy(Buffer, pNewRec, BfrLen);
					//  Release the hard lock
					CacheLock.hardunlock(id);
					return false;
				}

				//  Return the complete record
				memcpy(Buffer, pNewRec, RecLen);
				//MP.Log << "TRACE: Returning record, Cache Line: " << CRLine << ", from: 0x" << (void*)pNewRec << ", size: " << RecLen << "." << std::endl;

				//  Unlock cache
				CacheLock.hardunlock(id);

				//  Return the cached record
				return true;
			}

			//  Safety - if the size of the new entry exceeds the budget then increase the budget
			if (COpts & OBSERVE_BUDGET) {
				if (((RecLen + 511) / 1024) > Budget) Budget += ((RecLen + 511) / 1024);
				//  Evict records from the cache until there is sufficient space
				evictRecords(RecLen, MP);
			}

			//  Make sure that the cache line pool has capacity
			if (NCL == UCL) {
				CacheLine* pNewPool = (CacheLine*)realloc(pCL, (NCL + NumLines) * sizeof(CacheLine));
				if (pNewPool == nullptr) {
					Coherent = false;
					CacheLock.hardunlock(id);
					destroyCachedRecord(pNewRec, RecLen, MP);
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
				CRLine = 0;
			}
			else {
				//  LFU - new cache lines are appended to the existing
				memset(&pCL[UCL], 0, sizeof(CacheLine));
				CRLine = UCL;
			}

			//  Insert the new record into the cache
			pCL[CRLine].DirtyBit = false;
			if (TTL == 0) TTL = 24 * 60 * 60;													//  Default 24 hrs
			pCL[CRLine].Expiry = CLOCK::now() + MILLISECONDS(TTL * 1000);
			pCL[CRLine].LastRef = CLOCK::now();
			pCL[CRLine].RefCount = 1;
			pCL[CRLine].RKey = Keys.addString(Key);
			pCL[CRLine].RLen = RecLen;
			pCL[CRLine].RPtr = pNewRec;
			pCL[CRLine].CLX.initialise(3);

			//MP.Log << "TRACE: Cache Entry: " << CRLine << ", In-Memory: 0x" << (void*) pCL[CRLine].RPtr << ", size: " << pCL[CRLine].RLen << "." << std::endl;

			//  Bookeeping
			UCL++;
			Size += RecLen;
			if (UCL > StatRec.MaxEnts) StatRec.MaxEnts = UCL;
			if (((Size + 511) / 1024) > StatRec.MaxSize) StatRec.MaxSize = ((Size + 511) / 1024);

			//  If entry was NOT FOUND (but cached) return to caller
			if (pNewRec == nullptr) {
				CacheLock.hardunlock(id);
				return false;
			}
			else {
				//  Copy the record to the caller
				if (BfrLen < RecLen) {
					//  Return the truncated record
					memcpy(Buffer, pNewRec, BfrLen);

					//  Release the hard and soft locks on the cache
					CacheLock.hardunlock(id);
					return false;
				}

				//  Return the complete record
				memcpy(Buffer, pNewRec, RecLen);
				//MP.Log << "TRACE: Returning record, Cache Line: " << CRLine << ", from: 0x" << (void*)pNewRec << ", size: " << RecLen << "." << std::endl;
			}

			//  Release the hard and soft locks on the cache
			CacheLock.hardunlock(id);

			//  Return the entry
			return true;
		}

		//  peekCachedRecord
		//
		//  This is the primary function for the cache it returns the required record from the cache or initiates a read
		//  from the store and caches and returns that entry.
		//  Peek does not update the cache by counting hits or recording last accessed times.
		//  This variant of the call IS THREAD SAFE.
		//
		//  PARAMETERS:
		//
		//		char*		-		Pointer to the key for the desired record (NULL terminated string)
		//		BYTE*		-		Pointer to the record buffer to hold the returned data
		//		size_t&		-		Reference to the record variable to hold the buffer size (in) and returned record length (out)	
		//		size_t&		-		Reference to the record variable to hold the Time-To-Live (TTL) of the returned record
		//		Dispatcher&	-		Reference to the multi-programming dispatcher
		//
		//  RETURNS:
		//
		//		bool		-		true if the record was returned, otherwise false
		//
		//  NOTES:
		// 
		//		1.		In the case where the passed buffer is too small to hold the cached record then the call returns false
		//				and the buffer will hold the truncated record. The length will hold the length of the cached record.
		//

		bool peekCachedRecord(const char* Key, BYTE* Buffer, size_t& RecLen, size_t& TTL, Dispatcher& MP) {
			THREADID			id = MP.getThreadID();														//  ID of calling thread
			size_t				BfrLen = RecLen;															//  Size of the buffer to return the cached entry
			BYTE*				pNewRec = nullptr;															//  New record to be added to the cache
			size_t				CRLine = 0;																	//  Cache line index
			TIMER				NowTime = CLOCK::now();														//  Current time
			SECONDS				TTLSecs = {};																//  TTL in seconds

			//  Safety
			RecLen = 0;
			TTL = 0;
			if (Key == nullptr) return false;
			if (Key[0] == '\0') return false;
			if (Buffer == nullptr) return false;
			if (BfrLen == 0) return false;
			if (!Coherent) return false;

			//  Count number of peeks
			StatRec.Peeks.fetch_add(1);

			//  Expire any records that are due
			if (COpts & OBSERVE_EXPIRY) expireRecords(MP);

			//  Acquire the soft lock on the cache
			CacheLock.softlock(id);

			//  Search the cache to determine if the passed key is in the cache
			CRLine = findCacheLine(Key);
			if (CRLine != UCL) {

				//  Entry is already in the cache - perform cache management bookeeping and return the record
				pCL[CRLine].CLX.lock(id);
				pCL[CRLine].RefCount++;
				pCL[CRLine].LastRef = CLOCK::now();
				pCL[CRLine].CLX.unlock(id);

				//  Update the statistics
				StatRec.Hits.fetch_add(1);

				//  Compute the remaining TTL
				NowTime = CLOCK::now();
				TTLSecs = pCL[CRLine].Expiry - NowTime;
				TTL = size_t(TTLSecs.count());
				RecLen = pCL[CRLine].RLen;
				pNewRec = pCL[CRLine].RPtr;

				//  Copy the record to the caller
				if (BfrLen < RecLen) {
					//  Return the truncated record
					memcpy(Buffer, pNewRec, BfrLen);
					return false;
				}

				//  Return the complete record
				memcpy(Buffer, pNewRec, RecLen);

				//  Release the soft lock
				CacheLock.softunlock(id);

				//  Return the cached record
				return true;
			}

			//  Update the statistics
			StatRec.Misses.fetch_add(1);

			//  Release the soft lock
			CacheLock.softunlock(id);

			//  No cache entry was available to return
			return false;
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
		//		Dispatcher&	-		Reference to the multi-programming dispatcher
		//
		//  RETURNS:
		//
		//		bool		-		True if the record was written into the cache, otherwise false
		//
		//  NOTES:
		//

		bool		writeRecord(const char* Key, BYTE* Rec, size_t RecLen, size_t TTL, Dispatcher& MP) {
			THREADID			id = MP.getThreadID();														//  ID of calling thread
			size_t				CRLine = 0;																	//  Cache line index

			//  Safety
			if (Key == nullptr) return false;
			if (Key[0] == '\0') return false;
			if (Rec == nullptr) return false;
			if (RecLen == 0) return false;
			if (!Coherent) return false;

			//  Count number of writes
			StatRec.Writes++;

			//  Expire any records that are due
			if (COpts & OBSERVE_EXPIRY) expireRecords(MP);

			//  Assert the hard lock on the cache
			CacheLock.hardlock(id);

			CRLine = findCacheLine(Key);
			if (CRLine != UCL) {
				//  Entry is already in the cache - replace the record
				pCL[CRLine].RefCount++;
				pCL[CRLine].LastRef = CLOCK::now();

				//  Update the statistics
				StatRec.Hits.fetch_add(1);

				//  Destroy the existing cached record
				destroyCachedRecord(pCL[CRLine].RPtr, pCL[CRLine].RLen, MP);

				//  Update the cache entry
				pCL[CRLine].RPtr = Rec;
				pCL[CRLine].RLen = RecLen;
				pCL[CRLine].Expiry = CLOCK::now() + MILLISECONDS(TTL * 1000);

				//  If the cache policy is deferred write through then set the dirty bit, otherwise update the backing store.
				if (COpts & WRITE_DEFERRED) pCL[CRLine].DirtyBit = true;
				else {
					if (!putCachedRecord(Key, Rec, RecLen, MP)) {
						CacheLock.hardunlock(id);
						CacheLock.softunlock(id);
						return false;
					}
					pCL[CRLine].DirtyBit = false;
				}

				CacheLock.hardunlock(id);

				//  Return the showing success
				return true;
			}

			//
			//  Record was not found in the cache - create a new cache line to hold the record
			//
			StatRec.Misses.fetch_add(1);

			//  Safety - if the size of the new entry exceeds the budget then increase the budget
			if (COpts & OBSERVE_BUDGET) {
				if (((RecLen + 511) / 1024) > Budget) Budget += ((RecLen + 511) / 1024);
			}

			//  Evict records from the cache until there is sufficient space
			if (COpts & OBSERVE_BUDGET) evictRecords(RecLen, MP);

			//  Make sure that the cache line pool has capacity
			if (NCL == UCL) {
				CacheLine* pNewPool = (CacheLine*)realloc(pCL, (NCL + NumLines) * sizeof(CacheLine));
				if (pNewPool == nullptr) {
					Coherent = false;
					destroyCachedRecord(Rec, RecLen, MP);
					CacheLock.hardunlock(id);
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
				CRLine = 0;
			}
			else {
				//  LFU - new cache lines are appended to the existing
				memset(&pCL[UCL], 0, sizeof(CacheLine));
				CRLine = UCL;
			}

			//  Insert the new record into the cache
			pCL[CRLine].DirtyBit = true;
			if (TTL == 0) TTL = 24 * 60 * 60;													//  Default 24 hrs
			pCL[CRLine].Expiry = CLOCK::now() + MILLISECONDS(TTL * 1000);
			pCL[CRLine].LastRef = CLOCK::now();
			pCL[CRLine].RefCount = 1;
			pCL[CRLine].RKey = Keys.addString(Key);
			pCL[CRLine].RLen = RecLen;
			pCL[CRLine].RPtr = Rec;
			pCL[CRLine].CLX.initialise(3);

			//  If the cache policy is not deferred write then write through the new record
			if ((COpts & WRITE_DEFERRED) == 0) {
				if (!putCachedRecord(Key, Rec, RecLen, MP)) return false;
				pCL[CRLine].DirtyBit = false;
			}

			//  Bookeeping
			UCL++;
			Size += RecLen;
			if (UCL > StatRec.MaxEnts) StatRec.MaxEnts = UCL;
			if (((Size + 511) / 1024) > StatRec.MaxSize) StatRec.MaxSize = ((Size + 511) / 1024);

			CacheLock.hardunlock(id);

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

			//  Update the mutex statistics
			StatRec.SoftLocks = CacheLock.SoftLocks.load();
			StatRec.SoftWaits = CacheLock.SoftWaits.load();
			StatRec.SoftWaitQuanta = CacheLock.SoftWaitQuanta.load();
			StatRec.HardLocks = CacheLock.HardLocks;
			StatRec.HardWaits = CacheLock.HardWaits;
			StatRec.HardWaitQuanta = CacheLock.HardWaitQuanta;

			//  Return the pointer to the statistics
			return &StatRec;
		}

		//  dumpCacheControl
		//
		//  This function will dump the cache control table to the passed logging stream
		//
		//  PARAMETERS:
		//
		//		Dispatcher&	-		Reference to the multi-programming dispatcher
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void	dumpCacheControl(Dispatcher& MP) {
			THREADID			id = MP.getThreadID();														//  ID of calling thread
			time_t				TPTime = {};
			tm					TPTM = {};
			char				szPTime[64] = {};															//  Printable time

			//  Acquire a hard lock on the pool
			CacheLock.hardlock(id);

			//  Show the count and size of the cache
			MP.Log << new LOGMSG("TRACE: There are %i entries in the pool with total size: %i Kb.", UCL, ((Size + 511) / 1024));

			//  Show each entry in the pool
			for (size_t CCX = 0; CCX < UCL; CCX++) {
				MP.Log << "TRACE: Entry #" << (CCX + 1) << ": ";
				MP.Log << "ObjID: " << pCL[CCX].RKey;
				MP.Log << ", Refs: " << pCL[CCX].RefCount;
				TPTime = CLOCK::to_time_t(pCL[CCX].LastRef);
				localtime_safe(&TPTime, &TPTM);
				strftime(szPTime, 64, "%F %T", &TPTM);
				MP.Log << ", Last Ref: " << szPTime;
				MP.Log << ", In-mem: 0x" << (void*)pCL[CCX].RPtr;
				MP.Log << ", Size: " << pCL[CCX].RLen;
				if (pCL[CCX].DirtyBit) MP.Log << ", Dirty";
				TPTime = CLOCK::to_time_t(pCL[CCX].Expiry);
				localtime_safe(&TPTime, &TPTM);
				strftime(szPTime, 64, "%F %T", &TPTM);
				MP.Log << ", Expires: " << szPTime;
				MP.Log << ", key: '" << Keys.getString(pCL[CCX].RKey) << "'";
				MP.Log << "." << std::endl;
			}

			CacheLock.hardunlock(id);

			//  Return to caller
			return;
		}

		//  dismiss
		//
		//  Empties the cache and releases the content and the pool
		//
		//  PARAMETERS:
		// 
		//		Dispatcher	-		Reference to the multi-programming dispatcher
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void		dismiss(Dispatcher& MP) {

			//  If the cache is already incoherent DO NOTHING
			if (!Coherent) return;

			//  First purge all entries from the pool (writing any dirty entries)
			purge(true, MP);

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
		//		Dispatcher	-		Reference to the multi-programming dispatcher
		//
		//  RETURNS:
		//
		//  NOTES:
		//
		//		1.	To destroy a cache without writing dirty entries first call purge(false) then call dismiss()
		//  

		void		purge(bool WriteDirty, Dispatcher& MP) {
			THREADID		id = MP.getThreadID();													//  ID of the calling thread

			//  Safety
			if (!Coherent) return;

			//  Assert the hard locks on the cache
			CacheLock.hardlock(id);

			StatRec.Purges++;

			//  Process each entry in the pool in turn
			for (size_t CEIX = 0; CEIX < UCL; CEIX++) {

				//  If the entry is dirty then it will be written to the backing store (if enabled)
				if (WriteDirty && pCL[CEIX].DirtyBit) {
					if (!putCachedRecord(Keys.getString(pCL[CEIX].RKey), pCL[CEIX].RPtr, pCL[CEIX].RLen, MP)) {
						Coherent = false;
						return;
					}
					StatRec.DirtyWrites++;
				}

				//  Purge the current entry
				destroyCachedRecord(pCL[CEIX].RPtr, pCL[CEIX].RLen, MP);
				memset(&pCL[CEIX], 0, sizeof(CacheLine));
				StatRec.PurgeRecs++;
			}

			//  Clear the count and size of cached entries
			UCL = 0;
			Size = 0;

			//  Release the cache lock
			CacheLock.hardunlock(id);

			//  Return to caller
			return;
		}

	protected:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Protected Members																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  External Refetences
		Dispatcher& CMP;																//  Creating MP dispatcher

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		bool						Coherent;															//  Cache is coherent
		SWITCHES					COpts;																//  Cache control options
		CacheLine*					pCL;																//  Cache lines
		size_t						Budget;																//  Budget size (Kb)
		size_t						NCL;																//  Number of cache lines (size of pool)
		size_t						UCL;																//  Number of cache lines used
		size_t						Size;																//  Cache size (Kb)
		StringPool					Keys;																//  String pool holding the keys
		Stats						StatRec;															//  Statistics record

		//  Bi-Level mutex used to protect the cache controls
		BIMUTEX						CacheLock;

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
		//		size_t		-		Index of the entry in the cache ( == UCL if not found)
		//
		//  NOTES:
		//
		//		The cache is maintained in MFU/MRU order so must be searched exhaustively
		//  

		size_t		findCacheLine(const char* Key) {

			//  Safety
			if (!Coherent) return UCL;
			if (Key == nullptr) return UCL;
			if (Key[0] == '\0') return UCL;

			//  Search for the key
			for (size_t CEIX = 0; CEIX < UCL; CEIX++) {

				//  Update statistics
				StatRec.Inspects.fetch_add(1);

				if (COpts & OBSERVE_KEY_CASE) {
					if (strcmp(Key, Keys.getString(pCL[CEIX].RKey)) == 0) return CEIX;
				}
				else {
					if (_stricmp(Key, Keys.getString(pCL[CEIX].RKey)) == 0) return CEIX;
				}
			}

			//  Return Not Found
			return UCL;
		}

		//  expireRecords
		//
		//  This function will expire any records that are currently due to exire.
		//
		//  PARAMETERS:
		// 
		//		Dispatcher&		-		Reference to the MP dispatcher of the calling thread
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void		expireRecords(Dispatcher& MP) {
			THREADID	id = MP.getThreadID();														//  ID of calling thread
			TIMER		BaseLine = CLOCK::now();													//  Baseline time
			int			Expired = 0;																//  Lines expired
			size_t		Inspect = 0;																//  Item being inspected

			//  Determine if there are any records expired
			for (Inspect = 0; Inspect < UCL; Inspect++) if (pCL[Inspect].Expiry <= BaseLine) break;
			if (Inspect == UCL) return;																//  None to process

			//  Assert the hard lock on the cache
			CacheLock.hardlock(id);

			while (Inspect < UCL) {

				//  See if the current entry has expired
				if (pCL[Inspect].Expiry <= BaseLine) {
					//  Record has expired
					//  If the entry is dirty then it will be written to the backing store (if enabled)
					if (pCL[Inspect].DirtyBit) {
						if (!putCachedRecord(Keys.getString(pCL[Inspect].RKey), pCL[Inspect].RPtr, pCL[Inspect].RLen, MP)) {
							CacheLock.hardunlock(id);
							Coherent = false;
							return;
						}
						StatRec.DirtyWrites++;
					}

					//  Purge the entry from the cache
					Keys.deleteString(pCL[Inspect].RKey);
					destroyCachedRecord(pCL[Inspect].RPtr, pCL[Inspect].RLen, MP);
					Size = Size - pCL[Inspect].RLen;
					memset(&pCL[Inspect], 0, sizeof(CacheLine));

					//  Shuffle up any following entries
					if (Inspect < (UCL - 1)) memmove(&pCL[Inspect], &pCL[Inspect + 1], (UCL - (Inspect + 1)) * sizeof(CacheLine));
					UCL--;
					StatRec.Expires++;
				}
				else Inspect++;
			}

			//  Release the hard lock on the cache
			CacheLock.hardunlock(id);

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
		//		Dispatcher&		-		Reference to the MP dispatcher of the calling thread
		//
		//  RETURNS:
		//
		//  NOTES:
		// 
		//		1.		This function is always invoked with the hard lock asserted on the cache
		//  

		void		evictRecords(size_t ReqSize, Dispatcher& MP) {
			size_t		TargetSize = (Budget * 1024) - ReqSize;									//  Target size

			//  Process until the target size is acieved
			while (Size > TargetSize && UCL > 0 && Coherent) evictRecord(MP);

			//  Return to caller
			return;
		}

		//  evictRecord
		//
		//  This function will evict the oldest or least referenced entry in the cache
		//
		//  PARAMETERS:
		// 
		//		Dispatcher&		-		Reference to the MP dispatcher of the calling thread
		//
		//  RETURNS:
		//
		//  NOTES:
		// 

		void	evictRecord(Dispatcher& MP) {
			size_t		Evictee = 0;															//  Eviction candidate
			size_t		MRU = 0;																//  Most recently used entry
			size_t		MRUBU = 0;																//  Backup (penultimate MRU) entry
			TIMER		CTime = pCL[0].LastRef;
			TIMER		OTime = pCL[0].LastRef;

			//  Boundary condition - there is only a single entry in the cache
			if (UCL == 1) Evictee = 0;
			else {
				//  Find the oldest or least referenced record
				for (size_t CLX = 1; CLX < UCL; CLX++) {
					if (pCL[CLX].RKey != NULLSTRREF) {
						//  Entry is in use
						if (COpts & EVICTION_STRATEGY_LRU) {
							//  Oldest entry in the cache
							CTime = pCL[CLX].LastRef;
							OTime = pCL[Evictee].LastRef;
							if (CTime < OTime) Evictee = CLX;
						}
						else {
							//  Least used entry in the cache
							//  Prevention of bouncing (evicting recently added)
							if (pCL[CLX].RefCount < pCL[Evictee].RefCount) Evictee = CLX;
							//  Latest entry in the cache
							CTime = pCL[CLX].LastRef;
							OTime = pCL[MRU].LastRef;
							if (CTime > OTime) {
								MRUBU = MRU;
								MRU = CLX;
							}
						}
					}
				}

				//  If strategy is least frequently used then try to prevent eviction of the latest entry in the cache
				if (COpts & EVICTION_STRATEGY_LFU) {
					if (Evictee == MRU) Evictee = MRUBU;
				}
			}

			//  If the selected entry has the dirty bit set then perform a deferred write of the record
			if (pCL[Evictee].DirtyBit) {
				if (!putCachedRecord(Keys.getString(pCL[Evictee].RKey), pCL[Evictee].RPtr, pCL[Evictee].RLen, MP)) {
					Coherent = false;
					return;
				}
				StatRec.DirtyWrites++;
			}

			//  Evict the selected entry
			Keys.deleteString(pCL[Evictee].RKey);
			destroyCachedRecord(pCL[Evictee].RPtr, pCL[Evictee].RLen, MP);
			//  Adjust the size of the cache
			Size = Size - pCL[Evictee].RLen;
			memset(&pCL[Evictee], 0, sizeof(CacheLine));

			//  Drop the cache line
			if (Evictee < (UCL - 1)) memmove(&pCL[Evictee], &pCL[Evictee + 1], (UCL - (Evictee + 1)) * sizeof(CacheLine));
			UCL--;
			StatRec.Evictions++;

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
		//		Dispatche&	-		Reference to the MP dispatcher of the calling thread
		//
		//  RETURNS:
		//
		//		bool		-		true for a successful write, false will cause the cache to become incoherent (unuseable)
		//
		//  NOTES:
		// 
		//		1.		The cache takes ownerchip of the memory allocation passed, it should NOT be freed
		//				except in a call to destroyCachedRecord().
		//  

		virtual bool	putCachedRecord(const char* Key, const xymorg::BYTE* Rec, size_t RecLen, xymorg::Dispatcher& MP) = 0;

		//  getStoredRecord
		//
		//  This function is used to populate the cache with a record that is not currently in the cache.
		//
		//  PARAMETERS:
		//
		//		char*		-		Pointer to the key for the desired record (NULL terminated string)
		//		size_t&		-		Reference to the record variable holding the returned record length (out)
		//		size_t&		-		Reference to the record variable to hold the Time-To-Live (TTL) of the returned record
		//		Dispatche&	-		Reference to the MP dispatcher of the calling thread
		//
		//  RETURNS:
		//
		//		BYTE*		-		Pointer to the record in the cache
		//
		//  NOTES:
		//

		virtual xymorg::BYTE* getStoredRecord(const char* Key, size_t& RecLen, size_t& TTL, xymorg::Dispatcher& MP) = 0;

		//  destroyCachedRecord
		//
		//  This function is used to destroy a record that is being purged/evicted from the cache
		//
		//  PARAMETERS:
		//
		//		BYTE*		-		Pointer to the record in the cache
		//		size_t		-		Size of the record
		//		Dispatche&	-		Reference to the MP dispatcher of the calling thread
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		virtual void	destroyCachedRecord(xymorg::BYTE* Rec, size_t RecLen, xymorg::Dispatcher& MP) = 0;

	};

}

