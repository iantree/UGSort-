#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       AppConfig.h																						*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.0.4	(Build: 05)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the AppConfig class. The AppConfig class provides the 				*
//* basic configuration for the application and linkage to xrmorg sub-system components.							*
//*																													*
//*	USAGE:																											*
//*																													*
//*		Define the subsystem requirements (XY_NEEDS_XXX) before including this header.								*
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
//*	1.0.1 -		30/10/2022	-	Autonomics made configurable.														*
//*	1.0.2 -		09/11/2022	-	Cleanup.																			*
//*	1.0.3 -		10/11/2022	-	Changes to NetPnP.																	*
//*	1.0.4 -		24/11/2022	-	Inclusion of TextRenderer.															*
//*																													*
//*******************************************************************************************************************/

//
//  Include core xymorg headers
//

#include	"LPBHdrs.h"																		//  Language and Platform base headers
#include	"types.h"																		//  xymorg type definitions
#include	"consts.h"																		//  xymorg constant definitions

//  Include additional xymorg components

#include	"StringPool.h"																	//  String Pool
#include	"VRMapper.h"																	//  Virtual Resource Mapper
#include	"Logging.h"																		//  xymorg Logging
#include	"XMLMicroParser.h"																//  XML Micro Parser

//  Configuration nodes and attribute names
constexpr auto LOGGING_NODE = "logging";													//  Logging Node;
constexpr auto VERBOSE_PARM = "verbose";													//  Verbose state of logging;
constexpr auto ECHO_PARM = "echo";															//  Echo logging required;
constexpr auto AUTO_NODE = "autonomics";													//  Autonomics Node
constexpr auto ENABLED_PARM = "enabled";													//  Autonomics enabler/disabler
constexpr auto MCYCLES_PARM = "mcycles";													//  Autonomics monitor cycles
constexpr auto ACYCLES_PARM = "acycles";													//  Autonomics action cycles

//  Include optional xymorg components

//  1.  Multi-Processing

#ifdef XY_NEEDS_MP
#include	"MP/Logger.h"																	//  Log Server
#include	"MP/MPQueues.h"																	//  MP Queues
#include	"MP/ThreadPool.h"																//  MP ThreadPool
#include	"MP/Dispatcher.h"																//  MP Dispatcher

//  Configuration nodes and attribute names
#define		THREADS_NODE			"threads"
#define		MINT_PARM				"min"
#define		MAXT_PARM				"max"
#endif

//  2.  Cryptographic Processing

#ifdef  XY_NEEDS_CRYPTO
#include	"CRYPTO/SObjectPool.h"															//  Crypto  -  secure object pool
#endif

//  3.  Network I/O 

#ifdef  XY_NEEDS_NETIO
#include	"NET/NetInit.h"																	//  Network I/O Initialisation and termination
#include	"NET/NetPnP.h"																	//  Network UPnP controller
#endif

//  4.  Web Enabled UI
#ifdef  XY_NEEDS_WEBUI
#include	"WEB/Wezzer.h"																	//  Wezzer - Micro Web server

//  Configuration nodes and attribute names
#define		WEBUI_NODE				"webui"													//  Container Node
#define		IPV4_NODE				"ipv4"													//  IPv4 Node
#define		IPV6_NODE				"ipv6"													//  IPv6 Node
#define		ROOT_PARM				"root"													//  Web root directory
#define		SCOPE_PARM				"scope"													//  Listener scope
#define		BIND_PARM				"bind"													//  Adapter specific binding
#define		PORT_PARM				"port"													//  Desired (hint) port
#define		AUTOLAUNCH_PARM			"autolaunch"											//  Autolaunch setting
#define		MAXCONN_PARM			"maxconnections"										//  Max Connections setting
#define		LISTEN_PARM				"listen"												//  Listen assertion
#endif

//  5.  Text Rendering Engine
#ifdef XY_NEEDS_TEXT
#if (defined(_WIN32) || defined(_WIN64))
#include	"WINDOWS/TextRenderer.h"														//  Windows Text Rendering Engine
#else
#include	"UNIX/TextRenderer.h"															//  UNIX/Linux Text Rendering Engine
#endif
#endif

//
//  Use default config encryption if not defined
//

#ifndef SEC_CFG_SCHEME
#define SEC_CFG_SCHEME DEFAULT_SEC_CFG_SCHEME
#endif

#ifndef SEC_CFG_KSIZE
#define SEC_CFG_KSIZE DEFAULT_SEC_CFG_KSIZE
#endif

#ifndef SEC_CFG_KEY
#define SEC_CFG_KEY DEFAULT_SEC_CFG_KEY
#endif

//
//  All components are defined within the xymorg namespace
//
namespace xymorg {

	//
	//  Set the format to use for the timestamp portion of the log file name. If this has not been predefined then set it to the default
	//
#ifndef USE_LOGNAME_TIMESTAMP_FMT	
#define USE_LOGNAME_TIMESTAMP_FMT			DEFAULT_LOGNAME_TIMESTAMP_FMT
#endif

	//
	//  Set the format to use for the log file name. If this has not been predefined then set it to the default
	//
#ifndef USE_LOGNAME_FMT
#define USE_LOGNAME_FMT						DEFAULT_LOGNAME_FMT
#endif

