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
// must be difined in this namespace.
//
#define	_RPL_BEGIN		namespace RPL {
#define	_RPL_END		}
#define	_RPL			using namespace RPL;


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