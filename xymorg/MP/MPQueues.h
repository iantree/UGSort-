#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       MPQueues.h																						*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.0.0	(Build: 01)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the MPQueues class. The MPQueues class provides the				*
//* interface between client side and server side components and activities in the multi-processing kernel.			* 
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
//*	1.0.0 -		10/12/2017	-	Initial Release																		*
//*																													*
//*******************************************************************************************************************/

//  Include xymorg headers
#include	"../LPBHdrs.h"																			//  Language and Platform base headers
#include	"../types.h"																			//  xymorg type definitions
#include	"../consts.h"																			//  xymorg constant definitions
#include	"Primitives.h"																			//  MP prinitives
#include	"Task.h"																				//  MP TASK class

//  Additional Platform Headers
#include    <queue>																					//  STL queue

//
//  Define the hard maximum number of threads - if not already defined 
//

#ifndef  XY_MAX_THREAD
#define  XY_MAX_THREADS XY_DEFAULT_MAX_THREADS
#endif

//,
//  All components are defined within the xymorg namespace
//
namespace xymorg {

	//  
	//  isHigherPriority Class Definition
	//
	//  This is a comparator function class used to compare the priority of two tasks.
	//  This is used to maintain order in the (priority) pending execution queue.
	//

	class isHigherPriority {
	public:
		bool operator () (TASK* lhs, TASK* rhs) { if (lhs->Priority < rhs->Priority) return true; return false; }
	};

	//
	//  TimerQueueElement 
	//
	//  Structure used to define entries on the TimerQueue.
	//  The TimerQueue is used to manage tasks that are awaiting a timer expiry (delay) prior to execution
	//
	typedef struct TimerQueueElement {
		TIMER			Expires;														//  Delay expires at time
		TASK* DTask;															//  Pointer to the delayed task

		TimerQueueElement() { DTask = nullptr; }
	} TimerQueueElement;

	//
	//  isEarlier
	//  
	//	This is a comparator function class used to compare the expiry time of two TimerQueueEntries.
	//  This is used to maintain order in the TimerQueue.
	//

	class isEarlier {
	public:
		bool operator () (TimerQueueElement& lhs, TimerQueueElement& rhs) { if (lhs.Expires > rhs.Expires) return true; return false; }
	};

	//
	//  EXEQ Class Definition
	//
	//  The EXEQ is a wrapper class for a standard priority_queue container (adaptor + container). with a fixed implementation.
	//  The wrapper provides an additional function hasOwnerTasks() that interrogates the content of the queue.
	//

	class EXEQ : public std::priority_queue<TASK*, std::vector<TASK*>, isHigherPriority> {
	public:
		//  hasOwnerTasks
		//
		//  Interrogates the vector on which the queue is based to see if any tasks match the passed owner.
		//
		//  PARAMETERS:
		//
		//		void*		-		Identity of the owner
		//
		//  RETURNS:
		//
		//		bool		-		true if there are any tasks on the queue for the owner, otherwise fase
		//
		//  NOTES:
		//

		bool	hasOwnerTasks(void* Owner) {
			//  Search the queue for the first task with a matching owner
			for (std::vector<TASK*>::iterator itQ = c.begin(); itQ != c.end(); itQ++) {
				if ((*itQ)->Owner == Owner) return true;
			}
			//  Return signalling none found
			return false;
		}

		//  hasSubTasks
		//
		//  Interrogates the vector on which the queue is based to see if any tasks are characterised as sub-tasks
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		bool		-		true if there are any tasks on the queue that are sub-tasks
		//
		//  NOTES:
		//

		bool	hasSubTasks() {
			//  Search the queue for the first task with a matching owner
			for (std::vector<TASK*>::iterator itQ = c.begin(); itQ != c.end(); itQ++) {
				if ((*itQ)->Character & TASK::cSubTask) return true;
			}
			//  Return signalling none found
			return false;
		}

	};

	//
	//  TimerQueue Class Definition
	//
	//  The TimerQueue is a wrapper class for a standard priority_queue container (adaptor + container). with a fixed implementation.
	//

	class TimerQueue : public std::priority_queue<TimerQueueElement, std::vector<TimerQueueElement>, isEarlier> {
	public:

