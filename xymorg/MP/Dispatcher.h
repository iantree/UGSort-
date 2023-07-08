#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       Dispatcher.h																						*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.0.0	(Build: 01)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the Dispatcher class. The Dispatche class provides the				*
//* the client interface for all multi-processing functions in an application. One instace of a Dispatcher			* 
//* is provided for each thread in an application. 																	* 
//*																													*
//*	USAGE:																											*
//*																													*
//*		Main Program Thread (THREADID: 0) access the Dispatcher member of the Config singleton.						*
//*		TASKS executed in an MP ThreadPool are passed their dispatcher in the execute() interface.					*
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

#ifndef XY_NEEDS_MP
#define		XY_NEEDS_MP		1
#endif

//  Include xymorg headers
#include	"../LPBHdrs.h"																			//  Language and Platform base headers
#include	"../types.h"																			//  xymorg type definitions
#include	"../consts.h"																			//  xymorg constant definitions

//  Required sub-systems/components
#include	"../Logging.h"																			//  MP Logging interface
#include	"MPQueues.h"																			//  MP Queues
#include	"../StringPool.h"																		//  String Pool
#include	"../VRMapper.h"																			//  Virtual Resource Mapper

//
//  All components are defined within the xymorg namespace
//
namespace xymorg {

	//
	//  Dispatcher Class Definition
	//
	//  The class provides the interface for MP clients to all MP functionality
	//

	class Dispatcher {
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
		//		LogQueue&		-		Reference to the LogQueue singleton object
		//		MPQueues&		-		Reference to the MP Queues singleton object
		//		THREADID		-		ID of the thread for which this Dispatcher is being constructed
		//		StringPool&		-		Reference to the application level string pool
		//		VRMapper&		-		Reference to the resource mapper
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		Dispatcher(LogQueue& LQ, MPQueues& Q, THREADID Thread, StringPool& ALSP, VRMapper& VRM) :
			Log(new LogStreamBuf(LQ, Thread))
			, MPQ(Q), ThisThread(Thread)
			, IsDismissed(false)
			, SPool(ALSP)
			, VRMap(VRM) {

			//  Return to caller
			return;
		}

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

		~Dispatcher() {

			dismiss();

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Members                                                                                                *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Exposed sub-systems/components
	public:
		std::ostream			Log;																		//  Application Logging

		//  References that expose app level services
		StringPool& SPool;																			//  Application level string pool
		VRMapper& VRMap;																			//  Resource Mapper

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  dispatch
		//
		//  Dispatches the passed task for execution
		//
		//  PARAMETERS
		//
		//		TASK&			-			Reference to the task to be dispatched
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	dispatch(TASK& MyTask) {
			dispatch(&MyTask);
			//  Return to capper
			return;
		}

		//  dispatch
		//
		//  Dispatches the passed task for execution
		//
		//  PARAMETERS
		//
		//		TASK*			-			Pointer to the task to be dispatched
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	dispatch(TASK* pMyTask) {

			//  Do not accept a null task addtesss
			if (pMyTask == NULL) return;

			//  Clear the exception code in the task
			pMyTask->Exception = 0;

			//  Post the request to the MP queues, this call may block if the semaphore is already busy.
			MPQ.postTaskForExecution(pMyTask, ThisThread);

			//  Return to caller
			return;
		}

		//  operator << (TASK*)
		//
		//  Dispatches the passed task for execution
		//
		//  PARAMETERS
		//
		//		TASK*			-			Pointer to the task to be dispatched
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	operator << (TASK* pMyTask) {
			return dispatch(pMyTask);
		}

		//  operator << (TASK&)
		//
		//  Dispatches the passed task for execution
		//
		//  PARAMETERS
		//
		//		TASK&			-			Reference to the task to be dispatched
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void operator << (TASK& MyTask) {
			return dispatch(MyTask);
		}

		//  operator << (LOGMSG*)
		//
		//  Dispatches the passed message for logging
		//
		//  PARAMETERS
		//
		//		LOGMSG*			-			Pointer to the LOGMSG to be dispatched
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void operator << (LOGMSG* pMyLogMsg) {
			Log << pMyLogMsg;
			return;
		}

		//  operator << (LOGMSG&)
		//
		//  Dispatches the passed message for logging
		//
		//  PARAMETERS
		//
		//		LOGMSG&			-			Reference to the LOGMSG to be dispatched
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void operator << (LOGMSG& MyLogMsg) {
			Log << MyLogMsg;
			return;
		}

		//  getCompletedTask
		//
		//	This function will check the thread pool to determine if there are any tasks that have completed (and are marked for rejoin).
		//  If so they are returned. If there are no tasks to be returened then the response will discriminate between the state where
		//  there are tasks for the designated owner that are yet to complete or there are no more tasks for the designated owner.
		//  Tasks MUST be marked with the cTaskRejoin OR cTaskReoinOnException disposition in order to use this functionality.
		//
		//  PARAMETERS
		//
		//		void*			-			Void pointer value that identifies the owner
		//		TASK**			-			Address of the pointer in which thae task address will be returned
		//
		//  RETURNS
		//
		//		int				-			Return code indicating the state of the tasks for the designated owner
		//
		//									cTaskCompleted - A task has completed an it's address has been returned
		//									cTasksStillBusy - There are tasks that have not yet cimpleted for this woner
		//									cTasksAllCompleted - All tasks have now completed for this owner
		//
		//  NOTES:
		//

		int		getCompletedTask(void* pOwner, TASK** ppCompletedTask) {

			//  This call wraps the form that takes a reference to a pointer of the returned task
			return getCompletedTask(pOwner, *ppCompletedTask);
		}

		//  getCompletedTask
		//
		//	This function will check the thread pool to determine if there are any tasks that have completed (and are marked for rejoin).
		//  If so they are returned. If there are no tasks to be returened then the response will discriminate between the state where
		//  there are tasks for the designated owner that are yet to complete or there are no more tasks for the designated owner.
		//  Tasks MUST be marked with the cTaskRejoin OR cTaskReoinOnException disposition in order to use this functionality.
		//
		//  PARAMETERS
		//
		//		void*			-			Void pointer value that identifies the owner
		//		TASK*&			-			Reference of the pointer in which thae task address will be returned
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

		int		getCompletedTask(void* pOwner, TASK*& pCompletedTask) {

			//  Place the owner identifier in the return pointer
			pCompletedTask = reinterpret_cast<TASK*>(pOwner);

			//  Get the response from the thread pool
			return MPQ.returnCompletedTask(pCompletedTask, ThisThread);
		}

		//  getThreadID
		//
		//  Returns the ID of the current thread
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		THREADID		-		ID of the current thread.
		//
		//  NOTES:
		//  

		THREADID	getThreadID() { return ThisThread; }

		//  dismiss
		//
		//  Dismisses the MPLStreamBuf logging interface stream
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void	dismiss() {
			if (IsDismissed) return;
			delete Log.rdbuf();
			IsDismissed = true;
			return;
		}

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		MPQueues&		MPQ;																		//  MP server interface
		THREADID		ThisThread;																	//  Identifier of the thread for this dispatcher
		bool			IsDismissed;																//  Dismiss control

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

	};

}