	//
	//  AppConfig Class Definition
	//

	class AppConfig {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors			                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Default Constructor 
		//
		//  Constructs a new AppConfig (Base) with the requisite components
		//
		//  PARAMETERS:
		//
		//		char*				-		Const pointer to the application name
		//		int					-		Count of application invocation parameters
		//		char*[]				-		Array of pointers to the application invocation parameters
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		AppConfig(const char* szAppName, int argc, char** argv) : SPool()
			, LQ(*(new LogQueue(SLog)))
			, Log(new LogStreamBuf(LQ, 0))

			//  Optional static initialisations for CRYPTO enabled application
#ifdef XY_NEEDS_CRYPTO
			, SOPool()
			, RMap(SPool, SOPool, argc, argv)
#else
			, RMap(SPool, argc, argv)
#endif

#ifdef XY_NEEDS_TEXT
			, TRE()
#endif
			, VerboseLogging(false)
			, EchoLogging(false)
			, SLog()

			//  Optional static initialisations for Multi-Programming interface
#ifdef XY_NEEDS_MP
			, LServ(LQ, SLog)
			, MPQ(*(new MPQueues()))
			, MP(LQ, MPQ, 0, SPool, RMap)
			, MPTP(LQ, MPQ, SPool, RMap)
#endif

#ifdef  XY_NEEDS_NETIO
			, PnP(getDefaultNetConfig(), Log)
#endif

#ifdef  XY_NEEDS_WEBUI
			, Wez(WSConfig, PnP, MP)
#endif
			, IsDismissed(false)

		{

			//  Clear the charmed configuration settings
			CCEncKey = NULLSTRREF;
			CCEncScheme = 0;
#ifdef  XY_NEEDS_CRYPTO
			setConfigurationSec();
#endif

			//  Open the application log stream
			openLog(szAppName);

#ifdef XY_NEEDS_MP
			//
			//  Multi-Programming (MP) Logging initialisation
			//
			//	(1)		Start the logging thread
			mLogThread = std::thread(&Logger::run, &LServ);
#endif

			//  Build the configuration for execution
#ifdef  XY_NEEDS_MP
			//  Set the physical limit on maximum number of threads in the thread pool
#ifndef  XY_MAX_THREAD
#define  XY_MAX_THREADS XY_DEFAULT_MAX_THREADS
#endif
			MaxThreads = XY_MAX_THREADS;
#ifdef  XY_NEEDS_WEBUI
			//
			//  Set the default configuration for Wezzer web server.
			//  Minthreads = 5, Scope = NETOS_SCOPE_LOCAL_MACHINE, IPv4 only, Port = Auto, Maxconnections = MaxThreads

			MinThreads = 5;
			memset(&WSConfig, 0, sizeof(Wezzer::WezConfig));
			WSConfig.Scope = NETOS_SCOPE_LOCAL_MACHINE;
			WSConfig.Options = Wezzer::WEZCFG_AUTO_LAUNCH | Wezzer::WEZCFG_IPV4_LISTEN;
			if (VerboseLogging) WSConfig.Options = WSConfig.Options | Wezzer::WEZCFG_VERBOSE_LOGGING;
			WSConfig.IPv4.Port = 0;
			MPAutoOn = true;
			MPAutoMCycles = 20;
			MPAutoACycles = 10;
#else
			MinThreads = 2;
			MPAutoOn = true;
			MPAutoMCycles = 20;
			MPAutoACycles = 10;
#endif
			MinThreads = 1;
			MaxThreads = 1;
			MPAutoOn = true;
			MPAutoMCycles = 20;
			MPAutoACycles = 10;
#endif
			pCfgImg = nullptr;
			buildConfiguration(szAppName);

			//  If Log echoing is requested then set the log writer to echo mode
			if (EchoLogging) SLog.setEcho();

#ifdef  XY_NEEDS_MP
			//  (2)		Start the ThreadPool service thread
			if (MaxThreads > XY_MAX_THREADS) MaxThreads = XY_MAX_THREADS;
			MPTP.setWorkers(MinThreads, MaxThreads);
			MPTP.setAutonomics(MPAutoOn, MPAutoMCycles, MPAutoACycles);
			mTPThread = std::thread(&ThreadPool::run, &MPTP);
#endif

#ifdef  XY_NEEDS_NETIO
			//  Initialise Networking I/O services
			NIOAvailable = NetInit::init();
#endif

#ifdef  XY_NEEDS_WEBUI
			//  (3)		Start the Wezzer micro web server
			if (NIOAvailable) {

				//  Start the web server
				Wez.start(MP);

			}
			else Log << "ERROR: Unable to start the Wezzer Web Server,  brcause basic network I/O services were not available." << std::endl;
#endif

			//
			//  Debugging Assistant - Show block addresses for core components
			//
#ifdef XY_DEBUG_SHOW_BLOCKS
			Log << "DEBUG: AppConfig initialisation complete AppConfig at: 0x" << this << "." << std::endl;
			Log << "DEBUG: Application Level String Pool object at: 0x" << &SPool << "." << std::endl;
			Log << "DEBUG: Virtual Resource Mapper at: 0x" << &RMap << "." << std::endl;
			Log << "DEBUG: Client Log ostream object at: 0x" << &Log << "." << std::endl;
#ifdef XY_NEEDS_MP
			Log << "DEBUG: System Log Queue object at: 0x" << &LQ << "." << std::endl;
			Log << "DEBUG: System Log Writer service object at: 0x" << &SLog << "." << std::endl;
			Log << "DEBUG: Logger service object at: 0x" << &LServ << "." << std::endl;
			Log << "DEBUG: Multiprogramming Queues object at: 0x" << &MPQ << "." << std::endl;
			Log << "DEBUG: Thread0 Dispatcher object at: 0x" << &MP << "." << std::endl;
			Log << "DEBUG: System ThreadPool object at: 0x" << &MPTP << "." << std::endl;
#endif
#ifdef  XY_NEEDS_WEBUI
			Log << "DEBUG: Wezzer micro web server at: 0x" << &Wez << "." << std::endl;
#endif
#endif

			//  Return to caller
			return;
		}