		//  hasExpiredEntries
		//
		//  Interrogates the vector on which the queue is based to see if the head of the queue has expired
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		bool		-		true if the element at the head of the queue has an expired timer
		//
		//  NOTES:
		//

		bool	hasExpiredEntries() {
			TIMER		RefTime = CLOCK::now();														//  Current wall clock time

			//  Return if queue is empty
			if (size() == 0) return false;

#ifdef XY_MP_NEEDS_DEBUGGING
			std::cout << "TRACE: TIMER EXPIRY CHECK:    QSize: " << size() << "." << std::endl;
			std::time_t		TTQE = std::chrono::system_clock::to_time_t(c.begin()->Expires);
			std::time_t		TREF = std::chrono::system_clock::to_time_t(RefTime);
			std::cout << "TRACE: REF TIME: " << ctime(&TREF) << ", Epoch Count: " << RefTime.time_since_epoch().count() << "." << std::endl;
			std::cout << "TRACE: TQE TIME: " << ctime(&TTQE) << ", Epoch Count: " << c.begin()->Expires.time_since_epoch().count() << "." << std::endl;
#endif

			//  If the topmost element on the queue has expired return true
			if (c.begin()->Expires <= RefTime) return true;
			return false;
		}

	};

	//
	//  MPQueues Class Definition
	//
	//  The class provides the interface between MP client side and server side processing. It also provides encapsulation for the queues used in MP
	//

	class MPQueues {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Constructor 
		//
		//  Constructs an MPQueue object
		//
		//  PARAMETERS:
		//
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		MPQueues() {

			//  Clear the statistics
			XQAdds = 0;
			XQRems = 0;
			XQHWM = 0;
			RQAdds = 0;
			RQRems = 0;
			RQHWM = 0;
			RQBusy = 0;
			DQAdds = 0;
			DQRems = 0;
			DQHWM = 0;

			//  Return to caller
			return;
		}

		//
		//  Mark the object as not copyable/moveable/assignable
		//

		MPQueues(const MPQueues&) = delete;
		MPQueues(MPQueues&&) = delete;

