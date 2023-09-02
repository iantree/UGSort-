#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       Primitives.h																						*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.0.0	(Build: 01)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2018 Hadleigh Marshall Netherlands b.v.														*
//*******************************************************************************************************************
//*																													*
//*	This header file contains definitions for primitive types and classes for Multi-Processing (MP).				*
//* The following classes are defined in this file :-																*
//*																													*
//*		MUTEX		-		Mutual exclusion control																*
//*		BIMUTEX		-		Bi-Level Mutual exclusion control														*
//*		SEMAPHORE	-		Inter-Thread signalling control															*
//*		LATCH		-		Resource Ownership latch																*
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
//*	1.0.0 -		04/12/2017	-	Initial Release																		*
//*																													*
//*******************************************************************************************************************/

//  Include xymorg headers
#include	"../LPBHdrs.h"																			//  Language and Platform base headers
#include	"../types.h"																			//  xymorg type definitions
#include	"../consts.h"																			//  xymorg constant definitions

//  Additional Language Headers
#include    <atomic>
#include    <thread>
#include    <chrono>

//
//  All components are defined within the xymorg namespace
//
namespace xymorg {

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Type Definitions                                                                                              *
	//*                                                                                                                 *
	//*******************************************************************************************************************


	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Macros                                                                                                        *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Class Definitions                                                                                             *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   MUTEX Class                                                                                                   *
	//*                                                                                                                 *
	//*   Objects of this class, implement a lightweight mutual exclusion lock using atomic operations.                 *
	//*                                                                                                                 *
	//*   PROPERTIES:                                                                                                   *
	//*                                                                                                                 *
	//*     NOT COPYABLE                                                                                                *
	//*     NOT MOVEABLE                                                                                                *
	//*     NOT ASSIGNABLE                                                                                              *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	class MUTEX
	{
	public:
		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Default constructor
		MUTEX()
		{
			//  Initialise the Mutex
			initialise(1);
			return;
		}

		//  Constructor for a specific wait quantum
		MUTEX(int msWQ)
		{
			//  Initialise the Mutx
			initialise(msWQ);
			return;
		}

		//
		//  Mark the class as NOT copyable/moveable/assignable
		//

		//  Copy & Move Constructors
		MUTEX(const MUTEX&) = delete;
		MUTEX(const MUTEX&&) = delete;

		//  Assignment operators
		MUTEX& operator = (const MUTEX&) = delete;
		MUTEX& operator = (const MUTEX&&) = delete;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor                                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		~MUTEX() {}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		static const THREADID   cMutexFree = 0xFFFFFFFF;													//  Mutex is free

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Members                                                                                                *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		MILLISECONDS            WaitQuantum;                                                                 //  Wait quantum duration
		uint64_t				Waits;                                                                       //  Count of waits on the mutex
		uint64_t		        WaitQuanta;                                                                  //  Count of wait quanta on the mutex
		uint64_t	            Locks;                                                                       //  Count of times the mutex was acquired

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  initialise
		//
		//  The initialise function performs an in place initialisation of the Mutex internals.
		//
		//  PARAMETERS
		//
		//        const int          -        Wait quantum size in milliseconds
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void initialise(int WQSize)
		{
			Mutex.store(cMutexFree);
			WaitQuantum = MILLISECONDS(WQSize);
			Locks = 0;
			Waits = 0;
			WaitQuanta = 0;
			return;
		}

		//  lock
		//
		//  Acquire the mutex on behalf of the caller (identified by THREADID). The function blocks while another thread is holding the mutex.
		//  If the mutex is already held by the requesting thread then the call just returns. 
		//
		//  PARAMETERS
		//
		//        THREADID        -        ID of the thread requesting the mutex
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void        lock(THREADID id)
		{
			unsigned int    WaitCount = 0;
			THREADID        Expected = cMutexFree;

			while (Mutex.load() != id)
			{
				Expected = cMutexFree;
				if (!Mutex.compare_exchange_weak(Expected, id))
				{
					WaitCount++;
					sleep(WaitQuantum);
				}
			}

			//  NOTE:
			//  The wait counters do not need to be atomics as they are protected by the mutex itself. i.e. they are only updated by the
			//  thread currently holding the mutex.

			Locks++;
			if (WaitCount > 0)
			{
				Waits++;
				WaitQuanta += WaitCount;
			}
			return;
		}

		//  unlock
		//
		//  Releases the mutex. If the mutex is not locked or is held by another thread then this function does nothing.
		//
		//  PARAMETERS
		//
		//        THREADID        -        ID of the thread unlocking the mutex
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void        unlock(THREADID id)
		{
			Mutex.compare_exchange_weak(id, cMutexFree);
			return;
		}

		//  isLocked
		//
		//  Tests if the mutex is locked.
		//
		//  PARAMETERS
		//
		//        bool        -        true if the mutex is locked otherwise false
		//
		//  RETURNS
		//
		//  NOTES:
		//

		bool isLocked()
		{
			if (Mutex.load() == cMutexFree) return false;
			return true;
		}

	private:
		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members                                                                                               *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		std::atomic<THREADID>            Mutex;                                                              //  Internal mutex implementation

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

	};

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   BIMUTEX Class                                                                                                 *
	//*                                                                                                                 *
	//*   Objects of this class, implement a lightweight bi-level mutual exclusion lock using atomic operations.        *
	//*                                                                                                                 *
	//*   PROPERTIES:                                                                                                   *
	//*                                                                                                                 *
	//*     NOT COPYABLE                                                                                                *
	//*     NOT MOVEABLE                                                                                                *
	//*     NOT ASSIGNABLE                                                                                              *
	//*                                                                                                                 *
	//*   CONTRACT:		                                                                                                *
	//*                                                                                                                 *
	//*     The control has two levels of exclusion 'soft' and 'hard'.                                                  *
	//*     Any number of callers may hold the 'soft' lock at the same time, providing that the 'hard' lock is          *
	//*     not held.                                                                                                   *
	//*     The 'hard' lock may only be acquired by the single holder of the 'soft' lock.	                            *
	//*     The control is for use in policing multiple readers, single writer scenarios.	                            *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	class BIMUTEX {
	public:
		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Default constructor
		BIMUTEX() {
			initialise(1);
			return;
		}

