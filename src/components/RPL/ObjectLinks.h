/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		ObjectLinks.h												*/
/*																			*/
/*	Content:	Definition of CObjectLinks class							*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"
#include "PersistentObjects.h"

using namespace System;
using namespace System::Collections::Generic;


_RPL_BEGIN
ref class CPersistentObject;

/// <summary>
/// Store object relations.
/// </summary><remarks>
/// Link is only pointer to CPersistentObject object. To add or delete link
/// from parent, making the call with linked object as parameter is all that
/// you need to do. Class derive from CPersistentObjects and add built in
/// event support.
/// </remarks>
public ref class CObjectLinks sealed : CPersistentObjects
{
private:
	bool				m_changed;
	CPersistentObject	^m_owner;

internal:
	CObjectLinks( CPersistentObject ^owner );
	CObjectLinks( CPersistentObject ^owner,
				  IEnumerable<CPersistentObject^> ^e );
	explicit CObjectLinks( const CObjectLinks %links );

protected:
	virtual void OnClear( void ) override;
	virtual void OnInsert( CPersistentObject ^obj ) override;
	virtual void OnRemove( CPersistentObject ^obj ) override;
	virtual void OnInsertComplete( CPersistentObject ^obj ) override;
	virtual void OnRemoveComplete( CPersistentObject ^obj ) override;

public:
	property bool IsListChanged {
		bool get( void );
	}
};
_RPL_END