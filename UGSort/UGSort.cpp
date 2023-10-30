//*******************************************************************************************************************
//*																													*
//*   File:       UGSort.cpp																						*
//*   Suite:      Experimental Algorithms																			*
//*   Version:    1.16.1	(Build: 18)																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*	UGSort																											*
//*																													*
//*	This application will provide a testbed for the 'UGSort' algorithm.												*
//*																													*
//*	USAGE:																											*
//*																													*
//*		UGSort [<Project>] [<in> <out> <switches>]																	*
//*																													*
//*     where:-																										*
//*																													*
//*		<Project>	-	Is the path to the directory project files to use.											*
//*		<in>		-	is the sort input (sortin) file name (virtual)												*
//*		<out>		-	is the sort output (sortout) file name (virtual)											* 
//*		NOTE: Both or neither may be specified here																	*
//*																													*
//*		switches																									*
//*																													*
//*			-pm				Enables preemptive merging																*
//*			-nopm			Disables preemptive merging																*
//*			-maxrecl:l		Specifies the maximum record length (default: 16kB)										*
//*			-inmem			Use in-memory sorting model																*
//*			-ondisk			Use on-disk sorting model																*
//*			-pms:s			Specifies the maximum number of splitters allowed before PM is triggered				*
//*			-pmm:m			Specifies the merge count of the preemptive merge pattern								*
//*			-pmn:n			Specifies the count of the preemptive merge pattern, pm will merge m/n splitters		*
//*			-skoffset:o		Specifies the offset in the records to the sort key										*
//*			-sklen:l		Specifies the length of the sort key													*
//*			-ska			Specifies that the sort sequence is ascending											*
//*			-skd			Specifies that the sort sequence is descending											*
//*			-sks			Specifies that the record sequence is preserved (stable) for identical keys				*
//*																													*
//*	NOTES:																											*
//*																													*
//*	1.																												*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 -		08/01/2023	-	Initial Release																		*
//*	1.2.0 -		01/03/2023	-	Final merge done with preemptive merges												*
//*	1.3.0 -		06/03/2023	-	Adaptive PM																			*
//*	1.4.0 -		10/03/2023	-	Output iterators added																*
//*	1.5.0 -		13/03/2023	-	SS3 structure changes																*
//*	1.6.0 -		15/03/2023	-	Stable Key Handling																	*
//*	1.7.0 -		03/05/2023	-	Head Preemptive Merge 																*
//*	1.8.0 -		03/05/2023	-	Tail Preemptive Merge 																*
//*	1.9.0 -		03/05/2023	-	Revert to Alternate Preemptive Merge												*
//*	1.10.0 -	04/05/2023	-	Autonomous computation of Maximum Splitters											*
//*	1.11.0 -	06/05/2023	-	Tail first final merge																*
//*	1.12.0 -	06/05/2023	-	Revert Tail first final merge														*
//*	1.13.0 -	13/06/2023	-	PM Activity & T_SO sub-phase timing													*
//*	1.14.0 -	08/07/2023	-	Remove T_SO sub-phase timing and clarify timings									*
//*							-	Corrected PM control parameters														*
//*	1.15.0 -	28/08/2023	-	Binary-Chop search of Store Chain													*
//*	1.16.0 -	16/10/2023	-	Improved PM handling of Worst Case (Tail-Suppression)								*
//*	1.16.1 -	19/10/2023	-	Increase PM timer resolution														*
//*																													*
//*******************************************************************************************************************/

//  Application headers
#include	"UGSort.h"

//  Main Entry Point for the SplitSort application

int main(int argc, char* argv[])
{
	UGSCfg			Config(APP_NAME, argc, argv);									//  Application configuration singleton

	//  Check that a valid log has been established
	if (!Config.isLogOpen()) {
		std::cerr << "ERROR: The application logger was unable to start, " << APP_NAME << " will not execute." << std::endl;
		return EXIT_FAILURE;
	}

	//  Show that program is starting 
	Config.Log << APP_TITLE << " (" << APP_NAME << ") Version: " << APP_VERSION << " is starting." << std::endl;

	//  Verify the capture of configuration variables
	if (!Config.isValid()) {
		Config.Log << "ERROR: The application configuration is not valid, no further processing is possible." << std::endl;
		Config.dismiss();
		return EXIT_FAILURE;
	}

	//
	//  Perform the split sort as requested in the configuration
	//
	if (Config.isSortSequenceStable()) {
		if (!performStableSplitSort(Config)) {
			Config.Log << "ERROR: The requested Stable SplitSort could not be completed, see previous message(s)." << std::endl;
			Config.dismiss();
			return EXIT_FAILURE;
		}
	}
	else {
		if (!performSplitSort(Config)) {
			Config.Log << "ERROR: The requested SplitSort could not be completed, see previous message(s)." << std::endl;
			Config.dismiss();
			return EXIT_FAILURE;
		}
	}

	//  Show normal termination
	Config.Log << APP_TITLE << " (" << APP_NAME << ") Version: " << APP_VERSION << " has completed normally." << std::endl;

	//  Dismiss the xymorg sub-systems
	Config.dismiss();

	//
	//  PLATFORM SPECIFIC for Windows DEBUG ONLY
	//
	//  Check for memory leaks
	//
#if ((defined(_WIN32) || defined(_WIN64)) && defined(_DEBUG))
	CheckForMemoryLeaks();
#endif

	return EXIT_SUCCESS;
}

