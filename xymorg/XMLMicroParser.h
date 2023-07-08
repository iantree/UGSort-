#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       XMLMicroParser.h																					*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.0.0	(Build: 01)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2023 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the XMLMicroParser class. The XMLMicroParser class provides the	*
//* minimal non-validating XML parser needed for parsing simple, well-formed XML documents. 						*
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
//*	1.0.0 -		03/12/2017	-	Initial Release																		*
//*																													*
//*******************************************************************************************************************/

//  Include xymorg headers
#include	"LPBHdrs.h"																			//  Language and Platform base headers
#include	"types.h"																			//  xymorg type definitions
#include	"consts.h"																			//  xymorg constant definitions

//  Additional component headers
#include	"StringThing.h"																		//  String manipulations

namespace xymorg {

	//
	//   GLOBAL DEFINITIONS
	//

#define			XMP_MAX_NODE_NAME			250												//  Maximum size of an XML Node Name
#define			XMP_MAX_NODE_PATH			1024											//  Maximum size of a node XPATH

//
//  XMLMicroParser Class Definition
//
	class XMLMicroParser {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Conxstants		                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		static const int	cMaxNodeName = 250;												//  Max size for an individual node name
		static const int	cMaxNodePath = 1024;											//  Max size for a node XPATH

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Nested Classes	                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   XMLIterator Class		                                                                                        *
		//*																													*
		//*	  The XMLIterator class provides an iterator over a node and all sub-nodes in an XML document.					*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class XMLIterator {
		public:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Constructors			                                                                                        *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  Default Constructor 
			//
			//  Constructs a NULL XML Iterator
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//  NOTES:
			//

			XMLIterator() {
				pStartNode = nullptr;
				pEndNode = nullptr;
				pPosition = nullptr;
				pElement = nullptr;
				ElementSize = 0;
				pName = nullptr;

				Depth = 0;
				XPATH[0] = '\0';

				//  Return to caller
				return;
			}

			//  Constructor 
			//
			//  Constructs the XML Iterator over the span of a particuar node 
			//
			//  PARAMETERS:
			//
			//		char *			-		Const pointer to XML Node that is to be iterated
			//
			//  RETURNS:
			//
			//  NOTES:
			//

			XMLIterator(const char* pVRoot) {
				pStartNode = pVRoot;
				pEndNode = nullptr;
				pPosition = nullptr;
				pElement = nullptr;
				ElementSize = 0;
				pName = nullptr;

				Depth = 0;
				XPATH[0] = '\0';

				//  Boundary case - A nullptr XML Document
				if (pStartNode == nullptr) return;
				if (pStartNode[0] == '\0') return;

				//  Validate/Adjust the starting Node
				pStartNode = setStartNode(pStartNode);

				//  Set the correcponding end node position, this will either be the > for a self closing node or the start of the close node
				pEndNode = findCloseNode(pStartNode);
				if (pEndNode == nullptr) {
					//  If no end-node was located then the XML document is not well-formed
					pStartNode = nullptr;
					return;
				}

				//  Set the current position to the start node
				pPosition = pStartNode;

				//  Capture the content description
				setElement();

				//  Append the node name to the XPATH
				appendToXPATH();

				//  Return to caller
				return;
			}

			//  Constructor 
			//
			//  Constructs the XML Iterator over the span of a particuar node, at a specified path 
			//
			//  PARAMETERS:
			//
			//		char*			-		Pointer to the existing path
			//		char*			-		Const pointer to XML Node that is to be iterated
			//
			//  RETURNS:
			//
			//  NOTES:
			//

			XMLIterator(char* PPath, const char* pVRoot) {
				pStartNode = pVRoot;
				pEndNode = nullptr;
				pPosition = nullptr;
				pElement = nullptr;
				ElementSize = 0;
				pName = nullptr;

				strcpy_s(XPATH, cMaxNodePath + 1, PPath);
				//  Determine the initial depth from the path
				Depth = 0;
				while (*PPath != '\0') {
					if (*PPath == '/') Depth++;
					PPath++;
				}

				//  Boundary case - A nullptr XML Document
				if (pStartNode == nullptr) return;
				if (pStartNode[0] == '\0') return;

				//  Validate/Adjust the starting Node
				pStartNode = setStartNode(pStartNode);

				//  Set the correcponding end node position, this will either be the > for a self closing node or the start of the close node
				pEndNode = findCloseNode(pStartNode);
				if (pEndNode == nullptr) {
					//  If no end-node was located then the XML document is not well-formed
					pStartNode = nullptr;
					return;
				}

				//  Set the current position to the start node
				pPosition = pStartNode;

				//  Capture the content description
				setElement();

				//  Append the node name to the XPATH
				appendToXPATH();

				//  Return to caller
				return;
			}

			//  Copy Constructor 
			//
			//  Constructs the XML Iterator from an existing XML iterator
			//
			//  PARAMETERS:
			//
			//		XMLIterator&			-		Const reference to the source iterator
			//
			//  RETURNS:
			//
			//  NOTES:
			//

			XMLIterator(const XMLIterator& Src) {
				pStartNode = Src.pStartNode;
				pEndNode = Src.pEndNode;
				pPosition = Src.pPosition;
				pElement = Src.pElement;
				ElementSize = Src.ElementSize;
				pName = Src.pName;

				Depth = Src.Depth;
				strcpy_s(XPATH, cMaxNodePath, Src.XPATH);

				//  Return to caller
				return;
			}

			//  Move Constructor 
			//
			//  Constructs the XML Iterator from an existing XML iterator
			//
			//  PARAMETERS:
			//
			//		XMLIterator&&			-		Reference to the source iterator
			//
			//  RETURNS:
			//
			//  NOTES:
			//

