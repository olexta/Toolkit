/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentProperties.h										*/
/*																			*/
/*	Content:	Definition of PersistentProperties class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"
#include "ValueBox.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace Toolkit::Collections;


_RPL_BEGIN
/// <summary>
/// This class provide services for collection of properties.
/// </summary><remarks>
/// It store names, values and provide routines for manipulation. Access
/// to property value by it name is processed as O(log N).
/// </remarks>
[Serializable]
public ref class PersistentProperties : Map<String^, ValueBox>
{
public:
	PersistentProperties( void );
	explicit PersistentProperties( IEnumerable<KeyValuePair<String^, ValueBox>> ^e );
};
_RPL_END
