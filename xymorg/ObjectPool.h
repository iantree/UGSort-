#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       ObjectPool.h																						*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.0.0	(Build: 01)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the ObjectPool class. The class provides storege and reference		*
//* for collections of arbitrary objects.																			*
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
//*																													*
//*******************************************************************************************************************/

//
//  Include core xymorg headers
//

#include	"LPBHdrs.h"																		//  Language and Platform base headers
#include	"types.h"																		//  xymorg type definitions
#include	"consts.h"																		//  xymorg constant definitions

//
//  All components are defined within the xymorg namespace
//
namespace xymorg {

	//
	//  StringPool Class Definition
	//

	class ObjectPool {

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Constants		                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

	private:

		static const int DefaultNumObjects = 100;													//  Default number of objects
		static const int DefaultPoolSize = 4096;													//  Default string pool size
		static const OBJREF EmptySlot = 0xFFFFFFFF;													//  Empty slot value in the SRT

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Structures	                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		typedef struct ORef {
			size_t		OOff;																		//  Offset of this object in the pool
			size_t		OLen;																		//  Length of the object
		} SORef;

	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors			                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Default Constructor 
		//
		//  Constructs a new ObjectPool with the default size settings
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		ObjectPool() : ORT(nullptr), OP(nullptr) {

			//  Initialise the pool for the default capacity
			if (!initialisePool(DefaultNumObjects, DefaultPoolSize)) {
				//  Clear the pool to the default state
				clearPool();
			}

			//  Return to caller
			return;
		}

		//  Constructor 
		//
		//  Constructs a new ObjectPool with the specified size settings
		//
		//  PARAMETERS:
		//
		//		size_t			-		Capacity of the pool (number of strings)
		//		size_t			-		Capacity of the pool (object storage space)
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		ObjectPool(size_t RNS, size_t RSPS) : ORT(nullptr), OP(nullptr) {

			//  Safety checks on capacities
			if (RNS == 0) RNS = DefaultNumObjects;
			if (RSPS == 0) RSPS = DefaultPoolSize;

			//  Initialise the pool for the spacified capacity
			if (!initialisePool(RNS, RSPS)) {
				//  Clear the pool to the default state
				clearPool();
			}

			//  Return to caller
			return;
		}

		//  Copy Constructor 
		//
		//  Constructs a new ObjectPool with a copy of the underlying storage from the source
		//
		//  PARAMETERS:
		//
		//		StringPool&			-		Const reference to the source StringPool
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		ObjectPool(ObjectPool& Src) : ORT(nullptr), OP(nullptr) {

			//  Initialise the pool using the capacity from the source
			if (!initialisePool(Src.ORTCap, Src.OPCap)) {
				clearPool();
				return;
			}

			//  Unbutton the content of the source pool
			Src.unbuttonPool();

			//  Copy the content of the ORT & OP from the source
			if (ORT != nullptr) memcpy(ORT, Src.ORT, Src.ORTCap * sizeof(SORef));
			if (OP != nullptr) memcpy(OP, Src.OP, Src.OPCap);

			//  Rebutton the source pool
			Src.buttonPool();

			//  Button the local pool
			buttonPool();

			//  Copy the pool information from the source
			ORTCap = Src.ORTCap;
			ORTEnts = Src.ORTEnts;
			ORTHiWater = Src.ORTHiWater;
			OPCap = Src.OPCap;
			OPUsed = Src.OPUsed;

			//  Return to caller
			return;
		}

