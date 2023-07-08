#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       Logging.h																							*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.1.0	(Build: 02)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2022 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definitions for all components used in the xymorg logging chain.					*
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
//*	1.0.0 -		02/12/2017	-	Initial Release																		*
//*	1.1.0 -		18/04/2022	-	Simplified logging chain															*
//*																													*
//*******************************************************************************************************************/

//
//  Include core xymorg headers
//

#include	"LPBHdrs.h"																		//  Language and Platform base headers
#include	"types.h"																		//  xymorg type definitions
#include	"consts.h"																		//  xymorg constant definitions
#include	"StringThing.h"																	//  String manipulators

//  Additional Platform Headers
#include    <queue>																			//  STL queue

//  Additional headers for Multi-Threaded
#include	"MP/Primitives.h"																//  MP Primitives

//  Defaults for single threaded operation
#ifndef  XY_MAX_THREADS
#define  XY_MAX_THREADS XY_DEFAULT_MAX_THREADS
#endif

//
//  All components are defined within the xymorg namespace
//
namespace xymorg {

	//
	//  Set the format to use for the timestamp on log records. If this has not been predefined then set it to the default
	//
#ifndef USE_LOG_TIMESTAMP_FMT
#define USE_LOG_TIMESTAMP_FMT				DEFAULT_LOG_TIMESTAMP_FMT
#endif

	//
	//  LOGMSG Class Definition
	//
	//	LOGMSG objects hold information that is to be sent to the logging interface
	//

	class LOGMSG {
	public:
		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Constructor 
		//
		//  Constructs a new LOGMSG object
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		LOGMSG() {

			Issuer = 0;
			TimeStamp = CLOCK::now();
			Text[0] = '\0';
			Continuation = nullptr;

			//  Return to caller
			return;
		}

		//  Variadic Constructor 
		//
		//  Constructs a new LOGMSG object from the format string and appropriate parameters
		//
		//  PARAMETERS:
		//
		//		const char *		-		Pointer to the format string used to pattern the message
		//		...					-		Variadic parameters used by the format string
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		LOGMSG(const char* Fmt, ...)
		{
			va_list             vArgs;                                                                           //  Variable argument list
			char*				pScan = nullptr;                                                                 //  Scanning pointer
			char*				FmtBuffer = new char[2 * 4096];				                                     //  Temporary buffer for formatting

			Issuer = 0;
			TimeStamp = CLOCK::now();
			Continuation = nullptr;

			//  Boundary conditions the Format/Message string is not specified or empty
			Text[0] = '\0';
			if (Fmt == nullptr) return;
			if (Fmt[0] == '\0') return;

			//  Perform any printf substitutions to form the complete buffer
			va_start(vArgs, Fmt);
			vsprintf_s(FmtBuffer, 8192, Fmt, vArgs);
			va_end(vArgs);

			//  Trim any trailing control (cr/lf) characters from the end of the formatted message
			pScan = FmtBuffer + strlen(FmtBuffer);
			while (*pScan < ' ' && pScan > FmtBuffer) {
				*pScan = '\0';
				pScan--;
			}

			//  Build the multi-line message (will build a single line message if there are no line splits)
			buildMultiLineMessage(FmtBuffer);

			//  Dismiss the format buffer
			delete[] FmtBuffer;

			//  Return to caller
			return;
		}

