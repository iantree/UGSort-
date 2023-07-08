#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       QHierarchy.h																						*
//*   Suite:      Quick Hierarchy Template Class																	*
//*   Version:    1.0.0	(Build: 01)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*	QHierarchy																										*
//*																													*
//*	This header provides the class definition for the QHierarchy class.	This is a lightweight container for an		*
//* arbitrary concrete class with ocurrences held as an n-ary tree structure.										*
//* The class only implements forward iterators (explosions) on the data structure.									*
//*																													*
//*	USAGE:																											*
//*																													*
//*	NOTES:																											*
//*																													*
//*	1.																												*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 -		10/05/2018	-	Initial Release																		*
//*																													*
//*******************************************************************************************************************/

//  Language & Platform Headers
#include	"../LPBHdrs.h"
#include	"../types.h"
#include	"../consts.h"

namespace xymorg {

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Template Class Definitions                                                                                    *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   QHierarchy Template Class																						*
	//*                                                                                                                 *
	//*  This template class provides a hierarchical container for a concrete class that forms the nodes within the.	*
	//*  the hierarchy.																									*
	//*                                                                                                                 *
	//*******************************************************************************************************************

	template <typename T>

	class QHierarchy {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Forward Declarations                                                                                          *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class Node;																					//  Node in the Hierarchy
		class Explosion;																			//  Indented explosion iterator
		class Leaves;																				//  Iterator for terminal (leaf) nodes only

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

		//  Default Constructor
		//
		//  Constructs a new empty hierarchy containg only a dummy Root Node.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		QHierarchy() {

			//  Return to caller
			return;
		}

		//  Copy Constructor
		//
		//  Performs a deep copy to construct a new hierarchy containing a copy of all of the nodes in the source hierarchy.
		//
		//  PARAMETERS:
		//
		//		QHierarchy&			-		Const reference to the source hierarchy.
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		QHierarchy(QHierarchy<T>& Src) {
			Node*			pLastAdded = &Root;															//  Last node added to the target
			size_t			Level = 0;																	//  Level of the last node added

			//  Copy the Root node from the source
			Root = Src.Root;

			//  Clear the hierarchy from the target root node
			Root.pCF = nullptr;
			Root.pTF = nullptr;

			//  Perform an explosion on the source copying each node from the source into the appropriate position in the target hierarchy.
			for (Explosion BOMX = Src.startIndentedExplosion(); BOMX != endExplosion(); BOMX++) {
				if (BOMX.getLevel() > 0) {
					//  If the level is greater than the level last added then the node is a child of the last addition
					if (BOMX.getLevel() > Level) {
						pLastAdded = pLastAdded->addChild();
						*pLastAdded = *BOMX;
						Level = BOMX.getLevel();
					}
					else {
						//  If the level is the same then this is a twin of the last node added
						if (BOMX.getLevel() == Level) {
							pLastAdded = pLastAdded->addTwin();
							*pLastAdded = *BOMX;
						}
						else {
							//  the level indicates that we have a twin of a parent node -- cycle up to the correct level and add the twin
							while (Level > BOMX.getLevel()) {
								pLastAdded = pLastAdded->getParent();
								Level--;
							}

							pLastAdded = pLastAdded->addTwin();
							*pLastAdded = *BOMX;
						}
					}
				}
			}

			//  Return to caller
			return;
		}

		//  Copy Constructor (sub-hierarchy)
		//
		//  Performs a deep copy to construct a new hierarchy containing a copy of all of the nodes in the source sub-hierarchy.
		//
		//  PARAMETERS:
		//
		//		Node&			-		Const reference to the source root node of the sub-hierarchy.
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		QHierarchy(Node& Src) {
			Node*			pLastAdded = &Root;															//  Last node added to the target
			size_t			Level = 0;																	//  Level of the last node added
			size_t			SrcRootLevel = 0;															//  Level of the sub-hierarchy root

			//  Copy the Root node from the source
			Root = Src;

			//  Clear the hierarchy from the target root node
			Root.pCF = nullptr;
			Root.pTF = nullptr;

			//  Determine the level of the sub-hierarchy root in it's hierarchy
			Node* pParent = Src.getParent();
			while (pParent != nullptr) {
				SrcRootLevel++;
				pParent = pParent->getParent();
			}

			//  Perform an explosion on the source root copying each node into the target node
			for (Explosion BOMX = startIndentedExplosion(&Src); BOMX != endExplosion(); BOMX++) {
				size_t  EffectiveLevel = BOMX.getLevel() - SrcRootLevel;
				if (EffectiveLevel > 0) {
					//  If the effective level is greater than the last level added then the new node is added as a child
					if (EffectiveLevel > Level) {
						pLastAdded = pLastAdded->addChild();
						*pLastAdded = *BOMX;
						Level = EffectiveLevel;
					}
					else {
						//  If the level is the same then this is a twin of the last node added
						if (EffectiveLevel == Level) {
							pLastAdded = pLastAdded->addTwin();
							*pLastAdded = *BOMX;
						}
						else {
							//  the level indicates that we have a twin of a parent node -- cycle up to the correct level and add the twin
							while (Level > EffectiveLevel) {
								pLastAdded = pLastAdded->getParent();
								Level--;
							}

							pLastAdded = pLastAdded->addTwin();
							*pLastAdded = *BOMX;
						}
					}
				}
			}

			//  Return to caller
			return;
		}

