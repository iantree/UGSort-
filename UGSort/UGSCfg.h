#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       UGSCfg.h																							*
//*   Suite:      Experimental Algorithms																			*
//*   Version:    1.5.0	(Build: 06)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Hadleigh Marshall Netherlands b.v.														*
//*******************************************************************************************************************
//*	UGSCfg																											*
//*																													*
//*	This header extends the xymorg AppConfig class to define the class that provides the singleton containing		*
//* all application configuration data plus the xymorg service access objects.										*
//*																													*
//*	USAGE:																											*
//*																													*
//*		The class definition must expand the xymorg::AppConfig class												*
//*																													*
//*	NOTES:																											*
//*																													*
//*	1.																												*
//*																													*
//*******************************************************************************************************************
//*																													*
//*		Configuration XML																							*
//*		-----------------																							*
//*																													*
//*		<sort inmem="true|false" ondisk="true|false" pm="enable|disable" maxsplitters="s" maxinc="i"				*
//*			maxrecl="l>																								*
//*																													*
//*			This section contains the parameters that control the sort												*
//*			inmem and ondisk are mutually exclusive. If neither is specified then sort input file size and			*
//*			key length will determine which is in effect.															*
//*			preemptive merging (pm) is enabled by default so pm="disable" will disable it							*
//*			where l is the maximum record length (default: 16kB)													*
//*																													*
//*			<sortin>i</sortin>																						*
//*																													*
//*				Specifies the sort input																			*
//*				where i is the relative file name of the sort input													*
//*																													*
//*			<sortout>o</sortout>																					*
//*																													*
//*				Specifies the sort output																			*
//*				where o is the relative file name of the sort output												*
//*																													*
//*			<sortwork>w</sortwork>																					*
//*																													*
//*				Specifies the sort working (spill) file name														*
//*				where w is the relative file name of the sort working file											*
//*																													*
//*			<sortkey offset="o" length="l" ascending="true|false" descending="true|false" stable="true|false">		*
//*			</sortkey>																								*
//*																													*
//*				Specifies the sort key - Optional																	*
//*				where o is the offset in the record to the sort key													*
//*				where l is the length of the sort key																*
//*				Ascending is the default so setting ascending="false" or descending="true" will select				*
//*				descending sort order																				*
//*				where stable="true" causes the input record sequence to be preserved for identical keys				*
//*																													*
//*		</sort>																										*
//*																													*
//*******************************************************************************************************************
//*																													*
//*		Command Line																								*
//*		------------																								*
//*																													*
//*		UGSort [<in> <out>] <switches>																				*
//*																													*
//*		<in> is the sort input (sortin) file name (virtual)															*
//*		<out> is the sort output (sortout) file name (virtual)														* 
//*		NOTE: Both or neither may be specified here																	*
//*																													*
//*		switches																									*
//*																													*
//*			-pm				Enables preemptive merging																*
//*			-nopm			Disables preemptive merging																*
//*			-spill:f		Specifies the file name of the sort work (spill) file									*
//*			-maxrecl:l		Specifies the maximum record length (default: 16kB)										*
//*			-inmem			Use in-memory sorting model																*
//*			-ondisk			Use on-disk sorting model																*
//*			-skoffset:o		Specifies the offset in the records to the sort key										*
//*			-sklen:l		Specifies the length of the sort key													*
//*			-ska			Specifies that the sort sequence is ascending											*
//*			-skd			Specifies that the sort sequence is descending											*
//*			-sks			Specifies that the record sequence is preserved (stable) for identical keys				*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 -		04/12/2022	-	Initial Release																		*
//*	1.3.0 -		08/03/2023	-	Adaptive PM																			*
//*	1.5.0 -		13/03/2023	-	SS3 structure changes																*
//*																													*
//*******************************************************************************************************************/

//  Include xymorg headers
#include	"../xymorg/xymorg.h"															//  xymorg system headers

