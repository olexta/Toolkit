/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		ITransactionSupport.h										*/
/*																			*/
/*	Content:	Definition of ITransactionSupport interface.				*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
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
private interface class ITransactionSupport
{
	void TransactionBegin( void );
	void TransactionCommit( void );
	void TransactionRollback( void );
};
_RPL_END