			XMLIterator(XMLIterator&& Src) noexcept {
				pStartNode = Src.pStartNode;
				Src.pStartNode = nullptr;
				pEndNode = Src.pEndNode;
				Src.pEndNode = nullptr;
				pPosition = Src.pPosition;
				Src.pPosition = nullptr;
				pElement = Src.pElement;
				Src.pElement = nullptr;
				ElementSize = Src.ElementSize;
				Src.ElementSize = 0;
				pName = Src.pName;
				Src.pName = nullptr;

				Depth = Src.Depth;
				Src.Depth = 0;
				strcpy_s(XPATH, cMaxNodePath, Src.XPATH);
				Src.XPATH[0] = '\0';

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
			//  Destroys the XML Itearator
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//  NOTES:
			//  

			~XMLIterator() {
				pStartNode = nullptr;
				pEndNode = nullptr;
				pPosition = nullptr;
				pElement = nullptr;
				ElementSize = 0;

				Depth = 0;
				XPATH[0] = '\0';
				pName = nullptr;

				//  Boundary case - A nullptr XML Document
				if (pStartNode == nullptr) return;
				if (pStartNode[0] == '\0') return;

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

			//  getScope
			//
			//  This function will return a new iterator for the node identified by the passed tag.
			//	The Node IS within the scope of the current iterator position.
			//
			//  PARAMETERS:
			//
			//		char*				-		Const pointer to the tag name to locate.
			//
			//  RETURNS:
			//
			//		XMLIterator			-		A new (possibly nullptr) iterator positioned to the desired node
			//
			//  NOTES:
			//
			//  

			XMLIterator		getScope(const char* szTag) {
				const char*		pNode = nullptr;																	//  Pointer to the selected node
				const char*		pStartScope = pStartNode;															//  Start of search scope
				const char*		pEndScope = pEndNode;																//  End of search scope

				//  If the iterator has no current position
				if (pPosition == nullptr || pPosition == pStartNode || pPosition == pEndNode) {
					//  Safety - return a copy of the parent iterator
					if (szTag == nullptr) return XMLIterator(XPATH, pStartNode);
					if (szTag[0] == '\0') return XMLIterator(XPATH, pStartNode);
				}
				else {
					//  Safety - return an iterator on the current node
					if (szTag == nullptr) return XMLIterator(XPATH, pPosition);
					if (szTag[0] == '\0') return XMLIterator(XPATH, pPosition);

					//  Alter the search scope to the node at the current position
					pStartScope = pPosition;
					pEndScope = findCloseNode(pStartScope);

					if (pEndScope == nullptr || pEndScope == pStartScope || pEndScope == pEndNode) return XMLIterator(nullptr);
				}

				//  Search the scope of the current iterator for a node with the desired tag
				pNode = strstr(pStartScope, szTag);

				while (pNode != nullptr && pNode < pEndScope) {
					pNode--;
					if (*pNode == '<') {
						if (*(pNode + strlen(szTag) + 1) == ' ' || *(pNode + strlen(szTag) + 1) == '>') {
							//  Node is valid .. return an iterator for the node scope
							return XMLIterator(XPATH, pNode);
						}
					}

					pNode = strstr(pNode + 2, szTag);
				}

				//  Not found - return nullptr iterator
				return XMLIterator(nullptr);
			}

			//  isAtEnd
			//
			//  This function will indicates if the iterator has reached the end of scope. End-Of-Scope is positioned at the closing
			//	of the virtual root node.
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		bool				-		true if positioned at the end, otherwise false
			//
			//  NOTES:
			//
			//  

			bool	isAtEnd() {

				//  If no valid virtual root was established then signal End-Of-Scope
				if (pStartNode == nullptr) return true;

				//  If no valid position exists signal End-Of-Scope
				if (pPosition == nullptr) return true;

				//  If the current position is at the end node of the virtual root then signal End-Of-Scope
				if (pPosition == pEndNode) return true;

				//  Show not yet at end
				return false;
			}

			//  isNull
			//
			//  This function will indicate if the current node is null (not found or invalid)
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		bool				-		true if node is nullptr, otherwise false
			//
			//  NOTES:
			//
			//  

			bool	isNull() {
				//  If no valid position exists signal End-Of-Scope
				if (pPosition == nullptr) return true;
				return false;
			}

			//  isNode
			//
			//  This function will indicate if the current node is tagged with the passed tag
			//
			//  PARAMETERS:
			//
			//		char*				-		Const pointer to the tag
			//
			//  RETURNS:
			//
			//		bool				-		true if node is tagged with the padssed tag, otherwise false
			//
			//  NOTES:
			//
			//  

			bool	isNode(const char* szTag) {
				if (szTag == nullptr) return false;
				if (szTag[0] == '\0') return false;
				if (pName == nullptr) return false;
				if (strcmp(pName, szTag) == 0) return true;
				return false;
			}

			//  isClosing
			//
			//  This function will test if the current node is a closing node
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		bool				-		true if node is closing, otherwise false
			//
			//  NOTES:
			//
			//  

			bool	isClosing() {
				if (pPosition == nullptr) return false;
				if (pPosition[1] == '/') return true;
				return false;
			}

			//  getName
			//
			//  This function will return a pointer to the name (tag) of the current node
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		char*				-		Const pointer to the name of the current node
			//
			//  NOTES:
			//
			//  

			const char* getName() {
				if (pPosition == nullptr) return nullptr;
				return pName;
			}

			//  getPath
			//
			//  This function will return a pointer to the XPATH for the current node
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		char*				-		Pointer to the XPATH of the current node
			//
			//  NOTES:
			//
			//  

			char* getPath() {
				if (pPosition == nullptr) return nullptr;
				return XPATH;
			}

			//  getDepth
			//
			//  This function will return the nesting depth of the current XML Node
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		size_t				-		Depth
			//
			//  NOTES:
			//
			//  

			size_t getDepth() { return Depth; }

			//  getNode
			//
			//  This function will return a pointer to the current node
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		char*				-		Const pointer to the current node
			//
			//  NOTES:
			//
			//  

			const char* getNode() {
				return pPosition;
			}

			//  hasAttribute
			//
			//  This function will test if the current node has the given attribute.
			//
			//  PARAMETERS:
			//
			//		char*				-		Pointer to the name of the attribute
			//
			//  RETURNS:
			//
			//		bool				-		true if the attribute is present on the current node, otherwise false
			//
			//  NOTES:
			//
			//  

			bool	hasAttribute(const char* szAttrName) {
				size_t		AttrLen;
				if (getAttribute(szAttrName, AttrLen) != nullptr) return true;
				return false;
			}

			//  getAttribute
			//
			//  This function will return a pointer to the specified attribute value in the current node.
			//	The length of the attribute value is also returned.
			//
			//  PARAMETERS:
			//
			//		char*				-		Pointer to the name of the attribute
			//		size_t&				-		Reference to the variable to hold the length of the attribute value
			//
			//  RETURNS:
			//
			//		char*				-		Const pointer to the attribute value
			//
			//  NOTES:
			//
			//  

			const char* getAttribute(const char* szAttrName, size_t& AttrLen) {
				if (pPosition == nullptr) return nullptr;
				if (pPosition[1] == '/') return nullptr;
				return getAttribute(pPosition, szAttrName, AttrLen);
			}

			//  isAsserted
			//
			//  This function will return the boolean indicator if the named atribute is set to "yes" or "true".
			//	If the attribute is not present in the current node then it returns false.
			//
			//  PARAMETERS:
			//
			//		char*				-		Pointer to the name of the attribute
			//
			//  RETURNS:
			//
			//		bool				-		true if the named attribute is set to "true" or "yes" or "on" or "enable(d)", otherwise false
			//
			//  NOTES:
			//
			//  

			bool		isAsserted(const char* szAttrName) {
				const char*		pAValue = nullptr;
				size_t			AttrLen = 0;

				//  Get the pointer to the desiered attribute value
				pAValue = getAttribute(szAttrName, AttrLen);
				if (pAValue == nullptr || AttrLen == 0) return false;

				//  Check for the positive values
				if (AttrLen == 7 && _memicmp(pAValue, "enabled", 7) == 0) return true;
				if (AttrLen == 6 && _memicmp(pAValue, "enable", 6) == 0) return true;
				if (AttrLen == 4 && _memicmp(pAValue, "true", 4) == 0) return true;
				if (AttrLen == 3 && _memicmp(pAValue, "yes", 3) == 0) return true;
				if (AttrLen == 2 && _memicmp(pAValue, "on", 2) == 0) return true;

				return false;
			}

			//  getAttribute
			//
			//  This function will return a pointer to the specified attribute value in the current node.
			//	The length of the attribute value is also returned.
			//
			//  PARAMETERS:
			//
			//		chsr*				-		Const pointer to the node from which the attribute is to be extracted
			//		char*				-		Pointer to the name of the attribute
			//		size_t&				-		Reference to the variable to hold the length of the attribute value
			//
			//  RETURNS:
			//
			//		char*				-		Const pointer to the attribute value
			//
			//  NOTES:
			//
			//  

			static const char* getAttribute(const char* pNode, const char* szAttrName, size_t& AttrLen) {
				const char*			pENode = nullptr;														//  Pointer to the end of the node
				const char*			pAName = nullptr;														//  Pointer to the attribute name
				const char*			pValue = nullptr;														//  Pointer to the attribute value
				char				EQVal = 0;																//  Ending quote value

				AttrLen = 0;
				if (szAttrName == nullptr) return nullptr;
				if (szAttrName[0] == '\0') return nullptr;

				//  Get the pointer to the end of the node
				pENode = strchr(pNode, '>');
				if (pENode == nullptr) return nullptr;

				//  Find the attribute name
				pAName = strstr(pNode, szAttrName);
				if (pAName == nullptr || pAName > pENode) return nullptr;
				while (!isAttrName(pAName, szAttrName)) {
					pAName = strstr(pAName + 1, szAttrName);
					if (pAName == nullptr || pAName > pENode) return nullptr;
				}

				//  Find the start of the attribute value by skipping the assigment token
				pAName += strlen(szAttrName);
				while (*pAName == ' ' || *pAName == '=') pAName++;

				//  We should now be positioned at the start of the quoted string value
				if (*pAName == SCHAR_SQUOTE || *pAName == SCHAR_DQUOTE || *pAName == SCHAR_PSQUOTE) {
					//  Save the quoted string token
					EQVal = *pAName;
					if (EQVal == SCHAR_PSQUOTE) EQVal = SCHAR_PEQUOTE;
					pAName++;
					//  Save the pointer to the start of the value
					pValue = pAName;

					//  Determine the length of the attribute value
					while (*pAName != EQVal && *pAName != '>' && *pAName != '\0') {
						AttrLen++;
						pAName++;
					}

					if (*pAName == EQVal) return pValue;
					AttrLen = 0;
				}

				//  Invalid attribute value form
				return nullptr;
			}

			//  getAttributeInt
			//
			//  This function will return the integer value of the designated attribute
			//
			//  PARAMETERS:
			//
			//		char*				-		Pointer to the name of the attribute
			//
			//  RETURNS:
			//
			//		Int					-		Integer value of the attribute, 0 by default
			//
			//  NOTES:
			//
			//  

			int		getAttributeInt(const char* szAttrName) {
				const char* pAValue = nullptr;
				size_t			AttrLen = 0;

				//  Get the pointer to the desiered attribute value
				pAValue = getAttribute(szAttrName, AttrLen);
				if (pAValue == nullptr || AttrLen == 0) return 0;

				//  Return the integer value
				return atoi(pAValue);
			}

			//  getAttributeFloat
			//
			//  This function will return the floating point value of the designated attribute
			//
			//  PARAMETERS:
			//
			//		char*				-		Pointer to the name of the attribute
			//
			//  RETURNS:
			//
			//		double				-		Floating point value of the attribute, 0 by default
			//
			//  NOTES:
			//
			//  

			double		getAttributeFloat(const char* szAttrName) {
				const char* pAValue = nullptr;
				size_t			AttrLen = 0;

				//  Get the pointer to the desiered attribute value
				pAValue = getAttribute(szAttrName, AttrLen);
				if (pAValue == nullptr || AttrLen == 0) return 0;

				//  Return the floating point value
				return atof(pAValue);
			}

			//  getAttributeString
			//
			//  This function will return the null terminated string value of the designated attribute
			//
			//  PARAMETERS:
			//
			//		char*				-		Pointer to the name of the attribute
			//		char*				-		Pointer to the string buffer to receive the string value
			//		size_t				-		Size of the buffer to hold the string value
			//
			//  RETURNS:
			//
			//		size_t				-		Length of the string
			//
			//  NOTES:
			//
			//  

			size_t		getAttributeString(const char* szAttribute, char* pBfr, size_t BfrLen) {
				size_t			ASLen = 0;																	//  String attribute length
				const char*		pAttr = getAttribute(szAttribute, ASLen);									//  Pointer to attribute value

				//  Safety
				if (pBfr == nullptr) return 0;
				if (BfrLen == 0) return 0;
				pBfr[0] = '\0';
				if (pAttr == nullptr) return 0;

				//  If the string is too long - return nothing
				if (ASLen >= BfrLen) return 0;

				//  Copy the attribute value to the return buffer
				memcpy(pBfr, pAttr, ASLen);
				pBfr[ASLen] = '\0';

				//  Return the length of the string
				return ASLen;
			}

			//  copyAttribute
			//
			//  This function will return the names attribute value (null terminated) in a newly allocated buffer,
			//
			//  PARAMETERS:
			//
			//		char*				-		Pointer to the name of the attribute
			//
			//  RETURNS:
			//
			//		char*				-		Pointer to the buffer holding the value
			//
			//  NOTES:
			//
			//  

			char* copyAttribute(const char* szAttribute) {
				size_t			ASLen = 0;																	//  String attribute length
				const char*		pAttr = getAttribute(szAttribute, ASLen);									//  Pointer to attribute value
				char*			AVBfr = nullptr;															//  Buffer to hold the value

				if (pAttr == nullptr) return nullptr;
				if (ASLen == 0) return nullptr;

				//  Allocate the new buffer
				AVBfr = (char*)malloc(ASLen + 1);
				if (AVBfr == nullptr) return nullptr;

				//  Populate the buffer
				memcpy(AVBfr, pAttr, ASLen);
				AVBfr[ASLen] = '\0';

				//  Return the buffer
				return AVBfr;
			}

			//  getElementValue
			//
			//  This function will return a pointer to the value of the current element
			//	The length of the value is also returned.
			//
			//  PARAMETERS:
			//
			//		size_t&				-		Reference to the variable to hold the length of the elemnt value
			//
			//  RETURNS:
			//
			//		char*				-		Const pointer to the element value
			//
			//  NOTES:
			//
			//  

			const char* getElementValue(size_t& EValLen) {
				EValLen = ElementSize;
				return pElement;
			}

			//  getElementValue
			//
			//  This function will return a nullptr-terminated copy of the value of the current element
			//
			//  PARAMETERS:
			//
			//		char*				-		Pointer to the buffer that will hold the copy of the content
			//		size_t				-		Length of the buffer to hold the copy
			//
			//  RETURNS:
			//
			//  NOTES:
			//
			//  

			void	getElementValue(char* pBfr, size_t BfrLen) {
				const char*		pContent = nullptr;
				size_t			CSize = 0;

				if (pBfr == nullptr) return;
				if (BfrLen == 0) return;
				pBfr[0] = '\0';

				pContent = getElementValue(CSize);
				if (pContent == nullptr || CSize == 0) return;
				//  Truncate the content if it does not fit into the buffer
				if (CSize >= BfrLen) CSize = BfrLen - 1;
				memcpy(pBfr, pContent, CSize);
				pBfr[CSize] = '\0';
				return;
			}

			//  Prefix Increment operator ++I
			//
			//  This function will advance the iterator to the next position and return the updated iterator
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		XMLIterator		-		The updated iterator
			//
			//  NOTES:
			//
			//  

			XMLIterator		operator ++ () {
				//  Advance the iterator position
				advance();

				//  Return the updated iterator
				return *this;
			}

			//  Postfix Increment operator I++
			//
			//  This function will advance the iterator to the next position and return the original state of iterator
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//		XMLIterator		-		The original state of the iterator
			//
			//  NOTES:
			//
			//  

			XMLIterator		operator ++ (int) {
				XMLIterator			PreMod(*this);													//  Pre-modification copy of the iteratpor

				//  Advance the iterator position
				advance();

				//  Return the pre-modified iterator
				return PreMod;
			}

			//  copy assignment operator
			//
			//  This function will perform a copy assignent of the source iterator
			//
			//  PARAMETERS:
			//
			//		XMLIterator&		-		Const reference to the source iterator
			//
			//  RETURNS:
			//
			//		XMLIterator&		-		Reference to the updated iterator
			//
			//  NOTES:
			//
			//  

			XMLIterator& operator = (const XMLIterator& Src) {
				pStartNode = Src.pStartNode;
				pEndNode = Src.pEndNode;
				pPosition = Src.pPosition;
				pElement = Src.pElement;
				ElementSize = Src.ElementSize;
				pName = Src.pName;

				Depth = Src.Depth;
				strcpy_s(XPATH, cMaxNodePath, Src.XPATH);

				//  Return to caller
				return *this;
			}

			//  move assignment operator
			//
			//  This function will perform a move assignent of the source iterator
			//
			//  PARAMETERS:
			//
			//		XMLIterator&&		-		Const reference to the source iterator
			//
			//  RETURNS:
			//
			//		XMLIterator&		-		Reference to the updated iterator
			//
			//  NOTES:
			//
			// 

			XMLIterator& operator = (XMLIterator&& Src) noexcept {
				pStartNode = Src.pStartNode;
				Src.pStartNode = nullptr;
				pEndNode = Src.pEndNode;
				Src.pEndNode = nullptr;
				pPosition = Src.pPosition;
				Src.pPosition = nullptr;
				pElement = Src.pElement;
				Src.pElement = nullptr;
				ElementSize = Src.ElementSize;
				Src.ElementSize = 0;
				pName = Src.pName;
				Src.pName = nullptr;

				Depth = Src.Depth;
				Src.Depth = 0;
				strcpy_s(XPATH, cMaxNodePath, Src.XPATH);
				Src.XPATH[0] = '\0';

				//  Return to caller
				return *this;
			}

		private:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Members			                                                                                    *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			const char*			pStartNode;																	//  Starting node 
			const char*			pEndNode;																	//  Ending node
			const char*			pPosition;																	//  Current position in the hierarchy
			const char*			pElement;																	//  Pointer to the start of the current element value
			size_t				ElementSize;																//  Size of the current Element value

			size_t				Depth;																		//  Depth in the range being iterated
			const char*			pName;																		//  Pointer to the current node name
			char				XPATH[cMaxNodePath + 1];													//  XPATH storage

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Functions			                                                                                    *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  advance
			//
			//  This function will advance the position of the iterator to the next node
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//  NOTES:
			//
			//  

			void		advance() {
				const char*		pTemp = nullptr;																//  Working pointer
				bool			SCNode = false;																	//  Current node is self closing

				//  Safety
				if (pPosition == nullptr) return;

				//  Determine if the current node is self-closing
				pTemp = strchr(pPosition, '>');
				pTemp--;
				if (*pTemp == '/') SCNode = true;
				else SCNode = false;

				//  If this is a closing node then remove the lowest level of the XPATH
				pTemp = pPosition + 1;
				if (*pTemp == '/' || SCNode) {
					removeLastFromXPATH();
					Depth--;
				}

				//  Find the next node position
				pPosition = findNextNode(pPosition);
				if (pPosition == nullptr) {
					//  No position could be established - clear the position related information
					pElement = nullptr;
					ElementSize = 0;
					XPATH[0] = '\0';
					pName = nullptr;
					return;
				}

				//  Determine if the current node is self-closing
				pTemp = strchr(pPosition, '>');
				pTemp--;
				if (*pTemp == '/') SCNode = true;
				else SCNode = false;

				pTemp = pPosition + 1;
				if (*pTemp != '/') {
					appendToXPATH();
					Depth++;
					if (!SCNode) setElement();
				}

				//  Return to caller
				return;
			}

			//  setStartNode
			//
			//  This function will validate/adjust the starting node pointer
			//
			//  PARAMETERS:
			//
			//		char*				-		Const pointer to the presumptive start of the virtual root node
			//
			//  RETURNS:
			//
			//		char*				-		Const pointer to the definitive start of the virtual root node, nullptr if no valid node could be located
			//
			//  NOTES:
			//
			//  

			const char* setStartNode(const char* pNode) {
				if (pNode == nullptr) return nullptr;
				if (*pNode == '\0') return nullptr;
				pNode = strchr(pNode, '<');
				if (pNode == nullptr) return nullptr;

				//  Drop any excluded node types or closing nodes
				while (isExcluded(pNode) || isClosing(pNode)) pNode = findNextNode(pNode);

				//  Return the start address
				return pNode;
			}

			//  findCloseNode
			//
			//  This function will return the address of the end node or end-of-node for the passed start node.
			//
			//  PARAMETERS:
			//
			//		char*				-		Const pointer to the start node
			//
			//  RETURNS:
			//
			//		char*				-		Const pointer to the end or ending node, nullptr if none located
			//
			//  NOTES:
			//
			//  

			const char* findCloseNode(const char* pNode) {
				const char*		pTemp = nullptr;
				char			CloseNode[cMaxNodeName + 6] = {};											//  Close Node string
				size_t			TIndex = 0;																	//  Target index value

				if (pNode == nullptr) return nullptr;

				//
				//  If the node is self closing then return the pointer to passed node
				//
				pTemp = strchr(pNode, '>');
				if (pTemp == nullptr) return nullptr;
				pTemp--;
				if (*pTemp == '/') return pNode;

				//
				//  Build the closing node name for the current node
				//
				pTemp = pNode;
				pTemp++;
				CloseNode[TIndex++] = '<';
				CloseNode[TIndex++] = '/';
				while (*pTemp > ' ' && *pTemp != '>' && *pTemp != '\0') {
					CloseNode[TIndex++] = *pTemp;
					pTemp++;
				}
				CloseNode[TIndex++] = '>';
				CloseNode[TIndex++] = '\0';

				//  Attempt to locate the identified closing node
				pTemp = strstr(pTemp, CloseNode);
				if (pTemp == nullptr) return nullptr;

				//  Return the pointer to the closing node
				return pTemp;
			}

			//  setElement
			//
			//  This function will set the element description (address and size) for the current element
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//  NOTES:
			//
			//  

			void	setElement() {
				const char*		pEndOfNode = nullptr;

				pElement = nullptr;
				ElementSize = 0;

				if (pPosition == nullptr) return;

				pEndOfNode = findCloseNode(pPosition);

				if (pEndOfNode == nullptr) return;
				if (pPosition == pEndOfNode) return;

				pElement = strchr(pPosition, '>') + 1;

				ElementSize = pEndOfNode - pElement;
				if (ElementSize == 0) pElement = nullptr;

				//  Return to caller
				return;
			}

			//  appendToXPATH
			//
			//  This function will append the name of the current node to the XPATH
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//  NOTES:
			//
			//  

			void	appendToXPATH() {
				char*			pSOP = XPATH;																//  Start of parth
				size_t			PIndex = strlen(pSOP);														//  Index to the end of the path
				const char*		pNN = pPosition + 1;														//  Pointer to the node name

				if (pPosition == nullptr) return;

				//  If we are not at the start of the path then append a separator to the path
				if (PIndex > 0) pSOP[PIndex++] = '/';

				//  Set the pointer to the start of the name
				pName = pSOP + PIndex;

				//  Append the name to the path
				while (*pNN > ' ' && *pNN != '>' && PIndex < cMaxNodePath) {
					pSOP[PIndex++] = *pNN;
					pNN++;
				}

				//  Terminate the path name
				pSOP[PIndex] = '\0';

				//  Return to caller
				return;
			}

			//  removeLastFromXPATH
			//
			//  This function will remove the last node from the XPATH
			//
			//  PARAMETERS:
			//
			//  RETURNS:
			//
			//  NOTES:
			//
			//  

			void	removeLastFromXPATH() {
				char* pEOP = XPATH + strlen(XPATH);												//  End of the path

				//  Back up EOP to the last separator or the start of the path
				while (pEOP > XPATH && *pEOP != '/') pEOP--;

				//  Clear the last level
				*pEOP = '\0';

				//  Back up EOP to the last separator or the start of the path
				while (pEOP > XPATH && *pEOP != '/') pEOP--;
				if (*pEOP == '/') pName = pEOP + 1;
				else pName = pEOP;

				//  Return to caller
				return;
			}

			//  isExcluded
			//
			//  This function test to see if the passed node is a node that should be excluded.
			//
			//	Excluded Nodes:-
			//
			//		1.		XML Declaration
			//		2.		XML DTD (Document Type Definition)
			//		3.		XML Comment
			//
			//  PARAMETERS:
			//
			//		char*				-		Const pointer to the node to be tested
			//
			//  RETURNS:
			//
			//		bool				-		true if the node is one of the excluded types, otherwise false
			//
			//  NOTES:
			// 
			//  

			bool	isExcluded(const char* pNode) {
				if (pNode == nullptr) return false;

				//
				//  Exclude XML declaration
				//
				if (_memicmp(pNode, "<?xml", 5) == 0) return true;

				//
				//  Exclude XML DTD
				//
				if (_memicmp(pNode, "<!DOCTYPE ", 10) == 0) return true;

				//
				//  Exclude XML Comments
				//
				if (_memicmp(pNode, "<!--", 4) == 0) return true;

				//  NOT EXCLUDED
				return false;
			}

			//  isClosing
			//
			//  This function test to see if the passed node is a closing node
			//
			//  PARAMETERS:
			//
			//		char*				-		Const pointer to the node to be tested
			//
			//  RETURNS:
			//
			//		bool				-		true if the node is a closing node, otherwise false
			//
			//  NOTES:
			//
			//  

			bool	isClosing(const char* pNode) {
				if (pNode[1] == '/') return true;
				return false;
			}

			//  findNextNode
			//
			//  This function will locate the next start of an XML node
			//
			//  PARAMETERS:
			//
			//		char*				-		Const pointer to the start of an XML Node
			//
			//  RETURNS:
			//
			//		char*				-		Const pointer to the start of the next node, nullptr if none available
			//
			//  NOTES:
			//
			//  

			const char* findNextNode(const char* pNode) {

				//  Boundary
				if (pNode == nullptr) return nullptr;
				if (*pNode == '\0') return nullptr;

				//  Locate the node close
				pNode = strchr(pNode, '>');
				if (pNode == nullptr) return nullptr;

				//  Locate the next possible start
				pNode = strchr(pNode, '<');
				if (pNode == nullptr) return nullptr;

				//  Drop any excluded node types
				while (isExcluded(pNode)) pNode = findNextNode(pNode);

				//  Return the next node
				return pNode;
			}

			//  isAttrName
			//
			//  This function will test that we are pointing to a valid matching attrbute name
			//
			//  PARAMETERS:
			//
			//		char*				-		Const pointer to the possible matching attribute name
			//		char*				-		Const pointer to the attribute name string to be matched
			//
			//  RETURNS:
			//
			//		bool				-		true if the position is valid otherwise false
			//
			//  NOTES:
			//
			//		1.		We expect <name><white space><=> to qualify as a match
			//  

			static bool		isAttrName(const char* pAName, const char* szAttrName) {

				//  Safety
				if (pAName == nullptr || szAttrName == nullptr) return false;

				//  Back up to the first white space character
				while (*pAName != ' ') pAName--;
				//  Start of attribute name
				pAName++;

				//  Elimiate any white space following the attribute name
				pAName += strlen(szAttrName);
				while (*pAName == ' ') pAName++;

				//  Test for the '=' 
				if (*pAName == '=') return true;
				return false;
			}
		};


		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors			                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Default Constructor 
		//
		//  Constructs the XML Micro Parser with no underlying document
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		XMLMicroParser() {

			//  Clear persistent members
			pDoc = nullptr;
			pRoot = nullptr;
			pXMLDecl = nullptr;
			pXMLDTD = nullptr;
			XMLIsValid = false;

			//  Return to caller
			return;
		}

		//  Constructor 
		//
		//  Constructs the XML Micro Parser on an XML document loaded into memory
		//
		//  PARAMETERS:
		//
		//		char *			-		Const Pointer to XML Document (nullptr terminated)
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		XMLMicroParser(const char* szXMLDoc) {

			//  Load the passed document
			XMLIsValid = loadDocument(szXMLDoc);

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
		//  Destroys the XML Micro Parser
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		~XMLMicroParser() {

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

		//  isXML
		//
		//  Returns an indication if the passed pointer is pointing to an XML document
		//
		//  PARAMETERS:
		//
		//		const char*			-		Pointer to the document
		//
		//  RETURNS:
		//
		//		bool				-		true if the document looks like XML, otherwise false
		//
		//  NOTES:
		//
		//  

		static bool		isXML(const char* pDoc) {
			if (pDoc == nullptr) return false;
			if (pDoc[0] == '\0') return false;

			//  Scan to the start of significance
			while (*pDoc <= ' ' && *pDoc > '\0') pDoc++;
			if (*pDoc == '\0') return false;

			if (_memicmp(pDoc, "<?xml ", 6) == 0) return true;
			return false;
		}

		//  isValid
		//
		//  Returns the current validity state of the XML dcument
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//
		//  

		bool   isValid() { return XMLIsValid; }

		//  getScope
		//
		//  This function returns an iterator over the scope of the complete XML document
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		XMLIterator				-		Iterator over the complete document scope
		//
		//  NOTES:
		//  

		XMLIterator		getScope() {
			return XMLIterator(pRoot);
		}

		//  getScope
		//
		//  This function returns an iterator over the scope of the passed node name
		//
		//  PARAMETERS:
		//
		//		char*					-		Const pointer to the node name
		//
		//  RETURNS:
		//
		//		XMLIterator				-		Iterator over the complete document scope
		//
		//  NOTES:
		//  

		XMLIterator		getScope(const char* szTag) {
			const char* pNode = nullptr;																	//  Pointer to the selected node

			//  Safety
			if (szTag == nullptr) return XMLIterator(nullptr);
			if (szTag[0] == '\0') return XMLIterator(nullptr);

			//
			//  Search the complete document for a matching tag
			//

			pNode = strstr(pRoot, szTag);

			while (pNode != nullptr) {
				pNode--;
				if (*pNode == '<') {
					if (*(pNode + strlen(szTag) + 1) == ' ' || *(pNode + strlen(szTag) + 1) == '>') {
						//  Node is valid .. return an iterator for the node scope
						return XMLIterator(pNode);
					}
				}

				pNode = strstr(pNode + 2, szTag);
			}

			//  Not found
			return XMLIterator(nullptr);
		}

		//  loadDocument
		//
		//  Loads the passed XML document into the XML parser and sets the initial position
		//
		//  PARAMETERS:
		//
		//		cost char*				-		Const pointer to the XML document to be loaded (nullptr terminated)
		//
		//  RETURNS:
		//
		//		bool				-		true if the XML document is valid (so far)
		//
		//  NOTES:
		//
		//		Loading does not perform a complete validationof the XML document, only trivial checks are performed
		//  

		bool   loadDocument(const char* pXMLDoc) {
			XMLIterator				itX(nullptr);																//  Iterator

			//  Clear persistent members
			pDoc = nullptr;
			pRoot = nullptr;
			pXMLDecl = nullptr;
			pXMLDTD = nullptr;
			XMLIsValid = false;

			//  Safety checks
			if (pXMLDoc == nullptr) return false;
			if (pXMLDoc[0] == '\0') return false;

			//  Save the base document ppointer
			pDoc = pXMLDoc;

			//  Scan forward over whitespace
			while (*pXMLDoc > 0 && *pXMLDoc <= ' ') pXMLDoc++;
			if (*pXMLDoc == '\0') return false;

			//  Skip any XML Comments
			while (_strnicmp(pXMLDoc, "<!--", 4) == 0) {
				pXMLDoc = strstr(pXMLDoc, "-->");
				if (pXMLDoc == nullptr) return false;
				pXMLDoc += 3;
				//  Scan forward over whitespace
				while (*pXMLDoc > 0 && *pXMLDoc <= ' ') pXMLDoc++;
				if (*pXMLDoc == '\0') return false;
			}

			//  If we are pointing at an XML document definition then skip over it
			if (_strnicmp(pXMLDoc, "<?xml ", 6) == 0) {
				pXMLDecl = pXMLDoc;
				pXMLDoc = strstr(pXMLDoc, "?>");
				if (pXMLDoc == nullptr) return false;
				pXMLDoc += 2;
				//  Scan forward over whitespace
				while (*pXMLDoc > 0 && *pXMLDoc <= ' ') pXMLDoc++;
				if (*pXMLDoc == '\0') return false;
			}

			//  Skip any XML Comments
			while (_strnicmp(pXMLDoc, "<!--", 4) == 0) {
				pXMLDoc = strstr(pXMLDoc, "-->");
				if (pXMLDoc == nullptr) return false;
				pXMLDoc += 3;
				//  Scan forward over whitespace
				while (*pXMLDoc > 0 && *pXMLDoc <= ' ') pXMLDoc++;
				if (*pXMLDoc == '\0') return false;
			}

			//  If we are pointing and an XML Document Type Definition (DTD) then skip over it
			if (_strnicmp(pXMLDoc, "<!DOCTYPE ", 10) == 0) {
				pXMLDTD = pXMLDoc;
				pXMLDoc = strchr(pXMLDoc, '>');
				if (pXMLDoc == nullptr) return false;
				pXMLDoc++;
				//  Scan forward over whitespace
				while (*pXMLDoc > 0 && *pXMLDoc <= ' ') pXMLDoc++;
				if (*pXMLDoc == '\0') return false;
			}

			//  Skip any XML Comments
			while (_strnicmp(pXMLDoc, "<!--", 4) == 0) {
				pXMLDoc = strstr(pXMLDoc, "-->");
				if (pXMLDoc == nullptr) return false;
				pXMLDoc += 3;
				//  Scan forward over whitespace
				while (*pXMLDoc > 0 && *pXMLDoc <= ' ') pXMLDoc++;
				if (*pXMLDoc == '\0') return false;
			}

			//  We are now positioned at the root node of the XML document
			pRoot = pXMLDoc;

			//
			//  Perform an iteration over the XML document -- if the document is valid this should end up at the end-root node
			//
			for (itX = XMLIterator(pRoot); !itX.isAtEnd(); itX++);

			pXMLDoc = itX.getNode();
			if (pXMLDoc == nullptr) return false;
			if (*pXMLDoc != '<') return false;
			pXMLDoc++;
			if (*pXMLDoc != '/') return false;
			pXMLDoc++;

			return true;
		}

	//
	//  Static Functions for handling XML documents without using an XMLIterator
	//

	//  findAttribute
	//
	//  Returns the pointer to the value of the specified attribute in the passed node
	//
	//  PARAMETERS:
	//
	//		char *			-		Const pointer to the node containing (or not) the attribute
	//		char *			-		Const Name of the attribute that is to be located
	//		size_t			-		Pointer to a size_t to hold the length of the attribute value
	//
	//  RETURNS:
	//
	//		char *			-		Const pointer to the attribute value, NULL if the attribute was not found
	//
	//  NOTES:
	//  

		static const char* findAttribute(const char* pNode, const char* szAttrName, size_t* pAVLen) {
			const char*		pEndOfNode = nullptr;													//  Pointer to the end of the node
			const char*		pAttr = nullptr;														//  Candidate pointer to attribute name
			char			Delim = 0;																//  Delimiter
			const char*		pStartOfValue = nullptr;												//  Start of value

			//  Safety checks
			if (pAVLen == nullptr) return nullptr;
			*pAVLen = 0;
			if (pNode == nullptr) return nullptr;
			if (*pNode != '<') return nullptr;
			pEndOfNode = strchr(pNode, '>');
			if (pEndOfNode == nullptr) return nullptr;
			if (szAttrName == nullptr) return nullptr;
			if (szAttrName[0] == '\0') return nullptr;

			//  Find the first candidate for the attribute
			pAttr = st_stristr(pNode, szAttrName);

			//  Qualify the candidate
			while (pAttr != nullptr && pAttr < pEndOfNode) {

				//  Determine if this is the attribute
				if (pAttr[strlen(szAttrName)] == '=') {
					pAttr += (strlen(szAttrName) + 1);
					Delim = *pAttr;
					if (Delim == SCHAR_PSQUOTE) Delim = SCHAR_PEQUOTE;
					pAttr++;
					pStartOfValue = pAttr;

					//  Determine the length of the value
					while (*pAttr != Delim && *pAttr != '\0') {
						(*pAVLen)++;
						pAttr++;
					}

					//  BAD encounter with the end of the document
					if (*pAttr == '\0') return nullptr;

					//  Return the pointer to the attribute
					return pStartOfValue;
				}

				//  Move on to the next candidate
				pAttr = st_stristr(pAttr + strlen(szAttrName), szAttrName);
			}

			//  Attribute Not Found
			return nullptr;
		}

		//  extractAttribute
		//
		//  Extracts the content of the specified attribute into the passed buffer
		//
		//  PARAMETERS:
		//
		//		char *			-		Const pointer to the node containing (or not) the attribute
		//		char *			-		Const Name of the attribute that is to be located
		//		char *			-		Pointer to the buffer that the value will be extracted into
		//		size_t			-		Size of the buffer
		//
		//  RETURNS:
		//
		//		bool			-		true if the value was extracted, otherwise false
		//
		//  NOTES:
		//  

		static bool extractAttribute(const char* pNode, const char* szAttrName, char* pBfr, size_t BfrSize) {
			const char* pValue = nullptr;														//  Pointer to the value
			size_t		AttrLen = 0;															//  Length of the attribure value

			//  Safety checks
			if (pBfr == nullptr) return false;
			pBfr[0] = '\0';

			//  Find the attribute in the node
			pValue = findAttribute(pNode, szAttrName, &AttrLen);

			//  If the attribute was not found signal the caller
			if (pValue == nullptr) return false;

			//  Check for buffer too small
			if (AttrLen >= BfrSize) return false;

			//  Return the value
			strncpy_s(pBfr, BfrSize, pValue, AttrLen);
			pBfr[AttrLen] = '\0';

			//  Show that the value was returned
			return true;
		}

		//  copyAttribute
		//
		//  Extracts the content of the specified attribute into a newly allocated buffer and returns the buffer to the caller
		//
		//  PARAMETERS:
		//
		//		char *			-		Const pointer to the node containing (or not) the attribute
		//		char *			-		Const Name of the attribute that is to be located
		//		char *			-		Pointer to the buffer that the value will be extracted into
		//		size_t			-		Size of the buffer
		//
		//  RETURNS:
		//
		//		char *			-		Pointer to the buffer holding the attributte value, NULL if not found
		//
		//  NOTES:
		//
		//		It is the callers responsibility to free the returned buffer when they have finished with it.
		//  

		static char* copyAttribute(const char* pNode, const char* szAttrName) {
			const char* pValue = nullptr;															//  Pointer to the value
			size_t		AttrLen = 0;																//  Length of the attribure value
			char*		VBfr = nullptr;																//  Value buffer

			//  Find the attribute in the node
			pValue = findAttribute(pNode, szAttrName, &AttrLen);

			//  If the attribute was not found signal the caller
			if (pValue == nullptr) return nullptr;

			//  Allocate a buffer to hold the returned value (NULL terminated)
			VBfr = (char*)malloc(AttrLen + 1);
			if (VBfr == nullptr) return nullptr;

			//  Return the value
			strncpy_s(VBfr, AttrLen + 1, pValue, AttrLen);
			VBfr[AttrLen] = '\0';

			//  Return the buffer to the caller
			return VBfr;
		}

		//  findContent
		//
		//  Returns the pointer to the content of the passed node and the length of the content
		//
		//  PARAMETERS:
		//
		//		char *			-		Const pointer to the node containing (or not) the content
		//		size_t			-		Pointer to a size_t to hold the length of the attribute value
		//
		//  RETURNS:
		//
		//		char *			-		Const pointer to the content value, NULL if there is no content
		//
		//  NOTES:
		//  

		static const char* findContent(const char* pNode, size_t* pAVLen) {
			const char*		pEndOfNode = nullptr;														//  Pointer to the end of the node
			const char*		pScan = pNode;																//  Scanning pointer
			int				iIndex = 0;																	//  Generic index
			char			SearchString[XMP_MAX_NODE_NAME + 6] = {};									//  Generic search string

			//  Safety checks
			if (pAVLen == nullptr) return nullptr;
			*pAVLen = 0;
			if (pNode == nullptr) return nullptr;
			if (*pNode != '<') return nullptr;
			pEndOfNode = strchr(pNode, '>');
			if (pEndOfNode == nullptr) return nullptr;
			//  Check for stand alone node
			if (*(pEndOfNode - 1) == '/') return nullptr;

			//  Buuld the name of the closing node
			SearchString[0] = '<';
			SearchString[1] = '/';
			pScan++;
			iIndex = 2;
			while (*pScan > ' ' && *pScan != '>') {
				SearchString[iIndex] = *pScan;
				iIndex++;
				pScan++;
			}
			if (*pScan != ' ' && *pScan != '>' && iIndex != 2) return nullptr;
			SearchString[iIndex] = '>';
			iIndex++;
			SearchString[iIndex] = '\0';

			//  Find the closing node entry
			pScan = st_stristr(pNode, SearchString);
			if (pScan == nullptr) return nullptr;
			pEndOfNode++;

			if (pEndOfNode == pScan) return nullptr;

			//  Compute the length
			*pAVLen = pScan - pEndOfNode;

			//  Return the pointer to the content
			return pEndOfNode;
		}

		//  extractContent
		//
		//  Extracts the content of the specified node into the passed buffer
		//
		//  PARAMETERS:
		//
		//		char *			-		Const pointer to the node containing (or not) the content
		//		char *			-		Pointer to the buffer that the value will be extracted into
		//		size_t			-		Size of the buffer
		//
		//  RETURNS:
		//
		//		bool			-		true if the value was extracted, otherwise false
		//
		//  NOTES:
		//  

		static bool extractContent(const char* pNode, char* pBfr, size_t BfrSize) {
			const char* pValue = nullptr;														//  Pointer to the value
			size_t		AttrLen = 0;															//  Length of the attribure value

			//  Safety checks
			if (pBfr == nullptr) return false;
			pBfr[0] = '\0';

			//  Find the content in the node
			pValue = findContent(pNode, &AttrLen);

			//  If the content was not found signal the caller
			if (pValue == nullptr) return false;

			//  Check for buffer too small
			if (AttrLen >= BfrSize) return false;

			//  Return the value
			strncpy_s(pBfr, BfrSize, pValue, AttrLen);
			pBfr[AttrLen] = '\0';

			//  Show that the value was returned
			return true;
		}

		//  copyContent
		//
		//  Extracts the content of the specified node into a newly allocated buffer and returns the buffer to the caller
		//
		//  PARAMETERS:
		//
		//		char *			-		Const pointer to the node containing (or not) the content
		//		char *			-		Pointer to the buffer that the value will be extracted into
		//		size_t			-		Size of the buffer
		//
		//  RETURNS:
		//
		//		char *			-		Pointer to the buffer holding the attributte value, NULL if not found
		//
		//  NOTES:
		//
		//		It is the callers responsibility to free the returned buffer when they have finished with it.
		//  

		static char* copyContent(const char* pNode) {
			const char* pValue = nullptr;														//  Pointer to the value
			size_t		AttrLen = 0;															//  Length of the attribure value
			char*		VBfr = nullptr;															//  Value buffer

			//  Find the content in the node
			pValue = findContent(pNode, &AttrLen);

			//  If the attribute was not found signal the caller
			if (pValue == nullptr) return nullptr;

			//  Allocate a buffer to hold the returned value (nullptr terminated)
			VBfr = (char*)malloc(AttrLen + 1);
			if (VBfr == nullptr) return nullptr;

			//  Return the value
			strncpy_s(VBfr, AttrLen + 1, pValue, AttrLen);
			VBfr[AttrLen] = '\0';

			//  Return the buffer to the caller
			return VBfr;
		}

	private:
		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members			                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Internal Pointers
		const char* pDoc;																			//  Pointer to the XML Document
		const char* pRoot;																			//  Pointer to the XML Root Node
		const char* pXMLDecl;																		//  Pointer to the XML declaration
		const char* pXMLDTD;																		//  Pointer to the XML DTD node

		//  Validity Flag
		bool		XMLIsValid;																		//  Validity of the XML Document

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************


	};

}