constexpr		size_t		DEFAULT_SORTKEY_LENGTH = 32;									//  Default sort key length

//
//  UGSCfg Class Definition
//

class UGSCfg : public xymorg::AppConfig {
public:

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Constructors			                                                                                        *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Constructor 
	//
	//  Constructs the application configuration object and loads the persistent settings from the config file and command line
	//
	//  PARAMETERS:
	//
	//		char *			-		Const pointer to the application name
	//		int				-		Count of application invocation parameters
	//		char*[]			-		Array of pointers to the application invocation parameters
	//
	//  RETURNS:
	//
	//  NOTES:
	//

	UGSCfg(const char* szAppName, int argc, char* argv[]) : xymorg::AppConfig(szAppName, argc, argv) {

		//  Initialise persistent members
		ConfigValid = true;
		InFile = NULLSTRREF;
		OutFile = NULLSTRREF;
		WorkFile = NULLSTRREF;
		MaxRecl = 16 * 1024;												//  Maximum record length
		SInMem = false;
		SOnDisk = false;
		PMEn = true;
		SSA = true;															//  Default is ascending sequence
		SKOff = 0;															//  Key Offset is start of record
		SKLen = 0;															//  Key length MUST be specified
		KSS = false;														//  Record sequence is NOT maintained for identical keys

		//  Handle the local application configuration settings
		if (pCfgImg == nullptr) handleNoConfig();
		else handleConfig();

		//  Release the configuration image
		releaseConfigImage();

		//  Handle any overrides or additional parameters from the command line
		handleCmdLine(argc, argv);

		//  Validate the configuration
		ConfigValid = validateConfig();

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
	//  Destroys the SSCfg object, dismissing the underlying objects/allocations
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//  NOTES:
	//  

	~UGSCfg() {

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

	//  isValid
	//
	//  This function will return the state of the configuration
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	//
	//		bool		-		The current validity state true if the configuration is valid, otherwise false
	//
	//	NOTES:
	//

	bool		isValid() { return ConfigValid; }

	//  getSortin
	//
	//  This function will return a pointer to the sort input filename specified.
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	//
	//		char*		-		const pointer to the specified input file, nullptr if none
	//
	//	NOTES:
	//

	const char* getSortin() {
		if (InFile == NULLSTRREF) return nullptr;
		return SPool.getString(InFile);
	}

	//  updateSortin
	//
	//  This function will update the sort input filename specified.
	//
	//	PARAMETERS:
	//
	//		char*		-		const pointer to the new input file name
	//
	//	RETURNS:
	//
	//	NOTES:
	// 
	//		1.		Null or empty input string will not update the input file name.
	//

	void	updateSortin(const char* NewInFile) {
		if (NewInFile == nullptr) return;
		if (strlen(NewInFile) == 0) return;
		InFile = SPool.replaceString(InFile, NewInFile);
		return;
	}

	//  getSortout
	//
	//  This function will return a pointer to the sort output filename specified.
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	//
	//		char*		-		const pointer to the specified output file, nullptr if none
	//
	//	NOTES:
	//

	const char* getSortout() {
		if (OutFile == NULLSTRREF) return nullptr;
		return SPool.getString(OutFile);
	}

	//  updateSortout
	//
	//  This function will update the sort output filename specified.
	//
	//	PARAMETERS:
	//
	//		char*		-		const pointer to the new output file name
	//
	//	RETURNS:
	//
	//	NOTES:
	// 
	//		1.		Null or empty output string will not update the output file name.
	//

	void	updateSortout(const char* NewOutFile) {
		if (NewOutFile == nullptr) return;
		if (strlen(NewOutFile) == 0) return;
		OutFile = SPool.replaceString(OutFile, NewOutFile);
		return;
	}

	//  getMaxRecl
	//
	//  This function will return the maximum record length
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	// 
	//		size_t		-		Maximum record length
	//
	//	NOTES:
	//

	size_t	getMaxRecl() { return MaxRecl; }

	//  isModelSpecified
	//
	//  This function will indicate if the in-memory or on-disk model has been specified in the configuration
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	// 
	//		bool		-		Indicator that a model is specified in the configuration
	//
	//	NOTES:
	//

	bool	isModelSpecified() { return SInMem || SOnDisk; }

	//  isModelInMemory
	//
	//  This function will indicate if the in-memory model is specified
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	// 
	//		bool		-		true if the In-Memory mode is selected, otherwise false
	//
	//	NOTES:
	//

	bool	isModelInMemory() { return SInMem; }

	//  isModelOnDisk
	//
	//  This function will indicate if the on-disk model is specified
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	// 
	//		bool		-		true if the On-Disk mode is selected, otherwise false
	//
	//	NOTES:
	//

	bool	isModelOnDisk() { return SOnDisk; }

	//  setInMemoryModel
	//
	//  This function will set the in-memory model
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	//
	//	NOTES:
	//

	void	setInMemoryModel() { SInMem = true; return; }

	//  clearInMemoryModel
	//
	//  This function will clear the in-memory model
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	//
	//	NOTES:
	//

	void	clearInMemoryModel() { SInMem = false; return; }

	//  getSortKeyLength
	//
	//  This function will the length of the sort key
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	// 
	//		size_t		-		Sort key length
	//
	//	NOTES:
	//

	size_t	getSortKeyLength() { return SKLen; }

	//  getSortKeyOffset
	//
	//  This function will the offset of the sort key in the input records
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	// 
	//		size_t		-		Sort key offset
	//
	//	NOTES:
	//

	size_t	getSortKeyOffset() { return SKOff; }

	//  isSortSequenceAscending
	//
	//  This function will indicate if the sort sequence is ascending
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	// 
	//		bool		-		true if the sequence is ascending, false if descending
	//
	//	NOTES:
	//

	bool	isSortSequenceAscending() { return SSA; }

	//  isSortSequenceStable
	//
	//  This function will indicate if the sort sequence is stable (i.e. input sequence is maintained for duplicate keys)
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	// 
	//		bool		-		true if the sequence is stable, otherwise false
	//
	//	NOTES:
	//

	bool	isSortSequenceStable() { return KSS; }

	//  hasSortWork
	//
	//  This function will indicate if a sortwork file has been specified
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	// 
	//		bool		-		true if sortwork was specified, otherwise false
	//
	//	NOTES:
	//

	bool	hasSortWork() { if (WorkFile == NULLSTRREF) return false; return true; }

	//  getSortwork
	//
	//  This function will return a pointer to the sort work filename specified.
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	//
	//		char*		-		const pointer to the specified work file, nullptr if none
	//
	//	NOTES:
	//

	const char* getSortwork() {
		if (WorkFile == NULLSTRREF) return nullptr;
		return SPool.getString(WorkFile);
	}

	//  updateSortwork
	//
	//  This function will update the sort work filename specified.
	//
	//	PARAMETERS:
	//
	//		char*		-		const pointer to the new work file name
	//
	//	RETURNS:
	//
	//	NOTES:
	// 
	//		1.		Null or empty output string will not update the work file name.
	//

	void	updateSortwork(const char* NewWorkFile) {
		if (NewWorkFile == nullptr) return;
		if (strlen(NewWorkFile) == 0) return;
		OutFile = SPool.replaceString(WorkFile, NewWorkFile);
		return;
	}

	//  isPMEnabled
	//
	//  This function will indicate if preemptive merging is enabled
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	// 
	//		bool		-		true if the preemptive merging is enabled, otherwise false
	//
	//	NOTES:
	//

	bool	isPMEnabled() { return PMEn; }

private:

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Private Members			                                                                                    *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	bool					ConfigValid;										//  Configuration Validity

	//  Sort File Names (relative)
	xymorg::STRREF			InFile;												//  Sort input file
	xymorg::STRREF			OutFile;											//  Sort output file
	xymorg::STRREF			WorkFile;											//  Sort working file (spill)
	size_t					MaxRecl;											//  Maximum record length

	bool					SInMem;												//  Sort in-memory (true) or on-disk/don't care (false)
	bool					SOnDisk;											//  Sort on-disk (true) or in-memory/don't care (false)
	bool					PMEn;												//  Preemptive merging enabled (true) or disabled (false)

	bool					SSA;												//  Sort Sequence is ascending (true) or descending (false)
	size_t					SKOff;												//  Sort key offset in bytes within record
	size_t					SKLen;												//  Sort key length in bytes
	bool					KSS;												//  Key (identical) sequence is stable

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Private Functions                                                                                             *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  handleNoConfig
	//
	//  This function is the handler for the "No Config Loaded" event.
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//  NOTES:
	//

	void handleNoConfig() {

		//  Return to caller
		return;
	}

	//  handleConfig
	//
	//  This function is the handler for the "Config Loaded" event. It will parse the application specific values from the 
	//  configuration file.
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//  NOTES:
	//

	void	handleConfig() {
		xymorg::XMLMicroParser					Parser(pCfgImg);								//  XML Micro Parser for the application configuration file
		xymorg::XMLMicroParser::XMLIterator		SortNode = Parser.getScope("sort");				//  Sort node iterator

		//  Check for the presence of a valid 'sort' section
		if (SortNode.isNull() || SortNode.isAtEnd()) {
			Log << "ERROR: The application configuration xml does NOT contain a valid '<sort>' section." << std::endl;
			ConfigValid = false;
			return;
		}

		//  Determine if parameters on the <sort> node specify in-memory or on-disk sorting
		SInMem = SortNode.isAsserted("inmem");
		SOnDisk = SortNode.isAsserted("ondisk");

		//  Get the maximum record size (if specified)
		if (SortNode.hasAttribute("maxrecl")) {
			MaxRecl = SortNode.getAttributeInt("maxrecl");
		}

		//  Determine if there are any preemptive merge parameters specified on the node
		//  If none are specified then the application defaults remain in effect
		if (SortNode.hasAttribute("pm")) {
			PMEn = SortNode.isAsserted("pm");
		}

		//  Capture the sortin, sortout and sortwork file names
		InFile = captureFilename(SortNode, "sortin");
		OutFile = captureFilename(SortNode, "sortout");
		WorkFile = captureFilename(SortNode, "sortwork");

		//  Capture the sortkey specification
		captureSKSpec(SortNode);

		//  Return to caller
		return;
	}

	//  handleCmdLine
	//
	//  This function is the handler for any parameters that are passed via the command line. Command Line parameter values
	//  take precedence over (override) those supplied from the configuration xml file.
	//
	//  PARAMETERS:
	//
	//		int				-		Count of application invocation parameters
	//		char*[]			-		Array of pointers to the application invocation parameters
	// 
	//  RETURNS:
	//
	//  NOTES:
	//

	void	handleCmdLine(int argc, char* argv[]) {
		int				FirstPos = 1;																//  First positional parameter
		int				FirstSwitch = 1;															//  First switch parameter 
		bool			SWValid = false;															//  Switch validity

		//  No parameters are present on the command line
		if (argc == 1) return;

		//  If the first command line parameter is in use (xymorg project root directory) then bump the first positional
		if (isFirstCLPUsed()) {
			FirstPos = 2;
			FirstSwitch = 2;
		}

		//
		//  If positional parameters [1] and [2] then sortin and sortout file names are specified on the command
		//
		if (argc >= (2 + FirstPos)) {
			if (argv[FirstPos][0] != '-') {
				if (argv[FirstPos + 1][0] != '-') {
					//  Two positional parameters present - these will be the sort input and output file names
					InFile = SPool.addString(argv[FirstPos]);
					OutFile = SPool.addString(argv[FirstPos + 1]);
					FirstSwitch = FirstPos + 2;
				}
				else {
					Log << "ERROR: Parameter: '" << argv[FirstPos] << "' on the command was ignored, specify BOTH sort input and output file names." << std::endl;
				}
			}
		}

		//
		//  Capture each of the switches 
		//

		for (int SWX = FirstSwitch; SWX < argc; SWX++) {
			SWValid = false;

			//  Sortwork (spill) file name
			if (strlen(argv[SWX]) > 7) {
				if (_memicmp(argv[SWX], "-spill:", 7) == 0) {
					WorkFile = SPool.addString(argv[SWX] + 7);
					SWValid = true;
				}
			}

			//  Enable preemptive merging (-pm)
			if (strlen(argv[SWX]) == 3) {
				if (_memicmp(argv[SWX], "-pm", 3) == 0) {
					PMEn = true;
					SWValid = true;
				}
			}

			//  Disable preemptive merging (-nopm)
			if (strlen(argv[SWX]) == 5) {
				if (_memicmp(argv[SWX], "-nopm", 5) == 0) {
					PMEn = false;
					SWValid = true;
				}
			}

			//  Use in-memory sort model (-inmem)
			if (strlen(argv[SWX]) == 6) {
				if (_memicmp(argv[SWX], "-inmem", 6) == 0) {
					SInMem = true;
					SWValid = true;
				}
			}

			//  Use on-disk sort model (-ondisk)
			if (strlen(argv[SWX]) == 7) {
				if (_memicmp(argv[SWX], "-ondisk", 7) == 0) {
					SOnDisk = true;
					SWValid = true;
				}
			}

			//  Maximum record length (-maxrecl:l)
			if (strlen(argv[SWX]) > 8) {
				if (_memicmp(argv[SWX], "-maxrecl:", 8) == 0) {
					MaxRecl = atoi(argv[SWX] + 8);
					SWValid = true;
				}
			}

			//  Sort key offset (-skoffset:o)
			if (strlen(argv[SWX]) > 10) {
				if (_memicmp(argv[SWX], "-skoffset:", 10) == 0) {
					SKOff = atoi(argv[SWX] + 10);
					SWValid = true;
				}
			}

			//  Sort key length (-sklen:l)
			if (strlen(argv[SWX]) > 7) {
				if (_memicmp(argv[SWX], "-sklen:", 7) == 0) {
					SKLen = atoi(argv[SWX] + 7);
					SWValid = true;
				}
			}

			//  Sort Sequence Ascending (-ska)
			if (strlen(argv[SWX]) == 4) {
				if (_memicmp(argv[SWX], "-ska", 4) == 0) {
					SSA = true;
					SWValid = true;
				}
			}

			//  Sort Sequence Descending (-skd)
			if (strlen(argv[SWX]) == 4) {
				if (_memicmp(argv[SWX], "-skd", 4) == 0) {
					SSA = false;
					SWValid = true;
				}
			}

			//  Stable sort sequence - preseve input sequence for identical keys (-sks)
			if (strlen(argv[SWX]) == 4) {
				if (_memicmp(argv[SWX], "-sks", 4) == 0) {
					KSS = true;
					SWValid = true;
				}
			}

			//
			//  Invalid parameter
			//
			if (!SWValid) {
				Log << "ERROR: Unrecognised parameter: '" << argv[SWX] << "' on the command line has been ignored." << std::endl;
			}
		}

		//  Return to caller
		return;
	}

	//  validateConfig
	//
	//  This function will determine if the supplied parameters (config and command line) are complete and within
	//  the envelope where execution can be attempted. This is a shallow validation.
	//
	//  PARAMETERS:
	// 
	//  RETURNS:
	// 
	//		bool			-		true if the configuration is complete and valid, otherwise false
	//
	//  NOTES:
	//

	bool	validateConfig() {

		//  If the configuration is already flagged as invalid then no further validation is needed
		if (!ConfigValid) return false;

		//  Check for a sort input file name
		if (InFile == NULLSTRREF) {
			Log << "ERROR: No sort input file (sortin) was supplied, configuration is invalid." << std::endl;
			ConfigValid = false;
		}

		//  Check for an empty input file name
		if (strlen(SPool.getString(InFile)) == 0) {
			Log << "ERROR: No sort input file (sortin) was supplied, configuration is invalid." << std::endl;
			ConfigValid = false;
		}

		//  Check for a sort output file name
		if (OutFile == NULLSTRREF) {
			Log << "ERROR: No sort output file (sortout) was supplied, configuration is invalid." << std::endl;
			ConfigValid = false;
		}

		//  Check for an empty output file name
		if (strlen(SPool.getString(OutFile)) == 0) {
			Log << "ERROR: No sort output file (sortout) was supplied, configuration is invalid." << std::endl;
			ConfigValid = false;
		}

		//  Check for a valid sort key length - if not specified warn that default is being used
		if (SKLen == 0) {
			Log << "WARNING: No sort key length was specified, using the default: " << DEFAULT_SORTKEY_LENGTH << "." << std::endl;
			SKLen = DEFAULT_SORTKEY_LENGTH;
		}

		//  Check max record length
		if (MaxRecl < (16 * 1024)) MaxRecl = (16 * 1024);

		if (!ConfigValid) return false;

		//  Return showing valid config
		return true;
	}

	//  captureFilename
	//
	//  This function will capture the file name specified in the named section.
	//
	//  PARAMETERS:
	// 
	//		XMLIterator&		-		Reference to an XML iterator positioned to the sort section
	//		char*				-		The name of the section 
	//  RETURNS:
	// 
	//		xymorg::STRREF		-		String reference token for the captured filename
	//
	//  NOTES:
	//

	xymorg::STRREF	captureFilename(xymorg::XMLMicroParser::XMLIterator& SNode, const char* Section) {
		xymorg::XMLMicroParser::XMLIterator			FNode = SNode.getScope(Section);				//  Section Node iterator
		const char* pText = nullptr;								//  Pointer to the text
		size_t										TextLen = 0;									//  Text length

		if (FNode.isNull() || FNode.isAtEnd()) return NULLSTRREF;

		pText = FNode.getElementValue(TextLen);
		if (TextLen == 0) return NULLSTRREF;
		return SPool.addString(pText, TextLen);
	}

	//  captureSKSpec
	//
	//  This function will capture the sortkey specification (if supplied)
	//
	//  PARAMETERS:
	// 
	//		XMLIterator&		-		Reference to an XML iterator positioned to the sort section
	// 
	//  RETURNS:
	//
	//  NOTES:
	//

	void	captureSKSpec(xymorg::XMLMicroParser::XMLIterator& SNode) {
		xymorg::XMLMicroParser::XMLIterator			SKNode = SNode.getScope("sortkey");				//  Sortkey section Node iterator

		if (SKNode.isNull() || SKNode.isAtEnd()) return;

		//  Capture the offset and length of the key
		if (SKNode.hasAttribute("offset")) SKOff = SKNode.getAttributeInt("offset");
		if (SKNode.hasAttribute("length")) SKLen = SKNode.getAttributeInt("length");

		//  Determine the sort sequence (if specified)
		if (SKNode.hasAttribute("ascending")) {
			SSA = SKNode.isAsserted("ascending");
		}
		if (SKNode.hasAttribute("descending")) {
			SSA = !SKNode.isAsserted("descending");
		}

		//  Determine if record sequence is to be maintained for identical keys (stable)
		if (SKNode.hasAttribute("stable")) {
			KSS = SKNode.isAsserted("stable");
		}
		return;
	}
};
