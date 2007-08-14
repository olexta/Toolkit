/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		ObjectProperties.h											*/
/*																			*/
/*	Content:	Definition of CObjectProperties class						*/
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
ref class CPersistentProperty;
ref class CPersistentObject;

/// <summary>
/// Store object properties.
/// </summary><remarks>
/// This class derived from CPersistentProperties and was developed to link
/// properties with it's owner object.
/// </remarks>
public ref class CObjectProperties sealed : CPersistentProperties
{
private:
	bool				m_changed;
	CPersistentObject^	m_owner;

internal:
	CObjectProperties( CPersistentObject^ owner );
	CObjectProperties( CPersistentObject^ owner,
					   IEnumerable<CPersistentProperty^> ^e );
	explicit CObjectProperties( const CObjectProperties %props );

protected:
	virtual void OnClear( void ) override;
	virtual void OnRemove( CPersistentProperty ^prop ) override;
	virtual void OnInsert( CPersistentProperty ^prop ) override;
	virtual void OnRemoveComplete( CPersistentProperty ^prop ) override;
	virtual void OnInsertComplete( CPersistentProperty ^prop ) override;

public:
	property bool IsChanged {
		bool get( void );
	}
	property bool IsListChanged {
		bool get( void );
	}
};
_RPL_END