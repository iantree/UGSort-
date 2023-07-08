#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       ThreadPool.h																						*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.0.2	(Build: 03)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the ThreadPool class. The ThreadPool class provides the			*
//* the server component that delivers all Muti-Programming kernel functionality using shared threads.				* 
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
//*	1.0.0 -		13/12/2017	-	Initial Release																		*
//*	1.0.1 -		28/10/2022	-	Fixed incorrect clearing of tasks executed counter on thread restart.				*
//*							-	Reset monitor cycle after autonomous add worker										*
//*							-	Worker start/stop monitoring														*
//*	1.0.2 -		30/10/2022	-	Autonomics made configurable.														*
//*																													*
//*******************************************************************************************************************/

//  Include xymorg headers
#include	"../LPBHdrs.h"																			//  Language and Platform base headers
#include	"../types.h"																			//  xymorg type definitions
#include	"../consts.h"																			//  xymorg constant definitions

//  Required sub-systems/components
#include	"../Logging.h"																			//  Logging queue
#include	"MPQueues.h"																			//  MP Queues
#include	"Worker.h"																				//  Worker thread
#include	"../StringPool.h"																		//  String Pool
#include	"../VRMapper.h"																			//  Virtual Resource Mapper

//
//  All components are defined within the xymorg namespace
//
namespace xymorg {

	//
	//  ThreadPool Class Definition
	//

	class ThreadPool {
	public:
		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Constructor 
		//
		//  Constructs a Dispatche object for a given thread
		//
		//  PARAMETERS:
		//
		//		LogQueue&		-		Reference to the system log queue
		//		MPQueues&		-		Reference to the MP Queues singleton object
		//		StringPool&		-		Reference to the application level string pool
		//		VRMapper&		-		Reference to the resource mapper
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		ThreadPool(LogQueue& LQ, MPQueues& MPQ, StringPool& ALSP, VRMapper& VRM) : MyLQ(LQ), MyQueues(MPQ), SPool(ALSP), VRMap(VRM) {

			//  Set the initial state
			State = cNotStarted;
			MinWorkers = 3;
			MaxWorkers = XY_MAX_THREADS - 1;
			CurrentWorkers = 0;
			TTX = 0;
			DNW = 0;
			RQT = 0;
			TEPI = 0;

			//  Enable Thread Exhaustion Protection by default
			TEPEnabled = true;

			//  Enable Wait for Rejoins and Delayed tasks by default
			WFREnabled = true;
			WFDEnabled = true;

			//  Set the autonomics cycle triggers
			aActionTrigger = 10;
			aMonitorTrigger = 20;

			//  Clear autonomics stats and counters
			aEnabled = true;
			aMCycles = 20;
			aACycles = 10;
			aTicker = 0;
			aPXQSize = 0;
			aLastPXQSize = 0;
			aAvailableThreads = 0;
			aLastAvailableThreads = 0;
			aThreadsStarted = 0;
			aThreadsStopped = 0;

			//  Clear the Thread Control Blocks to the ground state
			for (int tIndex = 0; tIndex < XY_MAX_THREADS; tIndex++) {
				MyWorkers[tIndex].pWorker = nullptr;
				MyWorkers[tIndex].pTask = nullptr;
				MyWorkers[tIndex].TID = 0;
				MyWorkers[tIndex].TX = 0;
				MyWorkers[tIndex].Stops = 0;
				MyWorkers[tIndex].StartedAt = 0;
				MyWorkers[tIndex].ActiveTicks = 0;
			}

			//  Return to caller
			return;
		}

		//
		//  Mark the object as not copyable/moveable/assignable
		//

		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;

