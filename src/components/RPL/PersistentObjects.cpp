/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentObjects.cpp										*/
/*																			*/
/*	Content:	Implementation of PersistentObjects class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentObjects.h"

using namespace _RPL;


// Define macros to ignore exceptions
#define TRY(expr)		try { expr; } catch( Exception^ ) {};


//----------------------------------------------------------------------------
//				Toolkit::RPL::PersistentObjects::ICollection<T>
//----------------------------------------------------------------------------

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


//----------------------------------------------------------------------------
//						Toolkit::RPL::PersistentObjects
//----------------------------------------------------------------------------

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
/// </summary><remarks>
/// It's task is to create the synchronization object only.
/// </remarks>
//-------------------------------------------------------------------
PersistentObjects::PersistentObjects( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the PersistentObjects class initialized with
/// the all objects in the given collection.
/// </summary><remarks>
/// If object is not unique in the collection then only the first
/// reference will be added. Null references will not be added to the
/// instance.
/// </remarks>
//-------------------------------------------------------------------
PersistentObjects::PersistentObjects( IEnumerable<PersistentObject^> ^e )
{
	// pass entire through collection
	for each( PersistentObject ^obj in e ) {
		// disable add null references
		if( obj == nullptr ) continue;
		// and add object if it is not in collection already
		// (this will be done by ignoring all exceptions)
		TRY( m_list.Add( obj ) );
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Operator ==. Check for another instance of PersistentObjects to
/// be equal to this.
/// </summary><remarks>
/// I check content equivalence.
/// </remarks>
//-------------------------------------------------------------------
bool PersistentObjects::operator==( const PersistentObjects %objs )
{
	PersistentObjects	%val = const_cast<PersistentObjects%>( objs );

	// check for same instance
	if( this == %objs ) return true;

	// this is shallow check for equal
	if( m_list.Count != val.m_list.Count ) return false;

	// and this is deep check by content
	for each( PersistentObject ^obj in val.m_list ) {
		// check for exists in this instance
		if( !Contains( obj ) ) return false;
	}
	return true;
}


//-------------------------------------------------------------------
/// <summary>
/// Operator !=. Check for another instance of PersistentObjects to
/// be not equal to this.
/// </summary><remarks>
/// I use = (Equal) operator in implementation.
/// </remarks>
//-------------------------------------------------------------------
bool PersistentObjects::operator!=( const PersistentObjects %objs )
{
	return !(*this == objs);
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
/// Adds a object into the collection.
/// </summary>
//-------------------------------------------------------------------
void PersistentObjects::Add( PersistentObject ^obj )
{
	// validate object
	if( obj == nullptr ) throw gcnew ArgumentNullException("obj");
	// check for object already exists in the collection
	if( Contains( obj ) ) {
	
		throw gcnew ArgumentException("An object already exists in the collection!");
	}
	//fire event before the action
	OnInsert( obj );
	// add property to list
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
/// Clears the contents of the PersistentObjects instance. 
/// </summary><remarks>
/// THIS METHOD IS NOT SUPPORTED!
/// </remarks>
//-------------------------------------------------------------------
void PersistentObjects::Clear( void )
{
	throw gcnew NotImplementedException(
		"Method Clear() is not supported in this implementation." );
}


//-------------------------------------------------------------------
/// <summary>
/// Determines whether the PersistentObjects contains a specific
/// object.
/// </summary><remarks>
/// All objects are stored in Broker cache, so they can be presented
/// as singletones and we can check only for reference equals.
/// </remarks>
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
	// validate object
	if( !obj ) throw gcnew ArgumentNullException("obj");

	// OnRemove event must be fired before action,
	// so check for object exists
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
}


//-------------------------------------------------------------------
/// <summary>
/// Determines whether the specified Object is equal to the current
/// PersistentObjects instance.
/// </summary><remarks>
/// It uses Equal operator in implementation.
/// </remarks>
//-------------------------------------------------------------------
bool PersistentObjects::Equals( Object ^obj )
{
	// attempt object cast to PersistentObjects
	PersistentObjects	^objs = dynamic_cast<PersistentObjects^>( obj );
	
	// in case of unsuccessful cast return false,
	// otherwise use Equal operator
	return (objs != nullptr) ? *this == *objs : false;
}