		//  Constructor for a specific wait quantum
		BIMUTEX(const int msWQ) {
			initialise(msWQ);
			return;
		}

		//
		//  Mark the class as NOT copyable/moveable/assignable
		//

		//  Copy & Move Constructors
		BIMUTEX(const BIMUTEX&) = delete;
		BIMUTEX(const BIMUTEX&&) = delete;

		//  Assignment operators
		BIMUTEX& operator = (const BIMUTEX&) = delete;
		BIMUTEX& operator = (const BIMUTEX&&) = delete;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor                                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		~BIMUTEX() {
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		static const THREADID   cMutexFree = 0xFFFFFFFF;                                        //  Mutex is free

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Members                                                                                                *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		MILLISECONDS            WaitQuantum;                                                                 //  Wait quantum duration
		std::atomic<uint64_t>	SoftWaits;                                                                   //  Count of waits on the soft mutex
		std::atomic<uint64_t>	SoftWaitQuanta;                                                              //  Count of wait quanta on the soft mutex
		std::atomic<uint64_t>	SoftLocks;                                                                   //  Count of times the soft mutex was acquired
		uint64_t	            HardWaits;                                                                   //  Count of waits on the soft mutex
		uint64_t			    HardWaitQuanta;                                                              //  Count of wait quanta on the soft mutex
		uint64_t		        HardLocks;                                                                   //  Count of times the soft mutex was acquired

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  initialise
		//
		//  The initialise function performs an in place initialiseation of the Mutex internals.
		//
		//  PARAMETERS
		//
		//        const int          -        Wait quantum size in milliseconds
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void initialise(const int WQSize)
		{
			Mutex = cMutexFree;
			SoftLocksHeld = 0;
			WaitQuantum = MILLISECONDS(WQSize);
			SoftLocks = 0;
			SoftWaits = 0;
			SoftWaitQuanta = 0;
			HardLocks = 0;
			HardWaits = 0;
			HardWaitQuanta = 0;
			return;
		}

		//  softlock
		//
		//  Acquire the lock on behalf of the caller (identified by THREADID). The function blocks while another thread is holding the hard mutex.
		//
		//  PARAMETERS
		//
		//        const THREADID        -        ID of the thread requesting the mutex
		//
		//  RETURNS
		//
		//  NOTES:
		//
		//		1.	Acquire the soft lock before the hard lock.
		//

		void	softlock(THREADID id) {
			unsigned int    WaitCount = 0;
			bool			Acquired = false;

			//  Acquire the sodt-lock
			while (!Acquired) {
				//  Prevent deadly-self-embrace
				if (Mutex.load() != id) {
					//  Block while the hard lock mutex is held
					while (isHardLocked()) {
						WaitCount++;
						sleep(WaitQuantum);
					}
				}

				//  Increment the count of holders
				SoftLocksHeld.fetch_add(1);

				//  Recheck the hard lock and backout the softlock if now asserted
				if (Mutex.load() == id) Acquired = true;
				else {
					if (isHardLocked()) SoftLocksHeld.fetch_sub(1);
					else Acquired = true;
				}
			}

			//  Update statistics
			SoftLocks.fetch_add(1);
			if (WaitCount > 0) {
				SoftWaits.fetch_add(1);
				SoftWaitQuanta.fetch_add(WaitCount);
			}
			return;
		}

		//  softunlock
		//
		//  Releases the soft lock on behalf of the caller (identified by THREADID). 
		//
		//  PARAMETERS
		//
		//        const THREADID        -        ID of the thread requesting the mutex
		//
		//  RETURNS
		//
		//  NOTES:
		// 

		void	softunlock(THREADID id) {
			id = id;
			//  Decrement the count of holders of the soft lock
			if (!isSoftLocked()) return;
			SoftLocksHeld.fetch_sub(1);
			return;
		}

		//  hardlock
		//
		//  Acquire the hard mutex on behalf of the caller (identified by THREADID). The function blocks while another thread is holding the hard mutex
		//  or there are multiple holders of the soft lock. If the mutex is already held by the requesting thread then the call just returns. 
		//
		//  PARAMETERS
		//
		//        THREADID        -        ID of the thread requesting the mutex
		//
		//  RETURNS
		//
		//  NOTES:
		// 
		//		1.		Be sure to hold the soft lock before attempting to acquire the hard mutex
		//

		void	hardlock(THREADID id) {
			unsigned int    WaitCount = 0;
			THREADID        Expected = cMutexFree;
			bool			Acquired = false;

			//  Keep attempting the lock until it is acquired
			while (!Acquired) {

				//  The hard mutex may only be acquired while there are no callers holding the soft lock
				while (!isSoftExclusive()) {
					WaitCount++;
					sleep(WaitQuantum);
				}

				while (Mutex.load() != id)
				{
					Expected = cMutexFree;
					if (!Mutex.compare_exchange_weak(Expected, id))
					{
						WaitCount++;
						sleep(WaitQuantum);
					}
				}

				//  Recheck the soft lock - if that is not in the exclusive state (no holders) then back out the hard lock and
				//  retry

				if (isSoftExclusive()) Acquired = true;
				else hardunlock(id);
			}

			//  NOTE:
			//  The wait counters do not need to be atomics as they are protected by the mutex itself. i.e. they are only updated by the
			//  thread currently holding the mutex.

			HardLocks++;
			if (WaitCount > 0)
			{
				HardWaits++;
				HardWaitQuanta += WaitCount;
			}

			return;
		}

		//  hardunlock
		//
		//  Releases the hard mutex on behalf of the caller (identified by THREADID). 
		//
		//  PARAMETERS
		//
		//        const THREADID        -        ID of the thread requesting the mutex
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	hardunlock(THREADID id) {
			Mutex.compare_exchange_weak(id, cMutexFree);
			return;
		}

		//  isSoftLocked
		//
		//  Tests if the soft lock is held.
		//
		//  PARAMETERS
		//
		//        bool        -        true if the soft lock is held otherwise false
		//
		//  RETURNS
		//
		//  NOTES:
		//

		bool isSoftLocked()
		{
			if (SoftLocksHeld.load() == 0) return false;
			return true;
		}

		//  isSoftExclusive
		//
		//  Tests if the soft lock is held by a single caller
		//
		//  PARAMETERS
		//
		//        bool        -        true if the soft lock is held by a single callerotherwise false
		//
		//  RETURNS
		//
		//  NOTES:
		//

		bool isSoftExclusive() {
			if (SoftLocksHeld.load() == 0) return true;
			return false;
		}

		//  isHardLocked
		//
		//  Tests if the hard lock mutex is held.
		//
		//  PARAMETERS
		//
		//        bool        -        true if the hard lock mutex is held otherwise false
		//
		//  RETURNS
		//
		//  NOTES:
		//

		bool isHardLocked()
		{
			if (Mutex.load() == cMutexFree) return false;
			return true;
		}

	private:
		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members                                                                                               *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		std::atomic<THREADID>           Mutex;                                                              //  Internal mutex implementation
		std::atomic<int>				SoftLocksHeld;														//  Soft lock control

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

	};


	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   SEMAPHORE Template Class                                                                                      *
	//*                                                                                                                 *
	//*   Objects of this class, implement a value based signalling mechanism for inter-thread communication.           *
	//*   Semaphores can implement a one-way communication (in which case they are bistable) or a two-way               *
	//*   request/response model (in which case they are tristable).													*
	//*   The usual use model is to have a per-thread semaphore for requests, these are polled by the request handler   *
	//*   and then serviced by the request handler (and optionally responded to).										*
	//*                                                                                                                 *
	//*   PROPERTIES:                                                                                                   *
	//*                                                                                                                 *
	//*     NOT COPYABLE                                                                                                *
	//*     NOT MOVEABLE                                                                                                *
	//*     NOT ASSIGNABLE                                                                                              *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	template <typename T>
	class SEMAPHORE
	{
	public:
		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Default constructor
		SEMAPHORE()
		{
			//  Initialise the semaphore
			initialise(1, 1, 1);
		}

