/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistenObject.ObjectLinks.cpp								*/
/*																			*/
/*	Content:	Implementation of PersistentObject::ObjectLinks class		*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentObject.ObjectLinks.h"

using namespace _RPL;


//-----------------------------------------------------------------------------
//				Toolkit::RPL::PersistentObject::ObjectLinks
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Gets or sets log record for specified object.
//
// While setup new value it means "action" at object rather then
// objects's "state". So, setter process current state, specified
// action and make changes.
//
//-------------------------------------------------------------------
PersistentObject::ObjectLinks::STATE PersistentObject::
ObjectLinks::log_record::get( PersistentObject ^obj )
{
	// initialize as STATE::None
	STATE	state = STATE::None;
	// and try to get record
	m_log->TryGetValue( obj, state );

	return state;
}

void PersistentObject::
ObjectLinks::log_record::set( PersistentObject ^obj, STATE action )
{
	// ignore None action
	if( action == STATE::None ) return;

	// get log record for property with specified name
	STATE	state = STATE::None;
	m_log->TryGetValue( obj, state );

	// depend on current log record, write new
	switch( state ) {
		// there is no record in log, so write action
		case STATE::None:
			m_log[obj] = action; return;
		break;
		// link has been added early
		case STATE::New:
			if( action == STATE::Deleted ) {
				// mark log record as None (this is rollback)
				m_log[obj] = STATE::None; return;
			}
		break;
		// link has been deleted early
		case STATE::Deleted:
			if( action == STATE::New ) {
				// it was in initialized list, so mark it as changed
				m_log[obj] = STATE::None; return;
			}
		break;
	}
	// all other states of object are incompatible with
	// requested action: notify this, but don't raise exception
	System::Diagnostics::Trace::WriteLine( String::Format(
	ERR_LOG_STATES, state.ToString(), action.ToString() ));
}


//-------------------------------------------------------------------
//
// Fill ObjectLinks instance by the specified collection of objects.
//
// All null references will be ignored. If objects are not unique,
// only one reference will be stored.
//
//-------------------------------------------------------------------
void PersistentObject::
ObjectLinks::fill_by( IEnumerable<PersistentObject^> ^e )
{
	// pass through all objects in collection
	for each( PersistentObject ^obj in e ) {
		// ignore all null references
		if( obj == nullptr ) continue;
		// ignore duplicate links
		if( m_list.Contains( obj ) ) continue;

		// add link to this
		m_list.Add( obj );
		// and initialize link state to None
		m_log[obj] = STATE::None;
	}
}


//-------------------------------------------------------------------
//
// Remove all  properties from current ObjectLinks instance.
//
//-------------------------------------------------------------------
void PersistentObject::
ObjectLinks::clear( void )
{
	// remove all items
	m_list.Clear();
	// and clear log
	m_log->Clear();
}


//-------------------------------------------------------------------
//
// ITransaction::Begin implementation.
//
// Stores all collection's data until trans_commit will be called to
// have ability to restore data by trans_rollback.
//
//-------------------------------------------------------------------
void PersistentObject::
ObjectLinks::trans_begin( void )
{
	// create backup record
	RESTORE_POINT	point;
	// and fill it by current data
	point._objs = gcnew PersistentObjects(%m_list);
	point._log = gcnew Dictionary<PersistentObject^, STATE>(m_log);

	// push record to stack
	backup.Push( point );
}


//-------------------------------------------------------------------
//
// ITransaction::Commit implementation.
//
// Transaction was completed successfuly, so free backup record.
//
//-------------------------------------------------------------------
void PersistentObject::
ObjectLinks::trans_commit( void )
{
	// remove top record from stack
	backup.Pop();
}


//-------------------------------------------------------------------
//
// ITransaction::Rollback implementation.
//
// Transaction fails, so rollback collection to previous state.
//
//-------------------------------------------------------------------
void PersistentObject::
ObjectLinks::trans_rollback( void )
{
	// get top record from stack
	RESTORE_POINT	point = backup.Pop();

	// clear content
	m_list.Clear();

	// restore previous state
	m_list.AddRange( point._objs );
	m_log = point._log;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before remove all links from
/// the ObjectLinks instance.
/// </summary><remarks>
/// Call owner method to notify it about action.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::
ObjectLinks::OnClear( void )
{
	// notify parent using following specific call
	_owner->on_change( nullptr );

	// look through all links
	for( int i = 0; i < m_list.Count; i++ ) {
		// write 'deleted' log record
		log_record[m_list[i]] = STATE::Deleted;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before inserting a new link
/// into the ObjectLinks instance.
/// </summary><remarks>
/// Call owner method to notify it about action.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::
ObjectLinks::OnInsert( PersistentObject ^obj )
{
	// call parent method to perform addition processing
	_owner->on_change( obj );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes when removing an element
/// from the ObjectLinks instance.
/// </summary><remarks>
/// Call owner method to notify it about action.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::
ObjectLinks::OnRemove( PersistentObject ^obj )
{
	// call parent method to perform addition processing
	_owner->on_change( obj );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after inserting a new
/// element into the ObjectLinks instance.
/// </summary><remarks>
/// Add 'new' log record.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::
ObjectLinks::OnInsertComplete( PersistentObject ^obj )
{
	// add log record
	log_record[obj] = STATE::New;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after removing an element
/// from the ObjectLinks instance.
/// </summary><remarks>
/// Add 'deleted' log record.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::
ObjectLinks::OnRemoveComplete( PersistentObject ^obj )
{
	// add log record
	log_record[obj] = STATE::Deleted;
}


//-------------------------------------------------------------------
/// <summary>
/// Create ObjectLinks instance initialized with parent object.
/// </summary><remarks>
/// Stores parent object to give posibility of state checking and
/// event manipulations.
/// </remarks>
//-------------------------------------------------------------------
PersistentObject::
ObjectLinks::ObjectLinks( PersistentObject ^owner ): \
	_owner(owner)
{
	// check for initialized reference
	if( owner == nullptr ) throw gcnew ArgumentNullException("owner");

	// create change log
	m_log = gcnew Dictionary<PersistentObject^, STATE>();
}


//-------------------------------------------------------------------
/// <summary>
/// Create ObjectLinks instance initialized with all the items in the
/// given collection and store owner object.
/// </summary><remarks>
/// If some object is not unique in the collection, then only one
/// reference will be added. All null references will be ignored.
/// </remarks>
//-------------------------------------------------------------------
PersistentObject::
ObjectLinks::ObjectLinks( PersistentObject ^owner,			   \
						  IEnumerable<PersistentObject^> ^e ): \
	_owner(owner)
{
	// check for initialized references
	if( owner == nullptr ) throw gcnew ArgumentNullException("owner");
	if( e == nullptr ) throw gcnew ArgumentNullException("e");

	// create change log
	m_log = gcnew Dictionary<PersistentObject^, STATE>();

	// fill instance by specified collection
	fill_by( e );
}


//-------------------------------------------------------------------
/// <summary>
/// Class disposer.
/// </summary><remarks>
/// Clears content and log only.
/// </remarks>
//-------------------------------------------------------------------
PersistentObject::
ObjectLinks::~ObjectLinks( void )
{
	// just call 'clear'
	clear();
}


//-------------------------------------------------------------------
/// <summary>
/// Indicates that content was changed.
/// </summary>
//-------------------------------------------------------------------
bool PersistentObject::
ObjectLinks::IsChanged::get( void )
{
	// look through log records
	for each( STATE state in m_log->Values ) {
		// if exists record about change
		if( state != STATE::None ) return true;
	}
	return false;
}


//-------------------------------------------------------------------
/// <summary>
/// Accept all changes of content in this ObjectLinks instance.
/// </summary><remarks><para>
/// This function is used in 'Save' request to mark links as
/// up-to-date.</para><para>
/// After this function call, all links to non existing objects (new
/// and deleted) will be removed from the collection.
/// </para></remarks>
//-------------------------------------------------------------------
void PersistentObject::
ObjectLinks::Accept( void )
{
	// clear log
	m_log->Clear();

	// look through all links
	for( int i = 0; i < m_list.Count; i++ ) {
		// now, depend on object's ID performs:
		if( m_list[i]->m_id > 0 ) {
			// this is normal object, so create log record
			m_log[m_list[i]] = STATE::None;
		} else {
			// this is new/deleted object:
			// remove it from the collection
			m_list.RemoveAt( i-- );
		}
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Returns links that are in one of the specified states.
/// </summary><remarks>
/// This function returns links to existing objects only: all new and
/// deleted objects will be ignored.
/// </remarks>
//-------------------------------------------------------------------
PersistentObjects^ PersistentObject::
ObjectLinks::Get( STATE states )
{
	// create empty object collection
	PersistentObjects	^objs = gcnew PersistentObjects();

	// look through all pairs in log
	for each( KeyValuePair<PersistentObject^, STATE> pair in m_log ) {
		// and if states are equals
		if( ((states & pair.Value) != STATE::None) && (pair.Key->m_id > 0) ) {
			// add link to result collection
			objs->Add( pair.Key );
		}
	}
	return objs;
}


//-------------------------------------------------------------------
/// <summary>
/// Fill current ObjectLinks instance by specified collection of
/// links.
/// </summary><remarks>
/// This function is used in 'Retrieve' request to update object
/// links.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::
ObjectLinks::Reload( IEnumerable<PersistentObject^> ^e )
{
	// check for null reference
	if( e == nullptr ) throw gcnew ArgumentNullException( "e" );

	// remove all items
	clear();
	// and fill instance by given enumeration
	fill_by( e );
}
