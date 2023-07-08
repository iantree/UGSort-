#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       Logger.h																							*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.0.0	(Build: 01)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the Logger class. The Logger class provides the					*
//* core server logic for implementing the server side of the MP logging sub-system..								*
//* The core functionality is designed to be run on a dedicated thread.												*
//*																													*
//*	USAGE:																											*
//*																													*
//*	NOTES:																											*
//*																													*
//*	1.	NOT COPYABLE																								*
//*	2.	NOT MOVEABLE																								*
//*	3.	NOT ASSIGNABLE																								*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 -		02/12/2017	-	Initial Release																		*
//*																													*
//*******************************************************************************************************************/

//  Include xymorg headers
#include	"../LPBHdrs.h"																			//  Language and Platform base headers
#include	"../types.h"																			//  xymorg type definitions
#include	"../consts.h"																			//  xymorg constant definitions

//  Sub-System headers
#include	"../Logging.h"																		//  Log Writer (File Stream)

//
//  All components are defined within the xymorg namespace
//
namespace xymorg {

	//
	//  Logger Class Definition
	//

	class Logger {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		static const int		cPauseMillis = 5;														//  Pause duration (milliseconds)
		static const int		cMaxWriteBatch = 5;														//  Number of messages written in a single write cycle
		static const int		cDrainLoopCycles = 25;													//  Number of cycles in the drain loop

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Constructor 
		//
		//  Constructs the application Logger, the logger is resposible for orchestrating the movement of log messages
		//  from the issuing client through the log queue and on to the log writer.
		//
		//  PARAMETERS:
		//
		//		LogQueue&			-		Reference to the Log Queue component
		//		LogWriter&			-		Reference to the System Log Writer
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		Logger(LogQueue& Q, LogWriter& SLW) : LQ(Q), Writer(SLW) {

			//  Initialise the state
			State = cNotStarted;

			//  Return to caller
			return;
		}

		//
		//  Mark the object as not copyable/moveable/assignable
		//

		Logger(const Logger&) = delete;
		Logger(Logger&&) = delete;

		Logger& operator = (const Logger&) = delete;
		Logger& operator = (Logger&&) = delete;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor                                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Destructor
		//
		//  Destroys the Logger object, draining the log queue first.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		~Logger() {

			//  If the logger has not already drained then do so
			if (State != cDrained) {
				//  Signal the logger to drain
				drain();

				//  Pause until the logger has drained the queue
				while (State != cDrained) sleep(MILLISECONDS(cPauseMillis));

			}

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  run
		//
		//  Main running loop for the logger
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES:
		//
		//	(1)	The termination sequence once triggered will idle for a number of cycles, this allows applications that
		//  have a weak threading model to complete output from child threads without a huge penalty to well formed applications.
		//

		void	run() {
			int			NewWrites = 0;																		//  Count of additional messages written
			int			DrainLoops = 0;																		//  Draining loop count

			//  Transition the state to active
			State = cActive;

			//  Continue running the loop until the state has transitioned to drained
			while (State != cDrained) {
				//  Service any client threads by enquing log messages
				LQ.servicePosters();

				//  If there are any log messages waiting to be written then write up to 5
				while (NewWrites < cMaxWriteBatch && LQ.getQSize() > 0) {
					LOGMSG* pMsg = LQ.dequeueMsg();
					if (pMsg != NULL) {
						Writer << *pMsg;
						NewWrites++;
					}
					else break;
				}

				//  If nothing was written then pause the thread
				if (State == cActive && NewWrites == 0) sleep(MILLISECONDS(cPauseMillis));

				//  If the queue is draining then see if it has drained
				if (State == cDraining && NewWrites == 0) {
					if (DrainLoops < cDrainLoopCycles) {
						sleep(MILLISECONDS(cPauseMillis));
						DrainLoops++;
					}
					else {
						if (DrainLoops < (cDrainLoopCycles + 1)) {
							//  Request stats from the Log Queue
							LQ.logStats();
							DrainLoops++;
						}
						else State = cDrained;
					}
				}

				//  Clear the New Writes control
				NewWrites = 0;
			}
#ifdef XY_MP_NEEDS_DEBUGGING
			//  Show the Log Writer statistics
			Writer.logStats();
#endif
			//  Return to caller -  this will allow the join() to complete on the calling thread
			return;
		}

		//  drain
		//
		//  Signal the logger to drain the queue and terminate
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	drain() {

			//  Mark the logger as draining
			State = cDraining;

			//  Return to caller
			return;
		}

		//  scram
		//
		//  Signal the logger terminate without draining the queue
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES:
		//

		void	scram() {
			//  Mark the logger as drained
			State = cDrained;

			//  Return to caller
			return;
		}

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Constants																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Logger States
		static const int	cNotStarted = 0;
		static const int	cActive = 1;
		static const int	cDraining = 2;
		static const int	cDrained = 3;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		volatile int	State;																		//  State of the logger
		LogQueue&		LQ;																			//  Log Queue component
		LogWriter&		Writer;																		//  Log message writer

	};

	//
	//  Constants required at runtime
	//

	const int Logger::cPauseMillis;
}