		//  Constructor for a specific wait quantum for all values
		SEMAPHORE(const int usWQ)
		{
			//  Initialise the semaphore
			initialise(usWQ, usWQ, usWQ);
		}

		//  Constructor for a specific wait quantum for each value - sizes are in micro-seconds
		SEMAPHORE(const int PWQSize, const int AWQSize, const int RWQSize)
		{
			//  Initialise the semaphore
			initialise(PWQSize, AWQSize, RWQSize);
		}

		//
		//  Mark the class as NOT copyable/moveable/assignable
		//

		//  Copy and move constructors
		SEMAPHORE(const SEMAPHORE&) = delete;
		SEMAPHORE(const SEMAPHORE&&) = delete;

		//  Assignment operators
		SEMAPHORE& operator = (const SEMAPHORE&) = delete;
		SEMAPHORE& operator = (const SEMAPHORE&&) = delete;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor                                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		~SEMAPHORE() {}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		static const THREADID   cSemaphoreFree = 0xFFFFFFFF;												//  Semaphore is available (unsignalled)

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Members                                                                                                *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		MILLISECONDS        PostWaitQuantum;                                                                 //  Wait time for posting the semaphore
		MILLISECONDS        AcceptWaitQuantum;                                                               //  Wait time for accepting a semaphore value
		MILLISECONDS        RespondWaitQuantum;                                                              //  Wait time for a response to a semaphore post
		uint64_t		    Posts;                                                                           //  Post count
		uint64_t	        PostWaits;                                                                       //  Counts of waits for posting
		uint64_t	        PostWaitQuanta;                                                                  //  Duration of waits for posting
		uint64_t	        Accepts;                                                                         //  Accept count
		uint64_t	        Requests;                                                                        //  Request count
		uint64_t	        Responds;                                                                        //  Respond count
		volatile int        ResponseCode;                                                                    //  Return code response

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  initialise
		//
		//  The initialise function will initialise the semaphore in place.
		//
		//  PARAMETERS
		//
		//        const int              -        Post wait quantum in milli-seconds
		//        const int              -        Accept wait quantum in milli-seconds
		//        const int              -        Respond wait quantum in milli-seconds
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void initialise(const int PWQSize, const int AWQSize, const int RWQSize)
		{
			Semaphore.store(cSemaphoreFree);
			PostWaitQuantum = MILLISECONDS(PWQSize);
			AcceptWaitQuantum = MILLISECONDS(AWQSize);
			RespondWaitQuantum = MILLISECONDS(RWQSize);
			Posts = 0;
			PostWaits = 0;
			PostWaitQuanta = 0;
			ResponseCode = 0;
			Accepts = 0;
			Requests = 0;
			Responds = 0;
			return;
		}