		//  Move Constructor 
		//
		//  Constructs a new ObjectPool acquiring control of the underlying storage from the source
		//
		//  PARAMETERS:
		//
		//		StringPool&&			-		Reference to the source StringPool
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		ObjectPool(ObjectPool&& Src) noexcept : ORT(nullptr), OP(nullptr) {

			//  Unbutton the source pool
			Src.unbuttonPool();

			//  Acquire the underlying pool storage from the source
			ORT = Src.ORT;
			ORTCap = Src.ORTCap;
			ORTEnts = Src.ORTEnts;
			ORTHiWater = Src.ORTHiWater;

			OP = Src.OP;
			OPCap = Src.OPCap;
			OPUsed = Src.OPUsed;

			//  button this pool
			buttonPool();

			//  Disconnect the underlying storage from the source
			Src.ORT = nullptr;
			Src.ORTCap = 0;
			Src.ORTEnts = 0;
			Src.ORTHiWater = 0;

			Src.OP = nullptr;
			Src.OPCap = 0;
			Src.OPUsed = 0;

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
		//  Destroys the ObjectPool object releasing any underlying storage.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		~ObjectPool() {

			//  Clear the pool to the default state
			clearPool();

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

		//  addObject
		//
		//  Adds the passed object to the pool as the next available entry
		//
		//  PARAMETERS:
		//
		//		void*			-		Pointer to the object to be added
		//		size_t			-		Size of the object in bytes
		//
		//  RETURNS:
		//
		//		OBJREF			-		Reference to the object, NULLOBJREF if the object could not be added
		//
		//  NOTES:
		//

		OBJREF		addObject(const void* pNewObject, size_t NewObjLen) {
			OBJREF		NewRef = NULLOBJREF;												//  Object reference for the new string

			//  Safety Checks
			if (pNewObject == nullptr) NewObjLen = 0;

			//  Check (and adjust if necessary) the capacity of the pool
			if (!checkCapacity(NewObjLen)) return NULLSTRREF;

			//  Locate the Object Reference to be used
			NewRef = locateFreeObjectRef();

			//  If we acquired a valid reference then populate it with the new object
			if (NewRef != NULLSTRREF) {

				//
				//  Set the offset of the new object in the reference table
				//
				ORT[NewRef - 1].OOff = OPUsed;
				ORT[NewRef - 1].OLen = NewObjLen;

				//
				//  If we have a non-null object then add it to the pool
				//
				if (pNewObject != nullptr && NewObjLen > 0) {
					unbuttonPool();
					memcpy(OP + OPUsed, pNewObject, NewObjLen);
					OPUsed += NewObjLen;
					buttonPool();
				}

				//
				//  Update the pool information
				//
				ORTEnts++;
				if (NewRef > ORTHiWater) ORTHiWater++;
			}

			//  Return the reference
			return NewRef;
		}

		//  getObject
		//
		//  Returns a copy of the requested object from the pool.
		//
		//  PARAMETERS:
		//
		//		OBJREF			-		Reference to the object.
		//		void*			-		Pointer to the buffer in which to return the object
		//		size_t&			-		Reference to the buffer size (in), object size (out)
		//
		//  RETURNS:
		//
		//		bool			-		true if the object was retrieved, otherwise false
		//
		//  NOTES:
		//
		//		1.	NULLOBJREF returns an empty buffer
		//		2.	Object values must be updated using the updateObject() functions
		//

		bool		getObject(OBJREF Ref, void* pBuffer, size_t& BfrLen) {

			//  Clear the return buffer
			if (pBuffer == nullptr || BfrLen == 0) return false;
			memset(pBuffer, 0, BfrLen);

			//  Any invalid reference returns a pointer to a dummy empty string
			if (Ref == NULLOBJREF) {
				BfrLen = 0;
				return false;
			}
			if (Ref > ORTHiWater + 1) {
				BfrLen = 0;
				return false;
			}

			//  Check for an unallocated object
			if (ORT[Ref - 1].OOff == EmptySlot) {
				BfrLen = 0;
				return false;
			}

			//  Check that the buffer is large enough to hold the copy of the stored object
			if (BfrLen < ORT[Ref - 1].OLen) {
				BfrLen = 0;
				return false;
			}

			//  Return copy of the object in the pool
			unbuttonPool();
			memcpy(pBuffer, OP + ORT[Ref - 1].OOff, ORT[Ref - 1].OLen);
			buttonPool();
			BfrLen = ORT[Ref - 1].OLen;
			return true;
		}

		//  getLength
		//
		//  Returns the length of a referenced object.
		//
		//  PARAMETERS:
		//
		//		OBJREF			-		Reference to the object. 
		//
		//  RETURNS:
		//
		//		size_t			-		Length of the referenced object
		//
		//  NOTES:
		//
		//

		size_t		getLength(OBJREF Ref) {

			//  Any invalid reference returns a pointer to a dummy empty string
			if (Ref == NULLOBJREF) return 0;
			if (Ref > ORTHiWater + 1) return 0;
			if (ORT[Ref - 1].OOff == EmptySlot) return 0;

			//  Return the length
			return ORT[Ref - 1].OLen;
		}

		//  deleteObject
		//
		//  Removes the referenced object from the pool and frees the reference for reuse.
		//
		//  PARAMETERS:
		//
		//		OBJREF			-		Reference to the object to be deleted.
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		void		deleteObject(OBJREF Ref) {

			//  Any invalid reference returns with no further action
			if (Ref == NULLOBJREF) return;
			if (Ref > ORTHiWater + 1) return;
			if (ORT[Ref - 1].OOff == EmptySlot) return;

			//
			//  Compute the length to be excised from the pool
			//
			size_t		SnipLen = ORT[Ref - 1].OLen;

			//  Remove the Object from the pool
			unbuttonPool();
			if (((OPUsed - ORT[Ref - 1].OOff) - SnipLen) > 0) memmove(OP + ORT[Ref - 1].OOff, OP + ORT[Ref - 1].OOff + SnipLen, (OPUsed - ORT[Ref - 1].OOff) - SnipLen);

			//  Clear the residual space at the end of the pool
			memset((OP + OPUsed) - SnipLen, 0, SnipLen);
			buttonPool();

			//  Update the pool information
			OPUsed = OPUsed - SnipLen;

			//
			//  Update all object references in the table that lie above the current reference offset
			//
			for (size_t ORX = 0; ORX < ORTHiWater; ORX++) {
				if (ORT[ORX].OOff != EmptySlot && ORT[ORX].OOff > ORT[Ref - 1].OOff) ORT[ORX].OOff = ORT[ORX].OOff - SnipLen;
			}

			//  Remove the object reference from the table
			ORT[Ref - 1].OOff = EmptySlot;
			ORT[Ref - 1].OLen = EmptySlot;
			ORTEnts--;
			if (Ref == (ORTHiWater + 1)) ORTHiWater--;

			//  Return to caller
			return;
		}

		//  replaceObject
		//
		//  Replaces the content of the referenced object with the passed object value
		//
		//  PARAMETERS:
		//
		//		OBJREF			-		Reference to the object to be replaced.
		//		void*			-		Pointer to the the replacement value
		//		size_t			-		Size of the replacement object value
		//
		//  RETURNS:
		//
		//		OBJREF			-		Reference to the object, NULLOBJREF if the new string could not be added
		//
		//  NOTES:
		//
		//		1.	The reference will NOT change unless the addition fails
		//

		OBJREF		replaceObject(OBJREF Ref, const void* pNewObject, size_t NewObjLen) {
			//  Safety Checks
			if (pNewObject == nullptr) NewObjLen = 0;
			if (Ref == NULLOBJREF) return addObject(pNewObject, NewObjLen);

			//  Check to see if we can do an update in place
			if (NewObjLen == ORT[Ref - 1].OLen) {
				if (NewObjLen == 0) return Ref;

				//  Update the object in-place
				unbuttonPool();
				memcpy(OP + ORT[Ref - 1].OOff, pNewObject, NewObjLen);
				buttonPool();
				return Ref;
			}

			//  Delete the existing object from the pool
			deleteObject(Ref);

			//  Check (and adjust if necessary) the capacity of the pool
			if (!checkCapacity(NewObjLen)) return NULLOBJREF;

			//
			//  Set the offset of the new object in the reference table
			//
			ORT[Ref - 1].OOff = OPUsed;
			ORT[Ref - 1].OLen = NewObjLen;
			ORTEnts++;

			//
			//  If we have a non-null object then add it to the pool
			//
			if (pNewObject != nullptr && NewObjLen > 0) {
				unbuttonPool();
				memcpy(OP + OPUsed, pNewObject, NewObjLen);
				buttonPool();
				OPUsed += NewObjLen;
			}

			//  Return the reference
			return Ref;
		}

		//  dismiss
		//
		//  Empties the string pool and releases the underlying storage
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void		dismiss() {

			//  Clear the pool to the default state
			clearPool();

			//  Return to caller
			return;
		}

	protected:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Object Reference Table

		size_t			ORTCap;																//  Reference table capacity
		size_t			ORTHiWater;															//  Number of object reference entries (high watermark)
		size_t			ORTEnts;															//  Number of object reference entries (in use)
		SORef*			ORT;																//  Pointer to the object Reference Table

		//  Object Pool
		size_t			OPCap;																//  Object Pool capacity (bytes)
		size_t			OPUsed;																//  Object Pool Used (bytes)
		BYTE*			OP;																	//  Object Pool

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

		//  clearPool 
		//
		//  Clears the content of the pool to the default unallocated state
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		void	clearPool() {

			//  Clear eny existing pool content
			if (ORT != nullptr) free(ORT);
			ORT = nullptr;
			ORTCap = 0;
			ORTHiWater = 0;
			ORTEnts = 0;
			if (OP != nullptr) free(OP);
			OP = nullptr;
			OPCap = 0;
			OPUsed = 0;

			//  Return to caller
			return;
		}

		//  initialisePool 
		//
		//  Constructs a new StringPool with the specified size settings
		//
		//  PARAMETERS:
		//
		//		size_t			-		Capacity of the pool (number of strings)
		//		size_t			-		Capacity of the pool (string storage space)
		//
		//  RETURNS:
		//
		//		bool			-		true if the initialisation succeeded, otherwise false
		//
		//  NOTES:
		//

		bool	initialisePool(size_t RNS, size_t RSPS) {

			//  Clear eny existing pool content
			clearPool();

			//
			//  Allocate and initialise the Object Reference Table (ORT)
			//

			ORT = (SORef*)malloc(RNS * sizeof(SORef));
			if (ORT == nullptr) return false;
			ORTCap = RNS;
			ORTHiWater = 0;
			ORTEnts = 0;
			memset(ORT, 0xFF, RNS * sizeof(SORef));

			//
			//  Allocate and initialise the Object Pool (OP)
			//

			OP = (BYTE*)malloc(RSPS);
			if (OP == nullptr) return false;
			OPCap = RSPS;
			OPUsed = 0;
			memset(OP, 0, RSPS);

			//  Return to caller
			return true;
		}

		//  checkCapacity 
		//
		//  Checks and if necessary adjusts the capacity of the pool.
		//
		//  PARAMETERS:
		//
		//		size_t			-		Size if the object to be added to the pool
		//
		//  RETURNS:
		//
		//		bool			-		true if there is capacity for the string, otherwise false
		//
		//  NOTES:
		//

		bool	checkCapacity(size_t NewObjLen) {

			//
			//  1. If the pool is not yet initialised then force initialisation with sufficient capacity
			//

			if (ORTCap == 0 || OPCap == 0) {
				if (NewObjLen < 4096) NewObjLen = 4096;
				return initialisePool(100, 2 * NewObjLen);
			}

			//
			//  2. The Object Reference Table (ORT) must have capacity for one additional object
			//

			if (ORTEnts == ORTCap) {
				//  Expand the ORT Pool
				SORef* NewORT = (SORef*)realloc(ORT, (ORTCap + 100) * sizeof(SORef));
				if (NewORT == nullptr) {
					free(ORT);
					ORT = nullptr;
					return false;
				}
				else ORT = NewORT;
				memset(ORT + ORTCap, 0xFF, 100 * sizeof(SORef));
				ORTCap += 100;
			}

			//
			//  3. The Object Pool must have the capacity for the object
			//

			while (NewObjLen >= (OPCap - OPUsed)) {
				//  Expand the Object Pool (OP)
				BYTE* NewOP = (BYTE*)realloc(OP, OPCap + 4096);
				if (NewOP == nullptr) {
					free(OP);
					OP = nullptr;
					free(ORT);
					ORT = nullptr;
					return false;
				}
				else OP = NewOP;
				memset(OP + OPCap, 0, 4096);
				OPCap += 4096;
			}

			//  Return to caller
			return true;
		}

		//  locateFreeObjectRef 
		//
		//  Locates the first available object reference to use
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		STRREF			-		The first available string reference for a new string
		//
		//  NOTES:
		//

		STRREF	locateFreeObjectRef() {

			//  If there are no unused references in the table then return the next at the hi water mark
			if (ORTEnts == ORTHiWater) return ORTHiWater + 1;

			//  Search the Object Reference Table for the first unused entry
			for (size_t ORX = 0; ORX < ORTHiWater; ORX++) {
				if (ORT[ORX].OOff == EmptySlot) return ORX + 1;
			}

			//  SNO - no available slot was located
			return NULLSTRREF;
		}

		//  buttonPool
		//
		//  NULL default implementation of a function that is called to obfuscate the contents of the Object Pool.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		virtual void buttonPool() {
			return;
		}

		//  unbuttonPool
		//
		//  NULL default implementation of a function that is called to deobfuscate the contents of the Object Pool.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		virtual void unbuttonPool() {
			return;
		}
	};

}