//  performSplitSort
//
//  This function is top level function for performing the SplitSort sort as requested in the configuration
//
//  PARAMETERS:
//
//		UGSCfg&			-		Reference to the application configuration singleton
//
//  RETURNS:
// 
//		bool			-		true if the sort completed, otherwise false
//
//  NOTES:
//

bool	performSplitSort(UGSCfg& Config) {
	std::ofstream			Sortout;																//  Sort output stream
	Sorter					SWiz(Config.Log);														//  Sorting Wizzard

	//  Determine the run configuration
	if (!establishRunConfig(Config)) {
		Config.Log << "ERROR: Unable to establish a valid run configuration, see previous message(s)." << std::endl;
		return false;
	}

	//  Enable Notifications and Timings in the Sort Wizzard
	SWiz.enableNotifications();
	SWiz.enableTimings();

	//
	//  Open and close the sort output file
	//
	Sortout.open(Config.getSortout(), std::ofstream::out);
	if (!Sortout.is_open()) {
		Config.Log << "ERROR: Failed to open/create the designated sort output file: '" << Config.getSortout() << "'." << std::endl;
		return false;
	}
	Sortout.close();

	//  If the requested sort is to be performed in-memory then invoke the appropriate sort function
	if (Config.isModelInMemory()) {
		if (SWiz.sortFileInMemory(Config.getSortin(),
			Config.getSortout(),
			Config.getSortKeyOffset(),
			Config.getSortKeyLength(),
			Config.isSortSequenceAscending(),
			Config.isPMEnabled())) {
			Config.Log << "INFO: The sort operation has completed." << std::endl;
		}
		else {
			//  Sort failed
			return false;
		}
	}
	//  Requested sort is On-Disk invoke the appropriate sort function
	else {
		if (SWiz.sortFileOnDisk(Config.getSortin(),
			Config.getSortout(),
			Config.getMaxRecl(),
			Config.getSortKeyOffset(),
			Config.getSortKeyLength(),
			Config.isSortSequenceAscending(),
			Config.isPMEnabled())) {
			Config.Log << "INFO: The sort operation has completed." << std::endl;
		}
		else {
			//  Sort failed
			return false;
		}
	}

	//  Return to caller showing sort completed successfully
	return true;
}

//  performStableSplitSort
//
//  This function is top level function for performing the SplitSort sort as requested in the configuration.
//  All sorting uses stable key sequencing.
//
//  PARAMETERS:
//
//		UGSCfg&			-		Reference to the application configuration singleton
//
//  RETURNS:
// 
//		bool			-		true if the sort completed, otherwise false
//
//  NOTES:
//

bool	performStableSplitSort(UGSCfg& Config) {
	std::ofstream			Sortout;																//  Sort output stream
	Sorter					SWiz(Config.Log);														//  Sorting Wizzard

	//  Determine the run configuration
	if (!establishRunConfig(Config)) {
		Config.Log << "ERROR: Unable to establish a valid run configuration, see previous message(s)." << std::endl;
		return false;
	}

	//  Enable Notifications and Timings in the Sort Wizzard
	SWiz.enableNotifications();
	SWiz.enableTimings();

	//
	//  Open and close the sort output file
	//
	Sortout.open(Config.getSortout(), std::ofstream::out);
	if (!Sortout.is_open()) {
		Config.Log << "ERROR: Failed to open/create the designated sort output file: '" << Config.getSortout() << "'." << std::endl;
		return false;
	}
	Sortout.close();

	//  If the requested sort is to be performed in-memory then invoke the appropriate sort function
	if (Config.isModelInMemory()) {
		if (SWiz.sortStableFileInMemory(Config.getSortin(),
			Config.getSortout(),
			Config.getSortKeyOffset(),
			Config.getSortKeyLength(),
			Config.isSortSequenceAscending(),
			Config.isPMEnabled())) {
			Config.Log << "INFO: The sort operation has completed." << std::endl;
		}
		else {
			//  Sort failed
			return false;
		}
	}
	//  Requested sort is On-Disk invoke the appropriate sort function
	else {
		if (SWiz.sortStableFileOnDisk(Config.getSortin(),
			Config.getSortout(),
			Config.getMaxRecl(),
			Config.getSortKeyOffset(),
			Config.getSortKeyLength(),
			Config.isSortSequenceAscending(),
			Config.isPMEnabled())) {
			Config.Log << "INFO: The sort operation has completed." << std::endl;
		}
		else {
			//  Sort failed
			return false;
		}
	}

	//  Return to caller showing sort completed successfully
	return true;
}