		//  Chained Constructor 
		//
		//  Constructs a new LOGMSG object from the format string and appropriate parameters.
		//  The LOGMSG is appended to the chain of LOGMSGs
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		LOGMSG(LOGMSG* Chain, const char* Fmt, ...) {
			va_list             vArgs;                                                                           //  Variable argument list
			char*				pScan = nullptr;                                                                 //  Scanning pointer
			char*				FmtBuffer = new char[4096];					                                     //  Temporary buffer for formatting

			Issuer = 0;
			TimeStamp = CLOCK::now();
			Continuation = nullptr;

			//  Boundary conditions the Format/Message string is not specified or empty
			Text[0] = '\0';
			if (Fmt == nullptr) return;
			if (Fmt[0] == '\0') return;

			//  Perform any printf substitutions to form the complete buffer
			va_start(vArgs, Fmt);
			vsprintf_s(FmtBuffer, 4096, Fmt, vArgs);
			va_end(vArgs);

			//  Trim any trailing control (cr/lf) characters from the end of the formatted message
			pScan = FmtBuffer + strlen(FmtBuffer);
			while (*pScan < ' ' && pScan > FmtBuffer) {
				*pScan = '\0';
				pScan--;
			}

			//  Build the multi-line message (will build a single line message if there are no line splits)
			buildMultiLineMessage(FmtBuffer);

			//  Dismiss the format buffer
			delete[] FmtBuffer;

			//  Append this message (or chain) to the end of the passed chain
			if (Chain != nullptr) {
				while (Chain->Continuation != nullptr) Chain = Chain->Continuation;
				Chain->Continuation = this;
			}

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor                                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Default Destructor
		//
		//  Destroys the LOGMSG object and any chained LOGMSG objects.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		~LOGMSG() {

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Members                                                                                                *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		TIMER                   TimeStamp;                                                                   //  Timestamp of the message
		THREADID                Issuer;                                                                      //  ID of the thread that issued the message
		LOGMSG*					Continuation;                                                                //  Continutation message
		char                    Text[MAX_LOG_TEXT + 1];                                                      //  Message text

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  operator += (Append)
		//
		//  This function will append the passed log message to the end of the chain of continuation messages.
		//
		//  PARAMETERS
		//
		//      LOGMSG*     -       Pointer to the message string to be formed into segmented messages
		//
		//  RETURNS
		//
		//      LOGMSG&     -       Reference to the current log message
		//
		//  NOTES
		//
		LOGMSG& operator += (const LOGMSG* rhs)
		{
			LOGMSG* LastMsg = this;                                                                      //  Pointer to the last message on the chain

			while (LastMsg->Continuation != nullptr) LastMsg = LastMsg->Continuation;

			//  Append the passed message
			LastMsg->Continuation = (LOGMSG*)rhs;

			//  Return the reference to the current object
			return *this;
		}

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  buildMultiLineMessage
		//
		//  This function constructs a segmented log message from the passed text string. A segmented message consists of
		//  a chain of LOGMSG objects each with up to MAX_LOG_TEXT characters of the message.
		//	The message is split into multiple spegments depending on the presence of line splits '\n' in the passed text.
		//
		//  PARAMETERS
		//
		//      char*       -       Pointer to the message string to be formed into segmented messages
		//      size_t      -       Total length in bytes of the message string
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		void	buildMultiLineMessage(char* Msg) {

			//  If the message is single part then populate this and any chain as appropriate
			if (strlen(Msg) <= MAX_LOG_TEXT) strcpy_s(Text, MAX_LOG_TEXT + 1, Msg);
			else Continuation = buildSegmentedMessage(Msg, strlen(Msg));

			//  Return to caller
			return;
		}

		//  buildSegmentedMessage
		//
		//  This function constructs a segmented log message from the passed text string. A segmented message consists of
		//  a chain of LOGMSG objects each with up to MAX_LOG_TEXT characters of the message.
		//  The function will preferentially break at natural line breaks. If a line is too long then it will
		//  break at selected characters.
		//
		//  PARAMETERS
		//
		//      char*       -       Pointer to the message string to be formed into segmented messages
		//      size_t      -       Total length in bytes of the message string
		//
		//  RETURNS
		//
		//  NOTES
		//
		//  The function is ONLY invoked in conditions where the text length is greated than MAX_LOG_TEXT characters.
		//

		LOGMSG* buildSegmentedMessage(const char* Msg, size_t MsgLen)
		{
			LOGMSG*				FirstContinuation = nullptr;                                                     //  First continuation segment
			LOGMSG*				CurrentContinuation = nullptr;                                                   //  Current continuation segment
			LOGMSG*				PreviousContinuation = nullptr;                                                  //  Previous continuation segment
			size_t              RemainingChars = MsgLen;                                                         //  Count of charaters remaining to be written
			const char*			pStartSeg = Msg;                                                                 //  Pointer to the start of the message segment
			size_t              SegmentLen = MAX_LOG_TEXT;                                                       //  Length of the message segment

			//  Find the natural end of the primary message segment
			SegmentLen = findBestSplit(Msg);

			//  Copy the primary text segment into the message text
			memcpy(Text, pStartSeg, SegmentLen);
			Text[SegmentLen] = '\0';

			//  Update the pointers ready to process the overflow segments
			pStartSeg = pStartSeg + SegmentLen;
			RemainingChars = RemainingChars - SegmentLen;

			//  Loop generating continuation messages in turn
			while (RemainingChars > 0)
			{
				//  Form the chain of continuation messages
				PreviousContinuation = CurrentContinuation;
				CurrentContinuation = new LOGMSG();
				if (FirstContinuation == nullptr) FirstContinuation = CurrentContinuation;
				else PreviousContinuation->Continuation = CurrentContinuation;

				//  Determine if the residue will all fit into the current segment
				if (RemainingChars <= MAX_LOG_TEXT)
				{
					strcpy_s(CurrentContinuation->Text, MAX_LOG_TEXT + 1, pStartSeg);
					RemainingChars = 0;
				}
				else
				{
					//  Find the natural end of the primary message segment
					SegmentLen = findBestSplit(pStartSeg);

					//  Eliminate any whitespace at the start of the text segment
					while (*pStartSeg == ' ' && SegmentLen > 10)
					{
						RemainingChars--;
						SegmentLen--;
						pStartSeg++;
					}

					//  Copy the primary text segment into the message text
					memcpy(CurrentContinuation->Text, pStartSeg, SegmentLen);
					CurrentContinuation->Text[SegmentLen] = '\0';

					//  Update the pointers ready to process the overflow segments
					pStartSeg = pStartSeg + SegmentLen;
					RemainingChars = RemainingChars - SegmentLen;
				}
			}

			return FirstContinuation;
		}

		//  findBestSplit
		//
		//  This function determines the optimal point at which to split an oversized message
		//
		//  PARAMETERS
		//
		//      char*       -       Pointer to the message string to be formed into segmented messages
		//
		//  RETURNS
		//
		//      size_t      -       Size of the optimal initial segment
		//
		//  NOTES
		//
		//

		size_t		findBestSplit(const char* Msg) {
			const char*		pLB = nullptr;														//  Pointer to the first Line break
			size_t			SegLen = 0;															//  Segment length

			//  Safety
			if (Msg == nullptr) return 0;
			if (*Msg == '\0') return 0;
			if (strlen(Msg) <= MAX_LOG_TEXT) return strlen(Msg);

			//  Find the first line break
			pLB = strchr(Msg, '\n');
			if (pLB == nullptr) pLB = Msg + strlen(Msg);

			//  If the Line Break is within the max permissible size then use that as the split point
			SegLen = (pLB - Msg) + 1;
			if (SegLen <= MAX_LOG_TEXT) return SegLen;

			//  Search for a split character that is comfortably within the MAX Log Text size
			pLB = Msg + (MAX_LOG_TEXT - 35);
			while (pLB > Msg && !breakCharacter(*pLB)) pLB--;
			SegLen = (pLB - Msg) + 1;
			if (SegLen > 1) return SegLen;
			return MAX_LOG_TEXT - 35;
		}


		//  breakMessage
		//
		//  This function determines if the character passed is suitable for breaking the message;
		//
		//  PARAMETERS
		//
		//      char        -       Character from the message string
		//
		//  RETURNS
		//
		//      bool        -       true if the character is a break character otherwise false
		//
		//  NOTES
		//
		//

		bool  breakCharacter(char ThisChar)
		{
			//  Check for suitable break characters
			if (ThisChar == ' ') return true;
			if (ThisChar == ',') return true;
			if (ThisChar == '.') return true;
			if (ThisChar == ')') return true;
			if (ThisChar == ']') return true;
			if (ThisChar == '}') return true;
			if (ThisChar == ':') return true;
			if (ThisChar == ';') return true;
			if (ThisChar == '!') return true;
			if (ThisChar == '/') return true;
			if (ThisChar == '\\') return true;
			if (ThisChar == '-') return true;

			//  Return indicator that this is not a suitable character for breaking on
			return false;
		}
	};

