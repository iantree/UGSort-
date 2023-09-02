#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       Sorter.h																							*
//*   Suite:      Experimental Algorithms																			*
//*   Version:    1.15.0	(Build: 16)																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the Sorter class. Objects of this class provide the driver			*
//* interface for performing sort operations using the UGSort algorithm.	For applications it provides			*
//* the interface for sorting a file either in-memory or on-disk. It also provides the driver functions for			*
//* the UUGSort engine that load records into the engine, starting final merge processes							*
//* and finally writing the sorted output.																			*
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
//*	1.0.0 -		21/02/2023	-	Initial Release																		*
//*	1.3.0 -		08/03/2023	-	Adaptive PM																			*
//*	1.4.0 -		10/03/2023	-	Output iterators added																*
//*	1.5.0 -		13/03/2023	-	SS3 structure changes																*
//*	1.6.0 -		15/03/2023	-	Stable Key Handling																	*
//*	1.13.0 -	13/06/2023	-	PM Activity & T_SO sub-phase timing													*
//*	1.14.0 -	08/07/2023	-	Remove T_SO sub-phase timing and clarify timings									*
//*	1.15.0 -	25/08/2023	-	Binary-Chop search of Store Chain													*
//*																													*
//*******************************************************************************************************************/

//  Include xymorg headers
#include	"../xymorg/xymorg.h"															//  xymorg system headers

//  Application Headers
#include	"IStats.h"																		//  Instrumentation
#include	"Splitter.h"																	//  Splitter template class

//
//  Sorter class definition
//

class Sorter {
private:

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Private Nested Structures	(Sort Records)                                                                      *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Sort Record for In-Memory sorting
	typedef struct IMSR {
		const char* pKey;													//  Pointer tho the Sort Key
		const char* pRec;													//  Pointer to the in-memory record
	} IMSR;

	//  Sort Record for On-Disk sorting
	typedef struct ODSR {
		const char* pKey;													//  Pointer tho the Sort Key
		std::streampos	RecPos;													//  Record position in Sortin
	} ODSR;

	//  Sort Record for Memory Array sorting
	typedef struct MASR {
		const char* pKey;													//  Pointer tho the Sort Key
		size_t			AEX;													//  Array Element index
	} MASR;

public:

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Constructors			                                                                                        *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Constructor 
	//
	//  Constructs the Sorter with the passed output stream that will be used for any logging output.
	//
	//  PARAMETERS:
	//
	//		std::ostream&			-		Reference to the output stream to use for logging
	//
	//  RETURNS:
	//
	//  NOTES:
	//

	Sorter(std::ostream& RefOS) : Log(RefOS), Notifications(false), Timings(false) {

		//  Return to caller
		return;
	}

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Destructor			                                                                                        *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Destructor
	//
	//  Destroys the Sorter object, dismissing the underlying objects/allocations
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//  NOTES:
	//  

