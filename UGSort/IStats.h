#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       IStats.h																							*
//*   Suite:      Experimental Algorithms																			*
//*   Version:    1.17.0	(Build: 20)																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2026 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the IStats class.													*
//* The class is used to collect instrumentation measurements from deep within the UGSort implementation.			*
//*																													*
//*	NOTES:																											*
//*																													*
//*	1.																												*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*	1.15.0 -	10/08/2023	-	Initial Version																		*
//*	1.16.1 -	19/10/2023	-	Increase PM timer resolution														*
//*	1.17.0 -	28/01/2026	-	Include instrumentation package														*
//*																													*
//*******************************************************************************************************************/

//  Include xymorg headers
#include	"../xymorg/xymorg.h"															//  xymorg system headers

//  Constant expressions for instrumentation package

#ifdef INSTRUMENTED
constexpr		xymorg::SWITCHES	INSTRUMENT_PILEUP = 1;									//  Pileup instrument
constexpr		xymorg::SWITCHES	INSTRUMENT_MERGE = 2;									//  Merge instrument
constexpr		xymorg::SWITCHES	INSTRUMENT_INSERT = 4;									//  Insert instrument

constexpr		auto	HEADER_PILEUP = "\"Cycle\",\"Records\",\"Stores\",\"RecsInStore\"";
constexpr		auto	HEADER_MERGE = "\"Cycle\",\"Records\",\"Stores\",\"PMrgNo\",MrgNo\",\"Recs1\",\"Recs2\"";
constexpr		auto	HEADER_INSERT = "\"Cycle\",\"Records\",\"Stores\",\"Compares\",\"Hi-Hits\",\"Lo-Hits\",\"New-Stores\",\"PMs\"";
#endif

//
//		IStats Class definition
//

class IStats {
public:
	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Constructors			                                                                                        *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Default Constructor 
	//
	//  Constructs the IStats object with all measures initialised to their ground state.
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//  NOTES:
	//