		//  Copy Constructor 
		//
		//  NO COPY CONSTRUCTION IS PERMITTED FOR THE AppConfig CLASS
		//
		//  PARAMETERS:
		//
		//		AppConfig&			-		Const reference to the source AppConfig
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		AppConfig(const AppConfig& Src) = delete;


		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor			                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Destructor
		//
		//  Destroys the AppConfig object, dismissing the underlying objects/allocations
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		~AppConfig() {

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
		//*   Public Sub-System Objects/References                                                                          *
		//*                                                                                                                 *
		//*   NOTE:  Ensure that the order of declaration matches the desired order of initialisation.						*
		//*																													*
		//*******************************************************************************************************************

		//  Public Application Level String Pool
		StringPool		SPool;
		//  Client/Server Log Queue
		LogQueue&		LQ;
		//  Log Stream
		std::ostream	Log;

#ifdef  XY_NEEDS_CRYPTO
		//  Secure Objects Pool for cryptographic objects
		SObjectPool		SOPool;
#endif

		//  Virtual Resource Mapper
		VRMapper		RMap;

		//  MULTI-PROGRAMMING 
#ifdef  XY_NEEDS_MP

		//  MP Queues
	private:
		MPQueues& MPQ;																			//  MP Queues
	public:

		//  MP Dispatcher
		Dispatcher		MP;																				//  MP Dispatcher
#endif

#ifdef   XY_NEEDS_WEBUI
		Wezzer			Wez;																			//  Wezzer micro web server
#endif

#ifdef   XY_NEEDS_NETIO
		NetPnP			PnP;																			//  Plug and Play interface
#endif

#ifdef	XY_NEEDS_TEXT
		TextRenderer    TRE;																			//  Text Rendering Engine
#endif

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  isLogOpen
		//
		//  Returns the current open state of the Log
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		bool			-		true if open, otherwise false
		//
		//  NOTES:
		//  

		bool	isLogOpen() { return SLog.is_open(); }

		//  isVerboseLogging
		//
		//  Returns the current state of the verbose logging control
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		bool			-		true if verbose logging mode is asserted, otherwise false
		//
		//  NOTES:
		//  

		bool	isVerboseLogging() { return VerboseLogging; }

		//  releaseConfigImage
		//
		//  Releases the config file in-memory image.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void	releaseConfigImage() {
			//  Free the configuration image
			if (pCfgImg != nullptr) free(pCfgImg);
			pCfgImg = nullptr;
			return;
		}

		//  isFirstCLPUsed
		//
		//  This function will indicate if the first command line parameter is used by xymorg (project root directory).
		//  
		//  PARAMETERS:
		//
		//  RETURNS:
		// 
		//		bool		-		true if first parameter used, otherwise false
		//
		//  NOTES:
		//

		bool	isFirstCLPUsed() { return RMap.isFirstCLPUsed(); }

#ifdef XY_NEEDS_MP
		//  getMinThreads
		//
		//  Returns the minimum threads in use
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		size_t		-		Minimum thread count
		//
		//  NOTES:
		//  

		size_t		getMinThreads() { return MinThreads; }

		//  getMaxThreads
		//
		//  Returns the maximum threads in use
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		size_t		-		Maximum thread count
		//
		//  NOTES:
		//  

		size_t		getMaxThreads() { return MaxThreads; }
#endif
#ifdef XY_NEEDS_NETIO

		//  isNetIOAvailable
		//
		//  Test to determine if network I/O is available.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		bool		-		true if network I/O functions are available, otherwise false
		//
		//  NOTES:
		//  

		bool	isNetIOAvailable() { return NIOAvailable; }

		//  getDefaultNetConfig
		//
		//  Virtual function used to determine the default network configuration needed by the application.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		SWITCHES		-		Desired network configuration switches see NetPnP::xxxx
		//
		//  NOTES:
		//  

		virtual SWITCHES		getDefaultNetConfig() { return NetPnP::APP_DEFAULT_CONFIG; }

#endif

		//  showStats
		//
		//  Invokes stats display on loaded components
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void		showStats() {
#ifdef XY_NEEDS_MP
			//  Log the stats from the threadpool and MP Queues
			MPTP.showStats(Log);
#endif
			//  Return to caller
			return;
		}

#ifdef XY_NEEDS_MP
		//  waitUntilMPEmpty
		//
		//  This call will block until all activity in the threadpool has completed.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		void	waitUntilMPEmpty() {

