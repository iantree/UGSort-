#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       xymorg.h																							*
//*   Suite:      xymorg Integration																				*
//*   Version:    3.5.2	(Build: 43) - Dev Build:  XDB-051															*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file vectors the appropriate xymorg header includes according to the sub-system requirements of		*
//* the application.																								*
//*																													*
//*	USAGE:																											*
//*																													*
//*		Define the subsystem requirements (XY_NEEDS_XXX) before including this header.								*
//*		Where XXX identifies the particular sub-system or facility that is to be included.							*
//*																													*
//*		IDENTIFIER				SUBSYSTEM/FACILITY																	*
//*		----------				------------------																	*
//*																													*
//*			MP					Multi-Programming (threaded) sub-system												*
//*			DS					Directory Scanning facility															*
//*			IMG					Image Processing components															*
//*			GRAPHS				Graphing components																	*
//*			CRYPTO				Cryptographic components															*
//*			WEBUI				Web UI components including Wezzer													*
//*			NETIO				Network I/O																			*
//*			TLS					Secure Network I/O (e.g. HTTPS)														*
//*			TEXT				Text Rendering Engine																*
//*																													*
//*	NOTES:																											*
//*																													*
//*	1.		This is a minimal subset clone of xymorg supporting NO extensions.										*																										*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	3.5.0 -	02/03/2020	-	Version rebased to origin 3.5															*
//* 3.5.1 -	12/05/2020	-	Minimal Subset generated																*	
//* 3.5.2 -	20/08/2023	-	Minimal Subset regenerated																*	
//*																													*
//*******************************************************************************************************************/

//
//  Include core xymorg headers
//

#include	"LPBHdrs.h"																		//  Language and Platform base headers
#include	"types.h"																		//  xymorg type definitions
#include	"consts.h"																		//  xymorg constant definitions
#include	"AppConfig.h"																	//  xymorg Application Configuration Singleton