		//  Move Constructor
		//
		//  Creates a new Hierarchy that takes ownership of all of the nodes in the source hierarchy.
		//
		//  PARAMETERS:
		//
		//		QHierarchy&&			-		RHS Reference (non-const) to the source hierarchy.
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		QHierarchy(QHierarchy&& Src) {

			//  Copy the Root node from the source
			Root = Src.Root;

			//  Inherit the hierarchy from the source
			Root.pCF = Src.Root.pCF;
			Root.pTF = Src.Root.pTF;

			//  Update the parent pointers in the children
			Node* pTwin = Root.pCF;
			while (pTwin != nullptr) {
				pTwin->pP = &Root;
				pTwin = pTwin->getTwin();
			}

			//  Clear the hierarchy from the root of the source
			Src.Root.pCF = nullptr;
			Src.Root.pTF = nullptr;

			//  Return to caller
			return;
		}

		//  Destructor
		//
		//  Destroys the QHierarchy object and destroys all of the nodes in the hierarchy.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		~QHierarchy() {

			//  Dismiss the hierarchy
			dismiss();

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

		//  getRoot
		//
		//  Returns a pointer to the virtual root of the hierarchy.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		QHierarchy::Node*			-		Pointer to the virtual root node
		//
		//  NOTES:
		//

		Node* getRoot() { return &Root; }

		//  dismiss
		//
		//  Releases the content of the Hierarchy
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		void		dismiss() {
			Node*		pCurrentNode = nullptr;

			//  Delete all children of the Root
			pCurrentNode = Root.getChild();
			while (pCurrentNode != nullptr) {
				delete pCurrentNode;
				pCurrentNode = Root.getChild();
			}

			//  Delete all twins of the Root
			pCurrentNode = Root.getTwin();
			while (pCurrentNode != nullptr) {
				delete pCurrentNode;
				pCurrentNode = Root.getTwin();
			}

			//  Return to caller
			return;
		}

		//  getNodeCount
		//
		//  Returns a count of the nodes currently in the hierarchy
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		size_t			-		Count of nodes in the hierarchy
		//
		//  NOTES:
		//

		size_t		getNodeCount() {
			Node*			pCurrentNode = &Root;
			size_t			NodeCount = 0;

			while (pCurrentNode != nullptr) {
				NodeCount++;
				if (pCurrentNode->getChild() != nullptr) pCurrentNode = pCurrentNode->getChild();
				else {
					if (pCurrentNode->getTwin() != nullptr) pCurrentNode = pCurrentNode->getTwin();
					else {
						while (pCurrentNode != nullptr) {
							pCurrentNode = pCurrentNode->getParent();
							if (pCurrentNode != nullptr) {
								if (pCurrentNode->getTwin() != nullptr) {
									pCurrentNode = pCurrentNode->getTwin();
									break;
								}
							}
						}
					}
				}
			}

			return NodeCount;
		}

		//
		//  Explosion Iterator Creation functions
		//

		//  startIndentedExplosion
		//
		//  Returns an indented explosion iterator positioned at the root of the hierarchy
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		Explosion		-		Iterator object positioned at the root of the hierarchy
		//
		//  NOTES:
		//

		Explosion startIndentedExplosion() {
			return Explosion(&Root, 0);
		}

		//  startIndentedExplosion
		//
		//  Returns an indented explosion iterator positioned at the passed node of the hierarchy.
		//  The exlosion will only cover the children (sub-hierarchy) of the passed node.
		//
		//  PARAMETERS:
		//
		//		Node*			-		Pointer to the node that forms the root of the sub-hierarchy
		//
		//  RETURNS:
		//
		//		Explosion		-		Iterator object positioned at the root of the sub-hierarchy
		//
		//  NOTES:
		//

		Explosion startIndentedExplosion(Node* pSubRoot) {
			return Explosion(pSubRoot, 0);
		}

		//  startIndentedExplosion
		//
		//  Returns an indented explosion iterator positioned at the root of the hierarchy
		//  the explosion is restricted to the designated number of levels.
		//
		//  PARAMETERS:
		//
		//		size_t			-		Number of levels to restrict the explosion to.
		//
		//  RETURNS:
		//
		//		Explosion		-		Iterator object positioned at the root of the hierarchy
		//
		//  NOTES:
		//