	//
	//  LogWriter Class Definition
	//

	class LogWriter : public std::ofstream {
	public:
		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Default Constructor 
		//
		//  Constructs the standard application log object (non-proxy)
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		LogWriter() : Echoing(false), Decorating(true), ChainsWritten(0), MsgsWritten(0) {

			//  Return to caller
			return;
		}

		//
		//  Mark the object as not copyable/moveable/assignable
		//

		LogWriter(const LogWriter&) = delete;
		LogWriter(LogWriter&&) = delete;

		LogWriter& operator = (const LogWriter&) = delete;
		LogWriter& operator = (LogWriter&&) = delete;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor                                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Destructor
		//
		//  Destroys the LogWriter object, closing the ofstream associated with it.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		~LogWriter() {
			//  Close the logging stream
			close();

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Operator overload << LOGMSG
		//
		//  The stream ONLY supports inserting messages that have been encapsulated in a LOGMSG object.
		//  The LOGMSG object is deleted after it is written so must be made available from the heap.
		//
		//
		//  PARAMETERS:
		//
		//		LOGMSG&			-		Reference to the log message (or chain) to be inserted into the log stream
		//
		//  RETURNS:
		//
		//		LogWriter&		-		Self Reference
		//
		//  NOTES:
		//  

		LogWriter& operator << (LOGMSG& Msg) {
			time_t			ttNow = 0;																			//  Submission Timestamp
			char*			pTail = nullptr;																	//  Pointer to the tail of the message																

			//  Extract the timestamp
			ttNow = CLOCK::to_time_t(Msg.TimeStamp);

			//  Trim any trailing control (cr/lf) characters from the tail of the message text
			pTail = Msg.Text + strlen(Msg.Text);
			while (pTail >= Msg.Text && *pTail < ' ') {
				*pTail = '\0';
				pTail--;
			}

#ifdef   XY_NEEDS_MP
			//  Output the formatted message - timestamp : Message Text [Issuer]
			if (Decorating) {
				if (is_open()) stamp(ttNow) << Msg.Text << " [" << Msg.Issuer << "]" << std::endl;
				else stamp();
			}
			else {
				if (is_open()) *this << Msg.Text << std::endl;
			}

			//  If the log stream is not available or we are echoing the log then send the message to std::cout
			if (Decorating) {
				if ((!is_open()) || Echoing) std::cout << Msg.Text << " [" << Msg.Issuer << "]" << std::endl;
			}
			else {
				if ((!is_open()) || Echoing) std::cout << Msg.Text << std::endl;
			}
#else
			//  Output the formatted message - timestamp : Message Text
			if (Decorating) {
				if (is_open()) stamp(ttNow) << Msg.Text << std::endl;
				else stamp();
			}
			else {
				if (is_open()) *this << Msg.Text << std::endl;
			}

			//  If the log stream is not available or we are echoing the log then send the message to std::cout
			if ((!is_open()) || Echoing) std::cout << Msg.Text << std::endl;

#endif

			//  If the message is a chain then output the next segment
			if (Msg.Continuation != nullptr) *this << *(Msg.Continuation);

			//  Update the writer stats
			MsgsWritten++;
			if (Msg.Continuation == nullptr) ChainsWritten++;

			//  Destroy the message segment
			delete& Msg;

			//  Return to caller
			return *this;
		}