		ThreadPool& operator = (const ThreadPool&) = delete;
		ThreadPool& operator = (ThreadPool&&) = delete;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor                                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Destructor
		//
		//  Destroys the Dispatcher object, destroying the associated AppLog
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		~ThreadPool() {

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

		//  run
		//
		//  Main running loop for the ThreadPool, this should be invoked on a dedicated thread.
		//  The running loop provides all kernel services for Multi-Programming MP.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void run() {
			int			AvailableThreads = 0;																	//  Number of threads available dor dispatch
			int			FirstAvailableThread = 0;																//  Index of the first available thread
			int			NewTasks = 0;																			//  Number of new tasks added to the queue
			int			DisposedTasks = 0;																		//  Number of disposed tasks
			int			DispatchedTasks = 0;																	//  Number of tasks dispatched for execution
			int			RejoinedTasks = 0;																		//  Number of tasks that were rejoined
			void* Owners[XY_MAX_THREADS] = {};															//  Array identifying the owners of executing TASKS

			//  Start the minimum number of workers in the pool
			for (int tIndex = 0; tIndex < MinWorkers; tIndex++) {
				//  Add a new worker to the pool
				if (addWorker(tIndex)) CurrentWorkers++;
			}

			//  Transition the state to active
			State = cActive;

			//  Loop until requested to drain
			while (State != cDrained) {

				//
				//  1.  Clear the thread task dispose semaphores, this will make threads available for new tasks.
				//      The disposition of the task is signalled in the semaphore.
				//
				DisposedTasks = 0;
				FirstAvailableThread = 0;
				while (FirstAvailableThread != -1) {
					//  Dispose of the next task
					FirstAvailableThread = MyQueues.disposeOldTasks();
					if (FirstAvailableThread != -1) {
						//  Clear the thread to make it available
						MyWorkers[FirstAvailableThread].pTask = nullptr;
						DisposedTasks++;
					}
				}

				//
				//  2.  Move any delayed tasks that are now ready to process to the dispatch queue
				//
				MyQueues.requeueDelayedTasks();

				//
				//  3.  Process all of the task dispatch semaphores, adding the new tasks to the dispatch queue
				//
				NewTasks = MyQueues.enqueueNewTasks();

				//
				//  4.  If there is work to do and threads available then dispatch tasks from the queue to the available threads
				//
				DispatchedTasks = dispatchTasks();

				//
				//  5.  Service any requests to get completed tasks (rejoins)
				//

				//  Prepare the array of owners with executing threads

				for (int tIndex = 0; tIndex < XY_MAX_THREADS; tIndex++) {
					if (MyWorkers[tIndex].pWorker != nullptr && MyWorkers[tIndex].pTask != nullptr) {
						Owners[tIndex] = MyWorkers[tIndex].pTask->Owner;
					}
					else Owners[tIndex] = nullptr;
				}

				RejoinedTasks = MyQueues.dequeueOldTasks(Owners);

				//
				//  6.  Perform Autonomic actions -- if the thread pool is still active
				//
				if (State == cActive) performAutonomics();

				//
				//  7.  Rest or drain the thread pool
				//

				//  If no work was performed on this cycle then snooze for a bit
				int WorkDone = DispatchedTasks + NewTasks + DispatchedTasks + RejoinedTasks;
				if (WorkDone == 0) {
					//  If the thread pool is active then just rest for a bit
					if (State == cActive) {
						DNW++;
						sleep(MILLISECONDS(5));
					}
					else {
						//  The pool is draining - see if we can now transition to the drained state
						//  If not then move the draining process along a bit.

						//  If there is work still pending then rest a bit
						int		QWork = MyQueues.getPendingExecutionCount();
						if (WFREnabled) QWork += MyQueues.getPendingRejoinCount();
						else {
							//  Empty the rejoin queue
							if (MyQueues.getPendingRejoinCount() > 0) MyQueues.emptyRejoinQueue();
						}
						if (WFDEnabled) QWork += MyQueues.getDelayedCount();
						else {
							//  Empty the delayed task queue
							if (MyQueues.getDelayedCount() > 0) MyQueues.emptyDelayedQueue();
						}

						if (QWork > 0) {
							DNW++;
							sleep(MILLISECONDS(5));
						}
						else {
							//  Count the number of available threads and executing threads
							int ExecutingThreads = 0;
							AvailableThreads = 0;
							FirstAvailableThread = -1;

							for (int tIndex = 0; tIndex < XY_MAX_THREADS; tIndex++) {
								if (MyWorkers[tIndex].pWorker != nullptr && MyWorkers[tIndex].pTask == nullptr) {
									AvailableThreads++;
									if (FirstAvailableThread == -1) FirstAvailableThread = tIndex;
								}
								else if (MyWorkers[tIndex].pWorker != nullptr) ExecutingThreads++;
							}

							//  If the count of executing threads is zero and available threads is zero then mark the state as drained
							if (ExecutingThreads == 0 && AvailableThreads == 0) State = cDrained;
							else {
								//  If the executing count is zero - shutdown the first avaiable thread
								if (ExecutingThreads == 0) {
									removeWorker(FirstAvailableThread);
								}
								else {
									DNW++;
									sleep(MILLISECONDS(5));
								}
							}
						}
					}
				}
			}

			//  Return to caller
			return;
		}

