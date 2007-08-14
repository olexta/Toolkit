/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentObjects.cpp										*/
/*																			*/
/*	Content:	Implementation of CPersistentObjects class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentObjects.h"

using namespace System::Threading;
using namespace RPL;


// Define lock macroses
#define ENTER(lock)		try { Monitor::Enter( lock );
#define EXIT(lock)		} finally { Monitor::Exit( lock ); }


//----------------------------------------------------------------------------
//						ICollection & ICollection<T>
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Gets a value indicating whether the CPersistentObjects is
// read-only.
//
//-------------------------------------------------------------------
bool CPersistentObjects::IsReadOnly::get( void )
{
	return false;
}


//-------------------------------------------------------------------
//
// Gets a value indicating whether access to the CPersistentObjects
// is synchronized (thread safe).
//
//-------------------------------------------------------------------
bool CPersistentObjects::IsSynchronized::get( void )
{
	return false;
}


//-------------------------------------------------------------------
//
// Gets an object that can be used to synchronize access to the
// CPersistentObjects.
//
//-------------------------------------------------------------------
Object^ CPersistentObjects::SyncRoot::get( void  )
{
	return _lock_this;
}


//-------------------------------------------------------------------
//
// Copies the elements of the CPersistentObjects to an System.Array,
// starting at a particular System.Array index.
//
//-------------------------------------------------------------------
void CPersistentObjects::CopyTo( Array ^dest, int index )
{
	// get ICollection interface for list
	Collections::ICollection	^c = %m_list;
	// and copy it content
	c->CopyTo( dest, index );
}


//-------------------------------------------------------------------
//
// Copies the elements of the CPersistentObjects to an array of type
// CPersistentObject^, starting at the specified array index. 
//
//-------------------------------------------------------------------
void CPersistentObjects::CopyTo( array<CPersistentObject^> ^dest, int index )
{
	m_list.CopyTo( dest, index );
}


//-------------------------------------------------------------------
//
// Returns an enumerator that iterates through a collection.
//
//-------------------------------------------------------------------
Collections::IEnumerator^ CPersistentObjects::GetEnumerator( void )
{
	return GetGEnumerator();
}


//-------------------------------------------------------------------
//
// Returns an enumerator that iterates through a generic collection.
//
//-------------------------------------------------------------------
IEnumerator<CPersistentObject^>^ CPersistentObjects::GetGEnumerator( void )
{	
	return m_list.GetEnumerator();
}


//----------------------------------------------------------------------------
//								CPersistentObjects
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before clearing the contents
/// of the CPersistentObjects instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// collection is cleared.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObjects::OnClear( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before removing an object
/// from the CPersistentObjects instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// specified object is removed.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObjects::OnRemove( CPersistentObject ^obj )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before inserting a new object
/// into the CPersistentObjects instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// specified object is inserted.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObjects::OnInsert( CPersistentObject ^obj )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after clearing the contents
/// of the CPersistentObjects instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// collection is cleared.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObjects::OnClearComplete( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after removing an object from
/// the CPersistentObjects instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// specified property is removed.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObjects::OnRemoveComplete( CPersistentObject ^obj )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after inserting a new object
/// into the CPersistentObjects instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// specified object is inserted.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObjects::OnInsertComplete( CPersistentObject ^obj )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Default class constructor.
/// </summary><remarks>
/// It's task is to create the synchronization object only.
/// </remarks>
//-------------------------------------------------------------------
CPersistentObjects::CPersistentObjects( void ): \
	_lock_this(gcnew Object())
{
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the CPersistentObjects class initialized with
/// the all objects in the given collection.
/// </summary><remarks>
/// If object is not unique in the collection then only the first
/// reference will be added. Null references will not be added to the
/// instance.
/// </remarks>
//-------------------------------------------------------------------
CPersistentObjects::CPersistentObjects( IEnumerable<CPersistentObject^> ^e ): \
	_lock_this(gcnew Object())
{
	// pass entire through collection
	for each( CPersistentObject ^obj in e ) {
		// disable add null references
		if( obj == nullptr ) continue;
		// and add object if it is not
		// in collection already
		if( !Contains( obj ) ) m_list.Add( obj );
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Operator ==. Check for another instance of CPersistentObjects to
/// be equal to this.
/// </summary><remarks>
/// I check content equivalence.
/// </remarks>
//-------------------------------------------------------------------
bool CPersistentObjects::operator==( const CPersistentObjects %objs )
{
	CPersistentObjects	%val = const_cast<CPersistentObjects%>( objs );


	// check for same instance
	if( this == %objs ) return true;

	// this is shallow check for equal
	if( m_list.Count != val.m_list.Count ) return false;

	// and this is deep check by content
	for each( CPersistentObject ^obj in val.m_list ) {
		// check for exists in this instance
		if( !Contains( obj ) ) return false;
	}
	return true;
}


//-------------------------------------------------------------------
/// <summary>
/// Operator !=. Check for another instance of CPersistentObjects to
/// be not equal to this.
/// </summary><remarks>
/// I use = (Equal) operator in implementation.
/// </remarks>
//-------------------------------------------------------------------
bool CPersistentObjects::operator!=( const CPersistentObjects %objs )
{
	return !(*this == objs);
}


//-------------------------------------------------------------------
/// <summary>
/// Gets the number of objects contained in the CPersistentObjects
/// instance.
/// </summary>
//-------------------------------------------------------------------
int CPersistentObjects::Count::get( void )
{
	return m_list.Count;
}


//-------------------------------------------------------------------
/// <summary>
/// Searches for an object that matches the conditions defined by the
/// specified predicate, and returns the first occurrence within the
/// entire CPersistentObjects.
/// </summary>
//-------------------------------------------------------------------
CPersistentObject^ CPersistentObjects::Find( Predicate<CPersistentObject^> ^match )
{
	return m_list.Find( match );
}


//-------------------------------------------------------------------
/// <summary>
/// Retrieves the all the objects that match the conditions defined by
/// the specified predicate.
/// </summary>
//-------------------------------------------------------------------
IEnumerable<CPersistentObject^>^ CPersistentObjects::FindAll( 
									Predicate<CPersistentObject^> ^match )
{	
	return m_list.FindAll( match );
}


//-------------------------------------------------------------------
/// <summary>
/// Adds a object into the collection.
/// </summary>
//-------------------------------------------------------------------
void CPersistentObjects::Add( CPersistentObject ^obj )
{ENTER(_lock_this)

	// validate object
	if( obj == nullptr ) throw gcnew ArgumentNullException("obj");
	// check for object already exists in the collection
	if( Contains( obj ) ) {
	
		throw gcnew ArgumentException("An object already exists in the collection!");
	}
	//fire event before the action
	OnInsert( obj );
	// add property to dict (using name as key)
	m_list.Add( obj );
	// fire event after the action 
	try {
		OnInsertComplete( obj );
	} catch( Exception^ ) {
		// roll back changes
		m_list.Remove( obj );
		throw;
	}

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Clears the contents of the CPersistentObjects instance. 
/// </summary><remarks>
/// If error was raised in OnClearComplete no changes will be rolled
/// back.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObjects::Clear( void )
{ENTER(_lock_this)

	// fire event before action
	OnClear();
	// clear properties
	m_list.Clear();
	// fire event after action
	OnClearComplete();

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Determines whether the CPersistentObjects contains a specific
/// object.
/// </summary><remarks>
/// All objects are stored in Broker cache, so they can be presented
/// as singletones and we can check only for reference equals.
/// </remarks>
//-------------------------------------------------------------------
bool CPersistentObjects::Contains( CPersistentObject ^obj )
{
	return m_list.Contains( obj );
}


//-------------------------------------------------------------------
/// <summary>
/// Removes the specified object from the collection.
/// </summary>
//-------------------------------------------------------------------
bool CPersistentObjects::Remove( CPersistentObject ^obj )
{ENTER(_lock_this)

	// validate object
	if( !obj ) throw gcnew ArgumentNullException("obj");

	// OnRemove event must be fired before action, but
	// for allowed action only, so check for object exists
	if( !Contains( obj ) ) return false;

	// fire event before action
	OnRemove( obj );
	// remove from internall list
	m_list.Remove( obj );
	// fire event after action
	try {
		OnRemoveComplete( obj );
	} catch( Exception^ ) {
		// roll back changes
		m_list.Add( obj );
		throw;
	}

	return true;

EXIT(_lock_this)}