			//  Pass the call to the threadpool to wait.
			MPTP.waitUntilEmpty();
			//  Return to caller
			return;
		}

		//
		//  The following functions allow the disabling of MP Policies that are enabled by default
		//
		//	TEP - Thread Exhaustion Protection
		//	WFR - Wait For Rejoins (On Drain)
		//	WFD - Wait For Delayed (On drain)
		//

		void	disableTEP() { MPTP.disableTEP(); return; }
		void	disableWFR() { MPTP.disableWFR(); return; }
		void	disableWFD() { MPTP.disableWFD(); return; }

#endif

#ifdef XY_NEEDS_WEBUI
		//  performAutoLaunch
		//
		//  This function will perform (or not) the necessary autolaunch processing.
		//
		//  PARAMETERS:
		//
		//		char*		-		Const pointer to the Home (default) URI
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void		performAutoLaunch(const char* szHomeURI) {
			char		LaunchURL[MAX_PATH + 1] = {};																//  Launch URL

			//  Capture the Home URI (if provuded)
			WSConfig.HomeURI[0] = SCHAR_FS;
			WSConfig.HomeURI[1] = SCHAR_EOS;
			if (szHomeURI != nullptr) {
				if (szHomeURI[0] != SCHAR_EOS) {
					if (szHomeURI[0] == SCHAR_FS) strcpy_s(WSConfig.HomeURI, MAX_PATH, szHomeURI);
					else sprintf_s(WSConfig.HomeURI, MAX_PATH, "/%s", szHomeURI);
				}
			}

			//  Build the launch URL - This will launch on the IPv4 address if available (otherwise IPv6)
			if (WSConfig.Options & Wezzer::WEZCFG_IPV4_LISTEN) {
				Wez.qualifyURI(true, false, WSConfig.HomeURI, LaunchURL, MAX_PATH);
			}
			else {
				Wez.qualifyURI(true, true, WSConfig.HomeURI, LaunchURL, MAX_PATH);
			}

			MP.Log << new LOGMSG("WEZZER - INFO: Default Launch URL : '%s' to access the application.", LaunchURL);

			//  If autolaunch is on then automatically launch the URL
			if (WSConfig.Options & Wezzer::WEZCFG_AUTO_LAUNCH) {
				//  Launch the initial URL
				LaunchFile(LaunchURL);
			}
			else {
				//  Show the launch information on std::cout 
				std::cout << "Start your browser and enter the following URL: '" << LaunchURL << "' to access the application." << std::endl;
				if ((WSConfig.Options & (Wezzer::WEZCFG_IPV4_LISTEN | Wezzer::WEZCFG_IPV6_LISTEN)) == (Wezzer::WEZCFG_IPV4_LISTEN | Wezzer::WEZCFG_IPV6_LISTEN)) {
					//  Both protocols are enabled show the IPv6 as an alternative
					Wez.qualifyURI(true, true, WSConfig.HomeURI, LaunchURL, MAX_PATH);
					MP.Log << new LOGMSG("WEZZER - INFO: IPV6 Launch URL : '%s' to access the application.", LaunchURL);
					std::cout << "   Or enter the following URL: '" << LaunchURL << "' to access the application using IPv6." << std::endl;
				}
			}

			//  If the server is web accessible then show the external address(es)
			if ((WSConfig.Scope == NETOS_SCOPE_WAN)) {
				if (WSConfig.Options & Wezzer::WEZCFG_IPV4_LISTEN) {
					Wez.qualifyURI(false, false, WSConfig.HomeURI, LaunchURL, MAX_PATH);
					MP.Log << new LOGMSG("WEZZER - INFO: Launch URL : '%s' to access the application.", LaunchURL);
					//  Show the launch information of std::cout 
					std::cout << "Start your browser and enter the following URL: '" << LaunchURL << "' to access the application from the internet." << std::endl;
				}
				else {
					Wez.qualifyURI(false, true, WSConfig.HomeURI, LaunchURL, MAX_PATH);
					MP.Log << new LOGMSG("WEZZER - INFO: Launch URL : '%s' to access the application.", LaunchURL);
					//  Show the launch information of std::cout 
					std::cout << "Start your browser and enter the following URL: '" << LaunchURL << "' to access the application from the internet." << std::endl;
				}

				//  If both IPv4 & IPv6 enabled then report the IPv6 access as well
				if ((WSConfig.Options & (Wezzer::WEZCFG_IPV4_LISTEN | Wezzer::WEZCFG_IPV6_LISTEN)) == (Wezzer::WEZCFG_IPV4_LISTEN | Wezzer::WEZCFG_IPV6_LISTEN)) {
					Wez.qualifyURI(false, true, WSConfig.HomeURI, LaunchURL, MAX_PATH);
					MP.Log << new LOGMSG("WEZZER - INFO: Launch URL : '%s' to access the application.", LaunchURL);
					//  Show the launch information of std::cout 
					std::cout << "   Or enter the following URL: '" << LaunchURL << "' to access the application from the internet using IPv6." << std::endl;
				}
			}

			//  Return to caller
			return;
		}

#endif

