/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		IObjectFactory.h											*/
/*																			*/
/*	Content:	Definition of Factories::IObjectFactory interface			*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "..\RPL.h"

using namespace System;


_RPL_BEGIN
ref class PersistentObject;

namespace Factories {
	/// <summary>
	/// Encapsulates the behavior needed for object creation.
	/// </summary>
	public interface class IObjectFactory
	{
		/// <summary>
		/// Returns new instance of the specified type of object.
		/// </summary><remarks>
		/// This function calls every time new object retrieved from
		/// storage.
		/// </remarks>
		PersistentObject^ CreateInstance( String ^type, int id, \
										  DateTime stamp, String ^name );
	};
}_RPL_END
