#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       WTCache.h																							*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.0.0	(Build: 01)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the WTCache class. The class is the base for all 					*
//* Write through cache implementations.																			*
//*																													*
//*	USAGE:																											*
//*																													*
//*																													*
//*	NOTES:																											*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 -		02/12/2017	-	Initial Release																		*
//*																													*
//*******************************************************************************************************************/

//
//  Include core xymorg headers
//

#include	"LPBHdrs.h"																		//  Language and Platform base headers
#include	"types.h"																		//  xymorg type definitions
#include	"consts.h"																		//  xymorg constant definitions
#include	"StringPool.h"																	//  String Pool
#ifdef XY_NEEDS_MP
#include	"MP/Cache.h"																	//  (multi threaded) Cache base class
#else
#include	"Cache.h"																		//  (single threaded) Cache base class
#endif

//
//  All components are defined within the xymorg namespace 
//
namespace xymorg {

	//
	//  WTCache Class Definition
	//

	class WTCache : public Cache {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants		                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************


		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors			                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

#ifdef XY_NEEDS_MP
		//  Constructor 
		//
		//  Constructs a new Write Through Cache (base) with the given attributes
		//
		//  PARAMETERS:
		//
		//		SWITCHES			-		Cache configuration options
		//		size_t				-		Budget size (Kb)
		//		Dispatcher&			-		Reference to the creating thread MP dispatcher
		//
		//  RETURNS:
		//
		//  NOTES:
		//
		//	1.	Extending classes MUST invoke this constructor
		//

		WTCache(SWITCHES NewCfg, size_t NewBudget, xymorg::Dispatcher& MP) : Cache(NewCfg, NewBudget, MP) {

			//  Return to caller
			return;
		}
#else
		//  Constructor 
		//
		//  Constructs a new Write Through Cache (base) with the given attributes
		//
		//  PARAMETERS:
		//
		//		SWITCHES			-		Cache configuration options
		//		size_t				-		Budget size (Kb)
		//
		//  RETURNS:
		//
		//  NOTES:
		//
		//	1.	Extending classes MUST invoke this constructor
		//

		WTCache(SWITCHES NewCfg, size_t NewBudget) : Cache(NewCfg, NewBudget) {

			//  Return to caller
			return;
		}
#endif

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor			                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Destructor
		//
		//  Destroys the WTCache 
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		virtual ~WTCache() {
#ifdef XY_NEEDS_MP
			//  Invoke dismiss
			dismiss(CMP);
#else
			//  Invoke dismiss
			dismiss();
#endif
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

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

	};

}
