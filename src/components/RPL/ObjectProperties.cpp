/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		ObjectProperties.cpp										*/
/*																			*/
/*	Content:	Implementation of ObjectProperties class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentObject.h"
#include "PersistentProperty.h"
#include "ObjectProperties.h"

using namespace _RPL;


//-----------------------------------------------------------------------------
//						Toolkit::RPL::ObjectProperties
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Create ObjectProperties instance initialized with parent object.
//
// Stores parent object to give posibility of state checking and
// event manipulations. It is provided like internal constructor to
// deny public access.
//
//-------------------------------------------------------------------
ObjectProperties::ObjectProperties( PersistentObject ^owner ): \
	m_changed(false)
{
	// check for initialized reference
	if( owner == nullptr ) throw gcnew ArgumentNullException("owner");

	m_owner = owner;
}


//-------------------------------------------------------------------
//
// Create ObjectProperties instance initialized with all the items in
// the given collection and store parent object.
//
// If properties in collection have not unique names then only the
// last value will be stored. All null references will be ignored and
// no events will be faired. PersistentObject have internal handler
// routines, so we must subscribe it on each property on_change
// event. Constructor is provided like internal to deny public
// access.
//
//-------------------------------------------------------------------
ObjectProperties::ObjectProperties( PersistentObject ^owner,			   \
									IEnumerable<PersistentProperty^> ^e ): \
	PersistentProperties(e), m_changed(false)
{
	// check for initialized references
	if( owner == nullptr ) throw gcnew ArgumentNullException("owner");

	m_owner = owner;

	// pass through all stored properties
	for each( IIPersistentProperty ^prop in this ) {
		// and subscribe for events
		prop->OnChange += gcnew PP_CHANGE(m_owner, &IIPersistentObject::OnChange);
	}
}


//-------------------------------------------------------------------
//
// Copy content of ObjectProperties into new instance.
//
// This is copy constructor, so it provide deep copying: all property
// members will be copied so as internal properties of this.
// Constructor is provided like internal to deny public access.
//
//-------------------------------------------------------------------
ObjectProperties::ObjectProperties( const ObjectProperties %props )
{
	// store internal properties
	m_owner = props.m_owner;
	m_changed = props.m_changed;
	
	// path through collection
	for each( PersistentProperty ^prop in
			  const_cast<ObjectProperties%>( props ) ) {
		// always build new property instance using copy
		// constructor and store it for corespondin key in
		// dictionary.
		if( !Insert( prop->Name, gcnew PersistentProperty(*prop), false ) ) {
			// duplicate property
			throw gcnew ArgumentException(
				"Dublicate property exists in input collection.", "props");
		}
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before removing a property
/// from the ObjectProperties instance.
/// </summary><remarks>
/// Call parent routine to notify about.
/// </remarks>
//-------------------------------------------------------------------
void ObjectProperties::OnRemove( PersistentProperty ^prop )
{
	// call parent method to perform addition processing
	((IIPersistentObject^) m_owner)->OnChange( prop, prop->Value, nullptr );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before inserting a property
/// into the ObjectProperties instance.
/// </summary><remarks>
/// Call parent routine to notify about.
/// </remarks>
//-------------------------------------------------------------------
void ObjectProperties::OnInsert( PersistentProperty ^prop )
{
	// call parent method to perform addition processing
	((IIPersistentObject^) m_owner)->OnChange( prop, nullptr, prop->Value );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after removing an element
/// from the ObjectProperties instance.
/// </summary><remarks>
/// Mark collection as changed.
/// </remarks>
//-------------------------------------------------------------------
void ObjectProperties::OnRemoveComplete( PersistentProperty ^prop )
{
	// unsubscribe from old events
	((IIPersistentProperty^) prop)->OnChange -= 
			gcnew PP_CHANGE(m_owner, &IIPersistentObject::OnChange);
	
	// mark collection as changed
	m_changed = true;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after inserting a new
/// property into the ObjectProperties instance.
/// </summary><remarks>
/// Subscribe object "on_change" event handler to property internal
/// "OnChange" event. Also, mark property (for update support) and
/// collection as changed.
/// </remarks>
//-------------------------------------------------------------------
void ObjectProperties::OnInsertComplete( PersistentProperty ^prop )
{
	// subscribe for events
	((IIPersistentProperty^) prop)->OnChange +=
			gcnew PP_CHANGE(m_owner, &IIPersistentObject::OnChange);
	// mark property as changed to use in update
	((IIPersistentProperty^) prop)->SetChanged( true );

	// mark collection as changed
	m_changed = true;
}


//-------------------------------------------------------------------
/// <summary>
/// Determine, that at least one property in collection was changed.
/// </summary><remarks>
/// I use enumerator to pass throught all properties in collection.
/// </remarks>
//-------------------------------------------------------------------
bool ObjectProperties::IsChanged::get( void )
{
	for each( PersistentProperty ^prop in this ) {

		if( prop->IsChanged ) return true;
	}
	return false;
}


//-------------------------------------------------------------------
/// <summary>
/// Determine, that the list of properties was changed (remove, insert
/// operations was completed).
/// </summary>
//-------------------------------------------------------------------
bool ObjectProperties::IsListChanged::get( void )
{
	return m_changed;
}