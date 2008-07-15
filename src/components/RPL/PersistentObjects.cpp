/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentObjects.cpp										*/
/*																			*/
/*	Content:	Implementation of PersistentObjects class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentObjects.h"

using namespace _RPL;


//-----------------------------------------------------------------------------
//						Toolkit::RPL::PersistentObjects
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Returns an enumerator that iterates through a collection.
//
//-------------------------------------------------------------------
Collections::IEnumerator^ PersistentObjects::get_enumerator( void )
{
	return m_list.GetEnumerator();
}


//-------------------------------------------------------------------
//
// Gets a value indicating whether the PersistentObjects is
// read-only.
//
//-------------------------------------------------------------------
bool PersistentObjects::items_is_readonly( void )
{
	return false;
}


//-------------------------------------------------------------------
//
// Returns an enumerator that iterates through a generic collection.
//
//-------------------------------------------------------------------
IEnumerator<PersistentObject^>^ PersistentObjects::items_get_enumerator( void )
{
	return m_list.GetEnumerator();
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before removing all objects
/// from PersistentObjects instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before all
/// objects are removed.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObjects::OnClear( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before inserting a new object
/// into the PersistentObjects instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// specified object is inserted.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObjects::OnInsert( PersistentObject ^obj )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before removing an object
/// from the PersistentObjects instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// specified object is removed.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObjects::OnRemove( PersistentObject ^obj )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after removing all objects
/// from PersistentObjects instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after all
/// objects are removed.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObjects::OnClearComplete( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after inserting a new object
/// into the PersistentObjects instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// specified object is inserted.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObjects::OnInsertComplete( PersistentObject ^obj )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after removing an object from
/// the PersistentObjects instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// specified property is removed.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObjects::OnRemoveComplete( PersistentObject ^obj )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Default class constructor.
/// </summary>
//-------------------------------------------------------------------
PersistentObjects::PersistentObjects( void )
{
	// do nothing
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the PersistentObjects class initialized with
/// the all objects in the given collection.
/// </summary><remarks>
/// If object is not unique in the collection then only one reference
/// will be added. Null references will not be added to the instance.
/// </remarks>
//-------------------------------------------------------------------
PersistentObjects::PersistentObjects( IEnumerable<PersistentObject^> ^e )
{
	// check for null reference
	if( e == nullptr ) throw gcnew ArgumentNullException("e");

	// pass entire through collection
	for each( PersistentObject ^obj in e ) {
		// disable add null references
		if( obj == nullptr ) continue;
		// and add object if it is not in collection already
		if( !m_list.Contains( obj ) ) m_list.Add( obj );
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Gets the number of objects contained in the PersistentObjects
/// instance.
/// </summary>
//-------------------------------------------------------------------
int PersistentObjects::Count::get( void )
{
	return m_list.Count;
}


//-------------------------------------------------------------------
/// <summary>
/// Adds an object into the collection.
/// </summary>
//-------------------------------------------------------------------
void PersistentObjects::Add( PersistentObject ^obj )
{
	// check null reference
	if( obj == nullptr ) throw gcnew ArgumentNullException("obj");
	// check for object already exists in the collection
	if( m_list.Contains( obj ) ) {
		// throw exception
		throw gcnew ArgumentException(ERR_OBJECT_EXISTS);
	}
	
	// fire event before the action
	OnInsert( obj );
	// add object to list
	m_list.Add( obj );
	// fire event after the action 
	try {
		OnInsertComplete( obj );
	} catch( Exception^ ) {
		// roll back changes
		m_list.Remove( obj );
		throw;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Clears the content of the PersistentObjects instance. 
/// </summary>
//-------------------------------------------------------------------
void PersistentObjects::Clear( void )
{
	// fire event before the action
	OnClear();
	
	// create backup list
	List<PersistentObject^>		backup(%m_list);
	// clear collection
	m_list.Clear();
	
	// fire event after the action 
	try {
		OnClearComplete();
	} catch( Exception^ ) {
		// roll back changes
		m_list.AddRange( %backup );
		throw;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Determines whether the PersistentObjects contains a specific
/// object.
/// </summary>
//-------------------------------------------------------------------
bool PersistentObjects::Contains( PersistentObject ^obj )
{
	return m_list.Contains( obj );
}


//-------------------------------------------------------------------
///<summary>
/// Copies the elements of the PersistentObjects to an array of type
/// PersistentObject^, starting at the specified array index. 
/// </summary>
//-------------------------------------------------------------------
void PersistentObjects::CopyTo( array<PersistentObject^> ^dest, int index )
{
	m_list.CopyTo( dest, index );
}


//-------------------------------------------------------------------
/// <summary>
/// Removes the specified object from the collection.
/// </summary>
//-------------------------------------------------------------------
bool PersistentObjects::Remove( PersistentObject ^obj )
{
	// check for null reference
	if( obj == nullptr ) throw gcnew ArgumentNullException("obj");

	// OnRemove event must be fired before action,
	// so check for object exists
	if( !m_list.Contains( obj ) ) return false;

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
}