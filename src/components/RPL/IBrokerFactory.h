/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		IBrokerFactory.h											*/
/*																			*/
/*	Content:	Definition of IBrokerFactory interface.						*/
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
ref class CPersistenceBroker;

/// <summary>
/// Encapsulates the behavior needed for broker creation.
/// </summary><remarks>
///	It can be usefull to define custom creation algorithm for broker. For
/// example, you want create all instances of CPersistenceBroker in separated
/// domains.
/// </remarks>
public interface class IBrokerFactory
{
	CPersistenceBroker^ CreateInstance( void );
};
_RPL_END