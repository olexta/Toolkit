/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		ObjectProperties.h											*/
/*																			*/
/*	Content:	Definition of ObjectProperties class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"
#include "PersistentProperties.h"

using namespace System;
using namespace System::Collections::Generic;


_RPL_BEGIN
ref class PersistentProperty;
ref class PersistentObject;

/// <summary>
/// Store object properties.
/// </summary><remarks>
/// This class derived from PersistentProperties and was developed to link
/// properties with it's owner object.
/// </remarks>
[Serializable]
public ref class ObjectProperties sealed : PersistentProperties
{
private:
	bool				m_changed;
	PersistentObject^	m_owner;

internal:
	ObjectProperties( PersistentObject ^owner );
	ObjectProperties( PersistentObject ^owner,
					  IEnumerable<PersistentProperty^> ^e );
	explicit ObjectProperties( const ObjectProperties %props );

protected:
	virtual void OnRemove( PersistentProperty ^prop ) override;
	virtual void OnInsert( PersistentProperty ^prop ) override;
	virtual void OnRemoveComplete( PersistentProperty ^prop ) override;
	virtual void OnInsertComplete( PersistentProperty ^prop ) override;

public:
	property bool IsChanged {
		bool get( void );
	}
	property bool IsListChanged {
		bool get( void );
	}
};
_RPL_END