/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentObjects.h											*/
/*																			*/
/*	Content:	Definition of CPersistentObjects class						*/
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
ref class CPersistentObject;

/// <summary>
/// This class is used for storing collection of objects.
/// </summary><remarks>
/// I implement this class to get full control of internal data accessing that
/// is needed for thread-safe development. Also it provide some usefull
/// routines such as unique object contrtol, null reference control and find
/// functions. I implement OnXXX events to control collection access too.
/// </remarks>
public ref class CPersistentObjects : MarshalByRefObject,
									  ICollection<CPersistentObject^>,
									  Collections::ICollection 
{
private:
	property bool IsReadOnly {
		virtual bool get( void ) sealed =
			ICollection<CPersistentObject^>::IsReadOnly::get;
	}
	property bool IsSynchronized {
		virtual bool get( void ) sealed =
			Collections::ICollection::IsSynchronized::get;
	}
	property Object^ SyncRoot {
		virtual Object^ get( void  ) sealed =
			Collections::ICollection::SyncRoot::get;
	}

	virtual void CopyTo( Array ^dest, int index ) sealed = 
		Collections::ICollection::CopyTo;
	virtual void CopyTo( array<CPersistentObject^> ^dest, int index ) sealed = 
		ICollection<CPersistentObject^>::CopyTo;
	virtual Collections::IEnumerator^ GetEnumerator( void ) sealed =
		Collections::IEnumerable::GetEnumerator;
	virtual IEnumerator<CPersistentObject^>^ GetGEnumerator( void ) sealed =
		IEnumerable<CPersistentObject^>::GetEnumerator;

protected:
	List<CPersistentObject^>	m_list;

	Object	^_lock_this;

	virtual void OnClear( void );
	virtual void OnRemove( CPersistentObject ^prop );
	virtual void OnInsert( CPersistentObject ^prop );
	virtual void OnClearComplete( void );
	virtual void OnRemoveComplete( CPersistentObject ^prop );
	virtual void OnInsertComplete( CPersistentObject ^prop );

public:
	CPersistentObjects( void );
	CPersistentObjects( IEnumerable<CPersistentObject^> ^e );

	bool operator==( const CPersistentObjects %objs );
	bool operator!=( const CPersistentObjects %objs );

	virtual property int Count {
		int get( void );
	}

	CPersistentObject^ Find( Predicate<CPersistentObject^> ^match );
	IEnumerable<CPersistentObject^>^ FindAll( Predicate<CPersistentObject^> ^match );
	
	virtual void Add( CPersistentObject ^obj );
	virtual void Clear( void );
	virtual bool Contains( CPersistentObject ^obj );
	virtual bool Remove( CPersistentObject ^obj );
};
_RPL_END