	~Sorter() {

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

	//  Configuration Functions

	//  enableNotifications
	//
	//  This function will enable the Sorter to issue notification messages to the log stream.
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//  NOTES:
	//

	void	enableNotifications() { Notifications = true; return; }

	//  enableTimings
	//
	//  This function will enable the Sorter to issue timing messages to the log stream.
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//  NOTES:
	//

	void	enableTimings() { Timings = true; return; }

	//  Application Sorting API

	//  sortFileInMemory
	//
	//  This function will sort the passed file in-memory and write the sorted output to the passed file name.
	//  Sorting will conditionally use Preemptive Merging, the sort sequence is NOT stable.
	//
	//  PARAMETERS:
	// 
	//		char*		-		Const pointer to the sort input file name
	//		char*		-		Const pointer to the sort output file
	//		size_t		-		Offset (in records) to the sort key
	//		size_t		-		Length of the sort key
	//		bool		-		true if the sort sequence is ascending, false if descending
	//		bool		-		true if Preemptive Merging is enabled, false if disabled
	//
	//  RETURNS:
	// 
	//		bool		-		true if the sort was completed, otherwise false.
	//
	//  NOTES:
	//

	bool	sortFileInMemory(const char* SFIn,
		const char* SFOut,
		size_t SKOff,
		size_t SKLen,
		bool Ascending,
		bool PMEnabled) {

		char*					pSortin = nullptr;														//  Sort input in-memory buffer
		char*					pSortout = nullptr;														//  Sort output in-memory buffer
		size_t					SISize = 0;																//  Sort input size
		char*					pEOI = nullptr;															//  Pointer to the End-Of-Input
		char*					pNextRec = nullptr;														//  Pointer to the next record
		IMSR					SRec = {};																//  In-Memory sort record (internal)

		//  Root Splitter of the Splitter chain
		Splitter<IMSR>* pSR = nullptr;

		//  Instrumentation Statistics Object
		IStats					Stats;

		//  Load the designated sort input into memory
		Stats.startLoading();
		pSortin = loadSortInput(SFIn, SISize);
		if (pSortin == nullptr) {
			Log << "ERROR: Failed to load the sort input into memory, it may be too big to sort in-memory." << std::endl;
			return false;
		}
		Stats.finishLoading();

		//  The sort timing starts once the data has been loaded
		Stats.startSorting();

		pEOI = pSortin + SISize;
		pNextRec = pSortin;																				//  Next record is the first

		//  Setup the initial sort record
		SRec.pRec = pNextRec;
		SRec.pKey = pNextRec + SKOff;

		//  Adjust the next record pointer
		pNextRec = strchr(pNextRec, SCHAR_LF);
		if (pNextRec == nullptr) pNextRec = pEOI;
		else pNextRec++;

		//  Create the Root Splitter
		pSR = new Splitter<IMSR>(SRec, SKLen, Stats);
		if (pSR == nullptr) {
			Log << "ERROR: Unable to create the root Splitter to perform the sort." << std::endl;
			return false;
		}

		//
		//  Sort Input phase - load each record to the root splitter
		//

		Stats.startInput();

		//  Process each record in turn
		while (pNextRec < pEOI) {
			//  Build the internal sort record
			SRec.pRec = pNextRec;
			SRec.pKey = pNextRec + SKOff;
			pSR->add(SRec, PMEnabled);

			//  Adjust the next record pointer
			pNextRec = strchr(pNextRec, SCHAR_LF);
			if (pNextRec == nullptr) pNextRec = pEOI;
			else pNextRec++;
		}

		//  Record the ending time
		Stats.finishInput();

		//  If enabled then notify the end of the sort input phase
		if (Notifications) Log << "INFO: Sort input phase has completed." << std::endl;

		//
		//  Sort merge phase 
		//

		pSR->signalEndOfSortInput();

		//  If enabled then notify the end of the sort merge phase
		if (Notifications) Log << "INFO: Sort merge phase has completed." << std::endl;

		//
		//  Sort output phase
		//

		//  Check that the sort output is valid
		if (!pSR->isOutputValid()) {
			Log << "ERROR: The number of records in the sort is not valid, there was possibly not enough memory available to complete the sort operation." << std::endl;
			free(pSortin);
			delete pSR;
			return false;
		}

		//  Record starting time for the output preparation
		Stats.startOutput();

		//  Allocate a buffer to hold the sort output
		pSortout = (char*)malloc(SISize);
		if (pSortout == nullptr) {
			Log << "ERROR: Failed to allocate a buffer to hold the sort output (" << SISize << " bytes)." << std::endl;
			free(pSortin);
			delete pSR;
			return false;
		}

		//  Perform the sort output in ascending or descending sequence
		pNextRec = pSortout;
		if (Ascending) {
			//  Ascending sequence
			for (Splitter<IMSR>::Output O = pSR->lowest(); O <= pSR->highest(); O++) {
				const char* pOutRec = (*O).pRec;
				const char* pEOR = strchr(pOutRec, SCHAR_LF);
				if (pEOR == nullptr) pEOR = pEOI;
				size_t  RecLen = (pEOR - pOutRec) + 1;
				memcpy(pNextRec, pOutRec, RecLen);
				pNextRec += RecLen;
			}
		}
		else {
			//  Descending sequence 
			for (Splitter<IMSR>::Output O = pSR->highest(); O >= pSR->lowest(); O--) {
				const char* pOutRec = (*O).pRec;
				const char* pEOR = strchr(pOutRec, SCHAR_LF);
				if (pEOR == nullptr) pEOR = pEOI;
				size_t  RecLen = (pEOR - pOutRec) + 1;
				memcpy(pNextRec, pOutRec, RecLen);
				pNextRec += RecLen;
			}
		}

		//  The sort ending time is taken at this point
		Stats.finishOutput();
		Stats.finishSorting();

		//  Notify end of phase
		if (Notifications) Log << "INFO: Sort output phase completed." << std::endl;


		//  Write the sortout buffer to disk
		Stats.startStoring();
		if (!storeSortOutput(SFOut, pSortout, SISize)) {
			Log << "ERROR: Failed to store: " << SISize << "bytes of sort output data." << std::endl;
			free(pSortout);
			return false;
		}
		Stats.finishStoring();

		//  Free the input and the root splitter
		free(pSortin);
		delete pSR;
		free(pSortout);

		//  If enabled show the timings
		if (Timings) Stats.showStats(Log);

		//  Return showing success
		return true;
	}

	//  sortFileOnDisk
	//
	//  This function will sort the passed file on-disk and write the sorted output to the passed file name.
	//  Sorting will conditionally use Preemptive Merging, the sort sequence is NOT stable.
	//
	//  PARAMETERS:
	// 
	//		char*		-		Const pointer to the sort input file name
	//		char*		-		Const pointer to the sort output file
	//		size_t		-		Maximum record length
	//		size_t		-		Offset (in records) to the sort key
	//		size_t		-		Length of the sort key
	//		bool		-		true if the sort sequence is ascending, false if descending
	//		bool		-		true if Preemptive Merging is enabled, false if disabled
	//
	//  RETURNS:
	// 
	//		bool		-		true if the sort was completed, otherwise false.
	//
	//  NOTES:
	//

	bool	sortFileOnDisk(const char* SFIn,
		const char* SFOut,
		size_t MaxRecl,
		size_t SKOff,
		size_t SKLen,
		bool Ascending,
		bool PMEnabled) {

		std::ifstream			Sortin;																	//  Sort input stream
		std::ofstream			Sortout;																//  Sort output stream
		char*					SortRec = nullptr;														//  Input record buffer
		ODSR					SRec = {};																//  Sort Record (internal)

		//  Root Splitter of the Splitter chain
		Splitter<ODSR>* pSR = nullptr;

		//  Instrumentation Statistics Object
		IStats					Stats;

		//
		//  Setup ready for the input phase of the sort
		//

		Log << "WARNING: This sort is being performed on-disk, DO NOT use the timings for benchmarks." << std::endl;

		SortRec = (char*)malloc(MaxRecl);
		if (SortRec == nullptr) {
			Log << "ERROR: Failed to allocate a " << MaxRecl << " byte buffer for sort input records." << std::endl;
			return false;
		}
		memset(SortRec, 0, MaxRecl);

		Sortin.open(SFIn, std::istream::in);
		if (!Sortin.is_open()) {
			Log << "ERROR: Failed to open the designated sort input file: '" << SFIn << "'." << std::endl;
			free(SortRec);
			return false;
		}

		if (Sortin.eof()) {
			Log << "ERROR: The sort input file is empty." << std::endl;
			free(SortRec);
			Sortin.close();
			return false;
		}

		Sortin.getline(SortRec, MaxRecl);
		if (Sortin.fail()) {
			Log << "ERROR: Failed to read the first record from the sort input file: '" << SFIn << "'." << std::endl;
			free(SortRec);
			Sortin.close();
			return false;
		}

		//  Build the initial SortKey record
		SRec.RecPos = 0;
		SRec.pKey = SortRec + SKOff;

		//  Construct the root Splitter
		pSR = new Splitter<ODSR>(SRec, SKLen, 64, Stats);
		if (pSR == nullptr) {
			Log << "ERROR: Unable to create the root sort splitter." << std::endl;
			free(SortRec);
			Sortin.close();
			return false;
		}

		//
		//  Sort input phase
		//

		Stats.startInput();
		while (!Sortin.eof()) {
			SRec.RecPos = Sortin.tellg();
			Sortin.getline(SortRec, MaxRecl);
			if (Sortin.eof() && strlen(SortRec) == 0) break;
			SRec.pKey = SortRec + SKOff;

			//  Add the new record to the root splitter
			pSR->addExternalKey(SRec, PMEnabled);
		}

		Stats.finishInput();

		//  If enabled then notify the end of the sort input phase
		if (Notifications) Log << "INFO: Sort input phase has completed." << std::endl;

		//
		//  Sort merge phase
		//

		pSR->signalEndOfSortInput();

		//  If enabled then notify the end of the sort merge phase
		if (Notifications) Log << "INFO: Sort merge phase has completed." << std::endl;

		//
		//  Sort output phase
		//

		//  Check that the sort output is valid
		if (!pSR->isOutputValid()) {
			Log << "ERROR: The number of records in the sort is not valid, there was possibly not enough memory available to complete the sort operation." << std::endl;
			free(SortRec);
			Sortin.close();
			delete pSR;
			return false;
		}

		//  Open the output file
		Sortout.open(SFOut, std::ofstream::out);
		if (!Sortout.is_open()) {
			Log << "ERROR: Failed to open/create the designated sort output file: '" << SFOut << "'." << std::endl;
			free(SortRec);
			Sortin.close();
			delete pSR;
			return false;
		}

		//  Clear EOF and Fail on the sortin stream
		Sortin.clear(std::ifstream::goodbit);

		Stats.startOutput();

		if (Ascending) {

			for (Splitter<ODSR>::Output O = pSR->lowest(); O <= pSR->highest(); O++) {
				//  Position sortin to the record
				Sortin.seekg((*O).RecPos);
				//  Read the record
				Sortin.getline(SortRec, MaxRecl);
				//  Write the record to sort output
				Sortout << SortRec << std::endl;
			}
		}
		else {
			//  Descending sort sequence
			for (Splitter<ODSR>::Output O = pSR->highest(); O >= pSR->lowest(); O--) {
				//  Position sortin to the record
				Sortin.seekg((*O).RecPos);
				//  Read the record
				Sortin.getline(SortRec, MaxRecl);
				//  Write the record to sort output
				Sortout << SortRec << std::endl;
			}
		}

		Stats.finishOutput();

		//  Notify end of phase
		if (Notifications) Log << "INFO: Sort output phase completed." << std::endl;

		//  Record End of sort
		Stats.finishSorting();

		//  Free the input, output and the root splitter
		Sortin.close();
		Sortout.close();
		delete pSR;
		free(SortRec);

		//  If enabled show the timings
		if (Timings) Stats.showStats(Log);

		//  Return showing success
		return true;
	}

	//  sortStableFileInMemory
	//
	//  This function will sort the passed file in-memory and write the sorted output to the passed file name.
	//  Sorting will conditionally use Preemptive Merging, the sort sequence is stable.
	//
	//  PARAMETERS:
	// 
	//		char*		-		Const pointer to the sort input file name
	//		char*		-		Const pointer to the sort output file
	//		size_t		-		Offset (in records) to the sort key
	//		size_t		-		Length of the sort key
	//		bool		-		true if the sort sequence is ascending, false if descending
	//		bool		-		true if Preemptive Merging is enabled, false if disabled
	//
	//  RETURNS:
	// 
	//		bool		-		true if the sort was completed, otherwise false.
	//
	//  NOTES:
	//

	bool	sortStableFileInMemory(const char* SFIn,
		const char* SFOut,
		size_t SKOff,
		size_t SKLen,
		bool Ascending,
		bool PMEnabled) {

		char*					pSortin = nullptr;														//  Sort input in-memory buffer
		char*					pSortout = nullptr;														//  Sort output in-memory buffer
		size_t					SISize = 0;																//  Sort input size
		char*					pEOI = nullptr;															//  Pointer to the End-Of-Input
		char*					pNextRec = nullptr;														//  Pointer to the next record
		IMSR					SRec = {};																//  In-Memory sort record (internal)

		//  Root Splitter of the Splitter chain
		Splitter<IMSR>* pSR = nullptr;

		//  Instrumentation Statistics Object
		IStats					Stats;

		//  Load the designated sort input into memory
		Stats.startLoading();
		pSortin = loadSortInput(SFIn, SISize);
		if (pSortin == nullptr) {
			Log << "ERROR: Failed to load the sort input into memory, it may be too big to sort in-memory." << std::endl;
			return false;
		}
		Stats.finishLoading();

		//  The sort timing starts once the data has been loaded
		Stats.startSorting();

		pEOI = pSortin + SISize;
		pNextRec = pSortin;																				//  Next record is the first

		//  Setup the initial sort record
		SRec.pRec = pNextRec;
		SRec.pKey = pNextRec + SKOff;

		//  Adjust the next record pointer
		pNextRec = strchr(pNextRec, SCHAR_LF);
		if (pNextRec == nullptr) pNextRec = pEOI;
		else pNextRec++;

		//  Create the Root Splitter
		pSR = new Splitter<IMSR>(SRec, SKLen, Stats);
		if (pSR == nullptr) {
			Log << "ERROR: Unable to create the root Splitter to perform the sort." << std::endl;
			return false;
		}

		//
		//  Sort Input phase - load each record to the root splitter
		//

		Stats.startInput();

		//  Process each record in turn
		while (pNextRec < pEOI) {
			//  Build the internal sort record
			SRec.pRec = pNextRec;
			SRec.pKey = pNextRec + SKOff;
			pSR->addStableKey(SRec, Ascending, PMEnabled);

			//  Adjust the next record pointer
			pNextRec = strchr(pNextRec, SCHAR_LF);
			if (pNextRec == nullptr) pNextRec = pEOI;
			else pNextRec++;
		}

		//  Record the ending time
		Stats.finishInput();

		//  If enabled then notify the end of the sort input phase
		if (Notifications) Log << "INFO: Sort input phase has completed." << std::endl;

		//
		//  Sort final merge phase 
		//

		pSR->signalEndOfStableSortInput(Ascending);

		//  If enabled then notify the end of the sort merge phase
		if (Notifications) Log << "INFO: Sort merge phase has completed." << std::endl;

		//
		//  Sort output phase
		//

		//  Check that the sort output is valid
		if (!pSR->isOutputValid()) {
			Log << "ERROR: The number of records in the sort is not valid, there was possibly not enough memory available to complete the sort operation." << std::endl;
			free(pSortin);
			delete pSR;
			return false;
		}

		Stats.startOutput();

		//  Allocate a buffer to hold the sort output
		pSortout = (char*)malloc(SISize);
		if (pSortout == nullptr) {
			Log << "ERROR: Failed to allocate a buffer to hold the sort output (" << SISize << " bytes)." << std::endl;
			free(pSortin);
			delete pSR;
			return false;
		}

		//  Perform the sort output in ascending or descending sequence
		pNextRec = pSortout;
		if (Ascending) {
			//  Ascending sequence
			for (Splitter<IMSR>::Output O = pSR->lowest(); O <= pSR->highest(); O++) {
				const char* pOutRec = (*O).pRec;
				const char* pEOR = strchr(pOutRec, SCHAR_LF);
				if (pEOR == nullptr) pEOR = pEOI;
				size_t  RecLen = (pEOR - pOutRec) + 1;
				memcpy(pNextRec, pOutRec, RecLen);
				pNextRec += RecLen;
			}
		}
		else {
			//  Descending sequence 
			for (Splitter<IMSR>::Output O = pSR->highest(); O >= pSR->lowest(); O--) {
				const char* pOutRec = (*O).pRec;
				const char* pEOR = strchr(pOutRec, SCHAR_LF);
				if (pEOR == nullptr) pEOR = pEOI;
				size_t  RecLen = (pEOR - pOutRec) + 1;
				memcpy(pNextRec, pOutRec, RecLen);
				pNextRec += RecLen;
			}
		}

		//  The sort ending time is taken at this point
		Stats.finishOutput();
		Stats.finishSorting();

		//  Notify end of phase
		if (Notifications) Log << "INFO: Sort output phase completed." << std::endl;

		//  Write the sortout buffer to disk
		Stats.startStoring();
		if (!storeSortOutput(SFOut, pSortout, SISize)) {
			Log << "ERROR: Failed to store: " << SISize << "bytes of sort output data." << std::endl;
			free(pSortout);
			return false;
		}
		Stats.finishStoring();

		//  Free the input and the root splitter
		free(pSortin);
		delete pSR;
		free(pSortout);

		//  If enabled show the timings
		if (Timings) Stats.showStats(Log);

		//  Return showing success
		return true;
	}

	//  sortStableFileOnDisk
	//
	//  This function will sort the passed file on-disk and write the sorted output to the passed file name.
	//  Sorting will conditionally use Preemptive Merging, the sort sequence is stable.
	//
	//  PARAMETERS:
	// 
	//		char*		-		Const pointer to the sort input file name
	//		char*		-		Const pointer to the sort output file
	//		size_t		-		Maximum record length
	//		size_t		-		Offset (in records) to the sort key
	//		size_t		-		Length of the sort key
	//		bool		-		true if the sort sequence is ascending, false if descending
	//		bool		-		true if Preemptive Merging is enabled, false if disabled
	//
	//  RETURNS:
	// 
	//		bool		-		true if the sort was completed, otherwise false.
	//
	//  NOTES:
	//

	bool	sortStableFileOnDisk(const char* SFIn,
		const char* SFOut,
		size_t MaxRecl,
		size_t SKOff,
		size_t SKLen,
		bool Ascending,
		bool PMEnabled) {

		std::ifstream			Sortin;																	//  Sort input stream
		std::ofstream			Sortout;																//  Sort output stream
		char*					SortRec = nullptr;														//  Input record buffer
		ODSR					SRec = {};																//  Sort Record (internal)

		//  Root Splitter of the Splitter chain
		Splitter<ODSR>* pSR = nullptr;

		//  Instrumentation Statistics Object
		IStats					Stats;

		//
		//  Setup ready for the input phase of the sort
		//

		Log << "WARNING: This sort is being performed on-disk, DO NOT use the timings for benchmarks." << std::endl;

		SortRec = (char*)malloc(MaxRecl);
		if (SortRec == nullptr) {
			Log << "ERROR: Failed to allocate a " << MaxRecl << " byte buffer for sort input records." << std::endl;
			return false;
		}
		memset(SortRec, 0, MaxRecl);

		Sortin.open(SFIn, std::istream::in);
		if (!Sortin.is_open()) {
			Log << "ERROR: Failed to open the designated sort input file: '" << SFIn << "'." << std::endl;
			free(SortRec);
			return false;
		}

		if (Sortin.eof()) {
			Log << "ERROR: The sort input file is empty." << std::endl;
			free(SortRec);
			Sortin.close();
			return false;
		}

		Sortin.getline(SortRec, MaxRecl);
		if (Sortin.fail()) {
			Log << "ERROR: Failed to read the first record from the sort input file: '" << SFIn << "'." << std::endl;
			free(SortRec);
			Sortin.close();
			return false;
		}

		//  Build the initial SortKey record
		SRec.RecPos = 0;
		SRec.pKey = SortRec + SKOff;

		//  Construct the root Splitter
		pSR = new Splitter<ODSR>(SRec, SKLen, 64, Stats);
		if (pSR == nullptr) {
			Log << "ERROR: Unable to create the root sort splitter." << std::endl;
			free(SortRec);
			Sortin.close();
			return false;
		}

		//
		//  Sort input phase
		//

		Stats.startInput();
		while (!Sortin.eof()) {
			SRec.RecPos = Sortin.tellg();
			Sortin.getline(SortRec, MaxRecl);
			if (Sortin.eof() && strlen(SortRec) == 0) break;
			SRec.pKey = SortRec + SKOff;

			//  Add the new record to the root splitter
			pSR->addStableExternalKey(SRec, Ascending, PMEnabled);
		}

		Stats.finishInput();

		//  If enabled then notify the end of the sort input phase
		if (Notifications) Log << "INFO: Sort input phase has completed." << std::endl;

		//
		//  Sort merge phase
		//

		pSR->signalEndOfStableSortInput(Ascending);

		//  If enabled then notify the end of the sort merge phase
		if (Notifications) Log << "INFO: Sort merge phase has completed." << std::endl;

		//
		//  Sort output phase
		//

		//  Check that the sort output is valid
		if (!pSR->isOutputValid()) {
			Log << "ERROR: The number of records in the sort is not valid, there was possibly not enough memory available to complete the sort operation." << std::endl;
			free(SortRec);
			Sortin.close();
			delete pSR;
			return false;
		}

		//  Open the output file
		Sortout.open(SFOut, std::ofstream::out);
		if (!Sortout.is_open()) {
			Log << "ERROR: Failed to open/create the designated sort output file: '" << SFOut << "'." << std::endl;
			free(SortRec);
			Sortin.close();
			delete pSR;
			return false;
		}

		//  Clear EOF and Fail on the sortin stream
		Sortin.clear(std::ifstream::goodbit);

		Stats.startOutput();

		if (Ascending) {

			for (Splitter<ODSR>::Output O = pSR->lowest(); O <= pSR->highest(); O++) {
				//  Position sortin to the record
				Sortin.seekg((*O).RecPos);
				//  Read the record
				Sortin.getline(SortRec, MaxRecl);
				//  Write the record to sort output
				Sortout << SortRec << std::endl;
			}
		}
		else {
			//  Descending sort sequence
			for (Splitter<ODSR>::Output O = pSR->highest(); O >= pSR->lowest(); O--) {
				//  Position sortin to the record
				Sortin.seekg((*O).RecPos);
				//  Read the record
				Sortin.getline(SortRec, MaxRecl);
				//  Write the record to sort output
				Sortout << SortRec << std::endl;
			}
		}

		Stats.finishOutput();

		//  Notify end of phase
		if (Notifications) Log << "INFO: Sort output phase completed." << std::endl;

		//  Record End of sort
		Stats.finishSorting();

		//  Free the input, output and the root splitter
		Sortin.close();
		Sortout.close();
		delete pSR;
		free(SortRec);

		//  If enabled show the timings
		if (Timings) Stats.showStats(Log);

		//  Return showing success
		return true;
	}


private:

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Private Members			                                                                                    *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  External references
	std::ostream& Log;

	//  Configuration Controls
	bool				Notifications;										//  Notification messages enabled
	bool				Timings;											//  Timing messages enabled


	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Private Functions                                                                                             *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  loadSortInput
	//
	//  This function will load the sort input into memory and normalise the end-of-file, any spurious empty records
	//  will be removed from the end and the last record will be terminated with an appropriate end-line.
	//
	//  PARAMETERS:
	// 
	//		char*		-		Const pointer to the sort input file name
	//		size_t&		-		Reference to the variable to hold the size of the loaded file
	//
	//  RETURNS:
	// 
	//		char*		-		Pointer to the allocated buffer holding the file contents
	//
	//  NOTES:
	// 
	//		The sortin file MUST be pre-checked for validity
	//		Any problems encountered will result in a nullptr being returned and the size reported as zero
	//

	char* loadSortInput(const char* szSortin, size_t& SILen) {
		FILE*		pRFile = nullptr;																					//  Handle of the sortin file
		errno_t		Result = 0;																							//  Return from fopen_s()
		size_t		FSize = 0;																							//  File size
		size_t		ElementsRead = 0;																					//  Number of alements read
		char*		pFImg = nullptr;																					//  Sort input image
		size_t		FixLen = 0;																							//  Fixed length of the file content
		bool		B2IRS = false;																						//  2 byte IRS (cr/lf) in use
		char*		pIRS = nullptr;																						//  Pointer to an IRS

		//  Safety
		SILen = 0;

		//  Open the file
		Result = fopen_s(&pRFile, szSortin, "rb");
		if (Result != 0 || pRFile == nullptr) {
			Log << "ERROR: Unable to open file: '" << szSortin << "' - Open RC: " << Result << "." << std::endl;
			return nullptr;
		}

		//  Determine the file size
		fseek(pRFile, 0, SEEK_END);
		FSize = ftell(pRFile);
		rewind(pRFile);

		//  Allocate a buffer to hold the file contents
		//  3 additional bytes are allocated one for EOS (\0) and two for a possible cr/lf insert
		pFImg = (char*)malloc(FSize + 3);
		if (pFImg == nullptr) {
			fclose(pRFile);
			Log << "ERROR: Failed to allocate: " << FSize << " bytes to hold the sort input." << std::endl;
			return nullptr;
		}

		//  Read the content of the file into memory
		ElementsRead = fread(pFImg, 1, FSize, pRFile);
		fclose(pRFile);
		if (ElementsRead != FSize) {
			Log << "ERROR: Failed to load: " << FSize << " bytes of sort input into memory." << std::endl;
			free(pFImg);
			return nullptr;
		}

		//  Make the image a well-formed string
		pFImg[ElementsRead] = '\0';

		//  Determine the IRS in use
		FixLen = FSize;
		pIRS = strchr(pFImg, SCHAR_LF);
		if (pIRS == nullptr) {
			SILen = FixLen;
			return pFImg;
		}
		if (pIRS == pFImg) {
			SILen = FixLen;
			return pFImg;
		}
		pIRS--;
		if (*pIRS == SCHAR_CR) B2IRS = true;

		//  Remove any cr/lf bytes from the end of the file
		pIRS = pFImg + (FixLen - 1);
		while ((*pIRS == SCHAR_CR || *pIRS == SCHAR_LF) && pIRS > pFImg) {
			*pIRS = '\0';
			FixLen--;
			pIRS--;
		}
		pIRS++;

		//  Add a new IRS onto the old record
		if (B2IRS) {
			*pIRS = SCHAR_CR;
			FixLen++;
			pIRS++;
			*pIRS = SCHAR_LF;
			FixLen++;
		}
		else {
			*pIRS = SCHAR_LF;
			FixLen++;
		}

		//  Return the loaded, normalised image
		SILen = FixLen;
		return pFImg;
	}

	//  stortSortOutput
	//
	//  This function will store the sort output from the passed memory buffer to the sortout file.
	//
	//  PARAMETERS:
	// 
	//		char*		-		Const pointer to the sort output file name
	//		char*		-		Pointer to the buffer holding the sort output
	//		size_t		-		Size of the sort output buffer
	//
	//  RETURNS:
	// 
	//		bool		-		true if the oputput was stored, otherwise false
	//
	//  NOTES:
	// 

	bool	storeSortOutput(const char* szSortOut, char* pSO, size_t SOSize) {
		FILE*		pRFile = nullptr;																					//  Handle of the output file
		errno_t		Result = 0;																							//  Return from fopen_s()
		size_t		ElementsWrit = 0;																					//  Number of elements written

		//  Safety
		if (szSortOut == nullptr) return false;
		if (szSortOut[0] == '\0') return false;
		if (pSO == nullptr) return false;
		if (SOSize == 0) return false;

		//  Open the requested file for output
		Result = fopen_s(&pRFile, szSortOut, "wb");
		if (Result != 0) return false;

		//  Write the file contents
		if (pRFile != nullptr) {
			ElementsWrit = fwrite(pSO, 1, SOSize, pRFile);
			if (ElementsWrit != SOSize) {
				fclose(pRFile);
				return false;
			}

			//  Close the output file
			fclose(pRFile);
		}

		//  Return showing success
		return true;
	}
};
