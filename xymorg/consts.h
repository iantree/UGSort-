#pragma once
//
//  Suite:  XYMORG Generic Application Support
//
//  Common constant definitions
//

#include	"types.h"																//  Types

//
//  SIZES
//

//
//  MAX_PATH is defined here if it has not already been definied by the platform and represents the maximum expected size of file paths used in the
//  suite.
//
//  THIS VALUE SHOULD BE REVIEWED IN ALL DEPLOYMENTS
//

#ifndef MAX_PATH
#define		MAX_PATH				256												//  Maximum file path length
#endif

//  Names
#define		MAX_APP_NAME			32												//  Maximum size of an application name

//  Collection Related item sizes
#define		MAX_COLL_TYPE			64												//  Max size of a collection type
#define		MAX_COLL_NAME			64												//  Max size of a collection name
#define		MAX_COLL_VER			32												//  Max size of a collection version
#define		MAX_CKEY_SIZE			128												//  Max size of a collection key
#define		MAX_CPATH				256												//  Max size of collection file paths used
#define		MAX_CNODE_NAME			32												//  Max size of a node name
#define		MAX_CATTR_NAME			32												//  Max size of an attribute name
#define		MAX_CATTR_VALUE			512												//  Max size of an attribute value

//  URL component sizes
#define		MAX_URL_STRING			512												//  Max size for an URL string
#define		MAX_URL_SCHEME			32												//  Max size for an URL scheme
#define		MAX_URL_USER			64												//  Max size for an URL use name
#define		MAX_URL_PASSWORD		64												//  Max size for an URL password
#define		MAX_URL_PORT			10												//  Max size for an URL port identifier
#define		MAX_URL_DOMAIN			MAX_PATH										//  Max size for an URL domain name
#define		MAX_URL_URIPATH			MAX_PATH										//  Max size for a URI Path
#define		MAX_URL_RESOURCE		MAX_PATH										//  Max size for an URL resource name
#define		MAX_URL_QUERY			MAX_PATH										//  Max size for an URL query string
#define		MAX_URL_ANCHOR			64												//  Max size for an URL anchor name

//  EMail Address component sizes
#define		MAX_ADDR_STRING			256												//  Max size of an e-Mail address string
#define		MAX_ADDR_MONIKER		128												//  Max size of an e-Mail address moniker
#define		MAX_ADDR_LOCAL_PART		128												//  Max size of an e-Mail address local part
#define		MAX_ADDR_DOMAIN			128												//  Max size of an e-mail address domain

//
//  NETWORK OS global constants
//

#define		NETOS_SCOPE_LOCAL_MACHINE			1									//  Local machine only
#define		NETOS_SCOPE_LAN						2									//  Local Area Network Only
#define		NETOS_SCOPE_WAN						4									//  Wide Area Network and Internet
#define		NETOS_SCOPE_ALL						8									//  Indiscriminate scope
#define		NETOS_SCOPE_LINKLOCAL				16									//  Link Local only (special use)

//
//  Standard Strings
//

#define		TRUE_PVAL					"true"										//  true
#define		FALSE_PVAL					"false"										//  false
#define		YES_PVAL					"yes"										//  yes (alias for true)
#define		NO_PVAL						"no"										//  no (alias for false)

//
//  SPECIAL CHARACTERS
//

