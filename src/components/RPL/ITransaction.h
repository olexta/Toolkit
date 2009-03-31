/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		ITransactiont.h												*/
/*																			*/
/*	Content:	Definition of ITransaction interface.						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008 Alexey Tkachuk								*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"

using namespace System;


_RPL_BEGIN
/// <summary>
/// Encapsulates the behavior needed for transaction support.
/// </summary><remarks>
/// All objects that support transactions must implement this interface.
/// </remarks>
private interface class ITransaction
{
	void Begin( void );
	void Commit( void );
	void Rollback( void );
};
_RPL_END
