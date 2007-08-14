/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentProperties.h										*/
/*																			*/
/*	Content:	Definition of CPersistentProperties class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"

using namespace System;
using namespace System::Reflection;
using namespace System::Collections::Generic;


_RPL_BEGIN
ref class CPersistentProperty;

/// <summary>
/// This class provide services for collection of properties.
/// </summary><remarks>
/// It store pointers to CPersistentProperty objects and provide routines for
/// manipulation. Properties identify by it's unique names, so i chouse
/// Dictionary as internall storage. CPersistentProperties incapsulate check
/// routines for keeping equivalent between dictionary keys and property
/// internal names. So, access to property by it name is processed as O(1).
/// </remarks>
[DefaultMember("Item")]
public ref class CPersistentProperties : MarshalByRefObject,
										 ICollection<CPersistentProperty^>,
										 Collections::ICollection
{
private:
	property bool IsReadOnly {
		virtual bool get( void ) sealed =
			ICollection<CPersistentProperty^>::IsReadOnly::get;
	}
	property bool IsSynchronized {
		virtual bool get( void ) sealed =
			Collections::ICollection::IsSynchronized::get;
	}
	property Object^ SyncRoot {
		virtual Object^ get( void  ) sealed =
			Collections::ICollection::SyncRoot::get;
	}

	virtual bool Contains( CPersistentProperty ^prop ) sealed = 
		ICollection<CPersistentProperty^>::Contains;
	virtual void CopyTo( Array ^dest, int index ) sealed = 
		Collections::ICollection::CopyTo;
	virtual void CopyTo( array<CPersistentProperty^> ^dest, int index ) sealed = 
		ICollection<CPersistentProperty^>::CopyTo;
	virtual Collections::IEnumerator^ GetEnumerator( void ) sealed =
		Collections::IEnumerable::GetEnumerator;
	virtual IEnumerator<CPersistentProperty^>^ GetGEnumerator( void ) sealed =
		IEnumerable<CPersistentProperty^>::GetEnumerator;
	virtual bool Remove( CPersistentProperty ^prop ) sealed = 
		ICollection<CPersistentProperty^>::Remove;

protected:
	Dictionary<String^, CPersistentProperty^>	m_dict;

	Object	^_lock_this;

	virtual void OnClear( void );
	virtual void OnRemove( CPersistentProperty ^prop );
	virtual void OnInsert( CPersistentProperty ^prop );
	virtual void OnClearComplete( void );
	virtual void OnRemoveComplete( CPersistentProperty ^prop );
	virtual void OnInsertComplete( CPersistentProperty ^prop );

public:
	CPersistentProperties( void );
	CPersistentProperties( IEnumerable<CPersistentProperty^> ^e );
	~CPersistentProperties( void );

	bool operator==( const CPersistentProperties %props );
	bool operator!=( const CPersistentProperties %props );
	CPersistentProperty% operator[]( String ^name );

	property CPersistentProperty^ Item[String^] {
		CPersistentProperty^ get( String ^name );
	}
	property ICollection<String^>^ Names {
		ICollection<String^>^ get( void );
	}
	virtual property int Count {
		int get( void );
	}

	bool Contains( String ^name );
	bool Remove( String ^name );

	virtual void Add( CPersistentProperty ^prop );
	virtual void Clear( void );
};
_RPL_END