//  establishRunConfig
//
//  This function will establish a valid run configuration
//
//  PARAMETERS:
//
//		UGSCfg&			-		Reference to the application configuration singleton
//
//  RETURNS:
// 
//		bool			-		true if the configuration was established, otherwise false
//
//  NOTES:
//

bool		establishRunConfig(UGSCfg& Config) {
	size_t		SISize = 0;																		//  The sortin file size
	size_t		InMemLimit = size_t(1024) * size_t(1024) * size_t(1024);						//  In-Memory size limit (1 GB)
	char		RealFile[MAX_PATH + 1] = {};													//  Real file name

	//  Determine if there is a valid sort input file, if so update the file name (from relative to actual)
	Config.RMap.mapFile(Config.getSortin(), RealFile, MAX_PATH);
	SISize = Config.RMap.getResourceSize(Config.getSortin());

	//  Report the sortin file name and size
	if (strcmp(Config.getSortin(), RealFile) == 0) Config.Log << "INFO: Sort input file: '" << Config.getSortin() << "', size: " << SISize << "." << std::endl;
	else Config.Log << "INFO: Sort input file: '" << Config.getSortin() << "' ('" << RealFile << "'), size: " << SISize << "." << std::endl;
	if (SISize == 0) {
		Config.Log << "ERROR: The sort input file does not exist/cannot be accessed/is empty, sorting not possible." << std::endl;
		return false;
	}

	//  Update the sort input file name to hold the actual file name
	Config.updateSortin(RealFile);

	//  Report the sort output file
	Config.RMap.mapFile(Config.getSortout(), RealFile, MAX_PATH);
	if (strcmp(Config.getSortout(), RealFile) == 0) Config.Log << "INFO: Sort output file: '" << Config.getSortout() << "'." << std::endl;
	else Config.Log << "INFO: Sort output file: '" << Config.getSortout() << "' ('" << RealFile << "')." << std::endl;

	//  Update the sort output file name to hold the actual file name
	Config.updateSortout(RealFile);

	//
	//  Resolve the sort memory model to use (in-memory or on-disk), if not specified then in-memory will be selected if size of the input file is
	//  within the limit for in-memory sorting otherwise on-disk will be selected.
	//

	if (Config.isModelSpecified()) {
		//  If the on-disk model is explicitly selected then clear the in-memory selection
		if (Config.isModelOnDisk()) Config.clearInMemoryModel();
	}
	else {
		//  Model has not been explicitly selected - if the size is within the in-memory limit then use in memory, otherwise use on-disk
		if (SISize <= InMemLimit) Config.setInMemoryModel();
		else Config.clearInMemoryModel();
	}

	//  Report the model
	if (Config.isModelInMemory()) Config.Log << "INFO: The sort will be processed in-memory." << std::endl;
	else Config.Log << "INFO: The sort will be processed on-disk." << std::endl;

	//  Report the sort key specification
	Config.Log << "INFO: The sort will be on a key of length: " << Config.getSortKeyLength() << " at offset: " << Config.getSortKeyOffset();
	if (Config.isSortSequenceAscending()) Config.Log << ", sequence: Ascending." << std::endl;
	else Config.Log << ", sequence: Descending." << std::endl;
	if (Config.isSortSequenceStable()) Config.Log << "INFO: The sorting sequence is 'stable' for duplicate keys." << std::endl;

	//  Indicate the preemptive merging settings
	if (Config.isPMEnabled()) {
		Config.Log << "INFO: Preemptive merging is enabled." << std::endl;
	}
	else Config.Log << "INFO: Preemptive merging is NOT enabled." << std::endl;

	//  Return showing success
	return true;
}