		//  drain
		//
		//  Signal the thread pool to drain, it should empty the Pending Execution queue and drain all of the workers.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	drain() {
			//  Set the draining state
			State = cDraining;

			//  Return to caller
			return;
		}

		//  enable/disableTEP
		//
		//  Enables Thread Exhaustion Protection
		//	Disabled Thread Exhaustion Protection
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	enableTEP() { TEPEnabled = true; return; }
		void	disableTEP() { TEPEnabled = false; return; }

		//  enable/disableWFR
		//
		//  Enables Wait for rejoins (On drain)
		//	Disabled Wait for rejoins (On drain)
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	enableWFR() { WFREnabled = true; return; }
		void	disableWFR() { WFREnabled = false; return; }

		//  enable/disableWFD
		//
		//  Enables Wait for delayed (On drain)
		//	Disabled Wait for delayed (On drain)
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	enableWFD() { WFDEnabled = true; return; }
		void	disableWFD() { WFDEnabled = false; return; }

		//  waitUntilEmpty
		//
		//  This call will block until all activity in the threadpool has completed.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		void	waitUntilEmpty() {

			//  If the threadpool is already drained then return
			if (State == cDrained) return;

			//  Wait while the threadpool is still busy with work
			while (isBusy()) sleep(MILLISECONDS(5));

