#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       VRMapper.h																						*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.1.0	(Build: 02)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the VRMapper class. The class holds the Virtual Resource Maps		*
//* and provides functionality for locating and loading application resources.										*
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
//*	1.1.0 -		27/10/2022	-	MIME type resolver																	*
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
#include	"BOM/QHierarchy.h"																//  Basic Hierarchy
#include	"CODECS/Chimera.h"																//  Chimera Data Compression
#include	"XMLMicroParser.h"																//  XML Parsing

#ifdef  XY_NEEDS_CRYPTO
#include	"CRYPTO/SObjectPool.h"															//  Secure objects pool
#include	"CRYPTO/TripleDESCBC.h"															//  Triple-DES CODEC
#endif

//
//  NODE Identifiers for the Virtual Resource Map
//

constexpr auto NODE_VRES_MAP = "VRMap";
constexpr auto NODE_VRES_NODE = "node";
constexpr auto PARM_VRES_NAME = "name";
constexpr auto PARM_VRES_TYPE = "type";
constexpr auto PARM_VRES_TGT = "target";

//
//  All components are defined within the xymorg namespace
//
namespace xymorg {

	//
	//  RNode class definition
	//
	//	The RNode class defines a node in the resource map hierarchy
	//

	class RNode {
	public:
		RNode() {
			Name = NULLSTRREF;
			MappedName = NULLSTRREF;
			return;
		}

		~RNode() {
			Name = NULLSTRREF;
			MappedName = NULLSTRREF;
			return;
		}

		STRREF getName() { return Name; }
		STRREF getMappedName() { return MappedName; }
		void setName(STRREF NewName) { Name = NewName; return; }
		void setMappedName(STRREF NewMapping) { MappedName = NewMapping; return; }

	private:
		//  Mapping string references
		STRREF			Name;																//  Virtual name
		STRREF			MappedName;															//  Mapped Name
	};

	//
	//  VRMapper class definition
	//