		//  post
		//
		//  Posts the supplied value to the semaphore. The call will block until the semaphore is free.
		//
		//  PARAMETERS
		//
		//        const T&              -        Reference to the value to be posted 
		//        const THREADID        -        ID of the thread posting the value
		//
		//  RETURNS
		//
		//  NOTES:
		//
		void post(const T& PV, THREADID id)
		{
			unsigned int    WaitCount = 0;

			//  First wait for the semaphore to transition to the free state, a thread will block if it already has a value posted to the sempahore
			while (Semaphore.load() != cSemaphoreFree)
			{
				WaitCount++;
				sleep(PostWaitQuantum);
			}

			//  Post the value
			Value = PV;

			//  Signal that the semaphore is posted
			Semaphore.store(id);

			Posts++;
			if (WaitCount > 0)
			{
				PostWaits++;
				PostWaitQuanta += WaitCount;
			}
			return;
		}

		//  tryPosting
		//
		//  Attempts to post the supplied value to the semaphore. The call will return false if the value could not be posted because the samaphore is busy.
		//
		//  PARAMETERS
		//
		//        const T&              -        Reference to the value to be posted 
		//        const THREADID        -        ID of the thread posting the value
		//
		//  RETURNS
		//
		//        bool        -  true if the value was posted otherwise false
		//
		//  NOTES:
		//
		bool tryPosting(const T& PV, THREADID id)
		{
			if (Semaphore.load() != cSemaphoreFree) return false;
			//  Post the value
			Value = PV;

			//  Signal that the semaphore is posted
			Semaphore.store(id);
			Posts++;
			return true;
		}

