/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Collections											*/
/*																			*/
/*	Module:		Collections.h												*/
/*																			*/
/*	Content:	Common definitions, constants and macroses for this project	*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once


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