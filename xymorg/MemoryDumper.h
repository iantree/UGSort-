#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       MemoryDumper.h																					*
//*   Suite:      Xymorg Integration																				*
//*   Version:    1.6.0	  Build:  01																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2007 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*	MemoryDumper.h																									*
//*																													*
//*	This header file contains the class definition for the MemoryDumper class. This class provides static			*
//* functions for dumping blocks of memory in various formats.														*
//*																													*
//*	NOTES:																											*
//*																													*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 - 07/03/2014   -  Initial version																			*
//*																													*
//*******************************************************************************************************************

//  Include xymorg headers
#include	"LPBHdrs.h"																			//  Language and Platform base headers
#include	"types.h"																			//  xymorg type definitions
#include	"consts.h"																			//  xymorg constant definitions

namespace xymorg {

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Type Definitions                                                                                              *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Constant Definitions                                                                                          *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Local Definitions                                                                                             *
	//*                                                                                                                 *
	//*******************************************************************************************************************

#define		START_DUMP				"+---- Start of Dump: %s -------------- %lu (0x%.4lx) bytes @%p --------------"
#define		END_DUMP				"+---- End of Dump:   %s -------------- %lu (0x%.4lx) bytes @%p --------------"
#define		DUMP_LINE				"%04lx: %s  %s :%s %s"

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Macros                                                                                                        *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Structures                                                                                                    *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Class Definitions                                                                                             *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   MemoryDumper Class                                                                                            *
	//*																													*
	//*	  Provides static functions for dumping blocks of memory														*
	//*                                                                                                                 *
	//*******************************************************************************************************************

	class MemoryDumper {
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

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor                                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

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

		//  Memory dumping functions
		//  dumpMemory
		//
		//  This is a logging function that can be called to provide a Dump of a memory block in the log.
		//
		//  Parameters
		//
		//	1 - void *			-	Const pointer to the area of memory that is to be dumped
		//  2 - size_t			-	Length of the memory area to be dumped
		//  3 - char *			-	String that describes the memory are being dumped
		//  4 - std::ostream&	-	Reference to the output stream to produce the dump to
		//
		//  Returns
		//
		//  API Use Notes:
		//
		//

		static void dumpMemory(const void* lpMem, size_t iLen, const char* szSymbol, std::ostream& os) {
			size_t			iResidue;															//  Count of bytes left to dump
			size_t			iIndex;																//  Index counter
			size_t			iOffset = 0;														//  Offset in the memory buffer
			char			szMsg[512];															//  Generic message buffer
			char			szHexSeg1[30];														//  First segment of Hexadecimal
			char			szHexSeg2[30];														//  Second segment of Hexadecimal
			char			szRawSeg1[10];														//  First segment of Ascii data
			char			szRawSeg2[10];														//  Second segment of Ascii data
			char			szTemp[20];															//  Temporary format area
			char			cLocalBuffer[20];													//  Local buffer
			char*			BfrPtr;																//  Pointer into the memory buffer
			BYTE*			LocalBfrPtr;														//  Pointer into the local buffer

			//  Show start of the dump message
			sprintf_s(szMsg, 512, START_DUMP, szSymbol, iLen, iLen, lpMem);
			while (strlen(szMsg) < 75)
			{
				strcat_s(szMsg, 512, "-");
			}
			os << szMsg << std::endl;

			//  Dump each of the lines of the body of the dump
			iResidue = iLen;
			BfrPtr = (char*)lpMem;
			while (iResidue > 0) {
				szHexSeg1[0] = '\0';
				szHexSeg2[0] = '\0';
				szRawSeg1[8] = '\0';
				szRawSeg2[8] = '\0';
				//  Copy the data to the local buffer
				if (iResidue >= 16)
				{
					memcpy(cLocalBuffer, BfrPtr, 16);
				}
				else
				{
					memset(cLocalBuffer, 0, 16);
					memcpy(cLocalBuffer, BfrPtr, iResidue);
				}
				LocalBfrPtr = (BYTE*)cLocalBuffer;
				for (iIndex = 0; iIndex < 16; iIndex++)
				{
					sprintf_s(szTemp, 20, "%02X ", (unsigned short)*LocalBfrPtr);
					if (iIndex < 8)
					{
						strcat_s(szHexSeg1, 30, szTemp);
						if ((*LocalBfrPtr < 33) || (*LocalBfrPtr > 126))
						{
							szRawSeg1[iIndex] = '.';
						}
						else
						{
							szRawSeg1[iIndex] = *LocalBfrPtr;
						}
					}
					else
					{
						strcat_s(szHexSeg2, 30, szTemp);
						if ((*LocalBfrPtr < 33) || (*LocalBfrPtr > 126))
						{
							szRawSeg2[iIndex - 8] = '.';
						}
						else
						{
							szRawSeg2[iIndex - 8] = *LocalBfrPtr;
						}
					}
					LocalBfrPtr++;
				}
				//  Format the dump line
				sprintf_s(szMsg, 512, DUMP_LINE, iOffset, szHexSeg1, szHexSeg2, szRawSeg1, szRawSeg2);
				os << szMsg << std::endl;

				//  Position to the next chunk of the memory block
				iOffset = iOffset + 16;
				if (iResidue >= 16) iResidue = iResidue - 16;
				else iResidue = 0;
				BfrPtr = BfrPtr + 16;
			}

			//  Show the end of the dump in the log
			sprintf_s(szMsg, 512, END_DUMP, szSymbol, iLen, iLen, lpMem);
			while (strlen(szMsg) < 75)
			{
				strcat_s(szMsg, 512, "-");
			}
			os << szMsg << std::endl;

			//  Return to caller
			return;
		}