		//  stamp
		//
		//  Inserts a formatted date time stamp into the logging stream
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		LogWriter&			-		Reference to the LogWriter
		//
		//  NOTES:
		//  

		LogWriter& stamp() {
			time_t			ttNow = 0;																			//  Submission Timestamp

			//  Set current time stamp
			time(&ttNow);
			return stamp(ttNow);
		}

		//  stamp
		//
		//  Inserts a formatted date time stamp into the logging stream
		//
		//  PARAMETERS:
		//
		//		time_t			-		Date & time to be stamped on the log record
		//
		//  RETURNS:
		//
		//		LogWriter&			-		Reference to the LogWriter
		//
		//  NOTES:
		//  

		LogWriter& stamp(time_t& ttNow) {
			struct tm		tmLocalStore;																		//  Storage for local time
			struct tm*		ptmLocal = nullptr;																	//  Local time structure
			char			szPrefix[MAX_PATH + 1];																//  Log record prefix

			//  Format the timestamp
			ptmLocal = localtime_safe(&ttNow, &tmLocalStore);
			strftime(szPrefix, MAX_PATH, USE_LOG_TIMESTAMP_FMT, ptmLocal);

			//  If the log stream is not available or we are echoing the log then send the prefix to std::cout
			if (!is_open() || Echoing) std::cout << szPrefix << ": ";

			//  If the log stream is not available then do nothing more
			if (!is_open()) return *this;

			//  Write to the log
			*this << szPrefix << ": ";

			//  Return to caller
			return *this;
		}