		Explosion startIndentedExplosion(size_t NumLevels) {
			if (NumLevels == 0) NumLevels = 1;
			return Explosion(&Root, NumLevels);
		}

		//  startIndentedExplosion
		//
		//  Returns an indented explosion iterator positioned at the passed node of the hierarchy.
		//  The exlosion will only cover the children (sub-hierarchy) of the passed node.
		//  Also the explosion is restricted to the designated number of levels.
		//
		//  PARAMETERS:
		//
		//		Node*			-		Pointer to the node that forms the root of the sub-hierarchy
		//		size_t			-		Number of levels to restrict the explosion to.
		//
		//  RETURNS:
		//
		//		Explosion		-		Iterator object positioned at the root of the sub-hierarchy
		//
		//  NOTES:
		//

		Explosion startIndentedExplosion(Node* pSubRoot, size_t NumLevels) {
			if (NumLevels == 0) NumLevels = 1;
			return Explosion(pSubRoot, NumLevels);
		}

		//  endExplosion
		//
		//  Returns an indented explosion iterator positioned past the end of the explosion
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		Explosion		-		Iterator object positioned past the end of the explosion
		//
		//  NOTES:
		//

		Explosion endExplosion() {
			return Explosion(nullptr, 0);
		}

		//  startLeaves
		//
		//  Returns a leaves iterator positioned to the first leaf node in the hierarchy
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		Leaves		-		Iterator object positioned to the first leaf node
		//
		//  NOTES:
		//

		Leaves	startLeaves() {
			return Leaves(&Root);
		}

		//  startLeaves
		//
		//  Returns a leaves iterator positioned to the first leaf node in the sub-hierarchy
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		Leaves		-		Iterator object positioned to the first leaf node
		//
		//  NOTES:
		//

		Leaves	startLeaves(Node* pRoot) {
			return Leaves(pRoot);
		}

		//  endLeaves
		//
		//  Returns a leaves iterator positioned past the end of the explosion
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		Leaves		-		Iterator object positioned past the end of the explosion
		//
		//  NOTES:
		//

		Leaves	endLeaves() {
			return Leaves(nullptr);
		}

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members                                                                                               *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Dummy root node that forms the virtual base for the hierarchy
		Node		Root;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************


		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Nested Classes		                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Nested Classes		                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Node Class			                                                                                        *
		//*																													*
		//*   The Node class provides the wrapper for the contained objects that provides the hierarchic management			*
		//*   functions.																									*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class Node : public T {
		public:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Friendships                                                                                                   *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			friend QHierarchy<T>;

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Constructors                                                                                                  *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  Default Constructor
			//
			//  Constructs a new empty node - the node is free standing i.e. it is not incorporated into the hierarchy
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//  NOTES:
			//

			Node() : T() {

				//  Initialise the hierarchic pointers
				pCF = nullptr;
				pTF = nullptr;
				pP = nullptr;

				//  Return to caller
				return;
			}

			//  Child Constructor
			//
			//  Constructs a new empty node - the node is incorporated into the hierarchy at the indicated position
			//
			//  PARAMETERS:
			//
			//		Node*		-		Pointer to the parent node
			//		Node*		-		Pointer to the child node that the new node is to be placed after (may be nullptr)
			//
			//  RETURNS:
			//
			//  NOTES:
			//
			//		1.	If the second paraneter is NULL then the Node is appended to the end of the twin chain of existing children
			//		2.	If the senond parameter points to the parent then the Node is inserted at the start of the twin chain of children
			//

			Node(Node* pNewParent, Node* pNewTwinBefore) : T() {

				//  No parent specified - create a free-stading Node
				if (pNewParent == nullptr) {
					pCF = nullptr;
					pTF = nullptr;
					pP = nullptr;
					return;
				}

				//  No twin position specified - add the new node to then end of the existing children
				if (pNewTwinBefore == nullptr) {
					pP = pNewParent;
					pCF = nullptr;
					pTF = nullptr;
					if (pNewParent->pCF == nullptr) {
						pNewParent->pCF = this;
						return;
					}
					//  Tack the new node onto the end of the existing child chain
					pNewParent->getLastChild()->pTF = this;
					return;
				}

				//  The new node should be inserted at the start of the twin chain of chidren of the parent
				if (pNewTwinBefore == pNewParent) {
					pP = pNewParent;
					pCF = nullptr;
					pTF = pNewParent->pCF;
					pNewParent->pCF = this;
					return;
				}

				//  Find the position where the new child should be inserted
				pP = pNewParent;
				pCF = nullptr;
				pTF = nullptr;
				if (pNewParent->pCF == nullptr) {
					pNewParent->pCF = this;
					return;
				}
				Node* pTB = pNewParent->pCF;
				while (pTB != nullptr && pTB != pNewTwinBefore) pTB = pTB->pTF;
				if (pTB == nullptr) {
					//  Twin position was not found, the new node is appended to the twin chain
					pNewParent->getLastChild()->pTF = this;
					return;
				}

				//  Insert the child into the correct position in the hierarchy
				pTF = pTB->pTF;
				pTB->pTF = this;

				//  Return to caller
				return;
			}