		//  dismiss
		//
		//  Shuts down any active sub-systems and releases underlying storage from all xymorg components
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void		dismiss() {

			//  Dismiss protection
			if (IsDismissed) return;

			//  Destroy resources in parent class
			dismissConfig();

#ifdef	XY_NEEDS_TEXT
			TRE.dismiss();
#endif

#ifdef  XY_NEEDS_WEBUI
			//  Dismiss the web server
			Wez.dismiss(MP);
#endif

#ifdef XY_NEEDS_NETIO
			//  Dismiss networking services
			PnP.dismiss();
			NetInit::dismiss();
#endif

#ifdef XY_NEEDS_MP
			//  Signal the thread pool to drain
			MPTP.drain();

			//  Wait for the thread pool thread to drain
			mTPThread.join();
#ifdef XY_MP_NEEDS_DEBUGGING
			//  Log the stats from the threadpool and MP Queues
			MPTP.showStats(Log);
#endif
			//  Signal the Logger to drain
			LServ.drain();

			//  Wait for the logger thread to terminate
			mLogThread.join();

			MP.dismiss();

			delete& MPQ;
#endif

			//  Close the log
#ifndef XY_NEEDS_MP
			LQ.logStats();
#endif
			SLog.close();
			delete Log.rdbuf();
			delete& LQ;

			//  Clear the public string pool
			SPool.dismiss();

#ifdef  XY_NEEDS_CRYPTO
			SOPool.dismiss();
#endif

			//  Clear the Resource Mapper
			RMap.dismiss();

			//  Free the configuration image
			if (pCfgImg != nullptr) free(pCfgImg);
			pCfgImg = nullptr;

			//  Return to caller
			IsDismissed = true;
			return;
		}

		//  dismissConfig
		//
		//  This virtual function should be implemented in extending classes to allow for the dismissal of resources
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		virtual void	dismissConfig() {
			// Return to caller
			return;
		}

		//  sendStringToPool
		//
		//  This function will place a string (not terminated) to the default string pool, re-instating any XML special characters.
		//
		//  PARAMETERS:
		//
		//		char*		-		Const pointer to the source string
		//		size_t		-		Length of the string
		//
		//  RETURNS:
		//
		//		STRREF		-		Reference to the string in the pool
		//
		//  NOTES:
		//  

		STRREF		sendStringToPool(const char* InStr, size_t Length) {
			return sendStringToPool(SPool, InStr, Length);
		}

		//  sendStringToPool
		//
		//  This function will place a string (not terminated) to the designated string pool, re-instating any XML special characters.
		//
		//  PARAMETERS:
		//
		//		StringPool&	-		Reference to the pool in which to store the string
		//		char*		-		Const pointer to the source string
		//		size_t		-		Length of the string
		//
		//  RETURNS:
		//
		//		STRREF		-		Reference to the string in the pool
		//
		//  NOTES:
		//  

		STRREF		sendStringToPool(StringPool& TPool, const char* InStr, size_t Length) {
			char*		pString = nullptr;														//  Image of the input string
			STRREF		StringRef = NULLSTRREF;													//  String reference

			//  Safety
			if (InStr == nullptr || Length == 0) return NULLSTRREF;

			//  Allocate the buffer to hold the image of the string
			pString = (char*)malloc(2 * Length);
			if (pString == nullptr) return NULLSTRREF;

			//  Copy the passed (non-terminated string into the buffer).
			memcpy(pString, InStr, Length);
			pString[Length] = '\0';

			//  Fettle the XML special characters to their normal representation
			Length = st_strirep(pString, Length, "&lt;", "<");
			Length = st_strirep(pString, Length, "&gt;", ">");
			Length = st_strirep(pString, Length, "&amp;", "&");
			Length = st_strirep(pString, Length, "&quot;", "\"");
			Length = st_strirep(pString, Length, "&apos;", "'");

			//  Add the string to the pool
			StringRef = TPool.addString(pString, Length);

			//  Free the temporary string
			free(pString);

			//  Return the reference
			return StringRef;
		}

		//  launchFile
		//
		//  This function is a highly simplified wrapper for a Shellexecute() function on windows.
		//  No linux/unix equivalent is offered.
		//
		//  PARAMETERS:
		//
		//		char*		-		Const pointer to the file (relative) that is to be launched
		//
		//  RETURNS:
		//
		//		int			-		Return Code (0 - 32) ==> fail, > 32 ==> succeeded
		//
		//  NOTES:
		//  

		int		LaunchFile(const char* RFN) {
			int		LaunchRC = 0;																//  Return code from launch
			char	RFPath[MAX_PATH + 1];														//  Real file name buffer

			if (RFN == nullptr) return 0;
			if (RFN[0] == '\0') return 0;

			//  Check for a URL being launched
			if (_strnicmp(RFN, "http:", 5) == 0 || _strnicmp(RFN, "https:", 6) == 0) strcpy_s(RFPath, MAX_PATH, RFN);
			else {
				//  Map the file name from relative to absolute
				RMap.mapFile(RFN, RFPath, MAX_PATH);
			}

#if  (defined(_WIN32) || defined(_WIN64))
			HINSTANCE		Hi = 0;
			Hi = ShellExecute(NULL, NULL, RFPath, NULL, NULL, SW_SHOWNORMAL);
			if (!Hi) {
				Log << new LOGMSG("ERROR; Shell Execute failed on: '%s'.", RFPath);
				LaunchRC = -1;
			}
#else
			LaunchRC = 0;
#endif
			return LaunchRC;
		}