		MPQueues& operator = (const MPQueues&) = delete;
		MPQueues& operator = (MPQueues&&) = delete;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor                                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Destructor
		//
		//  Destroys the MPQueues object, in the process the queues that it encapsulates will be destroyed
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		~MPQueues() {

			//  Return tp caller
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

		//  postTaskForExecution
		//
		//  This function will post a task for execution by setting the address of the task in the sempahore
		//  belonging to the current thread.
		//
		//  PARAMETERS
		//
		//		TASK*		-		Pointer to the TASK to be posted, may not be NULL
		//		THREADID	-		ID of the thread posting the task
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	postTaskForExecution(TASK* pNewTask, THREADID Poster) {

			//  Ignore NULL requests
			if (pNewTask == NULL) return;

			//  Post the request to the semaphore for the current thread, this call will block if the semaphore is busy
			SendToXQ[Poster].post(pNewTask, Poster);

			//  Return to caller
			return;
		}

		//  requestCompletedTask
		//
		//  This function will request any completed rejoinable tasks that are available for the owner
		//  passed in the task pointer.
		//
		//  PARAMETERS
		//
		//		TASK**		-		(in) Reference to the pointer containing the owners identity
		//					-		(out) Reference to the pointer in which the address of the completed task will be returned
		//		THREADID	-		ID of the thread making the requestt
		//
		//  RETURNS
		//
		//		int				-			Return code indicating the state of the tasks for the designated owner
		//
		//									cTaskCompleted - A task has completed an it's address has been returned
		//									cTasksStillBusy - There are tasks that have not yet completed for this woner
		//									cTasksAllCompleted - All tasks have now completed for this owner
		//
		//  NOTES:
		//

		int		returnCompletedTask(TASK*& pCompletedTask, THREADID Requester) {
			//  Post the value to the semaphore, this call will block until a response is obtained from the thread pool
			return ReturnFromRQ[Requester].request(pCompletedTask, Requester);
		}

		//  enqueueNewTasks
		//
		//  This function will poll all of the new task semaphores and service any that are posted by
		//  adding the new task to the queue of tasks for execution.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		size_t		-		Count of tasks added to the pool
		//
		//  NOTES:
		//

		size_t	enqueueNewTasks() {
			size_t		TasksAdded = 0;

			//  Check each semaphore to see if it has been posted
			for (int tIndex = 0; tIndex <= XY_MAX_THREADS; tIndex++) {
				if (SendToXQ[tIndex].isPosted()) {
					//  Accept the task pointer from the semaphore and post it to the tasks for execution queue
					addTaskForExec(SendToXQ[tIndex].accept());
					TasksAdded++;
				}
			}

			//  Return to caller
			return TasksAdded;
		}

		//  dequeueOldTasks
		//
		//  This function will poll all of the rejoin request semaphores and service any that are posted by
		//  returning any completed (and rejoinable) tasks from the rejoin queue.
		//
		//  PARAMETERS
		//
		//		void*[]		-		Array of workers identifying the owner of the task they are executing
		//
		//  RETURNS
		//
		//		size_t		-		Count of tasks returned
		//
		//  NOTES:
		//

		size_t	dequeueOldTasks(void* TOwners[]) {
			size_t		TasksRemoved = 0;																//  Count of tasks removed from the queue

			//  Check each semaphore to see if it has been posted
			for (int tIndex = 0; tIndex <= XY_MAX_THREADS; tIndex++) {
				if (ReturnFromRQ[tIndex].isPosted()) {
					void* MyOwner = ReturnFromRQ[tIndex].peekValue();

					//  Search the rejoin queue to see if any completed tasks are available
					TASK* pOldTask = getNextRejoin(MyOwner);

					//  If a task was returned then post it back in the response to the semaphore
					if (pOldTask != NULL) {
						ReturnFromRQ[tIndex].respond(pOldTask, TASK::cTaskCompleted);
						TasksRemoved++;
						RQRems++;
					}
					else {
						bool	FoundTask = false;
						//  Check if the owner has a TASK currently being executed by one of the worker threads
						for (int wIndex = 0; wIndex < XY_MAX_THREADS; wIndex++) {
							if (MyOwner == TOwners[wIndex]) {
								ReturnFromRQ[tIndex].respond(NULL, TASK::cTasksStillBusy);
								FoundTask = true;
								RQBusy++;
								break;
							}
						}

						//  If no executing tasks were found then search the pending execution queue
						if (!FoundTask) {
							if (PendXQ.hasOwnerTasks(MyOwner)) {
								ReturnFromRQ[tIndex].respond(NULL, TASK::cTasksStillBusy);
								RQBusy++;
							}
							else ReturnFromRQ[tIndex].respond(NULL, TASK::cTasksAllCompleted);
						}
					}
				}
			}

			//  Return the count of tasks that were returned
			return TasksRemoved;
		}

		//  signalTaskCompleted
		//
		//  This function will post the task completed semaphore for the designated thread
		//
		//  PARAMETERS
		//
		//		TASK*		-		Pointer to the TASK to be posted, may not be NULL
		//		THREADID	-		ID of the thread posting the task
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	signalTaskCompleted(TASK* pNewTask, THREADID Poster) {

			//  Ignore NULL requests
			if (pNewTask == NULL) return;

			//  Post the request to the semaphore for the current thread, this call will block if the semaphore is busy
			DisposeTask[Poster - 1].post(pNewTask, Poster);

			//  Return to caller
			return;
		}

		//  disposeOldTasks
		//
		//  This function will find the first thread that has the disposition set and will process it.
		//  returning the index of the thread.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		int		-		intex of the thread holding the task that was disposed, -1 if none were disposed
		//
		//  NOTES:
		//

		size_t	disposeOldTasks() {

			//  Check each semaphore to see if it has been posted
			for (int tIndex = 0; tIndex < XY_MAX_THREADS; tIndex++) {
				if (DisposeTask[tIndex].isPosted()) {
					//  Accept the task pointer from the semaphore and dispose of it as necessary
					dispose(DisposeTask[tIndex].accept());
					return tIndex;
				}
			}

			//  Return to caller signalling that no threads were disposed
			return -1;
		}

		//  getPendingExecutionCount
		//
		//  This function will return the count of tasks that are on the pending execution queue.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		size_t		-		Count of tasks that are pending execution
		//
		//  NOTES:
		//

		size_t	getPendingExecutionCount() {
			return PendXQ.size();
		}

		//  getPendingRejoinCount
		//
		//  This function will return the count of tasks that are on the rejoin queue.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		size_t		-		Count of tasks that are waiting to rejoin
		//
		//  NOTES:
		//

		size_t	getPendingRejoinCount() {
			return RejoinQ.size();
		}

		//  getDelayedCount
		//
		//  This function will return the count of tasks that are on the delayed execution queue.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		size_t		-		Count of tasks that are waiting to execute after a delay
		//
		//  NOTES:
		//

		size_t	getDelayedCount() {
			return DelayedQ.size();
		}

		//  getNextTaskToExecute
		//
		//  This function will return a pointer to thye next task to be executed. The task will be dequeued from the pending execution queue.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		TASK*		-		Pointer to the next task to be executed
		//
		//  NOTES:
		//

		TASK* getNextTaskToExecute() {
			TASK* pNextTask = NULL;

			//  Safety
			if (PendXQ.size() == 0) return NULL;

			//  Extract the topmost (highest priority) task
			pNextTask = PendXQ.top();
			PendXQ.pop();
			XQRems++;

			//  Return the task to the caller
			return pNextTask;
		}

		//  addTaskForExec
		//
		//  This function will add the passed task address to the pending execution queue.
		//  If the delay flag is set in the task then it will have a new TQE created for it and be posted to the Delayed queue.
		//
		//  PARAMETERS
		//
		//		TASK*		-		Pointer to the next task to be executed
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	addTaskForExec(TASK* pTask) {

			//  Intercept tasks that must be delayed
			if (pTask->Disposition & TASK::cTaskDelayed) {
				TimerQueueElement		TQE;

				//  Compute the expiry time of the queue element (now + delay)
				TQE.Expires = CLOCK::now();
				TQE.Expires += MILLISECONDS(pTask->Delay);

				//  Set the task address in the TimerQueueElement
				TQE.DTask = pTask;

				//  Add the TimerQueueElement to the delayed queue
				DelayedQ.push(TQE);

				//  Clear the delayed properties of the task
				pTask->Disposition = pTask->Disposition ^ TASK::cTaskDelayed;
				pTask->Delay = 0;

				//  Accumulate the stats
				DQAdds++;
				if (DelayedQ.size() > DQHWM) DQHWM = DelayedQ.size();

				//  Return to caller
				return;
			}

			//  Add the task  to the queue
			PendXQ.push(pTask);

			//  Accumulate the stats
			XQAdds++;

			//  Maintain the high watermark
			if (PendXQ.size() > XQHWM) XQHWM = PendXQ.size();

			//  Return to caller
			return;
		}

		//  requeueDelayedTasks
		//
		//  This function will add any delayed tasks that have expired onto the pending execution queue.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	requeueDelayedTasks() {
			TimerQueueElement		TQE;																//  Expired queue element

			//  Process while the queue has expired entries
			while (DelayedQ.hasExpiredEntries()) {
				TQE = DelayedQ.top();

#ifdef XY_MP_NEEDS_DEBUGGING
				TIMER	TimeStamp = CLOCK::now();
				std::time_t		TTS = std::chrono::system_clock::to_time_t(TimeStamp);

				std::cout << ctime(&TTS) << "TRACE: Returning task from the delayed queue with timer: ";
				std::time_t		TTQE = std::chrono::system_clock::to_time_t(TQE.Expires);
				std::cout << ctime(&TTQE) << ", Epoch Count: " << TQE.Expires.time_since_epoch().count() << "." << std::endl;
#endif

				DelayedQ.pop();
				DQRems++;
				addTaskForExec(TQE.DTask);
			}

			//  Return to caller
			return;
		}