	class VRMapper {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors			                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Constructor 
		//
		//  Constructs a new VRMapper object, determining the project root location for the application
		//
		//  PARAMETERS:
		//
		//		StringPool&			-		Reference to the application level public string pool
		//		SOPool&				-		Reference to the secure object pool
		//		int					-		Count of application invocation parameters
		//		char*[]				-		Array of pointers to the application invocation parameters
		//		
		//
		//  RETURNS:
		//
		//  NOTES:
		//

#ifdef XY_NEEDS_CRYPTO
		VRMapper(StringPool& PubSpool, SObjectPool& SecPool, int argc, char* argv[]) : SPool(PubSpool), SOPool(SecPool), RString(NULLSTRREF), MTMap(nullptr), CLFPUsed(false) {
#else
		VRMapper(StringPool & PubSpool, int argc, char* argv[]) : SPool(PubSpool), RString(NULLSTRREF), MTMap(nullptr), CLFPUsed(false) {
#endif

			//  Discover the project root directory
			RString = setRoot(argc, argv);
			if (RString == NULLSTRREF) return;

			//  Setup the Resource Map root 
			RMap.getRoot()->setName(NULLSTRREF);
			RMap.getRoot()->setMappedName(RString);

			//  Setup the default nodes of the resource map in the hierarchy
			//
			//	1.		Config		--->		<root>/Config
			//  2.		Logs		--->		<root>/Logs
			//

			setupDefaultMap();

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
		//  Destroys the VRMapper object destroying any contained objects.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		~VRMapper() {

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

		bool	isFirstCLPUsed() { return CLFPUsed; }

		//  extendConfiguration
		//
		//  This function will extend the current configuration of the resource map based on the content of the
		//  <VRMap> element in the configuration file.
		//  
		//
		//  PARAMETERS:
		//
		//		XMLIterator&			-		Reference to the XML iterator for the scope <VRMap>
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		void	extendConfiguration(XMLMicroParser::XMLIterator & VRMIt) {
			QHierarchy<RNode>::Node*	pMapPos = RMap.getRoot();									//  Current position in the hierarchy
			QHierarchy<RNode>::Node*	pSearch = nullptr;											//  Search in the hierarchy
			const char*					pDir = nullptr;												//  Directory name from config
			size_t						DirLen = 0;													//  Length of directory name

			//  Check that we have a valid element scope
			if (VRMIt.isNull()) return;

			//
			//  Iterate over the content of the map building a new node in the hierarchy for each node identified
			//

			for (; !VRMIt.isAtEnd(); VRMIt++) {

				//
				//  Only process "node" elements
				//
				if (VRMIt.isNode(NODE_VRES_NODE)) {

					//  Check if this is the closing of the node, in which case move the current position up a level
					if (VRMIt.isClosing()) pMapPos = pMapPos->getParent();
					else {
						//  Verify that the node has name=, type= and target= attributes, discard the node if not well-formed
						if (VRMIt.hasAttribute(PARM_VRES_NAME) && VRMIt.hasAttribute(PARM_VRES_TYPE) && VRMIt.hasAttribute(PARM_VRES_TGT)) {

							//  Check that the node does not already exist as a child of the current map position
							pDir = VRMIt.getAttribute(PARM_VRES_NAME, DirLen);
							pSearch = pMapPos->getChild();
							while (pSearch != nullptr) {
								if (_memicmp(SPool.getString(pSearch->getName()), pDir, DirLen) == 0) break;
								pSearch = pSearch->getTwin();
							}

							//  If the node already exists then check if it needs to be updated, otherwise create the new node
							if (pSearch != nullptr) {
								if (_memicmp(SPool.getString(pSearch->getMappedName()), pDir, DirLen) != 0) {
									//  Mapped name has changed
									//  1. Replace the string in the pool
									SPool.replaceString(pSearch->getMappedName(), pDir, DirLen);

									//  2.  Create the directory for the new target if it does not already exist
									createTarget(pSearch);

									//  3.  Update the current position to the updated node
									pMapPos = pSearch;
								}
							}
							else {

								//  Build a new node as the child of the current position
								pMapPos = pMapPos->addChild();

								//  Add the name
								pMapPos->setName(SPool.addString(pDir, DirLen));

								//  Add the mapped name
								pDir = VRMIt.getAttribute(PARM_VRES_TGT, DirLen);
								pMapPos->setMappedName(SPool.addString(pDir, DirLen));

								//  Create the target directory
								createTarget(pMapPos);
							}
						}
						else VRMIt++;
					}
				}
			}

			//  Return to caller
			return;
		}

		//  mapFile (1)
		//
		//  This function resolves an application file reference (virtual) to an external file reference (qualified).
		//  
		//
		//  PARAMETERS:
		//
		//		STRREF			-		String reference of the virtual file to be mapped
		//		char *			-		Pointer to the buffer that is to hold the resulting qualified file name
		//		size_t			-		Size of the buffer that holds the qualified file name
		//
		//  RETURNS:
		//
		//		char *			-		Convenience pointer to the qualified file name
		//
		//  NOTES:
		//

		char* mapFile(STRREF VFR, char* szQFile, size_t BfrLen) {
			return mapFile(SPool.getString(VFR), szQFile, BfrLen);
		}


		//  mapFile (2)
		//
		//  This function resolves an application file string (virtual) to an external file reference (qualified).
		//  
		//
		//  PARAMETERS:
		//
		//		const char *	-		Const pointer to the virtual file name
		//		char *			-		Pointer to the buffer that is to hold the resulting qualified file name
		//		size_t			-		Size of the buffer that holds the qualified file name
		//
		//  RETURNS:
		//
		//		char *			-		Convenience pointer to the qualified file name
		//
		//  NOTES:
		//

		char* mapFile(const char* szVFile, char* szQFile, size_t BfrLen) {
			const char*		pSeg = szVFile;																//  Pointer to the next segment to process
			size_t			SegLen = 0;																	//  Next segment length
			size_t			MappedLen = 0;																//  Length of a mapped segment
			size_t			BfrUsed = 0;																//  Bytes used in the output buffer
			QHierarchy<RNode>::Node* pMapNode = nullptr;											//  Current node in the resource hierarchy

			//  Contract Defense
			if (szQFile == nullptr || BfrLen == 0) return nullptr;
			szQFile[0] = '\0';

			//
			//  If the virtual file name is absolute then copy it to the output (if there is room)
			//

			if (isAbsolute(szVFile)) {
				MappedLen = strlen(szVFile);
				if (MappedLen >= BfrLen) return nullptr;
				strcpy_s(szQFile, BfrLen, szVFile);
				return szQFile;
			}

			//
			//  Insert the <root> mapped string into the qualified file name
			//

			MappedLen = SPool.getLength(RString);
			if (MappedLen >= BfrLen) return nullptr;
			strcpy_s(szQFile, BfrLen, SPool.getString(RString));
			BfrUsed = MappedLen;

			//  Set position in the hierarchy
			pMapNode = RMap.getRoot();

			//
			//  Append each segment from the virtual file string onto the mapped file name
			//

			while (pSeg != nullptr) {
				while (*pSeg != '\0') {

					//  Determine the length of the next segment
					SegLen = getSegmentLen(pSeg);

					//  Attempt to locate the segment in the map
					if (findMapping(pSeg, SegLen, pMapNode)) {

						//  If the mapped path is an absolute path then it replaces any path that has been accumulated so far
						if (isAbsolute(SPool.getString(pMapNode->getMappedName()))) BfrUsed = 0;

						//  Check that there is sufficient space in the output buffer for the mapped path plus a delimiter
						MappedLen = SPool.getLength(pMapNode->getMappedName());
						if ((BfrLen - BfrUsed) <= MappedLen) {
							szQFile[0] = '\0';
							return nullptr;
						}

						//  If there is already a partial path then append a delimiter
						if (BfrUsed > 0) szQFile[BfrUsed++] = '/';

						//  Append the mapped path
						strcpy_s(szQFile + BfrUsed, BfrLen - BfrUsed, SPool.getString(pMapNode->getMappedName()));
						BfrUsed += MappedLen;

						//  Point to the next segment
						pSeg += SegLen;
						while (*pSeg == '/' || *pSeg == '\\') pSeg++;
					}
					else {

						//  There is no mapping for current segment - the remainder of the virtual path is appended
						MappedLen = strlen(pSeg);
						if ((BfrLen - BfrUsed) <= MappedLen) {
							szQFile[0] = '\0';
							return nullptr;
						}

						//  If there is already a partial path then append a delimiter
						if (BfrUsed > 0) szQFile[BfrUsed++] = '/';

						//  Append the virtual path
						strcpy_s(szQFile + BfrUsed, BfrLen - BfrUsed, pSeg);
						BfrUsed += MappedLen;

						pSeg += MappedLen;
					}
				}
				pSeg = nullptr;
			}

			//  Return the pointer to the mapped buffer
			return szQFile;
		}

		//  mapTSFile
		//
		//  This function resolves an application Tme Stamped file string (virtual) to an external file reference (qualified).
		//  
		//
		//  PARAMETERS:
		//
		//		const char *	-		Const pointer to the virtual file name
		//		char *			-		Pointer to the buffer that is to hold the resulting qualified file name
		//		size_t			-		Size of the buffer that holds the qualified file name
		//
		//  RETURNS:
		//
		//		char *			-		Convenience pointer to the qualified file name
		//
		//  NOTES:
		//
		//		The virtual file name passed MUST contain a %s at the position to insert the time stamp
		//

		char* mapTSFile(const char* szVFile, char* szQFile, size_t BfrLen) {
			time_t			ttNow = 0;																			//  Submission Timestamp
			struct tm		tmLocalStore = {};																	//  Storage for local time
			struct tm*		ptmLocal = nullptr;																	//  Local time structure
			char			szVirtFile[MAX_PATH + 1] = {};														//  Virtual file name
			char			szFinalVFile[MAX_PATH + 1] = {};													//  Final form of the virtual file name

			//  Insert the formatting string for date/time currently in use into the virtual file name
			sprintf_s(szVirtFile, MAX_PATH, szVFile, DEFAULT_LOGNAME_TIMESTAMP_FMT);

			//  Format the timestamp
			time(&ttNow);
			ptmLocal = localtime_safe(&ttNow, &tmLocalStore);
			strftime(szFinalVFile, MAX_PATH, szVirtFile, ptmLocal);

			//  Return the mapped file name
			return mapFile(szFinalVFile, szQFile, BfrLen);
		}

		//  makeDirectory
		//
		//  This function will ensure the a complete directory path exists for a given virtal or absolute directory
		//  
		//
		//  PARAMETERS:
		//
		//		const char *	-		Const pointer to the virtual or absolute directory name
		//
		//  RETURNS:
		//
		//		bool			-		true if the directory path now exists, otherwise false
		//
		//  NOTES:
		//

		bool		makeDirectory(const char* pVDir) {
			char		szRealDir[MAX_PATH] = {};															//  Real directory name

			//  Map the virtual directory to an absolute directory
			mapFile(pVDir, szRealDir, MAX_PATH);

			//  Create the directory path
			return createDirectoryPath(szRealDir);
		}

		//  loadTextResource
		//
		//  Loads the resource identified by the passed virtual file name. Returns a pointer to the loaded file and the length of the loaded file.
		//  The end of the file is normalised to ensure that the last record is followed by an IRS sequence (lf or cr/lf).
		//  Any empty records at the end of the file are removed.
		//
		//  PARAMETERS:
		//
		//		const vhar*		-		Const pointer to the virtual file name of the resource
		//		size_t&			-		Reference to the variabe that will hold the size of the loaded resource
		//
		//  RETURNS:
		//
		//		char*			-		Pointer to the loaded resource, nullptr if the resource could not be loaded
		//
		//  NOTES:
		//  

		char* loadTextResource(const char* szVRN, size_t & ResLen) {
			size_t		FixLen = 0;													//  Fixed length of the file
			char* pFImg = (char*)loadResource(szVRN, FixLen);				//  Loaded raw text resource
			bool		B2IRS = false;												//  2 byte IRS (cr/lf) in use
			char* pIRS = nullptr;												//  Pointer to an IRS

			//  If failed to load then just return
			ResLen = 0;
			if (pFImg == nullptr) return nullptr;

			//  Determine the IRS in use
			pIRS = strchr(pFImg, SCHAR_LF);
			if (pIRS == nullptr) {
				ResLen = FixLen;
				return pFImg;
			}
			if (pIRS == pFImg) {
				ResLen = FixLen;
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

			//  Return the fettled resource
			ResLen = FixLen;
			return pFImg;
		}

		//  loadResource
		//
		//  Loads the resource identified by the passed virtual file name. Returns a pointer to the loaded file and the length of the loaded file.
		//
		//  PARAMETERS:
		//
		//		const vhar*		-		Const pointer to the virtual file name of the resource
		//		size_t&			-		Reference to the variabe that will hold the size of the loaded resource
		//
		//  RETURNS:
		//
		//		BYTE*			-		Pointer to the loaded resource, nullptr if the resource could not be loaded
		//
		//  NOTES:
		//  

		BYTE* loadResource(const char* szVRN, size_t & ResLen) {
			FILE* pRFile = nullptr;																					//  Handle of the resource file
			errno_t		Result = 0;																							//  Return from fopen_s()
			size_t		FSize = 0;																							//  File size
			size_t		ElementsRead = 0;																					//  Number of alements read
			BYTE* pFImg = nullptr;																					//  Resource image
			char		szFileName[MAX_PATH + 1];																			//  Mapped file name

			//  Clear the returned resource length
			ResLen = 0;

			//  Safety
			if (szVRN == nullptr) return nullptr;
			if (szVRN[0] == '\0') return nullptr;

			//  Map the virtual file name to the real name
			mapFile(szVRN, szFileName, MAX_PATH + 1);
			if (szFileName[0] == '\0') return nullptr;

			//  Open the file
			Result = fopen_s(&pRFile, szFileName, "rb");
			if (Result != 0) return nullptr;
			if (pRFile == nullptr) return nullptr;

			//  Determine the file size
			fseek(pRFile, 0, SEEK_END);
			FSize = ftell(pRFile);
			rewind(pRFile);

			//  Allocate a buffer to hold the file contents
			//  3 additional bytes are allocated one for EOS (\0) and two for a possible cr/lf insert
			pFImg = (BYTE*)malloc(FSize + 3);
			if (pFImg == nullptr) {
				fclose(pRFile);
				return nullptr;
			}

			//  Read the content of the file into memory
			ElementsRead = fread(pFImg, 1, FSize, pRFile);
			fclose(pRFile);
			if (ElementsRead != FSize) {
				free(pFImg);
				return nullptr;
			}

			//  Make the image a well-formed string
			pFImg[ElementsRead] = '\0';

			//  Return the size and pointer to the imafe
			ResLen = FSize;
			return pFImg;
		}

		//  loadCharmedResource
		//
		//  Loads the charmed resource identified by the passed virtual file name. Returns a pointer to the loaded uncharmed file and the length of the loaded file.
		//
		//  PARAMETERS:
		//
		//		const vhar*		-		Const pointer to the virtual file name of the resource
		//		size_t&			-		Reference to the variabe that will hold the size of the loaded resource
		//		int				-		Encryption scheme for the charming
		//		STRREF			-		Reference of the key to use in the secure pool
		//
		//  RETURNS:
		//
		//		BYTE*			-		Pointer to the loaded resource, NULL if the resource could not be loaded
		//
		//  NOTES:
		//
		//	1.		If the resource loaded is NOT a charmed stream then it is returned as is.
		//  

		BYTE* loadCharmedResource(const char* szVRN, size_t & ResLen, int EncScheme, STRREF EncKey) {
			size_t		RSize = 0;																					//  Resource size
			BYTE* pResource = loadResource(szVRN, RSize);														//  Loaded resource

			//  Clear the returned length
			ResLen = 0;

			//  Check that the resource (charmed) was loaded
			if (pResource == nullptr) return nullptr;
			if (RSize < 5) {
				ResLen = RSize;
				return pResource;
			}

			//  Check that the loaded stream is charmed, if not then return the raw resource
			//  'CHx:' - where x is the encryption scheme
			if (pResource[0] != 'C') {
				ResLen = RSize;
				return pResource;
			}

			if (pResource[1] != 'H') {
				ResLen = RSize;
				return pResource;
			}

			if (pResource[3] != ':') {
				ResLen = RSize;
				return pResource;
			}

			//
			//  Decrypt and decompress the charmed stream
			//  The input stream is consumed by the uncharming process
			//

			pResource = uncharmStream(pResource, RSize, EncScheme, EncKey);

			//  Return the uncharmed resource to the caller
			ResLen = RSize;
			return pResource;
		}

		//  isValidResource
		//
		//  this function checks that the passed virtual file name maps to a real resource
		//
		//  PARAMETERS:
		//
		//		const vhar*		-		Const pointer to the virtual file name of the resource
		//
		//  RETURNS:
		//
		//		bool			-		true if resource is valid, otherwise false
		//
		//  NOTES:
		//  

		bool		isValidResource(const char* szVRN) {
			struct stat			FileInfo = {};																//  File stat information
			char				szFileName[MAX_PATH + 1] = {};												//  Mapped file name

			//  Safety
			if (szVRN == nullptr) return false;
			if (szVRN[0] == '\0') return false;

			//  Map the file to the real file space
			mapFile(szVRN, szFileName, MAX_PATH + 1);

			//  Check that the file name was mappable
			if (szFileName[0] == '\0') return false;

			//  Stat the file to check that it exists
			if (stat(szFileName, &FileInfo) == -1) return false;

			//  Return showing that the resource is valid
			return true;
		}

		//  getResourceSize
		//
		//  this function returns the (size_t) size of the named resource
		//
		//  PARAMETERS:
		//
		//		const vhar*		-		Const pointer to the virtual file name of the resource
		//
		//  RETURNS:
		//
		//		size_t			-		Size of the resource in bytes
		//
		//  NOTES:
		//  

		size_t		getResourceSize(const char* szVRN) {
			struct stat			FileInfo = {};																//  File stat information
			char				szFileName[MAX_PATH + 1] = {};												//  Mapped file name

			//  Safety
			if (szVRN == nullptr) return 0;
			if (szVRN[0] == '\0') return 0;

			//  Map the file to the real file space
			mapFile(szVRN, szFileName, MAX_PATH + 1);
			if (szFileName[0] == '\0') return 0;

			//  Stat the file
			if (stat(szFileName, &FileInfo) == -1) return 0;

			//  Return the size
			return size_t(FileInfo.st_size);
		}

		//  getResourceCreateTime
		//
		//  this function returns the (time_t) creation date/time - the time_t is in local time zone
		//
		//  PARAMETERS:
		//
		//		const vhar*		-		Const pointer to the virtual file name of the resource
		//
		//  RETURNS:
		//
		//		time_t			-		Localtime of the creation of the resource
		//
		//  NOTES:
		//  

		time_t	getResourceCreateTime(const char* szVRN) {
			struct stat			FileInfo = {};																//  File stat information
			struct tm			tmLocalStore = {};															//  Storage for local time
			struct tm*			ptmCreate = {};																//  Local time structure
			char				szFileName[MAX_PATH + 1] = {};												//  Mapped file name

			//  Safety
			if (szVRN == nullptr) return FileInfo.st_ctime;
			if (szVRN[0] == '\0') return FileInfo.st_ctime;

			//  Map the file to the real file space
			mapFile(szVRN, szFileName, MAX_PATH + 1);
			if (szFileName[0] == '\0') return FileInfo.st_ctime;

			//  Stat the file
			if (stat(szFileName, &FileInfo) == -1) return FileInfo.st_ctime;

			//  Cycle the time to local time zone
			ptmCreate = localtime_safe(&FileInfo.st_ctime, &tmLocalStore);
			return mktime(ptmCreate);
		}

		//  getResourceModTime
		//
		//  this function returns the (time_t) last modification date/time - the time_t is in local time zone
		//
		//  PARAMETERS:
		//
		//		const vhar*		-		Const pointer to the virtual file name of the resource
		//
		//  RETURNS:
		//
		//		time_t			-		Localtime of the last modification to the resource
		//
		//  NOTES:
		//  

		time_t	getResourceModTime(const char* szVRN) {
			struct stat			FileInfo = {};																//  File stat information
			struct tm			tmLocalStore = {};															//  Storage for local time
			struct tm*			ptmMod = {};																//  Local time structure
			char				szFileName[MAX_PATH + 1] = {};												//  Mapped file name

			//  Safety
			if (szVRN == nullptr) return FileInfo.st_mtime;
			if (szVRN[0] == '\0') return FileInfo.st_mtime;

			//  Map the file to the real file space
			mapFile(szVRN, szFileName, MAX_PATH + 1);
			if (szFileName[0] == '\0') return FileInfo.st_mtime;

			//  Stat the file
			if (stat(szFileName, &FileInfo) == -1) return FileInfo.st_mtime;

			//  Cycle the time to local time zone
			ptmMod = localtime_safe(&FileInfo.st_mtime, &tmLocalStore);
			return mktime(ptmMod);
		}

		//  storeResource
		//
		//  Stores the passed resource in the specified (virtual) file
		//
		//  PARAMETERS:
		//
		//		const vhar*		-		Const pointer to the virtual file name of the resource
		//		BYTE*			-		Pointer to the resource to be stored
		//		size_t			-		Size if the resource to be stored
		//
		//  RETURNS:
		//
		//		bool			-		true if the resource was stored, otherwise false
		//
		//  NOTES:
		//  

		bool	storeResource(const char* szVRN, BYTE * pRes, size_t ResLen) {
			return storeResource(szVRN, pRes, ResLen, true);
		}

		//  storeResourceNoDispose
		//
		//  Stores the passed resource in the specified (virtual) file BUT does not dispose of it after writing.
		//
		//  PARAMETERS:
		//
		//		const vhar*		-		Const pointer to the virtual file name of the resource
		//		BYTE*			-		Pointer to the resource to be stored
		//		size_t			-		Size if the resource to be stored
		//
		//  RETURNS:
		//
		//		bool			-		true if the resource was stored, otherwise false
		//
		//  NOTES:
		//  

		bool	storeResourceNoDispose(const char* szVRN, BYTE * pRes, size_t ResLen) {
			return storeResource(szVRN, pRes, ResLen, false);
		}

		//  storeResource
		//
		//  Stores the passed resource in the specified (virtual) file
		//
		//  PARAMETERS:
		//
		//		const char*		-		Const pointer to the virtual file name of the resource
		//		BYTE*			-		Pointer to the resource to be stored
		//		size_t			-		Size if the resource to be stored
		//		bool			-		true if the resource should be disposed of after write
		//
		//  RETURNS:
		//
		//		bool			-		true if the resource was stored, otherwise false
		//
		//  NOTES:
		//  

		bool	storeResource(const char* szVRN, BYTE * pRes, size_t ResLen, bool Dispose) {
			FILE*		pRFile = nullptr;																					//  Handle of the resource file
			errno_t		Result = 0;																							//  Return from fopen_s()
			size_t		ElementsWrit = 0;																					//  Number of alements written
			char		szFileName[MAX_PATH + 1];																			//  Mapped file name

			//  Safety
			if (szVRN == nullptr) return false;
			if (szVRN[0] == '\0') return false;
			if (pRes == nullptr) return false;

			//  Map the virtual file name to the real name
			mapFile(szVRN, szFileName, MAX_PATH + 1);
			if (szFileName[0] == '\0') return false;

			//  Open the requested file for output
			Result = fopen_s(&pRFile, szFileName, "wb");
			if (Result != 0) return false;

			//  Write the file contents
			if (pRFile != nullptr) {
				if (ResLen > 0) {
					ElementsWrit = fwrite(pRes, 1, ResLen, pRFile);
					if (ElementsWrit != ResLen) {
						fclose(pRFile);
						return false;
					}
				}

				//  Close the output file
				fclose(pRFile);
			}

			//  Consume the resource
			if (Dispose) free(pRes);

			//  Return success
			return true;
		}

		//  storeCharmedResource
		//
		//  Charms and stores the passed resource in the specified (virtual) file
		//
		//  PARAMETERS:
		//
		//		const vhar*		-		Const pointer to the virtual file name of the resource
		//		BYTE*			-		Pointer to the resource to be stored
		//		size_t			-		Size if the resource to be stored
		//		int				-		Encryption scheme to use for the charm
		//		STRREF			-		Reference of the key to use in the secure pool
		//
		//  RETURNS:
		//
		//		bool			-		true if the resource was stored, otherwise false
		//
		//  NOTES:
		//  

		bool	storeCharmedResource(const char* szVRN, BYTE * pRes, size_t ResLen, int EncScheme, STRREF EncKey) {
			BYTE* pCStream = nullptr;														//  Pointer to the charmed stream
			size_t		CSize = 0;																//  Charmed size

			//  Safety
			if (szVRN == nullptr) return false;
			if (szVRN[0] == '\0') return false;
			if (pRes == nullptr) return false;
			if (EncScheme > 1) return false;

			//
			//  Charm the input stream
			//
			CSize = ResLen;
			pCStream = charmStream(pRes, CSize, EncScheme, EncKey);
			if (pCStream == nullptr) return false;

			//
			//  Store the resulting resource
			//

			return storeResource(szVRN, pCStream, CSize);
		}

		//  deleteResource
		//
		//  Deletes the passed resource
		//
		//  PARAMETERS:
		//
		//		const char*		-		Const pointer to the virtual file name of the resource
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void	deleteResource(const char* szVRN) {
			char		szRealFile[MAX_PATH + 1] = {};

			//  Safety
			if (szVRN == nullptr) return;
			if (szVRN[0] == '\0') return;

			mapFile(szVRN, szRealFile, MAX_PATH);
			remove(szRealFile);
			return;
		}

		//  getRoot
		//
		//  Returns a const pointer to the root directory name
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		const char*			-		Pointer to the project root directory string
		//
		//  NOTES:
		//  

		const char* getRoot() {
			return SPool.getString(RString);
		}

		//  dismiss
		//
		//  Clears the hierarchy and releases any acquired resources
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void		dismiss() {
			QHierarchy<RNode>::Node* pNode = RMap.getRoot()->getChild();
			QHierarchy<RNode>::Node* pNextNode = nullptr;

			while (pNode != nullptr) {
				pNextNode = pNode->getTwin();
				delete pNode;
				pNode = pNextNode;
			}

			//  If the MIME type map was loaded then free it
			if (MTMap != nullptr) {
				free(MTMap);
				MTMap = nullptr;
			}

			//  Return to caller
			return;
		}

		//  getMIMEType
		//
		//  This function will locate and return the MIME type of a file given the name (file extension)
		//
		//  PARAMETERS:
		//
		//			char*			-		Const Pointer to the name of the file
		//			char*			-		Pointer to the buffer in which to return the MIME type
		//			size_t			-		Size of the buffer to hold the MIME type
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		void		getMIMEType(const char* FileName, char* MIMEType, size_t BfrLen) {
			const char* pFileExt = nullptr;														//  Pointer to the file type
			char* pMapEntry = nullptr;													//  Pointer to the entry in the mapping file
			char* pScan = nullptr;														//  Scanning pointer

			//  Safety
			if (MIMEType == nullptr) return;
			if (BfrLen == 0) return;
			MIMEType[0] = '\0';
			if (BfrLen > 20) strcpy_s(MIMEType, BfrLen, "application/unknown");

			if (FileName == nullptr) return;
			if (FileName[0] == '\0') return;
			if (strchr(FileName, '.') == nullptr) return;

			//  Find the File Type
			pFileExt = FileName + strlen(FileName);
			while (pFileExt > FileName) {
				if (*pFileExt == '.') break;
				pFileExt--;
			}

			if (*pFileExt != '.') return;

			//  Load the mapping file if not already loaded
			if (MTMap == nullptr) MTMap = loadMIMEMap();

			//  WINDOWS:	Search the registry first
#if (defined(_WIN32) || defined(_WIN64))
			HKEY			hRegKey = NULL;															//  Handle  to the registry key
			DWORD			BfrSize = DWORD(BfrLen);												//  Returned value size

			if (RegOpenKeyEx(HKEY_CLASSES_ROOT, pFileExt, 0, KEY_READ, &hRegKey) == ERROR_SUCCESS) {

				//  Read the Content Type value from the key
				RegQueryValueEx(hRegKey, "Content Type", nullptr, nullptr, (LPBYTE)MIMEType, &BfrSize);

				//  Close the key
				RegCloseKey(hRegKey);
			}

			//  If the value was returned then use that value, otherwise fall through for resolution in the map
			if (_stricmp(MIMEType, "application/unknown") != 0) return;
#endif
			//  Attempt to resolve the MIME type in the mapping file 
			pFileExt++;
			pMapEntry = findMIMEEntry(pFileExt);
			if (pMapEntry == nullptr) return;

			//  Copy the MIME type from the entry into the return buffer
			pScan = pMapEntry;
			while (*pScan != ' ' && *pScan != '\t') pScan++;
			if (size_t(pScan - pMapEntry) >= BfrLen) return;
			memcpy(MIMEType, pMapEntry, pScan - pMapEntry);
			MIMEType[pScan - pMapEntry] = '\0';
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Static Functions                                                                                       *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  isDirectory
		//
		//  Checks that the passed file name exists and is a directory
		//
		//  PARAMETERS:
		//
		//		const char *	-		Const pointer to the file name
		//
		//  RETURNS:
		//
		//		bool			-		true if the file exists and is a directory, otherwise false
		//
		//  NOTES:
		//

		static bool	isDirectory(const char* szDir) {
			struct stat			FInfo = {};																		//  stat structure to hold returned file information

			//  Stat the passed file name
			if (stat(szDir, &FInfo) == -1) return false;

			//  Check if the file is a directory
			if ((FInfo.st_mode & S_IFMT) == S_IFDIR) return true;
			return false;
		}

		//  isAbsolute
		//
		//  Indicates if a file name refers to an asolute (true) location or a relative (false) one
		//
		//  PARAMETERS:
		//
		//		const char *	-		Const pointer to the file name
		//
		//  RETURNS:
		//
		//		bool			-		true if the file name is absolute, false if it is relative
		//
		//  NOTES:
		//

		static bool isAbsolute(const char* szFile) {
			if (szFile == nullptr) return false;
			if (szFile[0] == '\0') return false;
			if (szFile[0] == '/') return true;
			if (szFile[0] == '\\') return true;
			if (strlen(szFile) < 2) return false;
			if (isalpha(szFile[0]) && szFile[1] == ':') return true;
			return false;
		}

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		StringPool&			SPool;													//  Public string pool
		STRREF				RString;												//  Root string reference
		QHierarchy<RNode>	RMap;													//  Resource Map
		char*				MTMap;													//  MIME type file image
		bool				CLFPUsed;												//  First command line parameter is used

#ifdef XY_NEEDS_CRYPTO
		SObjectPool&		SOPool;													//  Secure object pool
#endif

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  charmStream
		//
		//  This function will (if possible) encrypt and compress the passed charmed stream.
		//
		//  PARAMETERS:
		//
		//		BYTE*			-		Pointer to the plaintext stream, consumed during charming
		//		size_t&			-		Reference to the variabe holding the size of the stream, updated to the output size on return
		//		int				-		Encryption scheme for the charming
		//		STRREF			-		Reference of the key to use in the secure pool
		//
		//  RETURNS:
		//
		//		BYTE*			-		Pointer to the charmed stream
		//
		//  NOTES:
		//
		//	1.		If the stream cannot be charmed then it is returned as is.
		//  

		BYTE* charmStream(BYTE * pPStream, size_t & RSize, int EncScheme, STRREF EncKey) {
			BYTE* pCStream = nullptr;											//  Compressed Stream
			size_t		CSize = 0;													//  Size of the compressed stream

			//  Safety
			EncKey = EncKey;
			if (pPStream == nullptr) return nullptr;
			if (RSize == 0) return pPStream;

			//
			//  Compress the passed stream
			//
			CSize = RSize;
			pCStream = compressStream(pPStream, CSize);
			if (pCStream == nullptr) return pPStream;
			if (CSize == 0) return pPStream;

#ifdef  XY_NEEDS_CRYPTO
			//
			//  Encrypt the compressed stream
			//
			pCStream = encryptStream(pCStream, CSize, EncScheme, EncKey);
			if (pCStream == NULL) return NULL;
#endif
			//  Form the charmed stream
			BYTE* pNewCStream = (BYTE*)malloc(CSize + 4);
			if (pNewCStream == nullptr) {
				free(pCStream);
				return nullptr;
			}

			//  Insert the charm signature
			pNewCStream[0] = 'C';
			pNewCStream[1] = 'H';
			pNewCStream[2] = BYTE('1' + EncScheme);
			pNewCStream[3] = ':';

			//  Copy the charmed data into the new stream
			memcpy(pNewCStream + 4, pCStream, CSize);
			CSize += 4;
			free(pCStream);

			//  Return the charmed stream
			RSize = CSize;
			return pNewCStream;
		}

		//  uncharmStream
		//
		//  This function will (if possible) decrypt and decompress the passed charmed stream.
		//
		//  PARAMETERS:
		//
		//		BYTE*			-		Pointer to the charmed stream, consumed during uncharming
		//		size_t&			-		Reference to the variabe holding the size of the stream, updated to the output size on return
		//		int				-		Encryption scheme for the charming
		//		STRREF			-		Reference of the key to use in the secure pool
		//
		//  RETURNS:
		//
		//		BYTE*			-		Pointer to the uncharmed stream
		//
		//  NOTES:
		//
		//	1.		If the stream cannot be uncharmed then it is returned as is.
		//  

		BYTE* uncharmStream(BYTE * pCStream, size_t & RSize, int EncScheme, STRREF EncKey) {

			//  Safety
			EncKey = EncKey;
			if (pCStream == nullptr) return nullptr;
			if (RSize < 5) {
				free(pCStream);
				return nullptr;
			}

			//  Check that the loaded stream is charmed, if not then return the raw resource
			//  'CHx:' - where x is the encryption scheme
			if (pCStream[0] != 'C') {
				free(pCStream);
				return nullptr;
			}
			if (pCStream[1] != 'H') {
				free(pCStream);
				return nullptr;
			}
			if (pCStream[3] != ':') {
				free(pCStream);
				return nullptr;
			}

			//  Override the encryption scheme with the one from the stream
			EncScheme = int(pCStream[2]) - '1';

			//  Remove the charmed stream introducer from the stream
			memmove(pCStream, pCStream + 4, RSize - 4);
			RSize = RSize - 4;

			//
			//  Attempt to decrypt the charmed stream
			//
#ifdef  XY_NEEDS_CRYPTO
			pCStream = decryptStream(pCStream, RSize, EncScheme, EncKey);
			if (pCStream == nullptr) {
				RSize = 0;
				return nullptr;
			}
#endif
			//
			//  Decompress the passed stream
			//

			pCStream = decompressStream(pCStream, RSize);
			if (pCStream == nullptr) {
				RSize = 0;
				return nullptr;
			}

			//  Return the uncharmed resource
			return pCStream;
		}

		//  encryptStream
		//
		//  This function will (if possible) encrypt the passed compressed stream.
		//
		//  PARAMETERS:
		//
		//		BYTE*			-		Pointer to the compressed stream, consumed during charming
		//		size_t&			-		Reference to the variabe holding the size of the stream, updated to the output size on return
		//		int				-		Encryption scheme for the charming
		//		STRREF			-		Reference of the key to use in the secure pool
		//
		//  RETURNS:
		//
		//		BYTE*			-		Pointer to the encrypted stream
		//
		//  NOTES:
		//
		//	1.		If the stream cannot be encrypted then it is returned as is.
		//
#ifdef  XY_NEEDS_CRYPTO
		BYTE* encryptStream(BYTE * pCStream, size_t & RSize, int EncScheme, STRREF EncKey) {
			BYTE* pEStream = nullptr;															//  Encrypted stream
			size_t		ESize = 0;																	//  Encrypted size
			BYTE		IV[8] = { 'x', 'y', 'm', 'o', 'r', 'g', 'I', 'V' };							//  Default initialisation vector

			//  Safety
			if (pCStream == nullptr) return nullptr;
			if (RSize == 0) return pCStream;

			//  If the encryption scheme is 0 then the stream should not be encrypted so return it as is
			if (EncScheme == 0) return pCStream;

			//  Encryption key must be supplied for all other schemes
			if (EncKey == NULLSTRREF) return pCStream;

			//
			//  Encrypt according to the specified scheme
			//

			switch (EncScheme) {

				//  NULL encryption
			case 0:
				return pCStream;

				//  Triple DES CBC
			case 1:
				ESize = RSize;
				pEStream = TripleDESCBC::encrypt(pCStream, ESize, IV, EncKey, SOPool);
				break;

				//  Unknown scheme
			default:
				return pCStream;
			}

			//  Return the encrypted result
			RSize = ESize;
			return pEStream;
		}

		//  decryptStream
		//
		//  This function will (if possible) decrypt the passed charmed stream.
		//
		//  PARAMETERS:
		//
		//		BYTE*			-		Pointer to the encrypted stream, consumed during uncharming
		//		size_t&			-		Reference to the variabe holding the size of the stream, updated to the output size on return
		//		int				-		Encryption scheme for the charming
		//		STRREF			-		Reference of the key to use in the secure pool
		//
		//  RETURNS:
		//
		//		BYTE*			-		Pointer to the decrypted stream
		//
		//  NOTES:
		//
		//	1.		If the stream cannot be decrypted then it is returned as is.
		//

		BYTE* decryptStream(BYTE * pEStream, size_t & RSize, int EncScheme, STRREF EncKey) {
			BYTE* pDStream = nullptr;															//  Decrypted stream
			size_t		DSize = 0;																	//  Decrypted size
			BYTE		IV[8] = { 'x', 'y', 'm', 'o', 'r', 'g', 'I', 'V' };							//  Default initialisation vector

			//  Safety
			if (pEStream == NULL) return nullptr;
			if (RSize == 0) {
				free(pEStream);
				return nullptr;
			}

			//  If the encryption scheme is 0 then the stream is not encrypted so return it as is
			if (EncScheme == 0) {
				free(pEStream);
				return nullptr;
			}

			//  Encryption key must be supplied for all other schemes
			if (EncKey == NULLSTRREF) {
				free(pEStream);
				return pEStream;
			}

			//
			//  Decrypt according to the supplied scheme
			//

			switch (EncScheme) {
				//  NULL encryption
			case 0:
				free(pEStream);
				return nullptr;

				//  Triple DES CBC
			case 1:
				DSize = RSize;
				pDStream = TripleDESCBC::decrypt(pEStream, DSize, IV, EncKey, SOPool);
				break;
			}

			//  Return the decrypted result
			if (DSize == 0) return nullptr;
			RSize = DSize;
			return pDStream;
		}
#endif
		//  compressStream
		//
		//  This function will compress (chimera) the passed plaintext stream.
		//
		//  PARAMETERS:
		//
		//		BYTE*			-		Pointer to the plaintext stream, consumed during charming
		//		size_t&			-		Reference to the variabe holding the size of the stream, updated to the output size on return
		//
		//  RETURNS:
		//
		//		BYTE*			-		Pointer to the deccompressed stream
		//
		//  NOTES:
		//
		//	1.		If the stream cannot be compressed then it is returned as is.
		//

		BYTE* compressStream(BYTE * pPStream, size_t & RSize) {
			BYTE*				pComp = nullptr;																	//  Compressed Stream
			size_t				CompSize = 0;																		//  Size of the compressed stream
			xymorg::Chimera		MyEncoder(std::cout);																//  Chimera Encoder
			xymorg::ByteStream	bsIn(pPStream, RSize);																//  Input stream
			xymorg::ByteStream	bsOut(2 * RSize, 4096);																//  Output stream

			//  Safety
			if (pPStream == nullptr) return nullptr;
			if (RSize == 0) return pPStream;

			//
			//  Configure Chimera to be a pure entropy encoder/decoder
			//
			MyEncoder.permitOptions(0);

			//
			//  Compress the stream
			//
			CompSize = MyEncoder.compress(bsIn, bsOut);
			pComp = bsOut.acquireBuffer(CompSize);

			//  If compression failed return the plaintext stream
			if (RSize == 0) return pPStream;

			//  Free the passed plaintext stream and return the compressed stream
			free(pPStream);
			RSize = CompSize;
			return pComp;
		}

		//  decompressStream
		//
		//  This function will decompress (chimera) the passed charmed stream.
		//
		//  PARAMETERS:
		//
		//		BYTE*			-		Pointer to the compressed stream, consumed during uncharming
		//		size_t&			-		Reference to the variabe holding the size of the stream, updated to the output size on return
		//
		//  RETURNS:
		//
		//		BYTE*			-		Pointer to the deccompressed stream
		//
		//  NOTES:
		//
		//	1.		If the stream cannot be decompressed then it is returned as is.
		//

		BYTE* decompressStream(BYTE * pCStream, size_t & RSize) {
			BYTE*				pDecomp = nullptr;																	//  Decompressed Stream
			size_t				DecompSize = 0;																		//  Size of the decompressed stream
			xymorg::Chimera		MyDecoder(std::cout);																//  Chimera Decoder
			xymorg::ByteStream	bsIn(pCStream, RSize);																//  Input stream
			xymorg::ByteStream	bsOut(4 * RSize, 4096);																//  Output stream

			//  Safety
			if (pCStream == nullptr) return nullptr;
			if (RSize == 0) return pCStream;

			//
			//  Configure Chimera to be a pure entropy encoder/decoder
			//
			MyDecoder.permitOptions(0);

			//
			//  Decompress the stream
			//
			DecompSize = MyDecoder.decompress(bsIn, bsOut);

			//  If the decompression failed then return the native (compressed) stream
			if (DecompSize == 0) return pCStream;

			//  Free the passed compressed stream and return the decompressed stream
			free(pCStream);
			pDecomp = bsOut.acquireBuffer(DecompSize);
			RSize = DecompSize;
			return pDecomp;
		}

		//  SetRoot
		//
		//  Locates and sets the root directory for the project
		//
		//  PARAMETERS:
		//
		//		int				-		Count of application invocation parameters
		//		char*[]			-		Array of pointers to the application invocation parameters
		//
		//  RETURNS:
		//
		//		STRREF			-		String Reference for the project root directory name
		//
		//  NOTES:
		//
		//		The project root is selected in the following sequence, the first qualifying is used
		//	
		//		(1)		-		The first program invocation parameter if it contains a valid directory name
		//		(2)		-		The content of the XY_PROJECT_ROOT environment variable if it contains a valid directory name
		//		(3)		-		The parent directory of the directory from which the application was invoked
		//

		STRREF	setRoot(int argc, char* argv[]) {
			char			szRootDir[MAX_PATH + 1] = {};															//  Buffer to form the root directory name
			char* pSrc = nullptr;																		//  Source pointer
			char* pTgt = szRootDir;																		//  Target pointer
			char* pEnvVar = nullptr;																		//  Environment variable
			errno_t			Error = 0;																				//  Error return code
			int				SepCount = 0;																			//  Count of separators
			char			szXPath[MAX_PATH + 1];																	//  Executable directory

			//  
			//  (1)		-		If available try to use the first of the program invocation parameters
			//

			if (argc >= 2) {
				if (argv[1] != nullptr) {
					pSrc = argv[1];
					//  Empty string is not valid
					if (pSrc[0] != '\0') {
						//  Application switches are not valid
						if (pSrc[0] != '-') {
							//  The length must be acceptable
							if (strlen(pSrc) <= MAX_PATH - 50) {
								//  Copy the provisional root directory to the persistent member, the string is normalised in this process
								while (*pSrc != '\0') {
									if (*pSrc == '\\') *pTgt = '/';
									else *pTgt = *pSrc;
									pSrc++;
									pTgt++;
								}

								//  If the terminal character is a directory separator then lose it
								while (*(pTgt - 1) == '/' && pTgt > szRootDir) pTgt--;
								*pTgt = '\0';

								//  Check that the provisional directory exists and is a directory
								if (isDirectory(szRootDir)) {
									CLFPUsed = true;
									return SPool.addString(szRootDir);
								}

								//  Clear the provisional directory name
								szRootDir[0] = '\0';
							}
						}
					}
				}
			}

			//  
			//  (2)		-		If available try to use the value supplied in the XY_PROJECT_ROOT environment variable
			//

			Error = _dupenv_s(&pEnvVar, nullptr, "XY_PROJECT_ROOT");
			pSrc = pEnvVar;
			if (pSrc != nullptr) {
				if (Error == 0) {
					if (pSrc[0] != '\0') {
						//  The length must be acceptable
						if (strlen(pSrc) <= MAX_PATH - 50) {
							//  Copy the provisional root directory to the persistent member, the string is normalised in this process
							while (*pSrc != '\0') {
								if (*pSrc == '\\') *pTgt = '/';
								else *pTgt = *pSrc;
								pSrc++;
								pTgt++;
							}

							//  If the terminal character is a directory separator then lose it
							while (*(pTgt - 1) == '/' && pTgt > szRootDir) pTgt--;
							*pTgt = '\0';

							//  Check that the provisional directory exists and is a directory
							if (isDirectory(szRootDir)) {
								free(pEnvVar);
								return SPool.addString(szRootDir);
							}

							//  Clear the provisional directory name
							szRootDir[0] = '\0';
						}
					}
				}
				free(pEnvVar);
			}

			//
			//  (3)		-		Use the parent directory of the directory from which the applicaton was invoked
			//
			//  This assumes that a standalone project is in use with executables located in <project root>/<exectables directory>
			//

			getExecutablePath(szXPath, MAX_PATH + 1);
			pSrc = szXPath;

			//  Copy the provisional root directory to the persistent member, the string is normalised in this process
			while (*pSrc != '\0') {
				if (*pSrc == '\\') *pTgt = '/';
				else *pTgt = *pSrc;
				if (*pTgt == '/') SepCount++;
				pSrc++;
				pTgt++;
			}

			//  We need at least 3 separators in the program file path
			if (SepCount >= 3) {
				//  Back the target up to the previous directory separator
				while (*pTgt != '/') pTgt--;
				pTgt--;
				//  Back the target up to the previous directory separator
				while (*pTgt != '/') pTgt--;
				*pTgt = '\0';

				//  Check that the provisional directory exists and is a directory
				if (isDirectory(szRootDir)) return SPool.addString(szRootDir);

				//  Clear the provisional directory name
				szRootDir[0] = '\0';
			}

			//
			//  (4)		-		No project root directory could be determined
			//

			std::cerr << "ERROR: No project root directory is available, logging and other basic functions will not be available to the program." << std::endl;

			//  Return to caller
			return NULLSTRREF;
		}

		//  getExecutablePath
		//
		//  This function will return the executable path of the current program.
		//  If the path cannot be determined then a platform dependent default is returned.
		//
		//  PARAMETERS:
		//
		//		char*			-		Pointer to the buffer to hold the returned file name
		//		size_t			-		Size of the buffer
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		//
		//  PLATFORM DEPENDENT VERSIONS
		//

#if  (defined(_WIN32) || defined(_WIN64))
		//
		//  WINDOWS PLATFORM VERSIONS
		//
		void getExecutablePath(char* szXPath, size_t BfrSize) {
			int			Size = 0;																	//  Returned size

			//  Get the executable path
			Size = GetModuleFileName(NULL, szXPath, DWORD(BfrSize));

			//  If we failed to get one then pass back a dummy file name that will cause a default project root to be used
			if (Size == 0) strcpy_s(szXPath, BfrSize, "c:\\xybase\\Bin\\application.exe");

			//  Return to caller
			return;
		}

#else
		//
		//  UNIX/LINUX PLATFORM VERSIONS
		//
		void getExecutablePath(char* szXPath, size_t BfrSize) {
			int			Size = 0;																	//  Returned size
			char		szLink[64];																	//  Program file name symbolic link

			//  Build the symbolic link
			sprintf_s(szLink, 64, "/proc/%d/exe", getpid());

			//  Read the link
			Size = readlink(szLink, szXPath, BfrSize);

			//  If we failed to get one then pass back a dummy file name that will cause a default project root to be used
			if (Size == 0) strcpy_s(szXPath, BfrSize, "/var/xybase/Bin/application");

			//  Return to caller
			return;
		}

#endif

		//  setupDefaultMap
		//
		//  This function will setup the default resource map nodes.
		//
		//	DEFAULT MAP
		//	-----------
		//  
		//	1.		Config		--->		<root>/Config
		//  2.		Logs		--->		<root>/Logs
		//
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void	setupDefaultMap() {
			STRREF			CFGString = NULLSTRREF;														//  Config string
			STRREF			LogString = NULLSTRREF;														//  Logs string
			QHierarchy<RNode>::Node* pRNode = nullptr;												//  Resource Node

			//  Add the default map node strings to the String Pool
			CFGString = SPool.addString("Config");
			LogString = SPool.addString("Logs");

			//  Add the child nodes to the resource map
			pRNode = RMap.getRoot()->addChild();
			pRNode->setName(CFGString);
			pRNode->setMappedName(CFGString);
			createTarget(pRNode);

			pRNode = RMap.getRoot()->addChild();
			pRNode->setName(LogString);
			pRNode->setMappedName(LogString);
			createTarget(pRNode);

			//  Return to caller
			return;
		}

		//  getSegmentLen
		//
		//  This function will return the length of the next segment in the path
		//
		//
		//  PARAMETERS:
		//
		//		char *			-			Const pointer to the start of the segment
		//
		//  RETURNS:
		//
		//		size_t			-			Length of the segment
		//
		//  NOTES:
		//  

		size_t		getSegmentLen(const char* pSeg) {
			size_t			Length = 0;																//  Evaluated length

			//  Safety
			if (pSeg == nullptr) return 0;

			//
			//  Loop counting characters in the segment until a delimiter is hit
			//

			while (*pSeg != '\0') {
				if (*pSeg == '/') return Length;
				if (*pSeg == '\\') return Length;
				Length++;
				pSeg++;
			}

			//  Return the accumulated length
			return Length;
		}

		//  findMapping
		//
		//  This function will attempt to locate the passed segment in the virtual map
		//
		//
		//  PARAMETERS:
		//
		//		char *			-			Const pointer to the start of the segment
		//		size_t			-			Length of the segment
		//		Node*&			-			Reference to the pointer to the current position in the map
		//
		//  RETURNS:
		//
		//		bool			-			true if a mapping was found, otherwise false
		//
		//  NOTES:
		//
		//		1.	The current position in the map will be updated if a match is found
		//  

		bool		findMapping(const char* pSeg, size_t SegLen, QHierarchy<RNode>::Node * &pCPos) {
			QHierarchy<RNode>::Node*	pMatch = nullptr;														//  Matching Node
			const char*					pVNS = nullptr;															//  Virtual node string

			//  Safety
			if (pSeg == nullptr || SegLen == 0) return false;
			if (pCPos == nullptr) return false;

			//
			//  Loop through each child of the current position in the map attempting to match the segment
			//

			pMatch = pCPos->getChild();

			while (pMatch != nullptr) {

				//  Get the string of the virtual directory at the current node
				pVNS = SPool.getString(pMatch->getName());

				//  Check if we have a match to the segment
				if (SPool.getLength(pMatch->getName()) == SegLen) {
					if (_memicmp(pSeg, pVNS, SegLen) == 0) {
						//  Matched - update the current position and return
						pCPos = pMatch;
						return true;
					}
				}

				//  No match - try the next child in the hierarchy
				pMatch = pMatch->getTwin();
			}

			//  Return showing NO match was found
			return false;
		}

		//  createTarget
		//
		//  This function will attempt to create the target directory if it does not already exist.
		//	The target directory is identified by the Virtual Resource Map node.
		//
		//
		//  PARAMETERS:
		//
		//		QHierarchy<RNode>::Node*			-			Pointer to the target node
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		void	createTarget(QHierarchy<RNode>::Node * pTarget) {
			const char*		pDir = nullptr;															//  Directory name
			char*			pTemp = nullptr;														//  Manipulative pointer
			char			VPath[MAX_PATH + 1] = { 0 };											//  Virtual Path
			char			RPath[MAX_PATH + 1] = { 0 };											//  Real Path

			//  Safety
			if (pTarget == nullptr) return;

			//  Build up a dummy file name on the virtual pathfor the node
			strcpy_s(VPath, MAX_PATH, "X");

			//  Insert a new path name for every node up to the root of the hierarchy
			while (pTarget != RMap.getRoot()) {
				pDir = SPool.getString(pTarget->getName());
				if (pDir != nullptr) {
					if (pDir[0] != '\0') {
						st_inject(VPath, strlen(VPath), VPath, "/", 1);
						st_inject(VPath, strlen(VPath), VPath, pDir, strlen(pDir));
					}
				}

				//  Ascend up the hierarchy
				pTarget = pTarget->getParent();
			}

			//  Obtain the real path for the supplied virtual path
			mapFile(VPath, RPath, MAX_PATH);

			//  Remove the terminal file name from the real path
			pTemp = RPath + strlen(RPath);
			while (*pTemp != '/' && pTemp > RPath) pTemp--;
			*pTemp = '\0';

			//  Build the directory path
			createDirectoryPath(RPath);

			//  Return to caller
			return;
		}

		//  createDirectoryPath
		//
		//  This function will ensure thet the passed directory path exists. It will return false if it was unable to
		//  create the path and the path did not exist.
		//
		//  PARAMETERS:
		//
		//		char*			-		Const pointer to the directory path to be created
		//
		//  RETURNS:
		//
		//		bool			-		true if the path now exits otherwise false
		//
		//  NOTES:
		//  

		//
		//  PLATFORM DEPENDENT VERSIONS
		//

#if  (defined(_WIN32) || defined(_WIN64))
		//
		//  WINDOWS PLATFORM VERSIONS
		//
		bool createDirectoryPath(const char* szDir) {
			DWORD				dwWinError;												//  Last error code
			char* pScan = nullptr;										//  Scanning pointer
			char				szParent[MAX_PATH + 1];									//  Parent directory

			//  Attempt to create the directory
			if (!CreateDirectory(szDir, nullptr)) {
				dwWinError = GetLastError();
				if (dwWinError == ERROR_ALREADY_EXISTS) return true;					//  Directory already exists - everything copacetic

				//  Attempt to create the parent directory - this will recurse to the root
				strcpy_s(szParent, MAX_PATH + 1, szDir);
				pScan = szParent + (strlen(szParent) - 1);
				if (*pScan == '\\' || *pScan == '/') {
					*pScan = '\0';
					pScan--;
				}

				//  Scan back to find the previous separator
				while (*pScan != '\\' && *pScan != '/' && *pScan != ':') pScan--;
				if (*pScan == ':') return false;

				//  Create the parent directory
				*pScan = '\0';
				if (!createDirectoryPath(szParent)) return false;

				//  Retry the directory creation
				if (!CreateDirectory(szDir, nullptr)) {
					dwWinError = GetLastError();
					if (dwWinError == ERROR_ALREADY_EXISTS) return true;					//  Directory already exists - everything copacetic
					return false;
				}
			}

			//  Return to caller
			return true;
		}
#else
		//
		//  UNIX/LINUX PLATFORM VERSIONS
		//
		bool createDirectoryPath(const char* szDir) {
			int			Result = 0;															//  Result from MkDir
			char* pScan = nullptr;													//  Scanning pointer
			char		szParent[MAX_PATH + 1];												//  Parent directory

			//  Create the directory
			Result = mkdir(szDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);					//  Create (or not) the directory

			//  Acceptable outcomes
			if (Result == 0 || errno == EEXIST) return true;

			//  Attempt to create the parent directory - this will recurse to the root
			strcpy_s(szParent, MAX_PATH + 1, szDir);
			pScan = szParent + (strlen(szParent) - 1);
			if (*pScan == '\\' || *pScan == '/') {
				*pScan = '\0';
				pScan--;
			}

			//  Scan back to find the previous separator
			while (*pScan != '\\' && *pScan != '/' && *pScan != ':') pScan--;
			if (pScan == szParent) return false;

			//  Create the parent directory
			*pScan = '\0';

			if (!createDirectoryPath(szParent)) return false;

			//  Retry the creation
			//  Create the directory
			Result = mkdir(szDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);					//  Create (or not) the directory

			//  Acceptable outcomes
			if (Result == 0 || errno == EEXIST) return true;

			//  Return abject failure
			return false;
		}
#endif

		//  loadMIMEMap
		//
		//  This function will load a MIME mapping file (unix/linux: /etc/mime.types), windows: empty file.
		//  The loaded map is extended with some well-known exceptions that are needed 
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		char*			-		Pointer to the map
		//
		//  NOTES:
		//  

		char* loadMIMEMap() {
			char*		pNewMap = nullptr;														//  Pointer to the new map
			size_t		Slop = 8192;															//  Slop additional size to extend the loaded map
			size_t		MapSize = 0;															//  Size of the map

#if (defined(_WIN32) || defined(_WIN64))
			//  WINDOWS PLATFORM - Empty Map with slop
			pNewMap = (char*)malloc(Slop);
			if (pNewMap == nullptr) return nullptr;
			memset(pNewMap, 0, Slop);
			MapSize = Slop;
#else
			//  UNIX/Linux PLATFORM - load /etc/mime.types and extend with slop
			pNewMap = (char*)loadResource("/etc/mime.types", MapSize);
			if (MapSize == 0 || pNewMap == nullptr) {
				pNewMap = (char*)malloc(Slop);
				if (pNewMap == nullptr) return nullptr;
				memset(pNewMap, 0, Slop);
			}
			else {
				char* pXBfr = (char*)realloc(pNewMap, MapSize + Slop);
				if (pXBfr == nullptr) return pNewMap;
				pNewMap = pXBfr;
				memset(pNewMap + MapSize, 0, Slop);
				MapSize += Slop;
			}
#endif
			//
			//  Append any well-known additions to the list
			//

			//  All platforms
			strcat_s(pNewMap, MapSize, "application/notes\t\tnsf\n");

#if (defined(_WIN32) || defined(_WIN64))
			//  Windows
			strcat_s(pNewMap, MapSize, "application/javascript\t\tjs\n");
#else
			//  UNIX/Linux
#endif
			//  Return the pointer to the map
			return pNewMap;
		}

		//  findMIMEEntry
		//
		//  This function will locate the entry in the MIME mapping for the given file extension
		//
		//  PARAMETERS:
		//
		//		char*			-		Pointer to the file extension
		//
		//  RETURNS:
		//
		//		char*			-		Pointer to the map entry, nullptr if not found
		//
		//  NOTES:
		//  

		char* findMIMEEntry(const char* pFX) {
			char* pScan = MTMap;																	//  Scanning pointer

			//  Safety
			if (pFX == nullptr || MTMap == nullptr) return nullptr;
			if (pFX[0] == '\0' || MTMap[0] == '\0') return nullptr;

			//  Loop searching for possible instances of the file extension
			while (pScan != nullptr) {
				pScan = st_stristr(pScan, pFX);
				if (pScan != nullptr) {
					pScan--;
					if (*pScan == ' ' || *pScan == '\t') {
						pScan += (strlen(pFX) + 1);
						if (*pScan == '\n' || *pScan == '\r' || *pScan == ' ' || *pScan == '\t' || *pScan == '#' || *pScan == '\0') {
							//  Locate the start of this entry line
							char* pLastLoc = pScan;
							pScan--;
							while (pScan > MTMap && *pScan != '\n' && *pScan != '\r') pScan--;
							if (pScan > MTMap) pScan++;
							if (*pScan == '#') pScan = pLastLoc;
							else return pScan;
						}
					}
					else pScan += 2;
				}
			}

			//  NOT Found
			return nullptr;
		}
		};
	}
