/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		IBrokerFactory.h											*/
/*																			*/
/*	Content:	Definition of Factories::IBrokerFactory interface			*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "..\RPL.h"


_RPL_BEGIN
namespace Factories {
	ref class PersistenceBroker;

	/// <summary>
	/// Encapsulates the behavior needed for broker creation.
	/// </summary><remarks>
	/// It can be usefull to define custom creation algorithm for
	/// broker. For example, you want create all instances of
	/// PersistenceBroker in separated domains.
	/// </remarks>
	public interface class IBrokerFactory
	{
		/// <summary>
		/// Returns new instance of the PersistenceBroker class.
		/// </summary>
		PersistenceBroker^ CreateInstance( void );
	};
}_RPL_END
