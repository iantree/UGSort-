#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       ROCache.h																							*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.0.0	(Build: 01)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the ROCache class. The class is the base for all 					*
//* Read Only cache implementations.																				*
//* The class defines dummy functions for the unused virtual functions in the base Cache class.						*
//*																													*
//*	USAGE:																											*
//*																													*
//*																													*
//*	NOTES:																											*
//*																													*
//*		1.		The pointer to the cached record that is returned is ONLY valid until the next call to				*
//*				get or put a cached record.																			*
//*				It is the resposiblity of the caller to guard against use-after-free bugs using the					*
//*				returned pointer.																					*
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
	//  ROCache Class Definition
	//

	class ROCache : public Cache {
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
		//  Constructs a new Synchronous Read Only Cache (base) with the given attributes
		//
		//  PARAMETERS:
		//
		//		SWITCHES			-		Cache configuration options
		//		size_t				-		Budget size (Kb)
		//		Dispatcher&			-		Reference to the creation MP dispatcher
		//
		//  RETURNS:
		//
		//  NOTES:
		//
		//		1.	Extending classes MUST invoke this constructor
		//

		ROCache(SWITCHES NewCfg, size_t NewBudget, Dispatcher& MP) : Cache(NewCfg, NewBudget, MP) {

			//  Return to caller
			return;
		}
#else
		//  Constructor 
		//
		//  Constructs a new Synchronous Read Only Cache (base) with the given attributes
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
		//		1.	Extending classes MUST invoke this constructor
		//

		ROCache(SWITCHES NewCfg, size_t NewBudget) : Cache(NewCfg, NewBudget) {

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
		//  Destroys the SROCache 
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		virtual ~ROCache() {
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

		//
		//  The following functions define the interface that MUST be implemented in extending classes to link
		//  the cache to the underlying storage.
		//

#ifdef XY_NEEDS_MP
		//  putCachedRecord
		//
		//  This function is used by the cache to "Write Through" writing a dirty (updated) cache entry into the backing store.
		//
		//  PARAMETERS:
		//
		//		char*		-		Const pointer the Key of the record to be written
		//		BYTE*		-		Const pointer to the content of the record to be written
		//		size_t		-		Size of the record to be written
		//		Dispatcher&	-		Reference to the calling thread dispatcher
		//
		//  RETURNS:
		//
		//		bool		-		true for a successful write, false will cause the cache to become incoherent (unuseable)
		//
		//  NOTES:
		//

		virtual bool	putCachedRecord(const char* Key, const BYTE* Rec, size_t RecLen, Dispatcher& MP) {
			return true;
		}
#else
		//  putCachedRecord
		//
		//  This function is used by the cache to "Write Through" writing a dirty (updated) cache entry into the backing store.
		//
		//  PARAMETERS:
		//
		//		char*		-		Const pointer the Key of the record to be written
		//		BYTE*		-		Const pointer to the content of the record to be written
		//		size_t		-		Size of the record to be written
		//
		//  RETURNS:
		//
		//		bool		-		true for a successful write, false will cause the cache to become incoherent (unuseable)
		//
		//  NOTES:
		//

		virtual bool	putCachedRecord(const char* Key, const BYTE* Rec, size_t RecLen) {
			return true;
		}
#endif

	};

}

