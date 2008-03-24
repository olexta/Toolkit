/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		Settings.h													*/
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