		//  request
		//
		//  Posts the semaphore and waits for a response, the response consits of the value and a response code.
		//
		//  PARAMETERS
		//
		//        T&                    -        Reference to the value requested
		//        const THREADID        -        ID of the thread posting the value
		//
		//  RETURNS
		//
		//        int            -  The response code posted by the responder.
		//
		//  NOTES:
		//
		int request(T& PV, THREADID id)
		{
			unsigned int    WaitCount = 0;

			//  First wait for the semaphore to transition to the free state, a thread will block if it already has a value posted to the sempahore
			Requests++;
			while (Semaphore.load() != cSemaphoreFree)
			{
				WaitCount++;
				sleep(PostWaitQuantum);
			}

			//  Post the semaphore
			Value = PV;
			Semaphore.store(id);
			Posts++;
			if (WaitCount > 0)
			{
				PostWaits++;
				PostWaitQuanta += WaitCount;
			}

			//  Wait for the semaphore to be cleared indicating that response has been delivered
			while (Semaphore.load() != cSemaphoreFree) sleep(RespondWaitQuantum);
			PV = Value;

			//  Return the response code
			return ResponseCode;
		}

		//  accept
		//
		//  Accepts a value posted to the semaphore and clears the semaphore, the call will block until the semaphore is posted.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//        T        -  The value posted to the semaphore
		//
		//  NOTES:
		//
		T accept()
		{
			T                ReturnValue;
			Accepts++;
			while (Semaphore.load() == cSemaphoreFree) sleep(AcceptWaitQuantum);
			ReturnValue = Value;
			Semaphore.store(cSemaphoreFree);
			return ReturnValue;
		}

		//  isPosted
		//
		//  Tests if the semaphore is posted.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//        bool        -  true if the semaphore is posted otherwise false.
		//
		//  NOTES:
		//
		bool isPosted()
		{
			if (Semaphore.load() == cSemaphoreFree) return false;
			return true;
		}

		//  peekValue
		//
		//  Returns a reference to the value being carried by the semaphore
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//        const T&        -  Constant reference to the value posted to the semaphore
		//
		//  NOTES:
		//
		const T& peekValue()
		{
			return (const T&)Value;
		}

		//  respond
		//
		//  Posts a value and return code to the semaphore in response to a request
		//
		//  PARAMETERS
		//
		//        const T&               -        Reference to the value to be posted
		//        int                    -        The response code to be posted 
		//
		//  RETURNS
		//
		//  NOTES:
		//
		void respond(const T& PV, int RC)
		{
			Responds++;
			while (Semaphore.load() == cSemaphoreFree) sleep(RespondWaitQuantum);
			ResponseCode = RC;
			Value = PV;
			Semaphore.store(cSemaphoreFree);
			return;
		}

