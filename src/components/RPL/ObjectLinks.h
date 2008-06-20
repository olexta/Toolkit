/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		ObjectLinks.h												*/
/*																			*/
/*	Content:	Definition of ObjectLinks class								*/
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
ref class PersistentObject;

/// <summary>
/// Store object relations.
/// </summary><remarks>
/// Link is only pointer to PersistentObject object. To add or delete link
/// from parent, making the call with linked object as parameter is all that
/// you need to do. Class derive from PersistentObjects and add built in
/// event support.
/// </remarks>
[Serializable]
public ref class ObjectLinks sealed : PersistentObjects
{
private:
	bool				m_changed;
	PersistentObject	^m_owner;

internal:
	ObjectLinks( PersistentObject ^owner );
	ObjectLinks( PersistentObject ^owner,
				 IEnumerable<PersistentObject^> ^e );
	explicit ObjectLinks( const ObjectLinks %links );

protected:
	virtual void OnInsert( PersistentObject ^obj ) override;
	virtual void OnRemove( PersistentObject ^obj ) override;
	virtual void OnInsertComplete( PersistentObject ^obj ) override;
	virtual void OnRemoveComplete( PersistentObject ^obj ) override;

public:
	property bool IsListChanged {
		bool get( void );
	}
};
_RPL_END