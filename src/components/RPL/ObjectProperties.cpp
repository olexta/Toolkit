/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		ObjectProperties.cpp										*/
/*																			*/
/*	Content:	Implementation of CObjectProperties class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentObject.h"
#include "PersistentProperty.h"
#include "ObjectProperties.h"

using namespace System::Threading;
using namespace RPL;


//----------------------------------------------------------------------------
//								CObjectProperties
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Class constructor. Stores parent object to give posibility of
// state checking and event manipulations. It is provided like
// internal constructor to deny public access.
//
//-------------------------------------------------------------------
CObjectProperties::CObjectProperties( CPersistentObject^ owner ): \
	m_changed(false)
{
	// check for initialized reference
	if( owner == nullptr ) throw gcnew ArgumentNullException("owner");

	m_owner = owner;
}


//-------------------------------------------------------------------
//
// Create CObjectProperties instance initialized with all the items
// in the given collection and store parent object. If properties in
// collection have not unique names then only the last value will be
// stored. All null references will be ignored and no events will be
// faired. CPersistentObject have internal handler routines, so we
// must subscribe it on each property on_change event. Constructor is
// provided like internal to deny public access.
//
//-------------------------------------------------------------------
CObjectProperties::CObjectProperties( CPersistentObject^ owner, 
									  IEnumerable<CPersistentProperty^> ^e ): \
	CPersistentProperties(e), m_changed(false)
{
	// check for initialized references
	if( owner == nullptr ) throw gcnew ArgumentNullException("owner");

	m_owner = owner;

	// pass through all stored properties
	for each( IIPersistentProperty ^prop in m_dict.Values ) {
		// and subscribe for events
		prop->OnChange += gcnew PP_CHANGE(m_owner, &IIPersistentObject::on_change);
	}
}


//-------------------------------------------------------------------
//
// Copy content of CObjectProperties into new instance. This is copy
// constructor, so it provide deep copying: all property members will
// be copied so as internal properties of this. Constructor is
// provided like internal to deny public access.
//
//-------------------------------------------------------------------
CObjectProperties::CObjectProperties( const CObjectProperties %props )
{
	// store internal properties
	m_owner = props.m_owner;
	m_changed = props.m_changed;
	
	// path through collection
	for each( CPersistentProperty ^prop in
			  (const_cast<CObjectProperties%>( props )).m_dict.Values ) {
		// always build new property instance using copy
		// constructor and store it for corespondin key in
		// dictionary.
		m_dict[prop->Name] = gcnew CPersistentProperty(*prop);
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before clearing the contents
/// of the CObjectProperties instance.
/// </summary><remarks>
/// Set collection state to changed.
/// </remarks>
//-------------------------------------------------------------------
void CObjectProperties::OnClear( void )
{
	// pass throught all items and notify parent about changes
	for each( CPersistentProperty ^prop in m_dict.Values ) {
		// call parent method to perform addition processing
		((IIPersistentObject^) m_owner)->on_change( prop, prop->Value, nullptr );
	}
	for each( IIPersistentProperty ^prop in m_dict.Values ) {
		// usubscribe from old events
		prop->OnChange -= gcnew PP_CHANGE(m_owner, &IIPersistentObject::on_change);
	}
	// and mark collection as changed
	if( m_dict.Count > 0 ) m_changed = true;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before removing a property
/// from the CObjectProperties instance.
/// </summary><remarks>
/// Call parent routine to notify about.
/// </remarks>
//-------------------------------------------------------------------
void CObjectProperties::OnRemove( CPersistentProperty ^prop )
{
	// call parent method to perform addition processing
	((IIPersistentObject^) m_owner)->on_change( prop, prop->Value, nullptr );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before inserting a property
/// into the CObjectProperties instance.
/// </summary><remarks>
/// Call parent routine to notify about.
/// </remarks>
//-------------------------------------------------------------------
void CObjectProperties::OnInsert( CPersistentProperty ^prop )
{
	// call parent method to perform addition processing
	((IIPersistentObject^) m_owner)->on_change( prop, nullptr, prop->Value );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after removing an element
/// from the CObjectProperties instance.
/// </summary><remarks>
/// Mark collection as changed.
/// </remarks>
//-------------------------------------------------------------------
void CObjectProperties::OnRemoveComplete( CPersistentProperty ^prop )
{
	// unsubscribe from old events
	((IIPersistentProperty^) prop)->OnChange -= 
			gcnew PP_CHANGE(m_owner, &IIPersistentObject::on_change);
	
	// mark collection as changed
	m_changed = true;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after inserting a new
/// property into the CObjectProperties instance.
/// </summary><remarks>
/// Subscribe object "on_change" event handler to property internal
/// "OnChange" event. Also, mark property (for update support) and
/// collection as changed.
/// </remarks>
//-------------------------------------------------------------------
void CObjectProperties::OnInsertComplete( CPersistentProperty ^prop )
{
	// subscribe for events
	((IIPersistentProperty^) prop)->OnChange +=
			gcnew PP_CHANGE(m_owner, &IIPersistentObject::on_change);
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
bool CObjectProperties::IsChanged::get( void )
{
	for each( CPersistentProperty ^prop in m_dict.Values ) {

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
bool CObjectProperties::IsListChanged::get( void )
{
	return m_changed;
}