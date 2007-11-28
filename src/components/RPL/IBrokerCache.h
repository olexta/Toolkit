/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		IBrokerCache.h												*/
/*																			*/
/*	Content:	Definition of IBrokerCache interface.						*/
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
ref class PersistentObject;

namespace Storage {
	/// <summary>
	/// Encapsulates the behavior needed for broker's cache search.
	/// </summary><remarks>
	/// Search function return object from cache (in case of using second
	/// signature it will be up-to-date) if it exists. In other case null
	/// reference will be returned. This interface is used by persistent
	/// storage class representation in processing of PersistentCriteria.
	/// </remarks>
	public interface class IBrokerCache
	{
		PersistentObject^ Search( int id, String ^type );
		PersistentObject^ Search( int id, String ^type,
								  DateTime stamp, String ^name );
	};
}_RPL_END