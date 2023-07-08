#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       Worker.h																							*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.0.0	(Build: 01)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the Worker class. The Worker class provides the					*
//* a single thread in the thread pool on which tasks can be executed.												* 
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

//  Required sub-systems/components
#include	"../Logging.h"																			//  Log Queue
#include	"MPQueues.h"																			//  MP Queues
#include	"Dispatcher.h"																			//  MP Dispatcher
#include	"../StringPool.h"																		//  String Pool
#include	"../VRMapper.h"																			//  Virtual Resource Mapper

//
//  All components are defined within the xymorg namespace
//
namespace xymorg {

	//
	//  Worker Class Definition
	//

	class Worker {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Constructor 
		//
		//  Constructs a Worker object
		//
		//  PARAMETERS:
		//
		//		LogQueue&			-		Reference to the LogQueue singleton
		//		MPQueues&			-		Reference to the MPQueues singleton
		//		THREADID			-		Identity of this worker thread
		//		StringPool&			-		Reference to the application level string pool
		//		VRMapper&			-		Reference to the resource mapper
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		Worker(LogQueue& LogIF, MPQueues& MPQIF, THREADID Thread, StringPool& ALSP, VRMapper& VRM) : MyDispatcher(LogIF, MPQIF, Thread, ALSP, VRM)
			, MyTID(Thread), MPQ(MPQIF) {

			//  Set the initial state
			State = cNotStarted;
			Task = nullptr;

			//  Return to caller
			return;
		}

		//
		//  Mark the object as not copyable/moveable/assignable
		//

		Worker(const Worker&) = delete;
		Worker(Worker&&) = delete;

		Worker& operator = (const Worker&) = delete;
		Worker& operator = (Worker&&) = delete;

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

		~Worker() {

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

		//  run
		//
		//  Main running loop for the worker, this is invoked on a dedicated thread.
		//  The running loop monitors for tasks being posted and executes them then signals the threadpool that the task is completed.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void run() {

			//  Set the state to active
			State = cActive;

			//  Main processing loop - watch for tasks and execute them, otherwise idle
			while (State != cDrained) {

				//  Detect new work to be executed
				if (Task != nullptr) {
					//  Execute the newly passed task
					Task->Executor->executeThisTask(*Task, MyDispatcher, MyTID);

					//  If the task specifies the use of a Task Completion Port then invoke it
					//  Take a local copy of the completed task address, this allows us to clear the task address
					//  before we signal completion to the thread pool service. This ensures correct interlocking
					//  of the worker with the thread pool.
					TASK* pCompletedTask = Task;

					if (Task->Disposition & TASK::cTaskCompletionPort) {
						Task->Executor->tcp(*Task, MyDispatcher, MyTID);
						//  Clear the task
						Task = nullptr;
					}
					else {
						//  Clear the task
						Task = nullptr;
					}

					//  Post the task completed semaphore
					MPQ.signalTaskCompleted(pCompletedTask, MyTID);

				}
				else sleep(MILLISECONDS(5));

				//  Drain protocol
				if (State == cDraining && Task == nullptr) State = cDrained;
			}

			//  Return to caller
			return;
		}

		//  postTask
		//
		//  Posts a task for execution by the worker thread.
		//
		//  PARAMETERS
		//
		//		TASK*		-		Pointer to the task to be posted for execution
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	postTask(TASK* pNewTask) {

			//  Set the address of the task to be executed
			Task = pNewTask;

			//  Return to caller
			return;
		}

		//  drain
		//
		//  Signal the worker thread to drain
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

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Constants																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

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

		//  Dispatcher interface for this thread
		Dispatcher			MyDispatcher;

		//  MP Queues
		MPQueues&			MPQ;

		THREADID			MyTID;																	//  Thread ID
		volatile int		State;																	//  State of the Worker thread

		//  Posting slot for new work that is to be executed
		TASK* volatile		Task;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

	};

}