			//  Copy Constructor
			//
			//  Makes a new copy of the source node that is free-standing i.e. does not belong to a hierarchy
			//
			//  PARAMETERS:
			//
			//		Node&			-		Const reference to the source node.
			//
			//  RETURNS:
			//
			//  NOTES:
			//

			Node(const Node& Src) : T(Src) {

				//  Clear the hierarchy
				pCF = nullptr;
				pTF = nullptr;
				pP = nullptr;

				//  Return to caller
				return;
			}

			//  Destructor
			//
			//  Destroys this Node and all sub-ordinate nodes in the hierarchy
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//  NOTES:
			//

			~Node() {
				//  Isolate the node (sub-hierarchy) from the parent node
				if (pP != nullptr) {
					if (pP->pCF == this) pP->pCF = pTF;
					else {
						Node* pTB = pP->pCF;
						while (pTB->pTF != this) pTB = pTB->pTF;
						pTB->pTF = pTF;
					}
					pP = nullptr;
				}

				//  Eliminate the twin chain of the first child
				if (pCF == nullptr) return;
				while (pCF->pTF != nullptr) delete pCF->pTF;

				//  Eliminate the first child
				delete pCF;

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

			//  getParent
			//
			//  Returns the parent pointer from the current node
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		Node*		-		Pointer to the parent node
			//
			//  NOTES:
			//

			Node* getParent() { return pP; }

			//  getChild
			//
			//  Returns the child (first) pointer from the current node
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		Node*		-		Pointer to the first child node
			//
			//  NOTES:
			//

			Node* getChild() { return pCF; }

			//  getTwin
			//
			//  Returns the twin (forward) pointer from the current node
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		Node*		-		Pointer to the next twin node
			//
			//  NOTES:
			//

			Node* getTwin() { return pTF; }

			//  getLastTwin
			//
			//  Returns the last twin on the chain for this node
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		Node*		-		Pointer to the last twin node on the twin chain
			//
			//  NOTES:
			//

			Node* getLastTwin() {
				Node* pEOC = this;
				while (pEOC->pTF != nullptr) pEOC = pEOC->pTF;
				return pEOC;
			}

			//  getLastChild
			//
			//  Returns the last twin on the chain for the children of this node (if any)
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		Node*		-		Pointer to the last twin node on the child twin chain
			//
			//  NOTES:
			//

			Node* getLastChild() {
				if (pCF == nullptr) return nullptr;
				return pCF->getLastTwin();
			}

			//  countChildren
			//
			//  Returns the count of children of this node
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		size_t		-		Number of children
			//
			//  NOTES:
			//

			size_t		countChildren() {
				Node* pEOC = pCF;
				size_t		CCount = 1;

				if (pCF == nullptr) return 0;

				while (pEOC->pTF != nullptr) {
					CCount++;
					pEOC = pEOC->pTF;
				}

				return CCount;
			}

			//  addChild
			//
			//  Adds a new child to the end of the twin chain of children of the current node
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		Node*		-		Pointer to the new child node
			//
			//  NOTES:
			//

			Node* addChild() {
				return new Node(this, nullptr);
			}

			//  insertChild
			//
			//  Inserts a new child to the start of the twin chain of children of the current node
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		Node*		-		Pointer to the new child node
			//
			//  NOTES:
			//

			Node* insertChild() {
				return new Node(this, this);
			}

			//  addTwin
			//
			//  Adds a new twin after this node in the hierarchy
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		Node*		-		Pointer to the new twin node
			//
			//  NOTES:
			//

			Node* addTwin() {
				return new Node(pP, this);
			}

			//  insertTwin
			//
			//  Adds a new twin before this node in the hierarchy
			//
			//  PARAMETERS:
			//		
			//  RETURNS:
			//
			//		Node*		-		Pointer to the new twin node
			//
			//  NOTES:
			//

			Node* insertTwin() {
				Node* pPred = pP->getChild();													//  Predecessor Node
				if (pPred == nullptr) return new Node(pP, pP);

				while (pPred->getTwin() != nullptr && pPred->getTwin() != this) pPred = pPred->getTwin();
				return new Node(pP, pPred);
			}

			//  copy assignment operator =
			//
			//  DOES NOTHING - EXPLICITLY -- except invoke the base class assignment operator
			//
			//  PARAMETERS:
			//
			//		Node&		-		Const reference to the source of the assignment
			//
			//  RETURNS:
			//
			//		Node&		-		Reference to the target of the assignment
			//
			//  NOTES:
			//

			Node& operator = (const Node& Src) {
				//  Invoke the base class copy assignment operator
				T::operator=(Src);
				//  Return to caller
				return *this;
			}

			//  join (Hierarchy) operator +=
			//
			//  Joins the source Hierarchy into the hierarchy of the current node. The root of the source is added as the
			//  last child of the current node.
			//
			//  PARAMETERS:
			//
			//		QHierarchy&		-		Non-Const reference to the source hierarchy
			//
			//  RETURNS:
			//
			//		Node&		-		Reference to the target of the assignment
			//
			//  NOTES:
			//

			Node& operator += (QHierarchy<T>& Src) {
				Node*		pNewRoot = addChild();																//  Create the new root 

				//  Add the content from the root of the source hierarchy to the new root
				*pNewRoot = *(Src.getRoot());

				//  Take over the remainder of the hierarchy by copying the Child First pointer from the source root
				pNewRoot->pCF = Src.getRoot()->pCF;
				pNewRoot->pTF = Src.getRoot()->pTF;

				//  Update thye parent pointers in the copied hierarchy
				Node* pTwin = pNewRoot->getChild();
				while (pTwin != nullptr) {
					pTwin->pP = pNewRoot;
					pTwin = pTwin->getTwin();
				}

				//  Clear the hierarchy from the source
				Src.getRoot()->pCF = nullptr;
				Src.getRoot()->pTF = nullptr;

				//  Return to caller
				return *this;
			}

			//  join (Sub-Hierarchy) operator +=
			//
			//  Snips the passed node from the source sub-hierarchy from it's hierarchy and joins it as the last child of the currennt node.
			//
			//  PARAMETERS:
			//
			//		Node&		-		Non-Const reference to the source node
			//
			//  RETURNS:
			//
			//		Node&		-		Reference to the target of the assignment
			//
			//  NOTES:
			//

			Node& operator += (Node& Src) {
				Node*		pNewChild = &Src;

				//  Isolate the passed sub-hierarchy (node) from it's hierarchy
				if (Src.pP != nullptr) {
					//  If this is the first child of the parent then update the parent
					if (Src.pP->getChild() == &Src) Src.pP->pCF = Src.getTwin();
					else {
						//  Snip the node from the current twin chain
						Node* pTB = Src.pP->getChild();
						while (pTB != nullptr) {
							if (pTB->pTF == &Src) {
								pTB->pTF = Src.pTF;
								break;
							}
							pTB = pTB->pTF;
						}
						//  SNO ...  Twin chain invalid
						if (pTB == nullptr) return *this;
					}
					Src.pTF = nullptr;
				}
				else {
					//  The source sub-hierarchy is a complete hierarchy
					//  Create a new root as a copy of the source

					pNewChild = addChild();
					*pNewChild = Src;
				}

				//  Set the new parent pointer in the source
				pNewChild->pP = this;

				//  Add the new source to the end of the children twin chain
				getLastChild()->pTF = pNewChild;

				//  Return to caller
				return *this;
			}

			//  extract
			//
			//  Snips the passed node from the source sub-hierarchy from it's hierarchy and returns the result as a standalone hierarchy
			//
			//  PARAMETERS:
			//
			//		Node&		-		Non-Const reference to the source node
			//
			//  RETURNS:
			//
			//		QHIerarchy	-		Hierarchy containing the excised sub-hierarchy
			//
			//  NOTES:
			//

			QHierarchy<T>		extract(Node& Src) {
				QHierarchy<T>			SubHier;														//  Container for the excised sub-hierarchy

				//  Specuial case - the source node is the root of the source hierarch
				if (Src.getParent() == nullptr) {
					//  Copy the root content to the sub-hierarchy root
					*(SubHier.getRoot()) = Src;

					//  Set the pointers in the sub-hierarchy root
					SubHier.getRoot()->pCF = Src.pCF;
					SubHier.getRoot()->pTF = Src.pTF;

					//  Clear the sub-hierarchy from the source
					Src.pCF = nullptr;
					Src.pTF = nullptr;

				}
				else {
					//  Copy the root content to the sub-hierarchy root
					*(SubHier.getRoot()) = Src;

					//  Set the pointers in the sub-hierarchy root
					SubHier.getRoot()->pCF = Src.pCF;

					//  Disconnect the source root node 
					Src.pCF = nullptr;

					if (Src.getParent()->pCF == &Src) Src.getParent()->pCF = Src.pTF;
					else {
						Node* pTB = Src.getParent()->pCF;
						while (pTB != nullptr) {
							if (pTB->pTF == &Src) {
								pTB->pTF = Src.pTF;
								break;
							}
							pTB = pTB->pTF;
						}
					}

					Src.pP = nullptr;

					//  Delete the source node
					delete& Src;
				}

				//  Update the parent pointer in any children of the new root
				Node* pTwin = SubHier.getRoot()->getChild();
				while (pTwin != nullptr) {
					pTwin->pP = SubHier.getRoot();
					pTwin = pTwin->getTwin();
				}

				//  Return the excised hierarchy
				return SubHier;
			}

			//  isolate
			//
			//  Creates an isolated hierarchy containing the passed node and it's direct antecedents
			//
			//  PARAMETERS:
			//
			//		Node&		-		Const reference to the source node
			//
			//  RETURNS:
			//
			//		QHIerarchy	-		Hierarchy containing the isolated sub-hierarchy
			//
			//  NOTES:
			//

			QHierarchy<T>		isolate(Node& Src) {
				QHierarchy<T>			SubHier;														//  Container for the excised sub-hierarchy
				Node*					pNewNode = SubHier.getRoot();									//  Root
				Node*					pParent = Src.getParent();										//  Parents of the source

				//  Copy the content of the source node into the root of the new hierarchy
				*pNewNode = Src;

				//  Insert a new node above the root and populate it with a copy of the parent node from the shource
				while (pParent != nullptr) {
					pNewNode = pNewNode->pushDown(*pNewNode);
					*pNewNode = *pParent;

					//  Ascend to the next parent
					pParent = pParent->getParent();
				}

				//  Return the isolated hierarchy
				return SubHier;
			}

			//  pushDown
			//
			//  Pushes the passed node down the hierarchy (with it'd children) and returns a pointer to the new parent
			//
			//  PARAMETERS:
			//
			//		Node&		-		reference to the source node
			//
			//  RETURNS:
			//
			//		Node*	-		Pointer to the new parent node
			//
			//  NOTES:
			//

			Node* pushDown(Node& Src) {
				Node*		pNewNode = &Src;																//  Pointer to the new parent node
				Node*		pNewChild = new Node();														//  New child

				//  Copy the content of the parent node to the new child
				*pNewChild = Src;
				pNewChild->pP = pNewNode;

				//  Move the child pointer from parent to child
				pNewChild->pCF = pNewNode->pCF;
				pNewNode->pCF = pNewChild;

				//  Update the parent pointers in the children
				Node* pNode = pNewChild->pCF;
				while (pNode != nullptr) {
					pNode->pP = pNewChild;
					pNode = pNode->pTF;
				}

				//  Return the pointer to the new parent
				return pNewNode;
			}

			//  isBranch
			//
			//  Tests if the node is a Branch Node in the hierarchy
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		bool		-		true if the node is a branch, otherwise false
			//
			//  NOTES:
			//

			bool		isBranch() {
				if (pCF != nullptr) return true;
				return false;
			}

			//  isLeaf
			//
			//  Tests if the node is a Leaf Node in the hierarchy
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		bool		-		true if the node is a leaf, otherwise false
			//
			//  NOTES:
			//

			bool		isLeaf() {
				if (pCF != nullptr) return false;
				return true;
			}

		private:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Members                                                                                               *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  Hierarchy
			Node*		pCF;																		//  Child First Pointer
			Node*		pTF;																		//  Twin Forward Pointer
			Node*		pP;																		//  Parent node Pointer

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Functions                                                                                             *
			//*                                                                                                                 *
			//*******************************************************************************************************************

		};

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Explosion Class			                                                                                    *
		//*																													*
		//*   The Explosion class provides an iterator for performing an indented explosion on the contents of the			*
		//*   hierarchy, the explosion is indented in that it maintains a track of the level of each node in the			*
		//*   hierarchy.																									*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class Explosion {
		public:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Iterator Traits                                                                                               *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			typedef std::forward_iterator_tag			iterator_category;
			typedef Node								value_type;
			typedef bool								difference_type;
			typedef Node& reference;
			typedef Node* pointer;
			typedef Explosion							self;

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Constructors                                                                                                  *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  Standard Constructor
			//
			//  Constructs a new indented explosion iterator positioned at the passed node
			//
			//  PARAMETERS:
			//
			//		Node*		-		Pointer to the node that forms the root for the explosion
			//		size_t		-		Number of levels to restrict the explosion to, zero implies unrestricted
			//
			//  RETURNS:
			//
			//  NOTES:
			//