		//  setEcho
		//
		//  Sets the Log Writer to echoing mode, output is Tee'd to std::cout
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void	setEcho() { Echoing = true; return; }

		//  clearEcho
		//
		//  Sets the Log Writer to non-echoing mode, output is NOT Tee'd to std::cout
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void	clearEcho() { Echoing = false; return; }

		//  setDecorate
		//
		//  Sets the Log Writer to decorating mode, messages are decorated with timestamp and thread ID.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void	setDecorate() { Decorating = true; return; }

		//  clearDecorate
		//
		//  Sets the Log Writer to non decorating mode, messages are NOT decorated with timestamp and thread ID.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void	clearDecorate() { Decorating = false; return; }

		//  logStats
		//
		//  This function will write log stats to the log queue
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:

		void	logStats() {
			//  Log the basic statistics
			*this << *(new LOGMSG("LOG WRITER: Chains written: %i, Messages written: %i.", ChainsWritten, MsgsWritten));

			//  Return to caller
			return;
		}

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		bool		Echoing;																		//  Echo control
		bool		Decorating;																		//  Decorating Log Lines

		//  Statistics
		size_t		ChainsWritten;																	//  Message chains written
		size_t		MsgsWritten;																	//  Messages written

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

	};

	//
	//  LogQueue Class Definition for single threaded applications. There is no log queue, messages are passed directly to the writer.
	//

	class LogQueue : private std::queue<LOGMSG*> {
	public:
		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Constructor 
		//
		//  Constructs a new Log Queue object
		//
		//  PARAMETERS:
		//
		//		LogWriter&			-		Reference to the Log Writer
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		LogQueue(LogWriter& LW) : Writer(LW) {

			//  Set the default constraints
			QThrottleLimit = 200;

			//  Clear the statistics
			MaxQSize = 0;
			ChainsEnqueued = 0;
			ChainsDequeued = 0;

			//  Return to caller
			return;
		}

		//
		//  Mark the object as not copyable/moveable/assignable
		//

		LogQueue(const LogQueue&) = delete;
		LogQueue(LogQueue&&) = delete;