		//  hasTasksOnXQ
		//
		//  This function will check if the passed owner identity has any tasks on the pending execution queue.
		//
		//  PARAMETERS
		//
		//		TASK*		-		Pointer to the next task to be executed
		//
		//  RETURNS
		//
		//		bool		-		true if the owner has tasks pending execution, otherwise false
		//
		//  NOTES:
		//

		bool	hasTasksOnXQ(void* Owner) {
			return PendXQ.hasOwnerTasks(Owner);
		}

		//  hasSubTasksOnXQ
		//
		//  This function will check if there are any sub-tasks on the pending execution queue
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		bool		-		true if there are sub-tasks present on the queue, otherwise false
		//
		//  NOTES:
		//

		bool	hasSubTasksOnXQ() {
			return PendXQ.hasSubTasks();
		}

		//  emptyRejoinQueue
		//
		//  This function will dispose of the contents of the rejoin queue
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	emptyRejoinQueue() {

			while (RejoinQ.size() > 0) {
				TASK* DispTask = RejoinQ.back();

				if (DispTask->Disposition & TASK::cTaskDisposable) delete DispTask;
				RejoinQ.pop_back();
			}

			//  Return to caller
			return;
		}

		//  emptyDelayedQueue
		//
		//  This function will dispose of the contents of the delayed tasks queue
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	emptyDelayedQueue() {

			while (RejoinQ.size() > 0) {
				TimerQueueElement		TQE = DelayedQ.top();

				if (TQE.DTask->Disposition & TASK::cTaskDisposable) delete TQE.DTask;
				DelayedQ.pop();
			}

			//  Return to caller
			return;
		}

