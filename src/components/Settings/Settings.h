**************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		Settings.h													*/
/*																			*/
/*	Content:	Common definitions, constants and macroses for this project	*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008 Alexey Tkachuk								*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once


//
// Define error messages
//
#define ERR_STR_CONTAINS												\
	"{0} cann't contain any of the following strings: '{1}'."
#define ERR_PATH_STARTS											\
	"String '{0}' is not valid {1} path: must starts with '{2}'."
#define ERR_EMPTY_NAME													\
	"Name of the {0} cann't be empty string."
#define ERR_RESERVED_NAME												\
	"Names '{0}' and '{1}' are reserved."
#define ERR_PATH_NOT_FOUND												\
	"Path not found: '{0}'."
#define ERR_INVALID_TYPE												\
	"Type '{0}' is not supported."
#define ERR_CAST_FROM_TO												\
	"Specified cast is not valid: from '{0}' to '{1}'."
#define ERR_NODE_PARENT													\
	"Object of type '{0}' cann't be parent node to '{1}'."
#define ERR_NODE_OPERATION												\
	"Operation is not allowed for this type of node: '{0}'."
#define ERR_ADP_ACCEPT_PATH												\
	"Adapter '{0}' doesn't accept any of the following strings in the "	+\
	"path: '{1}'."
#define ERR_ADP_REMOVE													\
	"Adapter cann't be added/removed through Child's routines. Use "	+\
	"Manager::Add and Manager::Remove instead."
#define ERR_ITEM_CHAIN_PATH												\
	"Create chain failed: invalid path."
#define ERR_ITEM_EXISTS													\
	"An item with the same key already exists."
#define ERR_ITEM_PARENT_STATE											\
	"'{0}' item cann't be parent to '{1}' item."
#define ERR_ITEM_STATE													\
	"Invalid state of Item: '{0}'."
#define ERR_ITEM_SYNC													\
	"Cann't synchronize with adapter while switching from '{0}' to "	+\
	"'{1}' state."
#define ERR_ITEM_SYNC_STATE												\
	"Unsuitable state for items's synchronization with adapter."
#define ERR_ROOT_NAME													\
	"Name of the root must be empty string."
#define ERR_ROOT_LOCATE													\
	"Cann't locate root node."
#define ERR_ROOT_PARENT													\
	"Root cann't have parent."
#define ERR_ROOT_REMOVE													\
	"Cann't remove root node."
#define ERR_INI_FILE													\
	"INI file is corrupt. A key cann't contain any of the following "	+\
	"strings: '{0}'."
#define ERR_INI_WRITE													\
	"Write to INI '{0}' failed. Error code #{1}."
#define ERR_REGISTRY_KEY												\
	"'{0}' is not valid registry key."
#define ERR_REGISTRY_ROOT												\
	"'{0}' cann't be used as root for settings."


//
// Define namespace macros definition. All public classes
// must be defined in this namespaces.
//
#define _SETTINGS			Toolkit::Settings
#define _SETTINGS_BEGIN		namespace Toolkit{namespace Settings{
#define _SETTINGS_END		}}


//
// Define timeout for operations.
//
#ifdef _DEBUG
	#define TIMEOUT	30000
#else
	#define TIMEOUT -1
#endif


//
// Define lock macroses.
//
#define ENTER_READ(lock)		lock->AcquireReaderLock(TIMEOUT); try {
#define EXIT_READ(lock)			} finally { lock->ReleaseReaderLock(); }
#define ENTER_WRITE(lock)		lock->AcquireWriterLock(TIMEOUT); try {
#define EXIT_WRITE(lock)		} finally { lock->ReleaseWriterLock(); }


//
// Define integer, double and datetime formating.
//
#define INT_F		"0"
#define DOUBLE_F	"0.0##############"
#define DATETIME_F	"dd/MM/yyyy HH:mm:ss"
//
// Initialize custom CultureInfo for unambiguous converting to/from string.
// Set specific number and datetime formatting settings.
//
#define INIT_CI(ci)																\
	ci = dynamic_cast<CultureInfo^>( CultureInfo::CurrentCulture->Clone() );	\
	ci->NumberFormat->PositiveSign = "+";										\
	ci->NumberFormat->NegativeSign = "-";										\
	ci->NumberFormat->NumberDecimalSeparator = ".";								\
	ci->DateTimeFormat->DateSeparator = ".";									\
	ci->DateTimeFormat->TimeSeparator = ":";
