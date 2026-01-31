#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       UGSort.h																							*
//*   Suite:      Experimental Algorithms																			*
//*   Version:    1.17.1	(Build: 21)																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2026 Ian J. Tree																				*
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
//*	1.16.2 -	18/11/2024	-	Headers sanitized for gcc 8.5														*
//*	1.17.0 -	28/01/2026	-	Include instrumentation package														*
//*	1.17.1 -	31/01/2026	-	Tidy up for Linux Compatability														*
//*																													*
//*******************************************************************************************************************/

//  Include xymorg headers
#include	"../xymorg/xymorg.h"															//  xymorg system headers

//  Application Headers
#include	"UGSCfg.h"																		//  Application configuration
#include	"Sorter.h"																		//  Sorter class

//  Identification Constants
#define		APP_NAME			"UGSort"
#define		APP_TITLE			"UGSort Algorithm Testbed"
#ifdef _DEBUG
#define		APP_VERSION			"1.17.1 build: 21 Debug"
#else
#define		APP_VERSION			"1.17.1 build: 21"
#endif

//  Forward Declarations/ Function Prototypes
bool		performSplitSort(UGSCfg& Config);												//  Perform the split sort
bool		performStableSplitSort(UGSCfg& Config);											//  Perform the split sort with stable sequencing
bool		establishRunConfig(UGSCfg& Config);												//  Establish the run configuration
