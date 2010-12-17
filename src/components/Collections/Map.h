**************************************************************************/
/*																			*/
/*	Project:	Toolkit Collections											*/
/*																			*/
/*	Module:		Map.h														*/
/*																			*/
/*	Content:	Definition of Map class										*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "Collections.h"
#include "BinaryTree\RedBlackTree.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace _BINARY_TREE;


_COLLECTIONS_BEGIN
/// <summary>
/// Represents a collection of key/value pairs and provides storage services
/// that implement fast access to values by it's keys. 
/// </summary><remarks>
/// Values can be identified by it's unique key, so i chouse Red-Black tree
/// as internal storage. Access to value by it's key is processed as O(log N).
/// Tree traverse (for each) is implemnted as iteration algorithm, so it
/// process as O(N).
/// </remarks>
generic<typename TKey, typename TValue> 
	where TKey : IComparable<TKey>
[Serializable]
public ref class Map : RedBlackTree<TKey, TValue>, IDictionary<TKey, TValue>
{
private:
	// Enumerator class that provide map bypass
	ref class Enumerator : RedBlackVisitor,
						   IEnumerator<KeyValuePair<TKey, TValue>>
	{
	private:
		virtual KeyValuePair<TKey, TValue> current_item( void ) sealed =
			IEnumerator<KeyValuePair<TKey, TValue>>::Current::get;

	public:
		Enumerator( Map ^map );

		property Object^ Current {
			virtual Object^ get( void ) new;
		}
	};

private:
	// IEnumerable
	virtual System::Collections::IEnumerator^ get_enumarator( void ) sealed =
		System::Collections::ICollection::GetEnumerator;

	// ICollection<KeyValuePair<TKey, TValue>>
	virtual void pairs_add( KeyValuePair<TKey, TValue> pair ) sealed = 
		ICollection<KeyValuePair<TKey, TValue>>::Add;
	virtual bool pairs_contains( KeyValuePair<TKey, TValue> pair ) sealed =
		ICollection<KeyValuePair<TKey, TValue>>::Contains;
	virtual void pairs_copy_to( array<KeyValuePair<TKey, TValue>> ^dest, int index ) sealed =
		ICollection<KeyValuePair<TKey, TValue>>::CopyTo;
	virtual bool pairs_is_readonly( void ) sealed =
		ICollection<KeyValuePair<TKey, TValue>>::IsReadOnly::get;
	virtual bool pairs_remove( KeyValuePair<TKey, TValue> pair ) sealed =
		ICollection<KeyValuePair<TKey, TValue>>::Remove;
	virtual IEnumerator<KeyValuePair<TKey, TValue>>^ pairs_get_enumerator( void ) sealed =
		IEnumerable<KeyValuePair<TKey, TValue>>::GetEnumerator;

protected:
	virtual void OnClear( void );
	virtual void OnInsert( TKey key, TValue value );
	virtual void OnRemove( TKey key, TValue value );
	virtual void OnSet( TKey key, TValue value );
	virtual void OnClearComplete( void );
	virtual void OnInsertComplete( TKey key, TValue value );
	virtual void OnRemoveComplete( TKey key, TValue value );
	virtual void OnSetComplete( TKey key, TValue value );

public:
	Map( void );
	explicit Map( KeyValuePair<TKey, TValue> pair );
	explicit Map( IEnumerable<KeyValuePair<TKey, TValue>> ^e );

	property TValue default[TKey] {
		virtual TValue get( TKey key );
		virtual void set( TKey key, TValue value );
	}
	property int Count {
		virtual int get( void );
	}
	property ICollection<TKey>^ Keys {
		virtual ICollection<TKey>^ get( void );
	}
	property ICollection<TValue>^ Values {
		virtual ICollection<TValue>^ get( void );
	}

	virtual void Add( TKey key, TValue value );
	virtual void Clear( void );
	virtual bool ContainsKey(TKey key);
	virtual bool ContainsValue( TValue value ); 
	virtual bool Remove( TKey key );
	virtual bool TryGetValue( TKey key, TValue %value );
};
_COLLECTIONS_END