			//  Return to caller
			return;
		}

		//  showStats
		//
		//  Show that statistics from the ThreadPool and the MP Queues
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	showStats(std::ostream& Log) {
			uint64_t		ActiveTicks = 0;

			//  Show the configuration
			Log << "TP CONFIG: Min Threads: " << MinWorkers << ", Max Threads: " << MaxWorkers << ", Thread Exhaustion Prevention (TEP): ";
			if (TEPEnabled) Log << "ON";
			else Log << "OFF";
			Log << ", Wait For Rejoins (WFR): ";
			if (WFREnabled) Log << "ON";
			else Log << "OFF";
			Log << ", Wait For Delayed (WFD): ";
			if (WFDEnabled) Log << "ON";
			else Log << "OFF";
			Log << "." << std::endl;

			//  Show the queue stats
			MyQueues.showStats(true, Log);

			//  Show the activity stats from the thread pool
			Log << "TP: Tasks Executed: " << TTX << ", Tasks Requeued: " << RQT << ", dwells because no work could be done: " << DNW << "." << std::endl;
			if (TEPI > 0) Log << "TP: Thread Exhaustion Protection (TEP) interventions: " << TEPI << "." << std::endl;
			if (aEnabled) {
				Log << "AUTONOMICS: Monitor Cycles: " << aMCycles << ", Action Cycles: " << aACycles << "." << std::endl;
				Log << "AUTONOMICS: Additional threads started: " << aThreadsStarted << ", surplus threads stopped: " << aThreadsStopped << "." << std::endl;
			}
			else Log << "AUTONOMICS: Function is disabled." << std::endl;
			//  Show the per thread statistics
			for (int tIndex = 0; tIndex < XY_MAX_THREADS; tIndex++) {
				if (MyWorkers[tIndex].TX > 0) {
					Log << "WORKER: [" << MyWorkers[tIndex].TID << "] - ";
					if (MyWorkers[tIndex].pWorker == nullptr) Log << "STOPPED";
					else {
						if (MyWorkers[tIndex].pTask == nullptr) Log << "IDLE";
						else Log << "BUSY";
					}
					Log << ", Tasks Executed: " << MyWorkers[tIndex].TX;
					Log << ", Sessions: " << MyWorkers[tIndex].Stops;
					ActiveTicks = MyWorkers[tIndex].ActiveTicks;
					//  If the task is running add the current session
					if (MyWorkers[tIndex].pWorker != nullptr) ActiveTicks += (aTicker - MyWorkers[tIndex].StartedAt);
					Log << ", Active Ticks: " << ActiveTicks << "." << std::endl;
				}
			}

			// Return to caller
			return;
		}

		//  setWorkers
		//
		//  Set the Min and Max number of workers to run
		//
		//  PARAMETERS
		//
		//		int			-		Minimum number of workers
		//		int			-		Maximum number of workers
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	setWorkers(int Minw, int Maxw) {

			//  Set policy with default states
			setPolicy(Minw, Maxw, true, true, true);

			//  Return to caller
			return;
		}

		//  setAutonomics
		//
		//  Enable/Disable autonomics and set the monitor and action cycle counts
		//
		//  PARAMETERS
		//
		//		bool		-		Enabled/Disabled
		//		int			-		Monitor Cycles
		//		int			-		Action Cycles
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	setAutonomics(bool Enabler, int NewMCycles, int NewACycles) {
			aEnabled = Enabler;
			aMCycles = NewMCycles;
			aACycles = NewACycles;
			return;
		}

		//  setPolicy
		//
		//  Set the Min and Max number of workers to run, TEP, WFR and WFD enabled states
		//
		//  PARAMETERS
		//
		//		int			-		Minimum number of workers
		//		int			-		Maximum number of workers
		//		bool		-		Thread Exhaustion Policy (TEP) enabled state
		//		bool		-		Wait For Rejoins (WFR) enabled state
		//		bool		-		Wait for Delayed (WFD) enabled state
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	setPolicy(int Minw, int Maxw, bool TEPState, bool WFRState, bool WFDState) {
			MinWorkers = Minw;
			MaxWorkers = Maxw;
			TEPEnabled = TEPState;
			WFREnabled = WFRState;
			WFDEnabled = WFDState;

			//  Return to caller
			return;
		}

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Structures																							*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  TCB
		//
		//  The Thread Control Block (TCB) structure holds information about a single worker thread in the ThreadPool
		//

		typedef struct TCB {
		public:
			THREADID			TID;																//  Identity of the worker thread
			size_t				TX;																	//  Tasks executed counts
			Worker*				pWorker;															//  Pointer to the worker object for the thread
			TASK*				pTask;																//  Pointer to the task being executed by the worker
			std::thread			WT;																	//  Thread on which this worker is running
			size_t				Stops;																//  Stop count
			uint64_t			StartedAt;															//  Ticker value worker was last started
			uint64_t			ActiveTicks;														//  Number of active ticks

			TCB() : TID(0), TX(0), pWorker(nullptr), pTask(nullptr), Stops(0), StartedAt(0), ActiveTicks(0) {};
		} TCB;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Constants																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  References to be published by the dispatcher
		StringPool& SPool;																	//  Application level string pool
		VRMapper& VRMap;																	//  Resource Mapper

		//  ThreadPool States
		static const int	cNotStarted = 0;
		static const int	cActive = 1;
		static const int	cDraining = 2;
		static const int	cDrained = 3;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Policy Controls
		int				MinWorkers;																	//  Minimum number of workers
		int				MaxWorkers;																	//  Maximum number of workers
		bool			TEPEnabled;																	//  Thread Exhaustion Protection enabled
		bool			WFREnabled;																	//  On Drain - Wait for rejoins enabled
		bool			WFDEnabled;																	//  On Drain - Wait for delayed tasks enabled

		volatile int	State;																		//  State of the Threadpool
		LogQueue&		MyLQ;																		//  Log Queue
		MPQueues&		MyQueues;																	//  MP Queues
		int				CurrentWorkers;																//  Number of workers (active) in the thread pool
		TCB				MyWorkers[XY_MAX_THREADS];													//  Worker thread control blocks

		//  Statistics
		size_t			TTX;																		//  Total tasks executed
		size_t			DNW;																		//  Dwell (no work) waits
		size_t			RQT;																		//  Requeued task count
		size_t			TEPI;																		//  Thread Exhaustion Protection interventions

		//  AUTONOMICS controls and counters
		bool			aEnabled;																	//  Autonomics enabled control
		int				aMCycles;																	//  Monitor cycles (per action cycle)
		int				aACycles;																	//  Action cycles (per real action)
		uint64_t		aTicker;																	//  Cycle counter
		size_t			aMonitorTrigger;															//  Monitor cycle trigger
		size_t			aActionTrigger;																//  Action cycle trigger
		size_t			aPXQSize;																	//  Current size of the pending execution queue
		size_t			aLastPXQSize;																//  Previous size of the pending exescution queue
		size_t			aAvailableThreads;															//  Current count of available threads
		size_t			aLastAvailableThreads;														//  Previous count of available threads
		size_t			aThreadsStarted;															//  Count of additional threads started
		size_t			aThreadsStopped;															//  Count of surplus threads stopped

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  isBusy
		//
		//  This function will return a signal indicating that there is work in the threadpool queues or workers
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		bool			-			true if there is work in the threadpool, otherwise false
		//
		//  NOTES:
		//

		bool	isBusy() {

			//  If the pool is already drained then signal NOT BUSY
			if (State == cDrained) return false;

			//  If there are tasks pending execution then signal BUSY
			if (MyQueues.getPendingExecutionCount() > 0) return true;

			//  If there are rejoin tasks waiting then signal BUSY
			if (MyQueues.getPendingRejoinCount() > 0) return true;

			//  If there are delayed tasks waiting then signal BUSY
			if (MyQueues.getDelayedCount() > 0) return true;

			//  Count the number of worker threads that are currently executing tasks
			int		ExecutingThreads = 0;

			for (int tIndex = 0; tIndex < XY_MAX_THREADS; tIndex++) {
				if (MyWorkers[tIndex].pWorker != NULL && MyWorkers[tIndex].pTask != NULL) ExecutingThreads++;
			}

			//  If the count is greater than zero signal BUSY
			if (ExecutingThreads > 0) return true;

			//  Recheck the queues
			if (MyQueues.getPendingExecutionCount() > 0) return true;
			if (MyQueues.getPendingRejoinCount() > 0) return true;
			if (MyQueues.getDelayedCount() > 0) return true;

			//  Show that the threadpool is NOT BUSY
			return false;
		}

		//  dispatchTasks
		//
		//  This function will dispatch tasks from the pending execution queue to the available worker threads
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		int			-			the number of tasks dispatched
		//
		//  NOTES:
		//

		int		dispatchTasks() {
			int			TasksDispatched = 0;																	//  Number of tasks dispatched
			int			AvailableThreads = 0;																	//  Number of threads available dor dispatch
			int			FirstAvailableThread = -1;																//  Index of the first available thread
			std::vector<TASK*>	ParkedTasks;																	//  Parking for inhibited tasks

			for (int tIndex = 0; tIndex < XY_MAX_THREADS; tIndex++) {
				if (MyWorkers[tIndex].pWorker != nullptr && MyWorkers[tIndex].pTask == nullptr) {
					AvailableThreads++;
					if (FirstAvailableThread == -1) FirstAvailableThread = tIndex;
				}
			}

			//  Loop dispatching tasks as long as there is work to do and threads available
			while (MyQueues.getPendingExecutionCount() > 0 && AvailableThreads > 0) {
				//  Dispatch the next task into the first available worker thread
				//  If the task is flagged for redezvous then it can only be dispatched if there are no
				//  child tasks still in the pending execution queue or currently executing
				TASK* Candidate = nullptr;

				while (Candidate == nullptr) {
					Candidate = MyQueues.getNextTaskToExecute();

					//  Determine if the task can be executed
					if (Candidate->Disposition & TASK::cTaskRendezvous) {
						//  This task is to rendezvous - check that all of the sub-tasks owned by it are ready
						for (int tIndex = 0; tIndex < XY_MAX_THREADS; tIndex++) {
							if (MyWorkers[tIndex].pWorker != nullptr && MyWorkers[tIndex].pTask != nullptr) {
								if (MyWorkers[tIndex].pTask->Owner == Candidate) {
									//  Park the task and try the next one
									ParkedTasks.push_back(Candidate);
									Candidate = nullptr;
								}
							}
						}

						//  Check that there are no tasks waiting on the pending execution queue for the owner
						if (MyQueues.hasTasksOnXQ(Candidate)) {
							//  Park the task and try the next one
							ParkedTasks.push_back(Candidate);
							Candidate = nullptr;
						}
					}

					//
					//  Thread Exhaustion Prevention (TEP) filter
					//
					//	If enabled and we are dispatching to the last possible thread then we will filter out Generator and possible UnitOfWork
					//  tasks in favour of tasks from lower in the hierarchy.
					//

					if (TEPEnabled && Candidate != nullptr) {
						if (AvailableThreads == 1 && CurrentWorkers == MaxWorkers) {
							//  TEP filtering has been activated
							if (Candidate->Character & TASK::cGeneratorTask || ((Candidate->Character & TASK::cUnitOfWork) && MyQueues.hasSubTasksOnXQ())) {
								//  Generator tasks and if there are sub-tasks Unit Of Work tasks are filtered out
								ParkedTasks.push_back(Candidate);
								Candidate = nullptr;
								TEPI++;
							}
						}
					}

					//  If we have exhausted the pending execution queue then leave gracefully
					if (Candidate == nullptr && MyQueues.getPendingExecutionCount() == 0) {
						//  Re-instate any parked tasks to the queue
						if (ParkedTasks.size() > 0) {
							for (std::vector<TASK*>::iterator itPT = ParkedTasks.begin(); itPT != ParkedTasks.end(); itPT++) {
								MyQueues.addTaskForExec(*itPT);
								RQT++;
							}
						}

						return TasksDispatched;
					}
				}

				if (Candidate != nullptr) {

					//  If the ThreadPool is draining then mark the TASK to indicate that.
					if (State == cDraining) Candidate->Disposition = Candidate->Disposition | TASK::cTPDraining;

					MyWorkers[FirstAvailableThread].pTask = Candidate;
					MyWorkers[FirstAvailableThread].pWorker->postTask(MyWorkers[FirstAvailableThread].pTask);
					TasksDispatched++;
					TTX++;
					MyWorkers[FirstAvailableThread].TX++;
				}

				//  Check the available threads again
				AvailableThreads = 0;
				FirstAvailableThread = -1;

				for (int tIndex = 0; tIndex < XY_MAX_THREADS; tIndex++) {
					if (MyWorkers[tIndex].pWorker != nullptr && MyWorkers[tIndex].pTask == nullptr) {
						AvailableThreads++;
						if (FirstAvailableThread == -1) FirstAvailableThread = tIndex;
					}
				}
			}

			//  Re-instate any parked tasks to the queue
			if (ParkedTasks.size() > 0) {
				for (std::vector<TASK*>::iterator itPT = ParkedTasks.begin(); itPT != ParkedTasks.end(); itPT++) {
					MyQueues.addTaskForExec(*itPT);
					RQT++;
				}
			}

			//  Return the count of tasks dispatched
			return TasksDispatched;
		}

		//  performAutonomics
		//
		//  This function will perform autonomic minitoring and take actions to alter the thread pool
		//  if necessary.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	performAutonomics() {

			//  If disabled NOOP
			if (!aEnabled) return;

			//  Determine if we have hit a monitor cycle
			aMonitorTrigger--;
			if (aMonitorTrigger > 0) return;

			//  Reset the trigger
			aMonitorTrigger = aMCycles;

			//  Capture the monitor values
			//  Pending execution queue size
			if (aPXQSize >= aLastPXQSize) aLastPXQSize = aPXQSize;
			else aLastPXQSize = 99999999;
			aPXQSize = MyQueues.getPendingExecutionCount();

			//  Available thread count
			size_t		ThreadsAvailable = 0;
			for (int tIndex = 0; tIndex < XY_MAX_THREADS; tIndex++) {
				if (MyWorkers[tIndex].pWorker != nullptr && MyWorkers[tIndex].pTask == nullptr) ThreadsAvailable++;
			}
			if (aAvailableThreads >= aLastAvailableThreads) aLastAvailableThreads = aAvailableThreads;
			else aLastAvailableThreads = 99999999;
			aAvailableThreads = ThreadsAvailable;

			//  Determine if we have hit an action trigger cycle
			aActionTrigger--;
			if (aActionTrigger > 0) return;

			//  Reset the Action trigger & increment the cycle counter (ticker)
			aTicker++;
			aActionTrigger = aACycles;

			//
			//  Check for a persistent Pending Execution Queue 
			//
			if (aPXQSize >= aLastPXQSize && aPXQSize > 1 && ThreadsAvailable == 0) {
				//  If there is capacity then add another thread to the pool
				if (CurrentWorkers < MaxWorkers) {
					//  Find the index of the worker to add to the pool
					for (int tIndex = 0; tIndex < XY_MAX_THREADS; tIndex++) {
						if (MyWorkers[tIndex].pWorker == nullptr) {
							addWorker(tIndex);
							CurrentWorkers++;
							aThreadsStarted++;
							//  Reset the monitor cycle
							aPXQSize = 0;
							aLastPXQSize = 0;
							aAvailableThreads = 0;
							aLastAvailableThreads = 0;
							aMonitorTrigger = aMCycles;
							aActionTrigger = aACycles;
							return;
						}
					}
				}
			}

			//  Reset the queue size counter
			aPXQSize = 0;
			aLastPXQSize = 0;

			//
			//  Check for surplus threads
			//
			if (aAvailableThreads >= aLastAvailableThreads && aAvailableThreads > 1) {
				//  If we are above the minimum thread count then we can lose one
				if (CurrentWorkers > MinWorkers) {
					//  Find the last idle worker (it will be removed)
					for (int tIndex = (XY_MAX_THREADS - 1); tIndex >= 0; tIndex--) {
						if (MyWorkers[tIndex].pWorker != nullptr && MyWorkers[tIndex].pTask == nullptr) {
							removeWorker(tIndex);
							CurrentWorkers--;
							aThreadsStopped++;
							break;
						}
					}
				}
			}

			//  Reset the available thread counters
			aAvailableThreads = 0;
			aLastAvailableThreads = 0;

			//  Return to caller
			return;
		}

		//  addWorker
		//
		//  This function will add a new worker thread to the thread pool.
		//
		//  PARAMETERS
		//
		//		int				-			Index of the wrker thread to be added, TCB[i]
		//
		//  RETURNS
		//
		//		bool			-			true if the worker was added, otherwise false
		//
		//  NOTES:
		//

		bool	addWorker(int tIndex) {

			//  Setup the TCB
			MyWorkers[tIndex].pTask = nullptr;
			MyWorkers[tIndex].pWorker = new Worker(MyLQ, MyQueues, THREADID(tIndex + 1), SPool, VRMap);
			MyWorkers[tIndex].TID = THREADID(tIndex + 1);
			MyWorkers[tIndex].StartedAt = aTicker;

			//  Start the worker thread
			MyWorkers[tIndex].WT = std::thread(&Worker::run, MyWorkers[tIndex].pWorker);

			//  Return to caller
			return true;;
		}

		//  removeWorker
		//
		//  This function will remove a worker thread to the thread pool.
		//
		//  PARAMETERS
		//
		//		int				-			Index of the worker thread to be removed TCB[i]
		//
		//  RETURNS
		//
		//		bool			-			true if the worker was removed, otherwise false
		//
		//  NOTES:
		//

		bool	removeWorker(int tIndex) {

			//  Signal the worker thread to drain
			MyWorkers[tIndex].pWorker->drain();

			//  Wait for the thread to terminate
			MyWorkers[tIndex].WT.join();

			//  Destroy the worker object
			delete MyWorkers[tIndex].pWorker;
			MyWorkers[tIndex].pWorker = nullptr;
			MyWorkers[tIndex].pTask = nullptr;

			//  Bookkeeping
			MyWorkers[tIndex].Stops++;
			MyWorkers[tIndex].ActiveTicks += (aTicker - MyWorkers[tIndex].StartedAt);

			//  Return to caller
			return true;
		}

	};

}