		//  dumpMemoryAt
		//
		//  This is a logging function that can be called to provide a Dump of a memory block in the log.
		//  This function shows the offset within the context of the dump
		//
		//  Parameters
		//
		//	1 - void *			-  Const pointer to the area of memory that is to be dumped
		//  2 - size_t			-  Offset within the context of the block of memory to be dumped
		//  3 - size_t				-  Length of the memory area to be dumped
		//  4 - char *			-  String that describes the memory are being dumped
		//  5 - std::ostream&	-	Reference to the output stream to produce the dump to
		//
		//  Returns
		//
		//  API Use Notes:
		//
		//

		static void dumpMemoryAt(const void* lpMem, size_t iOff, size_t iLen, const char* szSymbol, std::ostream& os) {
			size_t			iResidue;															//  Count of bytes left to dump
			size_t			iIndex;																//  Index counter
			size_t			iOffset = 0;														//  Offset in the memory buffer
			char			szMsg[512];															//  Generic message buffer
			char			szHexSeg1[30];														//  First segment of Hexadecimal
			char			szHexSeg2[30];														//  Second segment of Hexadecimal
			char			szRawSeg1[10];														//  First segment of Ascii data
			char			szRawSeg2[10];														//  Second segment of Ascii data
			char			szTemp[20];															//  Temporary format area
			char			cLocalBuffer[20];													//  Local buffer
			char*			BfrPtr;																//  Pointer into the memory buffer
			BYTE*			LocalBfrPtr;														//  Pointer into the local buffer

			//  Show start of the dump message
			sprintf_s(szMsg, 512, START_DUMP, szSymbol, iLen, iLen, lpMem);
			while (strlen(szMsg) < 75)
			{
				strcat_s(szMsg, 512, "-");
			}
			os << szMsg << std::endl;

			//  Dump each of the lines of the body of the dump
			iResidue = iLen;
			BfrPtr = (char*)lpMem + iOff;
			while (iResidue > 0) {
				szHexSeg1[0] = '\0';
				szHexSeg2[0] = '\0';
				szRawSeg1[8] = '\0';
				szRawSeg2[8] = '\0';
				//  Copy the data to the local buffer
				if (iResidue >= 16)
				{
					memcpy(cLocalBuffer, BfrPtr, 16);
				}
				else
				{
					memset(cLocalBuffer, 0, 16);
					memcpy(cLocalBuffer, BfrPtr, iResidue);
				}
				LocalBfrPtr = (BYTE*)cLocalBuffer;
				for (iIndex = 0; iIndex < 16; iIndex++)
				{
					sprintf_s(szTemp, 20, "%02X ", (unsigned short)*LocalBfrPtr);
					if (iIndex < 8)
					{
						strcat_s(szHexSeg1, 30, szTemp);
						if ((*LocalBfrPtr < 33) || (*LocalBfrPtr > 126))
						{
							szRawSeg1[iIndex] = '.';
						}
						else
						{
							szRawSeg1[iIndex] = *LocalBfrPtr;
						}
					}
					else
					{
						strcat_s(szHexSeg2, 30, szTemp);
						if ((*LocalBfrPtr < 33) || (*LocalBfrPtr > 126))
						{
							szRawSeg2[iIndex - 8] = '.';
						}
						else
						{
							szRawSeg2[iIndex - 8] = *LocalBfrPtr;
						}
					}
					LocalBfrPtr++;
				}
				//  Format the dump line
				sprintf_s(szMsg, 512, DUMP_LINE, iOffset + iOff, szHexSeg1, szHexSeg2, szRawSeg1, szRawSeg2);
				os << szMsg << std::endl;

				//  Position to the next chunk of the memory block
				iOffset = iOffset + 16;
				if (iResidue >= 16) iResidue = iResidue - 16;
				else iResidue = 0;
				BfrPtr = BfrPtr + 16;
			}

			//  Show the end of the dump in the log
			sprintf_s(szMsg, 512, END_DUMP, szSymbol, iLen, iLen, lpMem);
			while (strlen(szMsg) < 75)
			{
				strcat_s(szMsg, 512, "-");
			}
			os << szMsg << std::endl;

			//  Return to caller
			return;
		}

	};
}
