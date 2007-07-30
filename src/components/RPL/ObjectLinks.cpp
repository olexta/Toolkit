/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		ObjectLinks.cpp												*/
/*																			*/
/*	Content:	Implementation of CObjectLinks class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentObject.h"
#include "ObjectLinks.h"

using namespace System::Threading;
using namespace RPL;


//----------------------------------------------------------------------------
//								CObjectLinks
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Class constructor. It is provided like internal constructor to
// deny public access.
//
//-------------------------------------------------------------------
CObjectLinks::CObjectLinks( CPersistentObject ^owner ): \
	m_changed(false)
{
	// check for initialized reference
	if( owner == nullptr ) throw gcnew ArgumentNullException("owner");

	m_owner = owner;
}


//-------------------------------------------------------------------
//
// Create CObjectLinks instance initialized with all the items in the
// given collection and store owner object. If some object is not
// unique in collection only the first reference will be added. Null
// references will not be added to instance. It is provided like
// internal constructor to deny public access.
//
//-------------------------------------------------------------------
CObjectLinks::CObjectLinks( CPersistentObject ^owner, \
							IEnumerable<CPersistentObject^> ^e ): \
	CPersistentObjects(e), m_changed(false)
{
	// check for initialized references
	if( owner == nullptr ) throw gcnew ArgumentNullException("owner");
	
	m_owner = owner;
}


//-------------------------------------------------------------------
//
// Class copy constructor. Create persistent links collection based
// on another CObjectLinks instance. Copy all internal data.
//
//-------------------------------------------------------------------
CObjectLinks::CObjectLinks( const CObjectLinks %links )
{
	// copy internal data
	m_owner = links.m_owner;
	m_changed = links.m_changed;
	// copy content
	m_list.AddRange( %(const_cast<CObjectLinks%> (links)) );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes when clearing the contents
/// of the CObjectLinks instance.
/// </summary><remarks>
/// If collection is not empty mark it as changed. Also, for each
/// link in collection call parent method to notify about change.
/// </remarks>
//-------------------------------------------------------------------
void CObjectLinks::OnClear( void )
{
	if( m_list.Count > 0 )  {
		
		for each( CPersistentObject ^obj in m_list ) {
			// call parent method to perform addition processing
			((IIPersistentObject^) m_owner)->on_change( obj );
		}
		m_changed = true;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before inserting a new link
/// into the CObjectLinks instance.
/// </summary><remarks>
/// Notify parent about action.
/// </remarks>
//-------------------------------------------------------------------
void CObjectLinks::OnInsert( CPersistentObject ^obj )
{
	// call parent method to perform addition processing
	((IIPersistentObject^) m_owner)->on_change( obj );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes when removing an element
/// from the CObjectLinks instance.
/// </summary><remarks>
/// Call owner method to notify it about action.
/// </remarks>
//-------------------------------------------------------------------
void CObjectLinks::OnRemove( CPersistentObject ^obj )
{
	// call parent method to perform addition processing
	((IIPersistentObject^) m_owner)->on_change( obj );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after inserting a new
/// element into the CObjectLinks instance.
/// </summary><remarks>
/// Mark collection as changed.
/// </remarks>
//-------------------------------------------------------------------
void CObjectLinks::OnInsertComplete( CPersistentObject ^obj )
{
	// mark collection as changed	
	m_changed = true;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after removing an element
/// from the CObjectLinks instance.
/// </summary><remarks>
/// Mark collection as changed.
/// </remarks>
//-------------------------------------------------------------------
void CObjectLinks::OnRemoveComplete( CPersistentObject ^obj )
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
bool CObjectLinks::IsListChanged::get( void )
{
	return m_changed;
}