/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Collections											*/
/*																			*/
/*	Module:		KeyedMap.h													*/
/*																			*/
/*	Content:	Definition of KeyedMap class								*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "Collections.h"
#include "IKeyedObject.h"
#include "RedBlackTree.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace _BINARY_TREE;


_COLLECTIONS_BEGIN
/// <summary>
/// This class provide storage services that implement fast access for keyed
/// objects by it's keys.
/// </summary><remarks>
/// It store pointers to IKeyedObjects and provide routines for manipulation.
/// Objects can be identified by it's unique key, so i chouse Red-Black tree
/// as internal storage. Class doesn't implement any check for key changing
/// during it's lifetime, so such actions will have unpredictable results.
/// Access to item by it's name is processed as O(log N). Tree traverse (for
/// each) is implemented as iteration algorithm, so it process as O(N).
/// </remarks>
generic<typename TKey, typename TItem> 
	where TKey : IComparable<TKey>
	where TItem : IKeyedObject<TKey>
public ref class KeyedMap : RedBlackTree<TKey, TItem>, ICollection<TItem>
{
private:
	// Enumerator class that provide map bypass
	ref class Enumerator : RedBlackVisitor, IEnumerator<TItem>
	{
	private:
		virtual TItem current_item( void ) sealed =
			IEnumerator<TItem>::Current::get;

	public:
		Enumerator( KeyedMap ^map );

		property Object^ Current {
			virtual Object^ get( void ) new;
		}
	};

private:
	// IEnumerable
	virtual System::Collections::IEnumerator^ get_enumarator( void ) sealed =
		System::Collections::ICollection::GetEnumerator;

	// ICollection<TItem>
	virtual bool items_is_readonly( void ) sealed =
		ICollection<TItem>::IsReadOnly::get;
	virtual IEnumerator<TItem>^ items_get_enumerator( void ) sealed =
		IEnumerable<TItem>::GetEnumerator;

protected:
	virtual void OnClear( void );
	virtual void OnInsert( TItem item );
	virtual void OnRemove( TItem item );
	virtual void OnClearComplete( void );
	virtual void OnInsertComplete( TItem item );
	virtual void OnRemoveComplete( TItem item );

public:
	KeyedMap( void );
	explicit KeyedMap( TItem item );
	explicit KeyedMap( IEnumerable<TItem> ^e );
	KeyedMap( const KeyedMap<TKey, TItem> %map );

	bool operator==( IEnumerable<TItem> ^e );
	bool operator!=( IEnumerable<TItem> ^e );
	KeyedMap<TKey, TItem>% operator=( const KeyedMap<TKey, TItem> %map );
	KeyedMap% operator+=( TItem item );
	KeyedMap% operator-=( TItem item );

	property TItem default[TKey] {
		virtual TItem get( TKey key );
	}
	property Object^ SyncRoot {
		Object^ get( void );
	}

	virtual void Add( TItem item );
	virtual void Clear( void );
	virtual bool Contains( TKey key );
	virtual bool Contains( TItem item ); 
	virtual void CopyTo( array<TItem> ^dest, int index );
	virtual bool Remove( TKey key );
	virtual bool Remove( TItem item );
	
	virtual bool Equals( Object ^obj ) override;
};
_COLLECTIONS_END