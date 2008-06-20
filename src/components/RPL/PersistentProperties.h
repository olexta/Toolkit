/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentProperties.h										*/
/*																			*/
/*	Content:	Definition of PersistentProperties class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"
#include "PersistentProperty.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace _COLLECTIONS;


_RPL_BEGIN
/// <summary>
/// This class provide services for collection of properties.
/// </summary><remarks>
/// It store pointers to PersistentProperty objects and provide routines for
/// manipulation. PersistentProperties incapsulate check routines for keeping
/// equivalent between dictionary keys and property internal names. Access
/// to property by it name is processed as O(log N). But this class haven't
/// Clear() method due to potential errors (i cann't guaranty errorless batch
/// processing).
/// </remarks>
[Serializable]
public ref class PersistentProperties : KeyedMap<String^, PersistentProperty^>
{
public:
	PersistentProperties( void );
	PersistentProperties( IEnumerable<PersistentProperty^> ^e );
	~PersistentProperties( void );

	property PersistentProperty^ default[String^] {
		virtual PersistentProperty^ get( String ^name ) override;
	}

	virtual bool Contains( PersistentProperty ^prop ) override sealed;
	virtual void Clear( void ) override sealed;
};
_RPL_END