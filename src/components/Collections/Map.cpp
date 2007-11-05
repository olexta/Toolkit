/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Collections											*/
/*																			*/
/*	Module:		Map.cpp														*/
/*																			*/
/*	Content:	Implementation of Map class									*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "Map.h"

using namespace _COLLECTIONS;


//----------------------------------------------------------------------------
//						Map<TKey, TValue>::Enumerator
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Returns item of generic type that iterator in current state is
// pointed on. Now i know about node data structure and return only
// stored item (RedBlackVisitor conains Association as data).
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
KeyValuePair<TKey, TValue> Map<TKey, TValue>::Enumerator::current_item( void )
{ENTER_READ(_lock)

	return (KeyValuePair<TKey, TValue>) RedBlackVisitor::Current;

EXIT_READ(_lock)}


//-------------------------------------------------------------------
//
// Creates new instance of the Enumerator class for specified
// collection. I need pass call to parent constructor only, all
// processing  will be done by it.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
Map<TKey, TValue>::Enumerator::Enumerator( Map ^map ): \
	RedBlackVisitor(map)
{
}


//-------------------------------------------------------------------
//
// Returns item (as Object) that iterator in current state is pointed
// on. This is "current_item" call only.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
Object^ Map<TKey, TValue>::Enumerator::Current::get( void )
{
	return current_item();
}


//----------------------------------------------------------------------------
//							PRIVATE METHODS
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Returns an enumerator that iterates through a collection of
// key-value pairs.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
Collections::IEnumerator^ Map<TKey, TValue>::get_enumarator( void )
{
	return gcnew Enumerator(this);
}


//-------------------------------------------------------------------
//
// Adds a pair to the Map.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void Map<TKey, TValue>::pairs_add( KeyValuePair<TKey, TValue> pair )
{
	// check for initialized key
	if( pair.Key == nullptr ) throw gcnew ArgumentNullException("pair.Key");

	// call to public Add method
	Add( pair.Key, pair.Value ); 
}


//-------------------------------------------------------------------
//
// Determines whether the Map contains a specific key-value pair.
// This method may use shalow comparison only: it search for the pair
// having key same as passed and use default equality comparer to
// check value. The default equality comparer checks whether type T
// implements the System.IEquatable generic interface and if so
// returns an EqualityComparer that uses that implementation.
// Otherwise it returns an EqualityComparer that uses the overrides
// of Object.Equals and Object.GetHashCode provided by T.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
bool Map<TKey, TValue>::pairs_contains( KeyValuePair<TKey, TValue> pair )
{
	// check for initialized key
	if( pair.Key == nullptr ) throw gcnew ArgumentNullException("pair.Key");

	TValue		value;

	// attempt to find item by key (in case of search
	// failed return false)
	if( !Find( pair.Key, value ) ) return false;

	// use equality comparer for specified type
	return EqualityComparer<TValue>::Default->Equals( pair.Value, value );
}


//-------------------------------------------------------------------
//
// Copies the key-value pairs of the Map to an Array, starting at a
// particular Array index. 
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void Map<TKey, TValue>::pairs_copy_to( array<KeyValuePair<TKey, TValue>> ^dest,
									   int index )
{
	// check for destination array is null reference
	if( dest == nullptr ) throw gcnew ArgumentNullException("dest");
	
	// check for array index is less than 0
	if( index < 0 ) throw gcnew ArgumentOutOfRangeException("index",
		"Number was less than the array's lower bound in the first dimension.");

	// check for array index is equal to or greater than the length of array
	// or the number of elements in the source ICollection is greater than
	// the available space from array index to the end of the destination array.
	if( (dest->Length - index) < RedBlackTree::Count ) {
		// throw exception
		throw gcnew ArgumentException(
			"Destination array was not long enough. Check destIndex and" + 
			"length, and the array's lower bounds." );
	}
	
	// copy collection content
	for each( KeyValuePair<TKey, TValue> pair in this ) dest[index++] = pair;
}


//-------------------------------------------------------------------
//
// Gets a value indicating whether the Map is read-only.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
bool Map<TKey, TValue>::pairs_is_readonly( void )
{
	return false;
}


//-------------------------------------------------------------------
//
// Removes a specific key-value pair from the Map.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
bool Map<TKey, TValue>::pairs_remove( KeyValuePair<TKey, TValue> pair )
{
	// validate key
	if( pair.Key == nullptr ) throw gcnew ArgumentNullException("pair.Key");

	// i cann't use remove only by key to prevent data loss,
	// so check for pair exists (this function may be override
	// in derived classes to provide "deep" compare of objects)
	if( !pairs_contains( pair ) ) return false;
	
	// i must use key to find pair to remove (keys in collections
	// are unique, so previous check guarantine existing item)
	// because of need passing real reference to handlers
	return Remove( pair.Key );
}


//-------------------------------------------------------------------
//
// Returns an enumerator that iterates through a generic collection
// of key-value pairs.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
IEnumerator<KeyValuePair<TKey, TValue>>^ Map<TKey, TValue>::pairs_get_enumerator( void )
{
	return gcnew Enumerator(this);
}


//----------------------------------------------------------------------------
//							PROTECTED METHODS
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before clearing the contents
/// of the Map instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// collection is cleared.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void Map<TKey, TValue>::OnClear( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before inserting a new pair
/// into the Map instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// specified pair is inserted.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void Map<TKey, TValue>::OnInsert( TKey key, TValue value )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before removing a pair from
/// the Map instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// specified pair is removed.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void Map<TKey, TValue>::OnRemove( TKey key, TValue value )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before setting a value in
/// the Map instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// specified element is set.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void Map<TKey, TValue>::OnSet( TKey key, TValue value )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after clearing the contents
/// of the Map instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// collection is cleared.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void Map<TKey, TValue>::OnClearComplete( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after inserting a new pair
/// into the Map instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// specified pair is inserted.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void Map<TKey, TValue>::OnInsertComplete( TKey key, TValue value )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after removing a pair from
/// the Map instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// specified pair is removed.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void Map<TKey, TValue>::OnRemoveComplete( TKey key, TValue value )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after setting a value in the
/// Map instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// specified element is set.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void Map<TKey, TValue>::OnSetComplete( TKey key, TValue value )
{
}


//----------------------------------------------------------------------------
//							CONSTRUCTORS
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Default class constructor.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
Map<TKey, TValue>::Map( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the Map class initialized with specified pair.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
Map<TKey, TValue>::Map( KeyValuePair<TKey, TValue> pair )
{
	if( pair.Key == nullptr ) throw gcnew ArgumentNullException("pair.Key");

	Insert( pair.Key, pair.Value, false );
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the Map class initialized with all pairs in
/// the given collection.
/// </summary><remarks>
/// If pairs in collection have not unique keys then only the last
/// pair will be stored. All pairs with null reference keys will be
/// ignored.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
Map<TKey, TValue>::Map( IEnumerable<KeyValuePair<TKey, TValue>> ^e )
{
	// path through collection
	for each( KeyValuePair<TKey, TValue> pair in e ) {
		// prevent errors by null references
		if( pair.Key != nullptr ) Insert( pair.Key, pair.Value, true );
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Copy content of Map into the new instance.
/// </summary><remarks>
/// This is copy constructor, but it provides shalow copying only.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
Map<TKey, TValue>::Map( const Map<TKey, TValue> %map )
{
	for each( KeyValuePair<TKey, TValue> pair in
			  const_cast<Map<TKey, TValue>^>(%map) ) {
		// add pair to collection (don't worry about key's null references,
		// we copy from Map which handle this issue)
		if( !Insert( pair.Key, pair.Value, false ) ) {
			// input collection is invalid
			throw gcnew ArgumentException(
				"Dublicate key exists in input collection.", "map");
		}
	}
}


//----------------------------------------------------------------------------
//								PROPERTIES
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Gets or sets the value associated with the specified key.
/// </summary><remarks>
/// If the specified key is not found, a get operation throws a
/// KeyNotFoundException, and a set operation creates a new element
/// with the specified key.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
TValue Map<TKey, TValue>::default::get( TKey key )
{	
	// validate input key
	if( key == nullptr ) throw gcnew ArgumentNullException("key");

	TValue		value;
	
	// find value with specified key (in case of unsuccessful search
	// exception KeyNotFoundException will be raised by)
	if( !Find( key, value ) ) throw gcnew KeyNotFoundException(
		"The given key was not present in the collection.");

	return value;
}

generic<typename TKey, typename TValue>
void Map<TKey, TValue>::default::set( TKey key, TValue value )
{	
	// validate input key
	if( key == nullptr ) throw gcnew ArgumentNullException("key");

	//fire event before the action
	OnSet( key, value );

	// add pair to red-black tree
	Insert( key, value, true );

	// fire event after the action (if error will be raised
	// all changes will be rolled back)
	try {
		// handler call
		OnSetComplete( key, value );
	} catch( Exception^ ) {
		// roll back changes
		Undo();
		// restore exception
		throw;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Gets an object that can be used to synchronize access to the Map.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
Object^ Map<TKey, TValue>::SyncRoot::get( void  )
{
	return _lock;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets a collection containing the keys in the Map.
/// </summary><remarks>
/// This propery returns readonly, standalone collection of keys.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
ICollection<TKey>^ Map<TKey, TValue>::Keys::get( void )
{
	List<TKey>	^keys = gcnew List<TKey>();

	
	// pass through collection as O(n) operation and create
	// standalone list of keys (i can't use array because
	// i don't know pairs count confidently and i need to
	// lock collection while fill it)
	for each( KeyValuePair<TKey, TValue> pair in this ) {
		// store to list
		keys->Add( pair.Key );
	}

	// return read-only wrapper for this list
	return keys->AsReadOnly();
}


//-------------------------------------------------------------------
/// <summary>
/// Gets a collection containing the values in the Map.
/// </summary><remarks>
/// This propery returns readonly, standalone collection of values.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
ICollection<TValue>^ Map<TKey, TValue>::Values::get( void )
{
	List<TValue>	^values = gcnew List<TValue>();

	
	// pass through collection as O(n) operation and create
	// standalone list of keys (i can't use array because
	// i don't know pairs count confidently and i need to
	// lock collection while fill it)
	for each( KeyValuePair<TKey, TValue> pair in this ) {
		// store to list
		values->Add( pair.Value );
	}

	// return read-only wrapper for this list
	return values->AsReadOnly();
}


//----------------------------------------------------------------------------
//								METHODS
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Adds the specified key and value to the collection.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void Map<TKey, TValue>::Add( TKey key, TValue value )
{
	// validate key
	if( key == nullptr ) throw gcnew ArgumentNullException("key");
	
	//fire event before the action
	OnInsert( key, value );

	// add pair to red-black tree (insert function return 'false'
	// in case of having pair with same key, so i can check tree for
	// pair exists)
	if( !Insert( key, value, false ) ) {
		// raise exception
		throw gcnew ArgumentException(
			"An item with the same key already exists.");
	}

	// fire event after the action (if error will be raised
	// all changes will be rolled back)
	try {
		// handler call
		OnInsertComplete( key, value );
	} catch( Exception^ ) {
		// roll back changes
		Undo();
		// restore exception
		throw;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Clears the content of the Map instance.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void Map<TKey, TValue>::Clear( void )
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
		// roll back changes
		Undo();
		// restore exception
		throw;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Determines whether the Map contains the value with specified key.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
bool Map<TKey, TValue>::ContainsKey( TKey key )
{
	// validate input parameters
	if( key == nullptr ) throw gcnew ArgumentNullException("key");

	// return result of the search request
	return Find( key, TValue() );
}


//-------------------------------------------------------------------
/// <summary>
/// Determines whether the Map contains a specific value by using a
/// linear search algorithm.
/// </summary><remarks>
/// This method may use shalow comparison only: it search for the
/// pair having value same as passed using equality comparer to
/// check. The default equality comparer checks whether type T
/// implements the System.IEquatable generic interface and if so
/// returns an EqualityComparer that uses that implementation.
/// Otherwise it returns an EqualityComparer that uses the overrides
/// of Object.Equals and Object.GetHashCode provided by T.
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform "deep" object
/// comparison through operator == or in some other case.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
bool Map<TKey, TValue>::ContainsValue( TValue value )
{
	// check for initialized input value
	if( value == nullptr ) throw gcnew ArgumentNullException("value");

	// i use implemented enumarator that process tree bypass as O(n) 
	for each( KeyValuePair<TKey, TValue> pair in this ) {
		// use equality comparer for specified type
		if( EqualityComparer<TValue>::Default->Equals( pair.Value, value ) ) {
			// search is succeeded	
			return true;
		}
	}
	// search failed
	return false;
}


//-------------------------------------------------------------------
/// <summary>
/// Removes value with the specified key from the collection.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
bool Map<TKey, TValue>::Remove( TKey key )
{
	// validate key
	if( key == nullptr ) throw gcnew ArgumentNullException("key"); 

	TValue		value;	// value corresponding specified key
	
	// find value with specified key and return false
	// in case of key was not found
	if( !Find( key, value) ) return false;

	// fire event before action
	OnRemove( key, value );
	// remove from collection by key
	Delete( key );
	// fire event after action
	try {
		// handler call
		OnRemoveComplete( key, value );
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
/// Gets the value associated with the specified key.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
bool Map<TKey, TValue>::TryGetValue( TKey key, TValue %value )
{
	// validate key
	if( key == nullptr ) throw gcnew ArgumentNullException("key"); 

	// find value with specified key
	return Find( key, value );
}