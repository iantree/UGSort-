#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       Task.h																							*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.0.0	(Build: 01)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains definitions for Task and related classes for Multi-Processing (MP).					*
//* The following classes are defined in this file :-																*
//*																													*
//*		TaskOwner		-		Task Owber Interface																*
//*		Taskexecuter	-		Task executer Interface																*
//*		Task			-		Task base class																		*
//*																													*
//*	USAGE:																											*
//*																													*
//*		#Define   XY_NEEDS_MP	1		before including xymorg.h headers											*
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
#include	"Primitives.h"																			//  MP primitives

//
//  All components are defined within the xymorg namespace
//
namespace xymorg {

	//  Forward declaration of classes used by the interfaces
	class TaskExecutor;
	class Dispatcher;

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Interfaces                                                                                                    *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   TASK Class                                                                                                    *
	//*                                                                                                                 *
	//*   Objects of this class, or more commonly of derived classes represent units of work (UOWs) that will be        *
	//*   executed asynchronously on the threads in the threadpool.                                                     *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	class TASK
	{
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Task Characterisation Constants
		static const SWITCHES   cServiceTask =		0x00000010;									//  Service task
		static const SWITCHES	cServiceProducer =	0x00000011;									//  A Producer service task
		static const SWITCHES	cServiceConsumer =	0x00000012;									//  A Consumer service task

		static const SWITCHES   cGeneratorTask =	0x00000020;									//  Generator task
		static const SWITCHES   cUnitOfWork =		0x00000040;									//  Unit Of Work (UOW) task
		static const SWITCHES   cSubTask =			0x00000080;									//  Sub task
		static const SWITCHES   cCategorisedTask =	0x000000FF;									//  Categorised task (any of the above types)

		//  Task Disposition Constants
		static const SWITCHES   cFireAndForget =	0x00000001;									//  Disposition is Fire and Forget
		static const SWITCHES   cTaskCompletionPort = 0x00000002;								//  Use the Task Completion Port
		static const SWITCHES   cTaskRejoin =		0x00000004;									//  Task will be rejoined with the owner
		static const SWITCHES   cTaskRejoinOnException = 0x00000008;							//  Task will be rejoined with the owner on exception
		static const SWITCHES   cTaskRoute =		0x00000010;									//  Task will be routed to the next executer
		static const SWITCHES   cTaskRendezvous =	0x00000020;									//  Task will be routed to the next executer when all children are ready
		static const SWITCHES   cTaskDisposable =	0x00000040;									//  Task is disposable
		static const SWITCHES	cTaskDelayed =		0x00000080;									//  Task will be delayed before execution
		static const SWITCHES	cTPDraining =		0x00000100;									//  Task dispatched while the ThreadPool is draining

		//  Rejoin request result constants
		static const int		cTaskCompleted = 200;											//  Task has completed and is returned
		static const int		cTasksStillBusy = 300;											//  Tasks are executing or waiting for execution
		static const int		cTasksAllCompleted = 400;										//  All tasks have now completed

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Default Constructor
		//
		//  Constructs an empty task object.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES:
		//

		TASK()
		{
			Owner = NULL;
			Executor = NULL;
			Parent = NULL;
			Character = 0;
			Disposition = 0;
			Priority = 0;
			Exception = 0;
			Delay = 0;

			//  Return to caller
			return;
		}

		//  Native Constructor
		//
		//  Constructs a task with the passed parameters
		//
		//  PARAMETERS
		//
		//		bool				-		If true the task is disposable
		//		TaskOwner*			-		Pointer to the object that "owns" this tasks
		//		TaskExecuter*		-		Pointer to the object that will execute this task
		//		Task*				-		Pointer to the parent task of this task
		//		SWITCHES			-		Characteristics of this task
		//		SWITCHES			-		Disposition of this task
		//		int					-		Scheduling priority of this task
		//
		//  RETURNS
		//
		//  NOTES:
		//

		TASK(bool Disp, void* pTO, TaskExecutor* pTX, TASK* pTP, SWITCHES TC, SWITCHES TD, int TP) {
			Owner = pTO;
			Executor = pTX;
			Parent = pTP;
			Character = TC;
			Disposition = TD;
			Priority = TP;
			Delay = 0;
			Exception = 0;

			//  Set the disposable option
			if (Disp) Disposition = Disposition | cTaskDisposable;

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor                                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		virtual ~TASK() {}


		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Members                                                                                                *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		void*						Owner;																	//  Task owner
		TaskExecutor*				Executor;																//  Task executor
		TASK*						Parent;																	//  Parent task
		SWITCHES                    Character;																//  Characteristics of the task
		SWITCHES					Disposition;															//  Disposition of the task
		int                         Priority;																//  Execution priority
		int							Delay;																	//  Execution delayed by (ms)
		int							Exception;																//  Exception code

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  delayExecution
		//
		//  Sets a delay (ms) before the task is executed
		//
		//  PARAMETERS
		//
		//      int             -  The number of milliseconds to delay the task before execution
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	delayExecution(int Delayms) {
			Delay = Delayms;
			Disposition = Disposition | cTaskDelayed;
			return;
		}

		//  setException
		//
		//  Sets an exception code in the task
		//
		//  PARAMETERS
		//
		//      int             -  The exception code to be set
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	setException(int XC) { Exception = XC; return; }

		//  routeTo
		//
		//  Sets the next destination for this TASK, once completed the TASK will be passed to the identified executer fo run it.
		//
		//  PARAMETERS
		//
		//      Taskexecuter*          -  Pointer to the executer to route to
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	routeTo(TaskExecutor* pTX) { Executor = pTX; Disposition = cTaskRoute; return; }

		//  rendezvousAt
		//
		//  Sets the next destination for this TASK, once completed the TASK will be passed to the identified executer fo run it.
		//  At the destination the task will meet up with it's children again.
		//
		//  PARAMETERS
		//
		//      Taskexecuter*          -  Pointer to the executer to route to
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	rendezvousAt(TaskExecutor* pTX) { Executor = pTX; Disposition = cTaskRendezvous; return; }

		//  isDisposable
		//
		//  Tests if the task is disposable or not.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		bool		-		true if the task is disposable, otherwise false
		//
		//  NOTES:
		//

		bool	isDisposable() {
			if (Disposition & cTaskDisposable) return true;
			return false;
		}

		//  clone()
		//
		//  Virtual function to provide a clone of the current object.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		TASK*		-		Pointer to a new copy of this object
		//
		//  NOTES:
		//

		virtual TASK* clone() { return new TASK(*this); }

	};

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Task Executor Interface                                                                                       *
	//*                                                                                                                 *
	//*   Classes implementing this interface can execute tasks asynchronously on threads belonging to the threadpool.  *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	class TaskExecutor
	{
	public:
		//  Virtual Destructor
		virtual ~TaskExecutor() {};

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public functions - must be implemented in any derived class                                                   *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  executeThisTask
		//
		//  Tasks are passed to this function for execution.
		//
		//  PARAMETERS
		//
		//        TASK&                 -        Reference to the TASK that is to be executed
		//		  Dispatcher&			-		 Reference to the MP Dipatcher interface for this thread
		//        const THREADID        -        ID of the current thread
		//
		//  RETURNS
		//
		//  NOTES:
		//

		virtual void executeThisTask(TASK& MyTask, Dispatcher& MP, const THREADID) = 0;

		//  tcp
		//
		//  Task Completion Port, as tasks are completed for this owner then they are passed to this function.
		//  The default implementation deletes the task if it is disposable.
		//
		//  PARAMETERS
		//
		//        TASK&                 -        Reference to the TASK that has just completed
		//		  Dispatcher&			-		 Reference to the MP Dipatcher interface for this thread
		//        const THREADID        -        ID of the thread that executed the task (and where the tcp is executing)
		//
		//  RETURNS
		//
		//  NOTES:
		//
		//  The Task Completion Port is executed on the thread where the task was executed.
		//

		virtual void tcp(TASK& MyTask, Dispatcher& MP, const THREADID) {
			//  If the passed task is disposable then delete it
			if (MyTask.isDisposable()) delete& MyTask;

			//  Return to caller
			return;
		}

	};

}