	private:
		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members                                                                                               *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		std::atomic<THREADID>        Semaphore;                                                        //  Internal semaphore
		T                            Value;                                                            //  Semaphore value

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

	};

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   LATCH Class                                                                                                   *
	//*                                                                                                                 *
	//*   Objects of this class, implement a lightweight latch mechanism.												*
	//*                                                                                                                 *
	//*   PROPERTIES:                                                                                                   *
	//*                                                                                                                 *
	//*     NOT COPYABLE                                                                                                *
	//*     NOT MOVEABLE                                                                                                *
	//*     NOT ASSIGNABLE                                                                                              *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	class LATCH {
	public:
		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Default Constructor
		LATCH() {
			//  Clear the counters
			UnlatchedWaits = 0;
			UnlatchWaits = 0;
			UnlatchWaitQuanta = 0;
			UnlatchCount = 0;
			LatchCount = 0;
			PeekCount = 0;

			//  Set the default wait interval
			WaitQuantum = MILLISECONDS(3);

			//  Clear the latch to the unlatched state
			LatchValue = cDefaultUnlatchCode;

			//  Return to caller
			return;
		}

		//  Constructor setting an explicit wait quantum
		LATCH(MILLISECONDS WQS) {
			//  Clear the counters
			UnlatchedWaits = 0;
			UnlatchWaits = 0;
			UnlatchWaitQuanta = 0;
			UnlatchCount = 0;
			LatchCount = 0;
			PeekCount = 0;

			//  Set the default wait interval
			WaitQuantum = WQS;

			//  Clear the latch to the unlatched state
			LatchValue = cDefaultUnlatchCode;

			//  Return to caller
			return;
		}

		//
		//  Mark the class as NOT copyable/moveable/assignable
		//

		LATCH(const LATCH&) = delete;
		LATCH(LATCH&&) = delete;

		LATCH& operator = (const LATCH&) = delete;
		LATCH& operator = (LATCH&&) = delete;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor                                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		~LATCH() {

		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		static const int	cStateLatched = 0;																//  Latch is in the "latched" state
		static const int	cDefaultUnlatchCode = 200;														//  Default unlatch code

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Members                                                                                                *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		MILLISECONDS        WaitQuantum;			                                                         //  Wait time for the latch
		uint64_t	        UnlatchedWaits;                                                                  //  Counts of waits for an already unlatched latch
		uint64_t	        UnlatchWaits;                                                                    //  Counts of waits for the latch to become unlatched
		uint64_t	        UnlatchWaitQuanta;                                                               //  Duration of waits for unlatching
		uint64_t		    LatchCount;				                                                         //  Counts of latches
		uint64_t	        UnlatchCount;				                                                     //  Counts of unlatches
		uint64_t	        PeekCount;				                                                         //  Counts of peeks

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  wait
		//
		//  Blocks the calling thread until the lach becomes unlatched
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		int			- the unlatch code
		//
		//  NOTES:
		//

		int		wait() {
			unsigned int        WaitQuanta = 0;																//  Duration of waits for unlatching
			int					LatchReturn = LatchValue;													//  Copy of the current latch value

			//
			//  If the latch is already unlatched then just return the unlatch code
			//
			if (LatchReturn != cStateLatched) {
				UnlatchedWaits++;
				return LatchReturn;
			}

			//
			//  Wait for the latch to become unlatched
			//

			while (LatchValue == cStateLatched) {
				WaitQuanta++;
				sleep(MILLISECONDS(WaitQuantum));
			}
			LatchReturn = LatchValue;

			if (WaitQuanta > 0) {
				UnlatchWaits++;
				UnlatchWaitQuanta += WaitQuanta;
			}

			//  Return the unlatch code
			return LatchReturn;
		}

		//  peek
		//
		//  Returns the current value of the latch
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		int			- the unlatch code
		//
		//  NOTES:
		//

		int		peek() {
			PeekCount++;
			return LatchValue;
		}

		//  latch
		//
		//  Sets the state of the latch to the latched state
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	latch() {
			LatchCount++;
			if (LatchValue == cStateLatched) return;
			LatchValue = cStateLatched;
			return;
		}

		//  unlatch
		//
		//  Clears the state of the latch with the default response code
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	unlatch() {
			//  If the latch is already unlatched then do nothing
			if (LatchValue != cStateLatched) return;

			//  Unlatch the latch
			UnlatchCount++;
			LatchValue = cDefaultUnlatchCode;
			return;
		}

		//  unlatch
		//
		//  Clears the state of the latch with the passed response code
		//
		//  PARAMETERS
		//
		//		Response code with which to clear the latch
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	unlatch(int Response) {
			//  If the latch is already unlatched then do nothing
			if (LatchValue != cStateLatched) return;

			//  Unlatch the latch
			UnlatchCount++;
			LatchValue = Response;
			return;
		}

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members                                                                                               *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		volatile int	LatchValue;

	};

}
