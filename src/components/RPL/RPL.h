/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		RPL.h														*/
/*																			*/
/*	Content:	Common definitions, constants and macroses for this project	*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#ifdef _DEBUG
using namespace System::Diagnostics;
#endif


//
// Define namespace macros definition. All public classes
// must be defined in this namespace.
//
#define	_RPL			Toolkit::RPL
#define	_RPL_BEGIN		namespace Toolkit{namespace RPL{
#define	_RPL_END		}}
#define _COLLECTIONS	Toolkit::Collections;


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
	#define dbgprint( m )
#endif