		//  showStats
		//
		//  This function will log the current processing statistics
		//
		//  PARAMETERS
		//
		//		bool		-		If true then show the detailed per semaphore statistics
		//		AppLog&		-		Logging interface on which to write the stats output
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	showStats(bool ShowSems, std::ostream& Log) {

			//  Show the per queue statistics
			Log << "PX QUEUE: Tasks added: " << XQAdds << ", tasks removed: " << XQRems << ", High Watermark: " << XQHWM << "." << std::endl;
			Log << "RJ QUEUE: Tasks added: " << RQAdds << ", tasks returned: " << RQRems << ", High Watermark: " << RQHWM << ", Busy signals: " << RQBusy << "." << std::endl;
			Log << "DX QUEUE: Tasks added: " << DQAdds << ", tasks removed: " << DQRems << ", High Watermark: " << DQHWM << "." << std::endl;

			//  If requested then show the per semaphore detailed statistics
			if (ShowSems) {
				//  Show the Send to Execution Queue semaphore statistics
				for (int sIndex = 0; sIndex <= XY_MAX_THREADS; sIndex++) {
					if (SendToXQ[sIndex].Posts > 0) {
						Log << "POST FOR EXEC SEM [" << sIndex << "], Posts: " << SendToXQ[sIndex].Posts << ", Waits: " << SendToXQ[sIndex].PostWaits << ", Wait Quanta: " << SendToXQ[sIndex].PostWaitQuanta << "." << std::endl;
					}
				}

				//  Show the task disposition semaphore stats
				for (int sIndex = 0; sIndex < XY_MAX_THREADS; sIndex++) {
					if (DisposeTask[sIndex].Posts > 0) {
						Log << "POST DISP TASK SEM [" << sIndex + 1 << "], Posts: " << DisposeTask[sIndex].Posts << ", Waits: " << DisposeTask[sIndex].PostWaits << ", Wait Quanta: " << DisposeTask[sIndex].PostWaitQuanta << "." << std::endl;
					}
				}

				//  Show the rejoin request semaphore stats
				for (int sIndex = 0; sIndex < XY_MAX_THREADS; sIndex++) {
					if (ReturnFromRQ[sIndex].Posts > 0) {
						Log << "RJ REQ SEM [" << sIndex << "], Posts: " << ReturnFromRQ[sIndex].Posts << ", Waits: " << ReturnFromRQ[sIndex].PostWaits << ", Wait Quanta: " << ReturnFromRQ[sIndex].PostWaitQuanta << "." << std::endl;
					}
				}

			}

			//  Return to caller
			return;
		}

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Per thread semaphores used for signalling that a task is to be placed on the for execution queue
		SEMAPHORE<TASK*>		SendToXQ[XY_MAX_THREADS + 1];

		//  Per thread semaphores used for signalling that a task has been completed by a worker thread and should be disposed
		SEMAPHORE<TASK*>		DisposeTask[XY_MAX_THREADS];