	protected:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Protected Members                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		char*			pCfgImg;

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		bool			VerboseLogging;																			//  Verbose logging mode
		bool			EchoLogging;																			//  Echo loggng from the start
		LogWriter		SLog;																					//  System log file interface

#ifdef XY_NEEDS_MP
		Logger			LServ;																					//  Logging Server
		std::thread		mLogThread;																				//  Logging Thread
		size_t			MaxThreads;																				//  Maximum thread count
		size_t			MinThreads;																				//  Minimum thread count
		ThreadPool		MPTP;																					//  MP thread pool
		std::thread		mTPThread;																				//  ThreadPool service thread
		bool			MPAutoOn;																				//  Autonomics are enabled
		int				MPAutoMCycles;																			//  Number of monitor cycles (per action)
		int				MPAutoACycles;																			//  Number of action cycles (per real action)
#endif

#ifdef XY_NEEDS_NETIO
		bool			NIOAvailable;																			//  Network I/O Availability
#endif

#ifdef XY_NEEDS_WEBUI
		Wezzer::WezConfig	WSConfig;																			//  Web Server configuration structure
#endif

		bool			IsDismissed;																			//  Dismissed state

		//  Settings for charmed configuration
		int				CCEncScheme;																			//  Encryption scheme
		STRREF			CCEncKey;																				//  Encryption Key

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  OpenLog
		//
		//  Determines the path/name of the current log file and opens the log stream on that file
		//
		//  PARAMETERS:
		//
		//		char*		-		Const pointer to the application name
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void	openLog(const char* szApp) {
			time_t			ttNow = 0;																				//  Submission Timestamp
			struct tm		tmLocalStore = {};																		//  Storage for local time
			struct tm*		ptmLocal = nullptr;																		//  Local time structure
			char			szVirtFile[MAX_PATH + 1] = {};															//  Virtual file name
			char			szLogFile[MAX_PATH + 50] = {};															//  File name of the log file

			//  Build the formatted timestamp for the log file name
			time(&ttNow);
			ptmLocal = localtime_safe(&ttNow, &tmLocalStore);
			strftime(szLogFile, MAX_PATH, USE_LOGNAME_TIMESTAMP_FMT, ptmLocal);

			//  Format the virtual file name
			//  Substitution Strings are :-
			//
			//		The formatted timestamp (current time)
			//		The name of the application
			//
			sprintf_s(szVirtFile, MAX_PATH, USE_LOGNAME_FMT, szLogFile, szApp);

			//  Map the virtual file name to a real file name
			RMap.mapFile(szVirtFile, szLogFile, MAX_PATH + 1);

			//  Open the system log stream
			SLog.open(szLogFile, std::ofstream::out);

			//  If the stream is not open then show that the log is not available
			if (!SLog.is_open()) std::cerr << "WARNING: Log file: '" << szLogFile << "' could not be opened, logging is not available." << std::endl;

			//  Return to caller
			return;
		}

		//  buildConfiguration
		//
		//  This function lods the application configuration file and parses the content to condition the application ready for execution
		//
		//  PARAMETERS:
		//
		//		char*		-		Const pointer to the application name
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void		buildConfiguration(const char* szApp) {
			char			szVirtFile[MAX_PATH + 1];															//  Virtual file name
			size_t			ImgSize = 0;																		//  Size of the image

			//  Construct the virtual file name
			strcpy_s(szVirtFile, MAX_PATH, "Config/");
			strcat_s(szVirtFile, MAX_PATH, szApp);
			strcat_s(szVirtFile, MAX_PATH, ".xml");

			//
			//  Set the configuration encryption scheme and key if they are defined
			//

			setConfigurationSec();

			//  Load the configuration file into memory
			//
			//  NOTE: The config file is ALWAYS loaded as a charmed resource. If the config is not charmed then it will e loaded as-is.
			//

			pCfgImg = (char*) RMap.loadCharmedResource(szVirtFile, ImgSize, CCEncScheme, CCEncKey);

			//
			//  If the image could not be loaded (no config found) then we invoke the no configuration handler and return.
			//  The no configuration handler may be overridden in a sub-class to take appropriate action. The default
			//  action is to log a message showing that there is no config and defauts will be used.
			//
			if (pCfgImg == nullptr) {

				//  Return to caller
				return;
			}

			//
			//  Check for re-direction of the configuration, if redirection is in effect then the config image will be replaced
			//
			pCfgImg = redirectConfiguration(pCfgImg);

			// 
			//  Parse the configuration for settings that apply to core xymorg components only
			//

			parseCoreConfiguration(pCfgImg);

			//  Return to caller
			return;
		}

		//  redirectConfiguration
		//
		//  This function will parse loaded configuration to determine if it should be redirected to an alternate
		//  source.
		//
		//  PARAMETERS:
		//
		//		char*		-		Pointer to the in-memory image of the configuration file
		//		
		//
		//  RETURNS:
		//
		//		char*		-		Pointer to the in-memory image of the re-directed configuration file
		//
		//  NOTES:
		//  

