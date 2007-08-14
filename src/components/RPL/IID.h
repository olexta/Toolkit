/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		IID.h														*/
/*																			*/
/*	Content:	Definition of IID interface.								*/
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
/// Encapsulates the behavior needed for object IDs, called	persistent IDs.
/// </summary><remarks>
/// Usually it contain HIGH and LOW numeric IDs, but in our case i use object
/// type string representation and numeric ID.
/// </remarks>
public interface class IID
{
	property int ID {
		int get( void );
	}
	property String^ Type {
		String^ get( void );
	}
};
_RPL_END