		LogQueue& operator = (const LogQueue&) = delete;
		LogQueue& operator = (LogQueue&&) = delete;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor                                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Destructor
		//
		//  Destroys the LogQueue object, including any contents of the queue.
		//	The LQ should only be destroyed after it has been drained (orderly sub-system shutdown).
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		~LogQueue() {
#ifdef XY_NEEDS_MP
			//  To prevent deadlocks during shutdown clear any semaphores
			for (THREADID tIndex = 0; tIndex < XY_MAX_THREADS; tIndex++) {
				if (LogMsg[tIndex].isPosted()) LogMsg[tIndex].accept();
			}
#endif
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

		//  setThrottleLimit
		//
		//  This function will set the limit on the Log Queue size before throttling will take place
		//
		//  PARAMETERS:
		//
		//		size_t		-		New Throttle limit
		//
		//  RETURNS:
		//
		//  NOTES:

		void	setThrottleLimit(size_t NewThrottleLimit) {
			if (NewThrottleLimit < 5) return;
			QThrottleLimit = NewThrottleLimit;
			return;
		}

		//  getQSize
		//
		//  This function will return the count of messages currently on the queue
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		size_t		-		Count of messages current;y on the queue
		//
		//  NOTES:

		size_t	getQSize() { return size(); }

		//  servicePosters
		//
		//  This function will check for any semaphores posted by log requesters on client threads and service them by moving
		//	the LOGMSG or chain to the log queue and clearing the semaphore.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//
		//	(1)		If the number of messages on the log queue is at or above the throttle limit then no semaphores are serviced
		//

		void	servicePosters() {
			THREADID			tIndex = 0;																				//  Index into the semaphore array
			THREADID			Selected = 0;																			//  Selected semaphore index

			//
			//  Enqueue all messages that are posted - in sequence until all are done or the queue throttling limit is reached
			//
			while (Selected != XY_MAX_THREADS && size() <= QThrottleLimit) {

				//  Find the oldest logmessage posted
				Selected = XY_MAX_THREADS;
				for (tIndex = 0; tIndex < XY_MAX_THREADS; tIndex++) {
					if (LogMsg[tIndex].isPosted()) {
						if (Selected == XY_MAX_THREADS) Selected = tIndex;
						else if (LogMsg[tIndex].peekValue()->TimeStamp < LogMsg[Selected].peekValue()->TimeStamp) Selected = tIndex;
					}
				}

				//  Append the selected message to the end of the log queue and clear the semaphore
				if (Selected != XY_MAX_THREADS) {
					push(LogMsg[Selected].accept());
					ChainsEnqueued++;

					//  Adjust the max queue size if necessary
					if (size() > MaxQSize) MaxQSize = size();
				}
			}

			//  Return to caller
			return;
		}

		//  dequeueMsg
		//
		//  This function will remove the message from the head of the queue and return it to the caller.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		LOGMSG*			-		Pointer to the dequeued message, NULL if the queue is empty
		//
		//  NOTES:
		//

		LOGMSG* dequeueMsg() {
			if (empty()) return nullptr;
			LOGMSG* Message = front();
			pop();
			ChainsDequeued++;
			return Message;
		}

		//  logStats
		//
		//  This function will write log stats to the log queue, if requested then the semaphore stats will be written
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		void	logStats() {

			//  Log the basic statistics
#ifdef XY_NEEDS_MP
			push(new LOGMSG("=== END OF LOG ===: Enqueued: %i, Dequeued: %i, Queue Size: %i, High Watermark: %i.", ChainsEnqueued, ChainsDequeued, size(), MaxQSize));
#ifdef XY_MP_NEEDS_DEBUGGING
			//  log the semaphore stats
			for (THREADID tIndex = 0; tIndex < XY_MAX_THREADS; tIndex++) {
				if (LogMsg[tIndex].Posts > 0) {
					push(new LOGMSG("LOG QUEUE SEM: [%i], Posts: %i, Waits: %i, Wait Quanta: %i.", tIndex, LogMsg[tIndex].Posts, LogMsg[tIndex].PostWaits, LogMsg[tIndex].PostWaitQuanta));
				}
			}
#endif
#else
			//  Log the basic statistics
			Writer << *(new LOGMSG("=== END OF LOG ===: Enqueued: %i, Dequeued: %i, Queue Size: %i, High Watermark: %i.", ChainsEnqueued, ChainsDequeued, 0, 1));
#endif
			//  Return to caller
			return;
		}

		//
		//  Client Interface
		//

		//  operator << (insert)
		//
		//  This function will post the semaphore belonging to the client thread with the address of the LOGMSG (chain) to be inserted into the queue.
		//	The semaphore will be serviced by server.
		//
		//  PARAMETERS:
		//
		//		LOGMSG*		-		Pointer to the log message to be posted
		//
		//  RETURNS:
		//
		//		LogQueue&	-		Self Reference
		//
		//  NOTES:
		//

		LogQueue& operator << (LOGMSG* pMsg) {

			//  Safety checks
			if (pMsg == nullptr) return *this;
#ifdef XY_NEEDS_MP	
			if (pMsg->Issuer >= XY_MAX_THREADS) {
				delete pMsg;
				return *this;
			}

			//  Post the requesters semaphore with the new log message - this may block
			LogMsg[pMsg->Issuer].post(pMsg, pMsg->Issuer);
#else
			ChainsDequeued++;
			ChainsEnqueued++;

			//  Pass the message to the log writer
			Writer << *pMsg;
#endif
			//  Return to caller
			return *this;
		}

		//  setDecorate
		//
		//  Sets the Log Writer to decorating mode, messages are decorated with timestamp and thread ID.
		//  Calls will block until the queue is drained.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void	setDecorate() {
#ifdef XY_NEEDS_MP
			while (size() > 0 || areAnySemsPosted()) sleep(MILLISECONDS(5));
#endif
			Writer.setDecorate();
			return;
		}