			Explosion(Node* pXRoot, size_t NumLevels) {
				pRoot = pXRoot;
				pCurrentNode = pXRoot;
				Level = 0;
				RestrictLevel = SIZE_MAX;

				if (pXRoot == nullptr) return;

				//  Set the level of the sub-root (or root)
				Node* pParent = pRoot->getParent();
				while (pParent != nullptr) {
					Level++;
					pParent = pParent->getParent();
				}

				//  Set the level to which the explosion will go
				if (NumLevels == 0) RestrictLevel = SIZE_MAX;
				else RestrictLevel = (Level + NumLevels) - 1;
				return;
			}

			//  Destructor
			//
			//  Destroys the iterator
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//  NOTES:
			//

			~Explosion() {
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

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Operator Overload Functions                                                                                   *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  Dereference * operator
			//
			//	Returns a non-const reference to the current node
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		Node&					-	Reference to the current node
			//
			//  NOTES
			//

			reference	operator * () { return *pCurrentNode; }

			//  Pointer dereference -> operator
			//
			//	Returns a pointer to the current node
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		Node*					-	Pointer to the current node
			//
			//  NOTES
			//

			pointer	operator -> () { return pCurrentNode; }

			//  Prefix Increment ++X operator
			//
			//  Continues the explosion to the next node in the hierarchy and returns the updated iterator
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		Explosion					-	The updated iterator
			//
			//  NOTES
			//

			self	operator ++ () {
				nextNode();
				return *this;
			}

			//  Postfix Increment X++ operator
			//
			//  Continues the explosion to the next node in the hierarchy and returns a copy of the the original iterator
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		Explosion					-	The NOT updated iterator
			//
			//  NOTES
			//

			self	operator ++ (int) {
				self		PreMod(*this);
				nextNode();
				return PreMod;
			}

			//  Equality comparison == operator
			//
			//  Determines if twi iterators are equal (are positioned at the same node)
			//
			//  PARAMETERS
			//
			//		Explosion&		-		Const reference to the iterator to be comared to this one
			//
			//  RETURNS
			//
			//		bool					-	true if the two iterators are positioned at the same node, otherwise false
			//
			//  NOTES
			//

			difference_type		operator == (const self& Comp) {
				if (pCurrentNode == Comp.pCurrentNode) return true;
				return false;
			}

			//  Inequality comparison != operator
			//
			//  Determines if twi iterators are not equal (are NOT positioned at the same node)
			//
			//  PARAMETERS
			//
			//		Explosion&		-		Const reference to the iterator to be comared to this one
			//
			//  RETURNS
			//
			//		bool					-	true if the two iterators are NOT positioned at the same node, otherwise false
			//
			//  NOTES
			//

			difference_type		operator != (const self& Comp) {
				if (*this == Comp) return false;
				return true;
			}

			//  getLevel
			//
			//  Returns the current level in the hierarchy
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		size_t					-	The current level in the hierarchy
			//
			//  NOTES
			//

			size_t		getLevel() { return Level; }

			//  getNode
			//
			//  Returns a pointer to the current node 
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		Node*					-	Pointer to the current node in the explosion.
			//
			//  NOTES
			//

			Node* getNode() { return pCurrentNode; }

		private:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Members                                                                                               *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			Node*		pRoot;																//  Pointer to the root of the hierarchy
			Node*		pCurrentNode;														//  Pointer to the current node in the hierarchy
			size_t		Level;																//  Level of the current node in the hierarchy
			size_t		RestrictLevel;														//  Level to which the explosion is restricted	

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Functions                                                                                             *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  nextNode
			//
			//  Continues the explosion to the next node in the hierarchy, updating the current position
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//  NOTES
			//

			void	nextNode() {

				//  Safety
				if (pCurrentNode == nullptr) return;

				//  If the current node has a child (or more than one) then position to the first child
				if (pCurrentNode->getChild() != nullptr && Level < RestrictLevel) {
					Level++;
					pCurrentNode = pCurrentNode->getChild();
					return;
				}

				//  If the current node has a twin then move the current position to the twin, unless it is the root
				if (pCurrentNode == pRoot) {
					pCurrentNode = nullptr;
					return;
				}
				if (pCurrentNode->getTwin() != nullptr) {
					pCurrentNode = pCurrentNode->getTwin();
					return;
				}

				//  Move to the new available twin of an antecedent - this terminates if we arrive back at the root of the explosion
				Node* pParent = pCurrentNode->getParent();

				while (pParent != nullptr && pParent != pRoot) {
					Level--;
					if (pParent->getTwin() != nullptr) {
						pCurrentNode = pParent->getTwin();
						return;
					}
					pParent = pParent->getParent();
				}

				//  End of the explosion
				pCurrentNode = nullptr;

				//  Return to caller
				return;
			}

		};

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Leaves Class																									*
		//*																													*
		//*   The Leaves class provides an iterator for iterating over the terminal (leaf) nodes in a hierarchy.			*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class Leaves {
		public:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Iterator Traits                                                                                               *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			typedef std::forward_iterator_tag			iterator_category;
			typedef Node								value_type;
			typedef bool								difference_type;
			typedef Node& reference;
			typedef Node* pointer;
			typedef Leaves								self;

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Constructors                                                                                                  *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  Standard Constructor
			//
			//  Constructs a new terminal (leaf) nodes iterator
			//
			//  PARAMETERS:
			//
			//		Node*		-		Pointer to the node that forms the root for the explosion
			//
			//  RETURNS:
			//
			//  NOTES:
			//

