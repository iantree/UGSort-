#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       IStats.h																							*
//*   Suite:      Experimental Algorithms																			*
//*   Version:    1.15.0	(Build: 16)																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
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
//*																													*
//*******************************************************************************************************************/

//  Include xymorg headers
#include	"../xymorg/xymorg.h"															//  xymorg system headers

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

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Public Functions                                                                                              *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Timing and other event recorders

	void		newKey() { NumKeys++; return; }
	void		startLoading() { StartLoad = xymorg::CLOCK::now(); return; }
	void		finishLoading() { EndLoad = xymorg::CLOCK::now(); return; }
	void		startSorting() { StartSort = xymorg::CLOCK::now(); return; }
	void		finishSorting() { EndSort = xymorg::CLOCK::now(); return; }
	void		startInput() { StartInput = xymorg::CLOCK::now(); return; }
	void		finishInput() { EndInput = xymorg::CLOCK::now(); return; }
	void		startFM() { StartMerge = xymorg::CLOCK::now(); return; }
	void		finishFM(size_t NSM) {
		EndMerge = xymorg::CLOCK::now();
		FMStoresMerged = NSM;
		return;
	}
	void		startOutput() { StartOut = xymorg::CLOCK::now(); return; }
	void		finishOutput() { EndOut = xymorg::CLOCK::now(); return; }
	void		startStoring() { StartStore = xymorg::CLOCK::now(); return; }
	void		finishStoring() { EndStore = xymorg::CLOCK::now(); return; }
	void		startPM() {
		StartPM = xymorg::CLOCK::now();
		NumPMs++;
		return;
	}
	void		finishPM(size_t NSM) {
		EndPM = xymorg::CLOCK::now();
		CumPMTime += DURATION(xymorg::MILLISECONDS, EndPM - StartPM);
		PMStoresMerged += NSM;
		return;
	}

	//  Computations and Display

	void		prepareStatistics() {
		xymorg::MILLISECONDS	PhaseTime(0);															//  Phase duration

		//  Compute all phase times
		PhaseTime = DURATION(xymorg::MILLISECONDS, EndLoad - StartLoad);
		LoadPhase = PhaseTime.count();
		PMPhase = CumPMTime.count();
		//  Note: Input time is computed as the duration of the phase less the cumulative Pre-emptive Merge duration
		PhaseTime = DURATION(xymorg::MILLISECONDS, EndInput - StartInput);
		InputPhase = PhaseTime.count() - PMPhase;
		PhaseTime = DURATION(xymorg::MILLISECONDS, EndMerge - StartMerge);
		FMPhase = PhaseTime.count();
		PhaseTime = DURATION(xymorg::MILLISECONDS, EndOut - StartOut);
		OutputPhase = PhaseTime.count();
		PhaseTime = DURATION(xymorg::MILLISECONDS, EndSort - StartSort);
		SortPhase = PhaseTime.count();
		PhaseTime = DURATION(xymorg::MILLISECONDS, EndStore - StartStore);
		StorePhase = PhaseTime.count();

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
	xymorg::MILLISECONDS	CumPMTime;											//  Cumulative Pre-emptive Merge duration

};
