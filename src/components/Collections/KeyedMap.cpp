/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Collections											*/
/*																			*/
/*	Module:		KeyedMap.cpp												*/
/*																			*/
/*	Content:	Implementation of KeyedMap class							*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2010 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "KeyedMap.h"

using namespace _COLLECTIONS;


//-----------------------------------------------------------------------------
//			Toolkit::Collections::KeyedMap<TKey, TItem>::Enumerator
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Returns item of generic type that iterator in current state is
// pointed on. Now i know about node data structure and return only
// stored item (RedBlackVisitor conains Association as data).
//
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
TItem KeyedMap<TKey, TItem>::Enumerator::current_item( void )
{
	return RedBlackVisitor::Current.Value;
}


//-------------------------------------------------------------------
//
// Creates new instance of the Enumerator class for specified
// collection. I need pass call to parent constructor only, all
// processing  will be done by it.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
KeyedMap<TKey, TItem>::Enumerator::Enumerator( KeyedMap ^map ): \
	RedBlackVisitor(map)
{
}


//-------------------------------------------------------------------
//
// Returns item (as Object) that iterator in current state is pointed
// on. This is "current_item" call only.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
Object^ KeyedMap<TKey, TItem>::Enumerator::Current::get( void )
{
	return current_item();
}


//-----------------------------------------------------------------------------
//				Toolkit::Collections::KeyedMap<TKey, TItem>
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Returns an enumerator that iterates through a collection of items.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
Collections::IEnumerator^ KeyedMap<TKey, TItem>::get_enumarator( void )
{
	return gcnew Enumerator(this);
}


//-------------------------------------------------------------------
//
// Gets a value indicating whether the KeyedMap is read-only.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
bool KeyedMap<TKey, TItem>::items_is_readonly( void )
{
	return false;
}


//-------------------------------------------------------------------
//
// Returns an enumerator that iterates through a generic collection
// of items.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
IEnumerator<TItem>^ KeyedMap<TKey, TItem>::items_get_enumerator( void )
{
	return gcnew Enumerator(this);
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before clearing the contents
/// of the KeyedMap instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// KeyedMap is cleared.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
void KeyedMap<TKey, TItem>::OnClear( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before inserting a new item
/// into the KeyedMap instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// specified item is inserted.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
void KeyedMap<TKey, TItem>::OnInsert( TItem item )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before removing an item from
/// the KeyedMap instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// specified item is removed.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
void KeyedMap<TKey, TItem>::OnRemove( TItem item )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after clearing the contents
/// of the KeyedMap instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// KeyedMap is cleared.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
void KeyedMap<TKey, TItem>::OnClearComplete( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after inserting a new item
/// into the KeyedMap instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// specified item is inserted.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
void KeyedMap<TKey, TItem>::OnInsertComplete( TItem item )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after removing an item from
/// the KeyedMap instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// specified item is removed.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
void KeyedMap<TKey, TItem>::OnRemoveComplete( TItem item )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Default class constructor.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
KeyedMap<TKey, TItem>::KeyedMap( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the KeyedMap class initialized with specified
/// item.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
KeyedMap<TKey, TItem>::KeyedMap( TItem item )
{
	if( item == nullptr ) throw gcnew ArgumentNullException("item");

	Insert( item->Key, item, false );
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the KeyedMap class initialized with all items
/// in the given collection.
/// </summary><remarks>
/// If items in collection have not unique keys then only the last
/// item will be stored. All null references will be ignored.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
KeyedMap<TKey, TItem>::KeyedMap( IEnumerable<TItem> ^e )
{
	if( e == nullptr ) throw gcnew ArgumentNullException("e");

	// path through collection
	for each( TItem item in e ) {
		// prevent errors by null references
		if( item != nullptr ) Insert( item->Key, item, true );
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Gets item with the specified key.
/// </summary><remarks>
/// No setter method defined because of dublicate parameters (each
/// item contains it's own key that will be used as collection key).
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
TItem KeyedMap<TKey, TItem>::default::get( TKey key )
{
	// validate input value
	if( key == nullptr ) throw gcnew ArgumentNullException("key");

	TItem		item;

	// find item with specified key (in case of unsuccessful search
	// exception KeyNotFoundException will be raised)
	if( !Find( key, item ) )
		throw gcnew KeyNotFoundException(ERR_KEY_NOT_FOUND);

	return item;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets the number of elements contained in the KeyedMap.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
int KeyedMap<TKey, TItem>::Count::get( void )
{
	return Size();
}


//-------------------------------------------------------------------
/// <summary>
/// Adds an item into the KeyedMap instance.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
void KeyedMap<TKey, TItem>::Add( TItem item )
{
	// validate item
	if( item == nullptr ) throw gcnew ArgumentNullException("item");

	//fire event before the action
	OnInsert( item );

	// add item to red-black tree (insert function return 'false'
	// in case of having item with same key, so i can check tree for
	// item exists)
	if( !Insert( item->Key, item, false ) ) {
		// raise exception
		throw gcnew ArgumentException(ERR_ITEM_EXISTS);
	}

	// fire event after the action (if error will be raised
	// all changes will be rolled back)
	try {
		// handler call
		OnInsertComplete( item );
	} catch( Exception^ ) {
		// roll back changes
		Undo();
		// restore exception
		throw;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Clears the content of the KeyedMap instance.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
void KeyedMap<TKey, TItem>::Clear( void )
{
	// fire event before action
	OnClear();
	// clear tree
	DeleteAll();
	// fire event after action (if error will be raised
	// all changes will be rolled back)
	try {
		// handler call
		OnClearComplete();
	} catch( Exception^ ) {
		// rollback changes
		Undo();
		// restore exception
		throw;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Determines whether the KeyedMap contains the item with specified
/// key.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
bool KeyedMap<TKey, TItem>::Contains( TKey key )
{
	// validate input parameters
	if( key == nullptr ) throw gcnew ArgumentNullException("key");

	// attempt to find item with specified key
	return Find( key, TItem() );
}


//-------------------------------------------------------------------
/// <summary>
/// Determines whether the KeyedMap contains a specific item.
/// </summary><remarks>
/// It checks content equivalence by using default equality comparer
/// for specified type.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
bool KeyedMap<TKey, TItem>::Contains( TItem item )
{
	// check for initialized input value
	if( item == nullptr ) throw gcnew ArgumentNullException("item");

	TItem	value;

	// attempt to find item by key (in case of search
	// failed return false)
	if( !Find( item->Key, value ) ) return false;

	// use equality comparer for specified type
	return EqualityComparer<TItem>::Default->Equals( value, item );
}


//-------------------------------------------------------------------
/// <summary>
/// Copies the elements of the KeyedMap to an Array, starting at a
/// particular Array index. 
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
void KeyedMap<TKey, TItem>::CopyTo( array<TItem> ^dest, int index )
{
	// check for destination array is null reference
	if( dest == nullptr ) throw gcnew ArgumentNullException("dest");

	// check for array index is less than 0
	if( index < 0 )
		throw gcnew ArgumentOutOfRangeException("index", ERR_OUT_OF_RANGE);

	// check for array index is equal to or greater than the length of array
	// or the number of elements in the source ICollection is greater than
	// the available space from array index to the end of the destination array.
	if( (dest->Length - index) < Size() ) {
		// throw exception
		throw gcnew ArgumentException(ERR_ARRAY_TOO_SMALL);
	}

	// copy collection content
	for each( TItem item in this ) dest[index++] = item;
}


//-------------------------------------------------------------------
/// <summary>
/// Removes item with the specified key from the KeyedMap instance.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
bool KeyedMap<TKey, TItem>::Remove( TKey key )
{
	// validate key
	if( key == nullptr ) throw gcnew ArgumentNullException("key"); 

	TItem		item;	// item corresponding specified key

	// find item with specified key (in case of search
	// failed return false)
	if( !Find( key, item ) ) return false;

	// fire event before action
	OnRemove( item );
	// remove from instance by key
	Delete( key );
	// fire event after action
	try {
		// handler call
		OnRemoveComplete( item );
	} catch( Exception^ ) {
		// roll back changes
		Undo();
		// restore exception
		throw;
	}
	return true;
}


//-------------------------------------------------------------------
/// <summary>
/// Removes the specific item from the KeyedMap.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
bool KeyedMap<TKey, TItem>::Remove( TItem item )
{
	// validate item
	if( item == nullptr ) throw gcnew ArgumentNullException("item");

	// i cann't use access only by key to prevent data loss,
	// so check for item exists (this function may be override
	// in derived classes to provide "deep" compare of objects)
	if( !Contains( item ) ) return false;

	// i must use key to find item to remove (keys in collection
	// are unique, so previous check guarantine existing item)
	// because of need passing real reference to handlers
	return Remove( item->Key );
}


//-------------------------------------------------------------------
/// <summary>
/// Determines whether the specified KeyedMap contains items that
/// match the conditions defined by the specified predicate.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
bool KeyedMap<TKey, TItem>::Exists( Predicate<TItem> ^match )
{
	if( match == nullptr ) throw gcnew ArgumentNullException("match");

	for each( TItem item in this ) {
		if( match( item ) ) return true;
	}
	return false;
}


//-------------------------------------------------------------------
/// <summary>
/// Searches for an item that matches the conditions defined by the
/// specified predicate, and returns the first occurrence within the
/// entire KeyedMap.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
TItem KeyedMap<TKey, TItem>::Find( Predicate<TItem> ^match )
{
	if( match == nullptr ) throw gcnew ArgumentNullException("match");

	for each( TItem item in this ) {
		if( match( item ) ) return item;
	}
	return TItem();
}


//-------------------------------------------------------------------
/// <summary>
/// Retrieves the all the item that match the conditions defined by
/// the specified predicate.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
array<TItem>^ KeyedMap<TKey, TItem>::FindAll( Predicate<TItem> ^match )
{
	if( match == nullptr ) throw gcnew ArgumentNullException("match");

	List<TItem>	^list = gcnew List<TItem>();

	for each( TItem item in this ) {
		if( match( item) ) list->Add( item );
	}
	return list->ToArray();
}


//-------------------------------------------------------------------
/// <summary>
/// Performs the specified action on each element of the KeyedMap.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
void KeyedMap<TKey, TItem>::ForEach( Action<TItem> ^action )
{
	if( action == nullptr ) throw gcnew ArgumentNullException("action");

	for each( TItem item in this ) action( item );
}


//-------------------------------------------------------------------
/// <summary>
/// Determines whether every element in the KeyedMap matches the
/// conditions defined by the specified predicate.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
bool KeyedMap<TKey, TItem>::TrueForAll( Predicate<TItem> ^match )
{
	if( match == nullptr ) throw gcnew ArgumentNullException("match");

	for each( TItem item in this ) {
		if( !match( item ) ) return false;
	}
	return true;
}