			Leaves(Node* pXRoot) {

				//  Set the initial position
				pRoot = pXRoot;
				pCurrentNode = pXRoot;
				if (pXRoot == nullptr) return;

				//  Move the first leaf node in the hierarchy
				while (pCurrentNode->getChild() != nullptr) pCurrentNode = pCurrentNode->getChild();

				//  Return to caller
				return;
			}

			//  Destructor
			//
			//  Destroys the iterator
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//  NOTES:
			//

			~Leaves() {
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

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Operator Overload Functions                                                                                   *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  Dereference * operator
			//
			//	Returns a non-const reference to the current node
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		Node&					-	Reference to the current node
			//
			//  NOTES
			//

			reference	operator * () { return *pCurrentNode; }

			//  Pointer dereference -> operator
			//
			//	Returns a pointer to the current node
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		Node*					-	Pointer to the current node
			//
			//  NOTES
			//

			pointer	operator -> () { return pCurrentNode; }

			//  Prefix Increment ++X operator
			//
			//  Continues the explosion to the next node in the hierarchy and returns the updated iterator
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		Explosion					-	The updated iterator
			//
			//  NOTES
			//

			self	operator ++ () {
				nextLeaf();
				return *this;
			}

			//  Postfix Increment X++ operator
			//
			//  Continues the explosion to the next node in the hierarchy and returns a copy of the the original iterator
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		Explosion					-	The NOT updated iterator
			//
			//  NOTES
			//

			self	operator ++ (int) {
				self		PreMod(*this);
				nextLeaf();
				return PreMod;
			}

			//  Equality comparison == operator
			//
			//  Determines if twi iterators are equal (are positioned at the same node)
			//
			//  PARAMETERS
			//
			//		Leaves&		-		Const reference to the iterator to be comared to this one
			//
			//  RETURNS
			//
			//		bool					-	true if the two iterators are positioned at the same node, otherwise false
			//
			//  NOTES
			//

			difference_type		operator == (const self& Comp) {
				if (pCurrentNode == Comp.pCurrentNode) return true;
				return false;
			}

			//  Inequality comparison != operator
			//
			//  Determines if twi iterators are not equal (are NOT positioned at the same node)
			//
			//  PARAMETERS
			//
			//		Leaves&		-		Const reference to the iterator to be comared to this one
			//
			//  RETURNS
			//
			//		bool					-	true if the two iterators are NOT positioned at the same node, otherwise false
			//
			//  NOTES
			//

			difference_type		operator != (const self& Comp) {
				if (*this == Comp) return false;
				return true;
			}

			//  getNode
			//
			//  Returns a pointer to the current node 
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		Node*					-	Pointer to the current node in the explosion.
			//
			//  NOTES
			//

			Node* getNode() { return pCurrentNode; }

		private:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Members                                                                                               *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			Node*		pRoot;																//  Pointer to the root of the hierarchy
			Node*		pCurrentNode;														//  Pointer to the current node in the hierarchy

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Functions                                                                                             *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  nextLeaf
			//
			//  Continues the explosion to the next leaf in the hierarchy, updating the current position
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//  NOTES
			//

			void	nextLeaf() {

				//  Safety
				if (pCurrentNode == nullptr) return;

				//  If the current node has a twin then move to that node
				if (pCurrentNode->getTwin() != nullptr) pCurrentNode = pCurrentNode->getTwin();
				else {
					//  Progress back up the hierarchy until we encounter a parent with a twin
					Node* pParent = pCurrentNode->getParent();

					if (pParent == nullptr || pParent == pRoot) {
						pCurrentNode = nullptr;
						return;
					}

					while (pParent != nullptr && pParent != pRoot) {
						if (pParent->getTwin() != nullptr) {
							pCurrentNode = pParent->getTwin();
							break;
						}
						pParent = pParent->getParent();
					}

					if (pParent == nullptr || pParent == pRoot) {
						pCurrentNode = nullptr;
						return;
					}
				}

				//  Descend to the first leaf node below the current position
				while (pCurrentNode->getChild() != nullptr) pCurrentNode = pCurrentNode->getChild();

				//  Return to caller
				return;
			}
		};

	};
}