	IStats()
		: NumKeys(0)
		, LoadPhase(0)
		, SortPhase(0)
		, InputPhase(0)
		, PMPhase(0)
		, FMPhase(0)
		, OutputPhase(0)
		, StorePhase(0)
		, NumPMs(0)
		, PMStoresMerged(0)
		, FMStoresMerged(0)
		, SortRate(0)
#ifdef INSTRUMENTED
		, AvailableInstruments(0)
		, Compares(0)
		, HiHits(0)
		, LoHits(0)
		, NewStores(0)
		, PMs(0)
		, Stores(1)
#endif
		, StartLoad(xymorg::CLOCK::now())
		, EndLoad(xymorg::CLOCK::now())
		, StartSort(xymorg::CLOCK::now())
		, EndSort(xymorg::CLOCK::now())
		, StartInput(xymorg::CLOCK::now())
		, EndInput(xymorg::CLOCK::now())
		, StartMerge(xymorg::CLOCK::now())
		, EndMerge(xymorg::CLOCK::now())
		, StartOut(xymorg::CLOCK::now())
		, EndOut(xymorg::CLOCK::now())
		, StartStore(xymorg::CLOCK::now())
		, EndStore(xymorg::CLOCK::now())
		, StartPM(xymorg::CLOCK::now())
		, EndPM(xymorg::CLOCK::now())
		, CumPMTime(0)
#ifdef INSTRUMENTED
		, ICNo(0)
		, Interval(0)
		, Trigger(0)
		, PMStores(0)
		, SavedPMCount(0)
#endif
	{

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
	//  Destroys the IStats object
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//  NOTES:
	//  

	~IStats() {

		//  Return to caller
		return;
	}

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Public Members                                                                                                *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Counters

	size_t			NumKeys;													//  Number of Keys Sorted

	//  Sort Phase Timings - All in milliseconds (ms)

	size_t			LoadPhase;													//  Loading data into memory phase
	size_t			SortPhase;													//  The complete sort excluding Load & Store data
	size_t			InputPhase;													//  Sort input phase (excluding PM)
	size_t			PMPhase;													//  Cumulative Pre-emptive Merge phase
	size_t			FMPhase;													//  Final Merge Phase
	size_t			OutputPhase;												//  Output phase
	size_t			StorePhase;													//  Store sorted data phase

	//  Pre-emptive Merge (PM) statistics
	size_t			NumPMs;														//  Number of Pre-emptive merges
	size_t			PMStoresMerged;												//  Number of stores merged by PM

	//  Final Merge (FM) statistics
	size_t			FMStoresMerged;												//  Number of stores merged by FM

	//  Computed Measures
	size_t			SortRate;													//  Sort rate Keys Per Second (kps)

#ifdef INSTRUMENTED

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Public Instrumentation Members                                                                                *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	xymorg::SWITCHES		AvailableInstruments;								//  Instruments available
	std::ofstream			PUIS;												//  Pile-Up instrument output stream
	std::ofstream			MIS;												//  Merge instrument output stream
	std::ofstream			IIS;												//  Insert instrument stream
	int						Compares;											//  Count of key compares (current cycle)
	int						HiHits;												//  Count of High Key Hits (current cycle)
	int						LoHits;												//  Count of Low Key Hits (current cycle)
	int						NewStores;											//  Count of new stores (current cycle)
	int						PMs;												//  Count of pre-emptive merges (current cycle)
	int						Stores;												//  Number of stores in the store chain
#endif

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Public Functions                                                                                              *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Timing and other event recorders

#ifdef INSTRUMENTED
	//  newKey() returns true if a reporting interval has been reached.
	bool		newKey() {
		NumKeys++;
		Trigger--;
		if (Trigger == 0) {
			ICNo++;
			performReporting();
			Trigger = Interval;
			return true;
		}
		return false;
	}
#else
	void		newKey() { NumKeys++; return; }
#endif
	void		startLoading() { StartLoad = xymorg::CLOCK::now(); return; }
	void		finishLoading() { EndLoad = xymorg::CLOCK::now(); return; }
	void		startSorting() { StartSort = xymorg::CLOCK::now(); return; }
	void		finishSorting() { EndSort = xymorg::CLOCK::now(); return; }
	void		startInput() { StartInput = xymorg::CLOCK::now(); return; }
	void		finishInput() { EndInput = xymorg::CLOCK::now(); return; }
	void		startFM() { 
		StartMerge = xymorg::CLOCK::now(); 
#ifdef INSTRUMENTED
		SavedPMCount = NumPMs;
		NumPMs = 0;
		PMStores = 0;
#endif
		return;
	}
	void		finishFM(size_t NSM) {
		EndMerge = xymorg::CLOCK::now();
		FMStoresMerged = NSM;
#ifdef INSTRUMENTED
		NumPMs = SavedPMCount;
#endif
		return;
	}
	void		startOutput() { StartOut = xymorg::CLOCK::now(); return; }
	void		finishOutput() { EndOut = xymorg::CLOCK::now(); return; }
	void		startStoring() { StartStore = xymorg::CLOCK::now(); return; }
	void		finishStoring() { EndStore = xymorg::CLOCK::now(); return; }
	void		startPM() {
		StartPM = xymorg::CLOCK::now();
		NumPMs++;
#ifdef INSTRUMENTED
		PMStores = 0;
#endif
		return;
	}
#ifdef INSTRUMENTED
	void		startStoreMerge(int Recs1, int Recs2) {
		PMStores++;
		Stores--;
		//  Write the merge instrument data line
		MIS << ICNo + 1 << "," << NumKeys << "," << Stores << "," << NumPMs << "," << PMStores << "," << Recs1 << "," << Recs2 << std::endl;
		return;
	}
#endif
	void		finishPM(size_t NSM) {
		EndPM = xymorg::CLOCK::now();
		CumPMTime += DURATION(xymorg::MICROSECONDS, EndPM - StartPM);
		PMStoresMerged += NSM;
		return;
	}

	//  Computations and Display

	void		prepareStatistics() {
		xymorg::MILLISECONDS	PhaseTime(0);															//  Phase duration

		//  Compute all phase times
		PhaseTime = DURATION(xymorg::MILLISECONDS, EndLoad - StartLoad);
		LoadPhase = size_t(PhaseTime.count());
		PMPhase = size_t(CumPMTime.count()) / 1000;
		//  Note: Input time is computed as the duration of the phase less the cumulative Pre-emptive Merge duration
		PhaseTime = DURATION(xymorg::MILLISECONDS, EndInput - StartInput);
		InputPhase = size_t(PhaseTime.count()) - PMPhase;
		PhaseTime = DURATION(xymorg::MILLISECONDS, EndMerge - StartMerge);
		FMPhase = size_t(PhaseTime.count());
		PhaseTime = DURATION(xymorg::MILLISECONDS, EndOut - StartOut);
		OutputPhase = size_t(PhaseTime.count());
		PhaseTime = DURATION(xymorg::MILLISECONDS, EndSort - StartSort);
		SortPhase = size_t(PhaseTime.count());
		PhaseTime = DURATION(xymorg::MILLISECONDS, EndStore - StartStore);
		StorePhase = size_t(PhaseTime.count());

		//  Compute the sort rate (kps)
		if (SortPhase > 0) {
			SortRate = size_t(double(NumKeys) / (double(SortPhase) / 1000.0));
		}

		//  Return to caller
		return;
	}

	void	showStats(std::ostream& Log) {

		//  If nothing was sorted - show that and leave
		if (NumKeys == 0) {
			Log << "INFO: NO keys were sorted during this run, there are no statistics to report." << std::endl;
			return;
		}

		// Prepare Statistics
		prepareStatistics();

		//  Report the number of keys sorted
		Log << "INFO: " << NumKeys << " keys were sorted during this run." << std::endl;

		//  The data load phase is optional - only display non-zero results
		if (LoadPhase > 0) Log << "INFO: Input data was loaded from disk into memory in: " << LoadPhase << " ms." << std::endl;

		//  Report the input phase time
		Log << "INFO: Sort input phase took: " << InputPhase << " ms (excluding time spent in Pre-emptive Merges)." << std::endl;

		//  If there was any PM activity then show it
		if (NumPMs > 0) {
			Log << "INFO: Pre-emptive Merges: " << NumPMs << ", merged: " << PMStoresMerged << " stores in: " << PMPhase << " ms." << std::endl;
		}

		//  Show the final merge stats
		Log << "INFO: Sort final merge phase for: " << FMStoresMerged << " stores took: " << FMPhase << " ms." << std::endl;

		//  Show the ouput phase
		Log << "INFO: Sort output phase took: " << OutputPhase << " ms." << std::endl;

		//  The data store phase is optional - only display non-zero results
		if (StorePhase > 0) Log << "INFO: Sorted data was stored on disk in: " << StorePhase << " ms." << std::endl;

		//  Show the overall sort time
		Log << "INFO: Sort for: " << NumKeys << " keys took: " << SortPhase << " ms (" << SortRate << " kps)." << std::endl;

		//  Return to caller
		return;
	}

#ifdef INSTRUMENTED
	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Instrumentation Public Functions                                                                              *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Activate the pile-up instrument
	void	activatePileUpInstrument(const char* PUIFn) {

		//  Validity check the file name
		if (PUIFn == nullptr) return;
		if (PUIFn[0] == '\0') return;

		//  Open the instrument output stream
		PUIS.open(PUIFn, std::ios::out);
		if (!PUIS.is_open()) return;

		//  Write the CSV Headers into the stream
		PUIS << HEADER_PILEUP << std::endl;

		//  Mark the instrument as available
		AvailableInstruments = AvailableInstruments | INSTRUMENT_PILEUP;

		//  Return to caller
		return;
	}

	//  Activate the merge instrument
	void	activateMergeInstrument(const char* MIFn) {

		//  Validity check the file name
		if (MIFn == nullptr) return;
		if (MIFn[0] == '\0') return;

		//  Open the instrument output stream
		MIS.open(MIFn, std::ios::out);
		if (!MIS.is_open()) return;

		//  Write the CSV Headers into the stream
		MIS << HEADER_MERGE << std::endl;

		//  Mark the instrument as available
		AvailableInstruments = AvailableInstruments | INSTRUMENT_MERGE;

		//  Return to caller
		return;
	}

	//  Activate the insert instrument
	void	activateInsertInstrument(const char* IIFn) {

		//  Validity check the file name
		if (IIFn == nullptr) return;
		if (IIFn[0] == '\0') return;

		//  Open the instrument output stream
		IIS.open(IIFn, std::ios::out);
		if (!IIS.is_open()) return;

		//  Write the CSV Headers into the stream
		IIS << HEADER_INSERT << std::endl;

		//  Mark the instrument as available
		AvailableInstruments = AvailableInstruments | INSTRUMENT_INSERT;

		//  Return to caller
		return;
	}

	//  De-activate the all active instruments
	void	deactivateInstruments() {
		//  Pile-up Instrument
		if (PUIS.is_open()) {
			PUIS.close();
			AvailableInstruments = AvailableInstruments ^ INSTRUMENT_PILEUP;
		}

		//  Merge Instrument
		if (MIS.is_open()) {
			MIS.close();
			AvailableInstruments = AvailableInstruments ^ INSTRUMENT_MERGE;
		}

		//  Insert Instrument
		if (IIS.is_open()) {
			IIS.close();
			AvailableInstruments = AvailableInstruments ^ INSTRUMENT_INSERT;
		}

		//  Return to caller
		return;
	}

	//  Determine if an instrument is available
	bool	isPileUpInstrumentActive() const { return AvailableInstruments & INSTRUMENT_PILEUP; }
	bool	isMergeInstrumentActive() const { return AvailableInstruments & INSTRUMENT_MERGE; }
	bool	isInsertInstrumentActive() const { return AvailableInstruments & INSTRUMENT_INSERT; }

	//  Functions for writing the pile-up stats
	void	writePileUpLeader() {
		PUIS << ICNo << "," << NumKeys << "," << Stores << ",";
		return;
	}
	void	writePileUpStore(int RecCount, bool isLast) {
		PUIS << RecCount;
		if (isLast) PUIS << std::endl;
		else PUIS << ",";
		return;
	}

	//  Set instrumentation controls
	void	setInstrumentationControls(int RInt) {
		//  Set the reporting interval
		Interval = RInt;

		//  Set the reporting trigger
		Trigger = Interval;

		//  Return to caller
		return;
	}

	//  Perform instrumentation reporting cycle
	void	performReporting() {

		//  Report and clear the insert instrument stats (values are reported per cycle)
		IIS << ICNo << "," << NumKeys << "," << Stores << "," << Compares << "," << HiHits << "," << LoHits << "," << NewStores << "," << PMs << std::endl;
		Compares = 0;
		HiHits = 0;
		LoHits = 0;
		NewStores = 0;
		PMs = 0;

		//  Return to caller
		return;
	}

#endif

private:

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Private Members                                                                                               *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Phase timing points

	xymorg::TIMER			StartLoad;											//  Start loading data file
	xymorg::TIMER			EndLoad;											//  End loading data file
	xymorg::TIMER			StartSort;											//  Start Sort
	xymorg::TIMER			EndSort;											//  End Sort
	xymorg::TIMER			StartInput;											//  Start of sort input phase
	xymorg::TIMER			EndInput;											//  End of sort input phase
	xymorg::TIMER			StartMerge;											//  Start of sort merge phase
	xymorg::TIMER			EndMerge;											//  End of sort merge phase
	xymorg::TIMER			StartOut;											//  Start of sort output phase
	xymorg::TIMER			EndOut;												//  End of sort output phase
	xymorg::TIMER			StartStore;											//  Start storing data file
	xymorg::TIMER			EndStore;											//  End storing data file

	//  Timing elements for collecting the cumulative Pre-emptive Merge Time and other PM statistics

	xymorg::TIMER			StartPM;											//  Start Pre-emptive Merge
	xymorg::TIMER			EndPM;												//  End Pre-emptive Merge
	xymorg::MICROSECONDS	CumPMTime;											//  Cumulative Pre-emptive Merge duration

#ifdef INSTRUMENTED
	int						ICNo;												//  Instrument Cycle Number
	int						Interval;											//  Instrument reporting interval
	int						Trigger;											//  Reporting cycle trigger
	int						PMStores;											//  Count of stores merged in this PM
	size_t					SavedPMCount;										//  Saved count of pre-emptive merges performed
#endif

};