		char* redirectConfiguration(char* pOCImage) {
			xymorg::XMLMicroParser			Parser(pOCImage);											//  XML Micro Parser for the application configuration file
			xymorg::XMLMicroParser::XMLIterator		XIt(nullptr);										//  XML Iterator
			size_t							ImgSize = 0;												//  Size of the image
			char*							pNewImage = nullptr;										//  Pointer to the new image
			char							RDCName[MAX_PATH + 1] = {};									//  Redirected file name

			//  Position to the root
			XIt = Parser.getScope("config");

			//  Check if we have a redirection - if not return a pointer to the original config image
			if (!XIt.hasAttribute("redirect")) return pOCImage;

			//  Extract the redirect target file name
			XIt.getAttributeString("redirect", RDCName, MAX_PATH);

			//  Load the configuration file into memory
			//
			//  NOTE: The config file is ALWAYS loaded as a charmed resource. If the config is not charmed then it will be loaded as-is.
			//
			pNewImage = (char*)RMap.loadCharmedResource(RDCName, ImgSize, CCEncScheme, CCEncKey);

			//  Free the existing image
			free(pOCImage);

			//  Return the pointer to the new image
			return pNewImage;
		}

		//  parseCoreConfiguration
		//
		//  This function will parse the core configuration extracting core settings and applying them to the core components
		//
		//  PARAMETERS:
		//
		//		char*		-		Pointer to the in-memory image of the configuration file
		//		
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void		parseCoreConfiguration(char* pImg) {
			xymorg::XMLMicroParser			Parser(pImg);												//  XML Micro Parser for the application configuration file
			xymorg::XMLMicroParser::XMLIterator		XIt(nullptr);										//  XML Iterator
			size_t			RLen = 0;																	//  Length of parameter value
			const char*		pRVal;																		//  Pointer to parameter value

			//  Safety checks
			if (!Parser.isValid()) return;

			//  Unconditional parameters
			XIt = Parser.getScope(LOGGING_NODE);
			if (!XIt.isNull()) {
				pRVal = XIt.getAttribute(VERBOSE_PARM, RLen);
				if (pRVal == nullptr) VerboseLogging = false;
				else {
					if (_strnicmp(pRVal, TRUE_PVAL, RLen) == 0) VerboseLogging = true;
					else VerboseLogging = false;
				}
				pRVal = XIt.getAttribute(ECHO_PARM, RLen);
				if (pRVal == nullptr) EchoLogging = false;
				else {
					if (_strnicmp(pRVal, TRUE_PVAL, RLen) == 0) EchoLogging = true;
					else EchoLogging = false;
				}
			}

#ifdef XY_NEEDS_MP

			//
			//  Build the iterator for each well known node in turn and extract the pertinent settings
			//

			//
			//  Threading control <threads>
			//
			XIt = Parser.getScope(THREADS_NODE);
			if (!XIt.isNull()) {
				//  Positioned at the desired element
				MinThreads = XIt.getAttributeInt(MINT_PARM);
				MaxThreads = XIt.getAttributeInt(MAXT_PARM);

				//  If present then parse the autonomics section
				XIt = XIt.getScope(AUTO_NODE);
				if (!XIt.isNull()) {
					if (XIt.hasAttribute(ENABLED_PARM)) {
						if (XIt.isAsserted(ENABLED_PARM)) MPAutoOn = true;
						else MPAutoOn = false;
					}
					if (XIt.hasAttribute(MCYCLES_PARM)) MPAutoMCycles = XIt.getAttributeInt(MCYCLES_PARM);
					if (XIt.hasAttribute(ACYCLES_PARM)) MPAutoACycles = XIt.getAttributeInt(ACYCLES_PARM);
				}
			}

			//  Adjust limits if necessary
			if (MaxThreads > XY_MAX_THREADS) MaxThreads = XY_MAX_THREADS;
#ifdef  XY_NEEDS_WEBUI
			if (MinThreads < 5) MinThreads = 5;
#else
			if (MinThreads < 2) MinThreads = 2;
#endif
#endif
			//
			//  Determinde if there is a Virtual Resource Map section in the configuration, if so pass the iterator to the VRMapper
			//  to extend the map with the contents of the section.
			//

			XIt = Parser.getScope(NODE_VRES_MAP);
			if (!XIt.isNull()) RMap.extendConfiguration(XIt);

#ifdef  XY_NEEDS_WEBUI

			//  Determine if there is any configuration specified for the Wezzer micro web server, if so condition the server with the settings

			XIt = Parser.getScope(WEBUI_NODE);
			if (!XIt.isNull()) {
				char			Root[MAX_PATH + 1] = {};

				//  Clear the configuration set (to unset the defaults that have already been set)
				memset(&WSConfig, 0, sizeof(Wezzer::WezConfig));

				//  Set the autolaunch option, if not available => true, otherwise from the supplied setting
				if (XIt.hasAttribute(AUTOLAUNCH_PARM)) {
					if (XIt.isAsserted(AUTOLAUNCH_PARM)) WSConfig.Options = WSConfig.Options | Wezzer::WEZCFG_AUTO_LAUNCH;
				}
				else WSConfig.Options = WSConfig.Options | Wezzer::WEZCFG_AUTO_LAUNCH;

				//  Get the web root directory (if supplied)
				pRVal = XIt.getAttribute(ROOT_PARM, RLen);
				if (pRVal != nullptr) {
					memcpy(Root, pRVal, RLen);
					Root[RLen] = '\0';
					strcpy_s(WSConfig.WebRoot, MAX_PATH, Root);
				}

				//  Get the Scope (if supplied)
				pRVal = XIt.getAttribute(SCOPE_PARM, RLen);
				if (pRVal != nullptr) {
					memcpy(Root, pRVal, RLen);
					Root[RLen] = '\0';

					if (_stricmp(Root, "local") == 0 || _stricmp(Root, "host") == 0 || _stricmp(Root, "localhost") == 0) WSConfig.Scope = NETOS_SCOPE_LOCAL_MACHINE;
					else {
						if (_stricmp(Root, "lan") == 0 || _stricmp(Root, "hood") == 0 || _stricmp(Root, "neighbourhood") == 0) WSConfig.Scope = NETOS_SCOPE_LAN;
						else {
							if (_stricmp(Root, "wan") == 0 || _stricmp(Root, "net") == 0 || _stricmp(Root, "web") == 0 || _stricmp(Root, "internet") == 0) WSConfig.Scope = NETOS_SCOPE_WAN;
						}
					}
				}

				if (WSConfig.Scope == 0) WSConfig.Scope = NETOS_SCOPE_LOCAL_MACHINE;

				//  Get the Max Connections (if supplied)
				WSConfig.MaxConn = XIt.getAttributeInt(MAXCONN_PARM);
				if (WSConfig.MaxConn == 0) WSConfig.MaxConn = MaxThreads;

				//  Process any IPv4 Binding (if supplied)
				XMLMicroParser::XMLIterator V4XIt = XIt.getScope(IPV4_NODE);
				if (!V4XIt.isNull()) {

					//  Determine if we are to listen on this protocol
					if (V4XIt.hasAttribute(LISTEN_PARM)) {
						if (V4XIt.isAsserted(LISTEN_PARM)) WSConfig.Options = WSConfig.Options | Wezzer::WEZCFG_IPV4_LISTEN;
						else WSConfig.Options = WSConfig.Options & (~Wezzer::WEZCFG_IPV4_LISTEN);
					}
					else WSConfig.Options = WSConfig.Options | Wezzer::WEZCFG_IPV4_LISTEN;

					//  Set the binding hint address (if supplied)
					if (V4XIt.hasAttribute(BIND_PARM)) {
						pRVal = V4XIt.getAttribute(BIND_PARM, RLen);
						if (pRVal != nullptr) {
							memcpy(Root, pRVal, RLen);
							Root[RLen] = '\0';
							WSConfig.IPv4.Bind = IPV4::getIP4A(Root);
						}
					}

					//  Set the Port Number hint
					WSConfig.IPv4.Port = V4XIt.getAttributeInt(PORT_PARM);
				}

				//  Process any IPv6 Binding (if Supplied)
				XMLMicroParser::XMLIterator V6XIt = XIt.getScope(IPV6_NODE);
				if (!V6XIt.isNull()) {

					//  Determine if we are to listen on this protocol
					if (V6XIt.hasAttribute(LISTEN_PARM)) {
						if (V6XIt.isAsserted(LISTEN_PARM)) WSConfig.Options = WSConfig.Options | Wezzer::WEZCFG_IPV6_LISTEN;
						else WSConfig.Options = WSConfig.Options & (~Wezzer::WEZCFG_IPV6_LISTEN);
					}
					else WSConfig.Options = WSConfig.Options | Wezzer::WEZCFG_IPV6_LISTEN;

					//  Set the binding hint address (if supplied)
					if (V6XIt.hasAttribute(BIND_PARM)) {
						pRVal = V6XIt.getAttribute(BIND_PARM, RLen);
						if (pRVal != NULL) {
							memcpy(Root, pRVal, RLen);
							Root[RLen] = '\0';
							WSConfig.IPv6.Bind = IPV6::getIP6A(Root);
						}
					}

					//  Set the Port Number hint
					WSConfig.IPv4.Port = V4XIt.getAttributeInt(PORT_PARM);
				}

				//  If NO IPv4 nor IPv6 protocols were requested then set the default for IPv4
				if ((WSConfig.Options & (Wezzer::WEZCFG_IPV4_LISTEN | Wezzer::WEZCFG_IPV6_LISTEN)) == 0) WSConfig.Options = WSConfig.Options | Wezzer::WEZCFG_IPV4_LISTEN;
			}

			//  If verbose logging in effect then assert it in the Wezzer config
			if (VerboseLogging) WSConfig.Options = WSConfig.Options | Wezzer::WEZCFG_VERBOSE_LOGGING;

#endif

			//  Return to caller
			return;
		}

		//  setConfigurationSec
		//
		//  This function will set the security settings (encryption scheme and key) for the configuration if they are defined
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void	setConfigurationSec() {

			BYTE		SecKey[SEC_CFG_KSIZE] = SEC_CFG_KEY;

			SecKey[0] = SecKey[0];
			CCEncScheme = SEC_CFG_SCHEME;
#ifdef  XY_NEEDS_CRYPTO
			CCEncKey = SOPool.addObject(SecKey, SEC_CFG_KSIZE);
#endif
			return;
		}

	};
}