#define		SCHAR_CR				13												//  Carriage Return
#define		SCHAR_LF				10												//  Line Feed
#define		SCHAR_TAB				9												//  Tab character
#define		SCHAR_CPLUS				'+'												//  Plus sign
#define		SCHAR_SQUOTE			'\''											//  Single quote
#define		SCHAR_DQUOTE			'"'												//  Double quote
#define		SCHAR_PSQUOTE			'\x93'											//  Start of Paired quotes
#define		SCHAR_PEQUOTE			'\x94'											//  End of Paired quotes
#define		SCHAR_ASSIGN			'='												//  Assignment token
#define		SCHAR_ITEMREF			'#'												//  Item reference
#define		SCHAR_PCT				'%'												//  Percent qualifier
#define		SCHAR_EOS				'\0'											//  End-Of-String token
#define		SCHAR_LS				','												//  Default list separator
#define		SCHAR_OB				'('												//  Open default bracket
#define		SCHAR_OSB				'['												//  Open square bracket
#define		SCHAR_OCB				'{'												//  Open curly brackets
#define		SCHAR_OAB				'<'												//  Open angled brackets
#define		SCHAR_CB				')'												//  Close default brackets
#define		SCHAR_CSB				']'												//  Close square brackets
#define		SCHAR_CCB				'}'												//  Close curly brackets
#define		SCHAR_CAB				'>'												//  Close angled brackets
#define		SCHAR_NEG				'-'												//  Negative integer qualifier
#define		SCHAR_COLON				':'												//  Colon marker
#define		SCHAR_TILDE				'~'												//  Tilde separator
#define		SCHAR_FS				'/'												//  Forward slash separator
#define		SCHAR_BS				'\\'											//  Backwasd slash separator

//
//  DEFAULT FORMATS
//

#define	DEFAULT_LOGNAME_TIMESTAMP_FMT		"%Y%m%d-%H%M%S"							//  Default format for the timestamp format part of the log file name
#define DEFAULT_LOGNAME_FMT					"Logs/%s-%s.log"						//  Default format for the log file name. substitutiona are timestamp and application name
#define DEFAULT_LOG_TIMESTAMP_FMT			"%Y/%m/%d %H:%M:%S"						//  Default format for the timestamp on log records
#define INET_TIME_FMT						"%a, %d %b %Y %T %Z"					//  Internet Time Format
#define DEFAULT_DATE_DISPLAY				"%d/%m/%Y"								//  Default format for displaying dates

//
//  SPECIAL VALUES
//

#define		NULLSTRREF		(xymorg::STRREF) 0										//  Null String reference
#define		NULLOBJREF		(xymorg::OBJREF) 0										//  Null Object reference

//
//  STRUCTURAL DEFAULTS
//

#define		XY_DEFAULT_MAX_THREADS			33										//  Default hard maximum threads in threadpool

//
//  CARDINAL POINTS, DIRECTION VECTORS, IMAGE PLACEMENT
//

#define		CARDINAL_NONE					0
#define		CARDINAL_NORTH					1
#define		TOP_CENTER						CARDINAL_NORTH
#define		CARDINAL_NORTHEAST				2
#define		TOP_RIGHT						CARDINAL_NORTHEAST
#define		CARDINAL_EAST					3
#define		MIDDLE_RIGHT					CARDINAL_EAST
#define		CARDINAL_SOUTHEAST				4
#define		BOTTOM_RIGHT					CARDINAL_SOUTHEAST
#define		CARDINAL_SOUTH					5
#define		BOTTOM_CENTER					CARDINAL_SOUTH
#define		CARDINAL_SOUTHWEST				6
#define		BOTTOM_LEFT						CARDINAL_SOUTHWEST
#define		CARDINAL_WEST					7
#define		MIDDLE_LEFT						CARDINAL_WEST
#define		CARDINAL_NORTHWEST				8
#define		TOP_LEFT						CARDINAL_NORTHWEST

//
//  CRYPTOGRAPHIC DEFINITIONS
//

#define		XY_MIN_ENC_KEY_PHRASE_LEN		32										//  Minimum size of an encryption key phrase

#define		DEFAULT_SEC_CFG_SCHEME			0										//  Secure configuration encryption scheme
#define		DEFAULT_SEC_CFG_KSIZE			16										//  Secure configuration encryption key size
#define		DEFAULT_SEC_CFG_KEY				{}										//  Secure configuration encryption key 

//  COMPUTATIONAL CONSTANTS
#define		Pi						3.14159265										//  Pi
#define		RADS(x)					((x * Pi) / 180.0)								//  Degrees to radians
#define		DEGS(x)					((x * 180.0) / Pi)								//  Radians to degrees