		//  clearDecorate
		//
		//  Sets the Log Writer to non decorating mode, messages are NOT decorated with timestamp and thread ID.
		//  Calls will block until the queue is drained.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void	clearDecorate() {
#ifdef XY_NEEDS_MP
			while (size() > 0 || areAnySemsPosted()) sleep(MILLISECONDS(5));
#endif
			Writer.clearDecorate();
			return;
		}

		//  setEcho
		//
		//  Sets the Log Writer to echoing mode, output is Tee'd to std::cout
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void	setEcho() { Writer.setEcho(); return; }

		//  clearEcho
		//
		//  Sets the Log Writer to non-echoing mode, output is NOT Tee'd to std::cout
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void	clearEcho() { Writer.clearEcho(); return; }

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		LogWriter&				Writer;																					//  Log Writer

		//  Per thread semaphores for signalling that LOGMSG (or chain) is available for posting
		SEMAPHORE<LOGMSG*>		LogMsg[XY_MAX_THREADS];																	//  Message logging semaphore

		//  Queue constraints
		size_t					QThrottleLimit;																			//  Queue size limit for throttling

		//  Statistics
		size_t					MaxQSize;																				//  Maximum size of the queue
		size_t					ChainsEnqueued;																			//  Count of chains enqueued
		size_t					ChainsDequeued;																			//  Count of chains dequeued

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  areAnySemsPosted
		//
		//  Tests if any semaphores are posted
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		bool		-		true if any are posted, false if none are posted
		//
		//  NOTES:
		//  

		bool	areAnySemsPosted() {

			for (int tIndex = 0; tIndex < XY_MAX_THREADS; tIndex++) {
				if (LogMsg[tIndex].isPosted()) return true;
			}
			return false;
		}

	};

	//
	//  LogStreamBuf Class Definition
	//
	//  The LogStreamBuf extends the std::streambuf class to override the sync() and overflow() methods.
	//  The synch() method dequeues the current buffer content into a new LOGMSG object and dispatches it to the Log Queue.
	//

	class LogStreamBuf : public std::streambuf {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Constructor 
		// 
		//  Constructs the stream buffer for MPL interfacing
		//
		//  PARAMETERS:
		//
		//		LogQueue&			-		Reference to the LogQueue singleton object
		//		THREADID			-		ID of the thread that owns the log stream
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		LogStreamBuf(xymorg::LogQueue& Q, THREADID Thread) : Owner(Thread), LQ(Q), Buffer(nullptr) {
			//  The buffer is allocated as double the size to allow for "%" to "%%" safe conversion
			Buffer = new char[2 * 4096];																//  Allocate the buffer
			setp(Buffer, Buffer + 4096);																//  Setup the buffer
			return;
		}

		~LogStreamBuf() {
			dismiss();
			return;
		}

		//  Getters for underlying MP constructs
		THREADID		getOwner() { return Owner; }
		LogQueue& getQueue() { return LQ; }

		//  dismiss
		//
		//  Clears the buffer allocation
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void dismiss() {
			setp(nullptr, nullptr);
			if (Buffer != nullptr) {
				delete[] Buffer;
				Buffer = nullptr;
			}

			return;
		}

	protected:
		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Protected Functions                                                                                           *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  sync
		//
		//  Override for the std::streambuf sync(). Marshalls the current buffer content to a LOGMSG object.
		//  Sets the Timestamp & Owner in the LOGMSG and dispatches the LOGMSG to the Log Queue.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		int			-		0 implies success, -1 implies failure
		//
		//  NOTES:
		//  

		virtual int sync() {

			//  Put a '\0' character to terminate the buffer contents as a string
			*pptr() = '\0';

			//  Convert all "%" to "%%" for safe substitution
			st_strrepall(pbase(), strlen(pbase()), "%", "%%");

			//  Build the LOGMSG -  This will automatically get converted to a chain if the content is longer than the internal LOGMSG buffer
			LOGMSG* pMsg = new LOGMSG(pbase());

			//  Set the timestamp and issuer
			pMsg->TimeStamp = CLOCK::now();
			pMsg->Issuer = Owner;

			//  Dispatch to the log queue
			LQ << pMsg;

			//  Reset the buffer positions
			setp(Buffer, Buffer + 4096);

			//  Return success
			return 0;
		}

