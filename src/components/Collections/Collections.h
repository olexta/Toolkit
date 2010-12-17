/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Collections											*/
/*																			*/
/*	Module:		Collections.h												*/
/*																			*/
/*	Content:	Common definitions, constants and macroses for this project	*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once


//
// Define error messages
//
#define ERR_ENUM_EXEC														\
	"Collection was modified; enumeration operation may not execute."
#define ERR_ENUM_NOT_STARTED												\
	"Enumeration has either not started or has already finished."
#define ERR_ITEM_EXISTS														\
	"An item with the same key already exists."
#define ERR_DUBLICATE_KEY													\
	"Dublicate key exists in input collection."
#define ERR_KEY_NOT_FOUND													\
	"The given key was not present in the collection."
#define ERR_OUT_OF_RANGE													\
	"Number was less than the array's lower bound in the first dimension."
#define ERR_ARRAY_TOO_SMALL													\
	"Destination array was not long enough. Check destIndex and length, "	+\
	"and the array's lower bounds."


//
// Define namespace macros definition. All public classes
// must be defined in this namespaces.
//
#define _COLLECTIONS			Toolkit::Collections
#define _COLLECTIONS_BEGIN		namespace Toolkit{namespace Collections{
#define _COLLECTIONS_END		}}

#define	_BINARY_TREE			_COLLECTIONS::BinaryTree
#define _BINARY_TREE_BEGIN		_COLLECTIONS_BEGIN namespace BinaryTree{
#define _BINARY_TREE_END		_COLLECTIONS_END}


//
// Define timeout for operations.
//
#ifdef _DEBUG
	#define TIMEOUT	10000
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
