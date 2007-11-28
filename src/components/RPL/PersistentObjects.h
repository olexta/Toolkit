/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentObjects.h											*/
/*																			*/
/*	Content:	Definition of PersistentObjects class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"

using namespace System;
using namespace System::Collections::Generic;


_RPL_BEGIN
ref class PersistentObject;

/// <summary>
/// This class is used for storing collection of objects.
/// </summary><remarks>
/// It provides some usefull routines such as unique object contrtol,
/// null reference control and find functions. I implement OnXXX
/// events to control collection access too. But this class haven't
/// Clear() method due to potential errors (i cann't guaranty 
/// errorless batch processing).
/// </remarks>
public ref class PersistentObjects : MarshalByRefObject,
									 ICollection<PersistentObject^>
{
private:
	// IEnumerable
	virtual System::Collections::IEnumerator^ get_enumerator( void ) sealed =
		System::Collections::IEnumerable::GetEnumerator;
	
	// IEnumerable<PersistenObject^>
	virtual bool items_is_readonly( void ) sealed =
		ICollection<PersistentObject^>::IsReadOnly::get;
	virtual IEnumerator<PersistentObject^>^ items_get_enumerator( void ) sealed =
		IEnumerable<PersistentObject^>::GetEnumerator;

protected:
	List<PersistentObject^>	m_list;

	virtual void OnInsert( PersistentObject ^prop );
	virtual void OnRemove( PersistentObject ^prop );
	virtual void OnInsertComplete( PersistentObject ^prop );
	virtual void OnRemoveComplete( PersistentObject ^prop );

public:
	PersistentObjects( void );
	PersistentObjects( IEnumerable<PersistentObject^> ^e );

	bool operator==( const PersistentObjects %objs );
	bool operator!=( const PersistentObjects %objs );

	property int Count {
		virtual int get( void );
	}
	
	virtual void Add( PersistentObject ^obj );
	virtual void Clear( void );
	virtual bool Contains( PersistentObject ^obj );
	virtual void CopyTo( array<PersistentObject^> ^dest, int index );
	virtual bool Remove( PersistentObject ^obj );

	virtual bool Equals( Object ^obj ) override;
};
_RPL_END