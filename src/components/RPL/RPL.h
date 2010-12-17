/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		RPL.h														*/
/*																			*/
/*	Content:	Common definitions, constants and macroses for this project	*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2009 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#ifdef _DEBUG
using namespace System::Diagnostics;
#endif


//
// Define error strings
//
#define ERR_INVALID_TYPE													\
	"Type {0} is not supported."
#define ERR_CAST_FROM_TO													\
	"Specified cast is not valid: from {0} to {1}."
#define ERR_COMPARISON_TYPE													\
	"Only {0} comparison for value of type {1} is allowed."
#define ERR_SUPPORTED_OPERATION												\
	"Operation isn't supported by {0}."
#define ERR_OBJECT_EXISTS													\
	"An object already exists in the collection."
#define ERR_PERFORM_OPERATION												\
	"Cann't perform operation for {0}."
#define ERR_LESS_THEN_ZERRO													\
	"Value can't be less then zerro."
#define ERR_OBJECT_HEADER													\
	"Invalid object header."
#define ERR_OBJECT_FACTORY													\
	"ObjectFactory has not been initialized."
#define ERR_OBJECT_CREATION													\
	"ObjectFactory returned null reference while creating {0}."
#define ERR_BROKER_OPENED													\
	"Persistent mechanism has been already opened."
#define ERR_BROKER_CLOSED													\
	"Persistent mechanism is closed."
#define ERR_BROKER_DISCONNECTED												\
	"Operation is not allowed while in disconnected state."
#define ERR_STREAM_CLOSED													\
	"Cannot access a closed stream."
#define ERR_STREAM_LENGTH													\
	"Stream is to big for serialization."
#define ERR_STREAM_ASSIGN													\
	"Instance of the Stream cann't be assigned to multiple properties."
#define ERR_LOG_STATES														\
	"ERROR! Incompatible log record states: from {0} to {1}."
#define ERR_DELETE_FILE														\
	"ERROR! Cann't delete file '{0}': {1}"
#define ERR_DISPOSE															\
	"ERROR! Object {0} dispose failed: {1}"


//
// Define namespace macros definition. All public classes
// must be defined in this namespace.
//
#define	_RPL			Toolkit::RPL
#define	_RPL_BEGIN		namespace Toolkit{namespace RPL{
#define	_RPL_END		}}


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
// Define macro to output formated error message
//
#ifdef _DEBUG   
	#define dbgprint( msg ) {															\
		Debug::Print( "{0} " + (String::IsNullOrEmpty(msg) ? "" : "\n\t{1}") + "\n",	\
					  __FUNCSIG__ , String::Copy(msg)->Replace("\n", "\n\t") );			\
	}
#else
	#define dbgprint( msg )
#endif