		//  overflow
		//
		//  Override for the std::streambuf overflow(). forces a synch() which will empty the buffer.
		//
		//  PARAMETERS:
		//
		//		int			-		Character to be written
		//
		//  RETURNS:
		//
		//		int			-		0 implies success, -1 implies failure
		//
		//  NOTES:
		//  

		virtual int overflow(int ctowrite) {

			//  Force a synch 
			sync();

			//  Store the char
			Buffer[0] = char(ctowrite);
			pbump(1);

			//  Return the character to be inserted
			return traits_type::to_int_type(char(ctowrite));
		}

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		THREADID			Owner;																		//  Owning thread
		LogQueue&			LQ;																			//  Log Queue
		char*				Buffer;																		//  Buffer space

	};

	//
	//  Stream insert << non-member functions
	//

	//
	//  LOGMSG Inserts
	//

	std::ostream& operator << (std::ostream& Stream, xymorg::LOGMSG* pLMsg) {
		xymorg::LogStreamBuf* SB = (xymorg::LogStreamBuf*)Stream.rdbuf();

		//  Set the thread id of the stream into each message segment in the chain
		pLMsg->Issuer = SB->getOwner();
		xymorg::LOGMSG* pNextLM = pLMsg->Continuation;
		while (pNextLM != nullptr) {
			pNextLM->Issuer = SB->getOwner();
			pNextLM = pNextLM->Continuation;
		}

		//  Dispatch the message to the log queue
		SB->getQueue() << pLMsg;

		//  Return to caller
		return Stream;
	}

	std::ostream& operator << (std::ostream& Stream, xymorg::LOGMSG& LMsg) {
		xymorg::LOGMSG* pLMsg = new xymorg::LOGMSG(LMsg);
		xymorg::LogStreamBuf* SB = (xymorg::LogStreamBuf*)Stream.rdbuf();

		//  Set the thread id of the stream into each message segment in the chain
		pLMsg->Issuer = SB->getOwner();
		xymorg::LOGMSG* pNextLM = pLMsg->Continuation;
		while (pNextLM != nullptr) {
			pNextLM->Issuer = SB->getOwner();
			pNextLM = pNextLM->Continuation;
		}

		//  Dispatch the message to the log queue
		SB->getQueue() << pLMsg;

		//  Return to caller
		return Stream;
	}

	//
	//  Non-member functions for xymorg stream modifiers
	//

	//  decorate
	//
	//  Sets the log stream to "decorated" mode, timestamp and thread ID are added to each log line
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//		std::ostream&			-		Reference to the log stream
	//
	//  NOTES:
	//  

	std::ostream& decorate(std::ostream& Stream) {
		LogStreamBuf* SB = (LogStreamBuf*)Stream.rdbuf();

		SB->getQueue().setDecorate();

		return Stream;
	}

	//  undecorate
	//
	//  Sets the log stream to "undecorated" mode, timestamp and thread ID are NOT added to each log line
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//		std::ostream&			-		Reference to the log stream
	//
	//  NOTES:
	//  

	std::ostream& undecorate(std::ostream& Stream) {
		LogStreamBuf* SB = (LogStreamBuf*)Stream.rdbuf();

		SB->getQueue().clearDecorate();

		return Stream;
	}

	//  echo
	//
	//  Sets the log stream to "echo" mode, all messages are copied to the std::cout stream
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//		std::ostream&			-		Reference to the log stream
	//
	//  NOTES:
	//  

	std::ostream& echo(std::ostream& Stream) {
		LogStreamBuf* SB = (LogStreamBuf*)Stream.rdbuf();

		SB->getQueue().setEcho();

		return Stream;
	}

	//  noecho
	//
	//  Sets the log stream to "noecho" mode, NO messages are copied to the std::cout stream
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//		std::ostream&			-		Reference to the log stream
	//
	//  NOTES:
	//  

	std::ostream& noecho(std::ostream& Stream) {
		LogStreamBuf* SB = (LogStreamBuf*)Stream.rdbuf();

		SB->getQueue().clearEcho();

		return Stream;
	}

}