		//  Per thread semaphores used to request the return of completed rejoinable threads to the owner
		SEMAPHORE<TASK*>		ReturnFromRQ[XY_MAX_THREADS + 1];

		//  Queue holding tasks that are for execution, held in priority order
		EXEQ					PendXQ;																				//  Tasks Pending Execution Queue

		//  Queue holding tasks that are to be rejoined with their owner
		std::vector<TASK*>		RejoinQ;																			//  Tasks Pending Rejoin Queue

		//  Queue holding tasks and timers for tasks that are delayed pending execution
		TimerQueue				DelayedQ;																			//  Delayed Tasks Queue

		//  Statistics
		size_t					XQAdds;																				//  Added to pending exec queue
		size_t					XQRems;																				//  Removed from pending exec queue
		size_t					XQHWM;																				//  Pending exec queue high watermark

		size_t					RQAdds;																				//  Added to Rejoin queue
		size_t					RQRems;																				//  Removed from Rejoin queue
		size_t					RQHWM;																				//  Rejoin queue high watermark
		size_t					RQBusy;																				//  Number of busy resposes

		size_t					DQAdds;																				//  Added to the delayed queue
		size_t					DQRems;																				//  Removed from the delayed queue
		size_t					DQHWM;																				//  Delayed queue high watermark

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  dispose
		//
		//  This function will dispose of the passed task as is appropriate.
		//
		//  PARAMETERS
		//
		//		TASK*			-		Pointer to the task to be disposed of
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	dispose(TASK* pOldTask) {

			//  If the task is "Fire and Forget" then delete it if it is disposable
			if (pOldTask->Disposition & TASK::cFireAndForget) {
				if (pOldTask->Disposition & TASK::cTaskDisposable) delete pOldTask;
				return;
			}

			//  If the task was disposed of in the Task Completeion Port (TCP) then do nothing
			if (pOldTask->Disposition & TASK::cTaskCompletionPort) return;

			//  If the task is rejoin then enqueue it to the rejoin queue
			if (pOldTask->Disposition & TASK::cTaskRejoin) {
				RejoinQ.push_back(pOldTask);
				RQAdds++;
				if (RejoinQ.size() > RQHWM) RQHWM = RejoinQ.size();
				return;
			}

			//  If the task is rejoin on exception - then add it to the rejoin queue if there is an exception
			//  otherwise delete it if it is disposable.
			if (pOldTask->Disposition & TASK::cTaskRejoinOnException) {
				if (pOldTask->Exception != 0) {
					RejoinQ.push_back(pOldTask);
					RQAdds++;
					if (RejoinQ.size() > RQHWM) RQHWM = RejoinQ.size();
				}
				else {
					if (pOldTask->Disposition & TASK::cTaskDisposable) delete pOldTask;
				}
				return;
			}

			//  If the task is to be routed then return it to the pending execution queue
			if (pOldTask->Disposition & TASK::cTaskRoute) {
				addTaskForExec(pOldTask);
				return;
			}

			//  If the task is to be routed for a rendezvous then return it to the pending execution queue
			if (pOldTask->Disposition & TASK::cTaskRendezvous) {
				addTaskForExec(pOldTask);
				return;
			}

			//  SNO - anything else delete it if it is disposable
			if (pOldTask->Disposition & TASK::cTaskDisposable) delete pOldTask;

			//  Return to caller
			return;
		}

		//  getNextRejoin
		//
		//  This function will return the address of the next completed task for the designated owner and
		//  remove it from the rejoin pool.
		//
		//  PARAMETERS
		//
		//		void*			-		Designated owner
		//
		//  RETURNS
		//
		//		TASK*			-		Pointer to the returned task, NULL if none were available
		//
		//  NOTES:
		//

		TASK* getNextRejoin(void* Owner) {

			// Search the rejoin queue for any task that matches the owner
			for (std::vector<TASK*>::iterator itRJQ = RejoinQ.begin(); itRJQ != RejoinQ.end(); itRJQ++) {
				if ((*itRJQ)->Owner == Owner) {
					//  The entry matches - remove the entry from the queue and return the address of the task
					TASK* pOldTask = *itRJQ;
					RejoinQ.erase(itRJQ);
					return pOldTask;
				}
			}

			//  Return showing that none were available
			return nullptr;
		}

	};

}
