/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Collections											*/
/*																			*/
/*	Module:		KeyedMap.cpp												*/
/*																			*/
/*	Content:	Implementation of KeyedMap class							*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "KeyedMap.h"

using namespace _COLLECTIONS;


//----------------------------------------------------------------------------
//			Toolkit::Collections::KeyedMap<TKey, TItem>::Enumerator
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Returns item of generic type that iterator in current state is
// pointed on. Now i know about node data structure and return only
// stored item (RedBlackVisitor conains Association as data).
//
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
TItem KeyedMap<TKey, TItem>::Enumerator::current_item( void )
{ENTER_READ(_lock)

	return RedBlackVisitor::Current->Value;

EXIT_READ(_lock)}


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


//----------------------------------------------------------------------------
//				Toolkit::Collections::KeyedMap<TKey, TItem>
//----------------------------------------------------------------------------

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
/// collection is cleared.
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
/// collection is cleared.
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
/// Copy content of KeyedMap into the new instance.
/// </summary><remarks>
/// This is copy constructor, but it provide shalow copying: only
/// item's pointers will be copied.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
KeyedMap<TKey, TItem>::KeyedMap( const KeyedMap<TKey, TItem> %map )
{
	for each( TItem item in const_cast<KeyedMap<TKey, TItem>^>(%map) ) {
		// add item to collection (don't worry about null references, we
		// copy from KeyedMap which handle this issue)
		if( !Insert( item->Key, item, false ) ) {
			// input collection is invalid
			throw gcnew ArgumentException(
				"Dublicate key exists in input collection.", "map");
		}
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Operator ==. Check for another instance of KeyedMap to be equal
/// to this.
/// </summary><remarks>
/// I check content equivalence by using Contains(TItem) call.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
bool KeyedMap<TKey, TItem>::operator==( IEnumerable<TItem> ^e )
{
	// check for nullptr
	if( e == nullptr ) return false;
	// check for same instance
	if( ReferenceEquals( this, e ) ) return true;

	// this is shallow check for no equal (by items count)
	int count = 0;
	for each( TItem item in e ) count++;
	if( Size() != count ) return false;

	// and this is deep check by content
	for each( TItem item in e ) {
		// check for exists and for item equivalent (i can
		// use only Contains call because it may check for
		// item equivalent stored in collection.
		if( !Contains( item ) ) return false;
	}
	return true;
}


//-------------------------------------------------------------------
/// <summary>
/// Operator !=. Check for another instance of KeyedMap to be not
/// equal to this.
/// </summary><remarks>
/// It uses Equal operator in implementation.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
bool KeyedMap<TKey, TItem>::operator!=( IEnumerable<TItem> ^e )
{
	return !(*this == e);
}


//-------------------------------------------------------------------
/// <summary>
/// Operator +=. Add specified item to collection.
/// </summary><remarks>
/// This operator is equivalent to Add(TItem) method.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
KeyedMap<TKey, TItem>% KeyedMap<TKey, TItem>::operator+=( TItem item )
{
	// validate input value
	if( item == nullptr ) throw gcnew ArgumentNullException("item");
	
	// add item to tree
	Add( item );

	return *this;
}


//-------------------------------------------------------------------
/// <summary>
/// Operator -=. Remove specified item from collection.
/// </summary><remarks>
/// This operator is equivalent to Remove(TItem) method, but throws
/// KeyNotFoundException exception if specified item is not present
/// in the collection.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
KeyedMap<TKey, TItem>% KeyedMap<TKey, TItem>::operator-=( TItem item )
{
	// validate input value
	if( item == nullptr ) throw gcnew ArgumentNullException("item");

	// remove item from tree
	if( !Remove( item ) ) throw gcnew KeyNotFoundException(
		"The given key was not present in the collection." );;

	return *this;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets an object that can be used to synchronize access to the
/// KeyedMap.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
Object^ KeyedMap<TKey, TItem>::SyncRoot::get( void  )
{
	return _lock;
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
	if( !Find( key, item ) ) throw gcnew KeyNotFoundException(
		"The given key was not present in the collection.");

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
/// Adds an item into the collection.
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
		throw gcnew ArgumentException(
			"An item with the same key already exists.");
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
/// This method may use shalow comparison only: it search for the
/// item having key same as passed and use default equality comparer
/// to check. The default equality comparer checks whether type T
/// implements the System.IEquatable generic interface and if so
/// returns an EqualityComparer that uses that implementation.
/// Otherwise it returns an EqualityComparer that uses the overrides
/// of Object.Equals and Object.GetHashCode provided by T.
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform "deep" object
/// comparison through operator == or in some other case.
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
///	<summary>
///	Copies the elements of the KeyedMap to an Array, starting at a
///	particular Array index. 
///	</summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
void KeyedMap<TKey, TItem>::CopyTo( array<TItem> ^dest, int index )
{
	// check for destination array is null reference
	if( dest == nullptr ) throw gcnew ArgumentNullException("dest");
	
	// check for array index is less than 0
	if( index < 0 ) throw gcnew ArgumentOutOfRangeException("index",
		"Number was less than the array's lower bound in the first dimension.");

	// check for array index is equal to or greater than the length of array
	// or the number of elements in the source ICollection is greater than
	// the available space from array index to the end of the destination array.
	if( (dest->Length - index) < Size() ) {
		// throw exception
		throw gcnew ArgumentException(
			"Destination array was not long enough. Check destIndex and " + 
			"length, and the array's lower bounds." );
	}
	
	// copy collection content
	for each( TItem item in this ) dest[index++] = item;
}


//-------------------------------------------------------------------
/// <summary>
/// Removes item with the specified key from the collection.
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
	// remove from collection by key
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
	
	// i must use key to find item to remove (keys in collections
	// are unique, so previous check guarantine existing item)
	// because of need passing real reference to handlers
	return Remove( item->Key );
}


//-------------------------------------------------------------------
/// <summary>
/// Determines whether the specified Object is equal to the current
/// KeyedMap instance.
/// </summary><remarks>
/// It uses Equal operator in implementation.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TItem>
bool KeyedMap<TKey, TItem>::Equals( Object ^obj)
{
	return (*this == dynamic_cast<IEnumerable<TItem>^>( obj ));
}