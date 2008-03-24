/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		ObjectLinks.cpp												*/
/*																			*/
/*	Content:	Implementation of ObjectLinks class							*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentObject.h"
#include "ObjectLinks.h"

using namespace _RPL;


//-----------------------------------------------------------------------------
//							Toolkit::RPL::ObjectLinks
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Create ObjectLinks instance initialized with owner object.
//
//-------------------------------------------------------------------
ObjectLinks::ObjectLinks( PersistentObject ^owner ): \
	m_changed(false)
{
	// check for initialized reference
	if( owner == nullptr ) throw gcnew ArgumentNullException("owner");

	m_owner = owner;
}


//-------------------------------------------------------------------
//
// Create ObjectLinks instance initialized with all the items in the
// given collection and store owner object.
//
// If some object is not unique in collection only the first
// reference will be added. Null references will not be added to
// instance. It is provided like internal constructor to deny public
// access.
//
//-------------------------------------------------------------------
ObjectLinks::ObjectLinks( PersistentObject ^owner,			   \
						  IEnumerable<PersistentObject^> ^e ): \
	PersistentObjects(e), m_changed(false)
{
	// check for initialized references
	if( owner == nullptr ) throw gcnew ArgumentNullException("owner");
	
	m_owner = owner;
}


//-------------------------------------------------------------------
//
// Class copy constructor.
//
// Create persistent links collection based on another ObjectLinks
// instance. Copy all internal data.
//
//-------------------------------------------------------------------
ObjectLinks::ObjectLinks( const ObjectLinks %links )
{
	// copy internal data
	m_owner = links.m_owner;
	m_changed = links.m_changed;
	// copy content
	m_list.AddRange( %(const_cast<ObjectLinks%>( links )) );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before inserting a new link
/// into the ObjectLinks instance.
/// </summary><remarks>
/// Notify parent about action.
/// </remarks>
//-------------------------------------------------------------------
void ObjectLinks::OnInsert( PersistentObject ^obj )
{
	// call parent method to perform addition processing
	((IIPersistentObject^) m_owner)->OnChange( obj );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes when removing an element
/// from the ObjectLinks instance.
/// </summary><remarks>
/// Call owner method to notify it about action.
/// </remarks>
//-------------------------------------------------------------------
void ObjectLinks::OnRemove( PersistentObject ^obj )
{
	// call parent method to perform addition processing
	((IIPersistentObject^) m_owner)->OnChange( obj );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after inserting a new
/// element into the ObjectLinks instance.
/// </summary><remarks>
/// Mark collection as changed.
/// </remarks>
//-------------------------------------------------------------------
void ObjectLinks::OnInsertComplete( PersistentObject ^obj )
{
	// mark collection as changed	
	m_changed = true;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after removing an element
/// from the ObjectLinks instance.
/// </summary><remarks>
/// Mark collection as changed.
/// </remarks>
//-------------------------------------------------------------------
void ObjectLinks::OnRemoveComplete( PersistentObject ^obj )
{
	// mark collection as changed
	m_changed = true;
}


//-------------------------------------------------------------------
/// <summary>
/// Determine, that the list of links was changed (remove, insert 
/// operations was completed).
/// </summary>
//-------------------------------------------------------------------
bool ObjectLinks::IsListChanged::get( void )
{
	return m_changed;
}