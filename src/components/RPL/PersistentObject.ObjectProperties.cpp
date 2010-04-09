/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistenObject.ObjectProperties.cpp						*/
/*																			*/
/*	Content:	Implementation of PersistenObject::ObjectProperties class	*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentStream.h"
#include "PersistentObject.ObjectProperties.h"

using namespace _RPL;


//-----------------------------------------------------------------------------
//				Toolkit::RPL::PersistenObject::ObjectProperties
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Gets or sets log record for specified property name.
//
// While setup new value it means "action" at property rather then
// property's "state". So, setter process current state, specified
// action and make changes.
//
//-------------------------------------------------------------------
PersistentObject::ObjectProperties::STATE PersistentObject::
ObjectProperties::log_record::get( String ^key )
{
	// initialize as STATE::None
	STATE	state = STATE::None;
	// and try to get record
	m_log->TryGetValue( key, state );

	return state;
}

void PersistentObject::
ObjectProperties::log_record::set( String ^key, STATE action )
{
	// ignore None action
	if( action == STATE::None ) return;

	// get log record for property with specified name
	STATE	state = STATE::None;
	m_log->TryGetValue( key, state );

	// depend on current log record, write new
	switch( state ) {
		// there is no record in log, so write action
		case STATE::None:
			m_log[key] = action; return;
		break;
		// property has been added early
		case STATE::New:
			// ignore change action
			if( action == STATE::Changed ) return;
			if( action == STATE::Deleted ) {
				// mark log record as None (this is rollback)
				m_log[key] = STATE::None; return;
			}
		break;
		// property has been changed early
		case STATE::Changed:
			// ignore change action
			if( action == STATE::Changed ) return;
			if( action == STATE::Deleted ) {
				// create "deleted" log record
				m_log[key] = STATE::Deleted; return;
			}
		break;
		// property has been deleted early
		case STATE::Deleted:
			if( action == STATE::New ) {
				// it was in initialized list, so mark it as changed
				m_log[key] = STATE::Changed; return;
			}
		break;
	}
	// all other states of property are incompatible with
	// requested action: notify this, but don't raise exception
	System::Diagnostics::Trace::WriteLine( String::Format(
	ERR_LOG_STATES, state.ToString(), action.ToString() ));
}


//-------------------------------------------------------------------
//
// Attempt to (un)subscribe to PersistentStream on_change events.
//
// If specified object is PersistentStream returns true, in other
// case returns false.
// Only one subscription to PersistentStream can exist.
//
//-------------------------------------------------------------------
bool PersistentObject::
ObjectProperties::subscribe_to( ValueBox %value, bool subscribe )
{
	// check for being stream
	PersistentStream	^ps = dynamic_cast<PersistentStream^>( value.ToObject() );
	if( ps != nullptr ) {
		// create subscription delegate
		PersistentStream::ON_CHANGE		^change = nullptr;
		change = gcnew PersistentStream::ON_CHANGE( this,
						&PersistentObject::ObjectProperties::on_ps_change );

		// and (un)subscribe depend on passed parameter
		if( subscribe )
			ps->on_change += change;
		else
			ps->on_change -= change;

		return true;
	}
	return false;
}


//-------------------------------------------------------------------
//
// Event handler to catch stream change events.
//
// Handler occurs after stream content change.
//
//-------------------------------------------------------------------
void PersistentObject::
ObjectProperties::on_ps_change( PersistentStream ^sender )
{
	// look through all values
	for each( KeyValuePair<String^, ValueBox> pair in this ) {
		// check for specified value being sender
		if( pair.Value == sender ) {
			// stream conten is already changed, so
			// log action for property before event
			log_record[pair.Key] = STATE::Changed;
			// fair OnChange event
			_owner->on_change( pair.Key, pair.Value, pair.Value );
			// exit loop (collection can contain only
			// one instance of the PersistentStream class)
			break;
		}
	}
}


//-------------------------------------------------------------------
//
// Fill ObjectProperties instance by the specified collection of
// KeyValue pairs.
//
// All pairs with null reference keys will be ignored. If pairs are
// not unique, only last value will be stored.
//
//-------------------------------------------------------------------
void PersistentObject::
ObjectProperties::fill_by( IEnumerable<KeyValuePair<String^, ValueBox>> ^e )
{
	// pass through all pairs in collection
	for each( KeyValuePair<String^, ValueBox> pair in e ) {
		// ignore all null references
		if( pair.Key == nullptr ) continue;

		// if property with same name already exists
		// try to unsubscribe from PersistentStream events
		ValueBox	old;
		if( Find( pair.Key, old ) ) subscribe_to( old, false );

		// add to value to this 
		Insert( pair.Key, pair.Value, true );
		// subscribe to events (if needed)
		subscribe_to( pair.Value, true );
		// and initialize property state to None
		m_log[pair.Key] = STATE::None;
	}
}


//-------------------------------------------------------------------
//
// Remove all  properties from current ObjectProperties instance.
//
//-------------------------------------------------------------------
void PersistentObject::
ObjectProperties::clear( void )
{
	// look through all pairs
	for each( KeyValuePair<String^, ValueBox> pair in this ) {
		// and unsubscribe from PersistentStream events if needed
		subscribe_to( pair.Value, false );
	}
	// then, remove all items
	DeleteAll();
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
ObjectProperties::trans_begin( void )
{
	// process all properties in the collection
	for each( KeyValuePair<String^, ValueBox> pair in this ) {
		// if property value supports transactions
		ITransaction	^trans = dynamic_cast<ITransaction^>(
									pair.Value.ToObject() );
		// notify it about beginning
		if( trans != nullptr ) trans->Begin();
	}

	// create backup record
	RESTORE_POINT	point;
	// and fill it by current data
	point._props = gcnew PersistentProperties(this);
	point._log = gcnew Map<String^, STATE>(m_log);

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
ObjectProperties::trans_commit( void )
{
	// removes top record from stack
	RESTORE_POINT	point = backup.Pop();

	// look through old properties
	for each( KeyValuePair<String^, ValueBox> pair in point._props ) {
		// if property value supports transactions
		ITransaction	^trans = dynamic_cast<ITransaction^>(
									pair.Value.ToObject() );
		// notify it about commit request
		if( trans != nullptr ) trans->Commit();
	}
}


//-------------------------------------------------------------------
//
// ITransaction::Rollback implementation.
//
// Transaction fails, so rollback collection to previous state.
//
//-------------------------------------------------------------------
void PersistentObject::
ObjectProperties::trans_rollback( void )
{
	// get top record from stack
	RESTORE_POINT	point = backup.Pop();

	// clear instance
	clear();
	// restore previous state
	fill_by( point._props );
	m_log = point._log;

	// process all properties in the collection
	for each( KeyValuePair<String^, ValueBox> pair in this ) {
		// if property value supports transactions
		ITransaction	^trans = dynamic_cast<ITransaction^>(
									pair.Value.ToObject() );
		// notify it about aborting
		if( trans != nullptr ) trans->Rollback();
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before remove all properties
/// from the ObjectProperties instance.
/// </summary><remarks>
/// Call parent routine to notify about.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::
ObjectProperties::OnClear( void )
{
	// notify parent using following specific call
	_owner->on_change( nullptr, DBNull::Value, DBNull::Value );

	// process all properties in collection
	for each( KeyValuePair<String^, ValueBox> pair in this ) {
		// if value is PersistentStream, unsubscribe from events
		subscribe_to( pair.Value, false );

		// write 'deleted' log record
		log_record[pair.Key] = STATE::Deleted;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before inserting a property
/// into the ObjectProperties instance.
/// </summary><remarks><para>
/// 1. If inserted property is PersistentStream, subscribe this to
/// on_change event handler. </para><para>
/// 2. Call parent routine to notify about.
/// </para></remarks>
//-------------------------------------------------------------------
void PersistentObject::
ObjectProperties::OnInsert( String ^key, ValueBox value )
{
	// if value is PersistentStream, try subscribe to events
	// (this call can throw exception (in case of multiple
	// subscriptions), so execute it before owner call)
	subscribe_to( value, true );
	try {
		// call parent method to perform addition processing
		_owner->on_change( key, DBNull::Value, value );
	} catch( Exception^ ) {
		// unsubscribe from PersistentStream events
		subscribe_to( value, false );
		// restore exception
		throw;
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
void PersistentObject::
ObjectProperties::OnRemove( String ^key, ValueBox value )
{
	// call parent method to perform addition processing
	_owner->on_change( key, value, DBNull::Value );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before set a value to some
/// property from the ObjectProperties instance.
/// </summary><remarks><para>
/// 1. If property is PersistentStream, subscribe this to on_change
/// event handler. </para><para>
/// 2. Call parent routine to notify about.
/// </para></remarks>
//-------------------------------------------------------------------
void PersistentObject::
ObjectProperties::OnSet( String ^key, ValueBox value )
{
	// initialize old value to DBNull
	ValueBox	old;
	// try to get existing value by key
	bool	exists = Find( key, old );

	// unsubscribe old PersistentStream value from events
	if( exists ) subscribe_to( old, false );
	try {
		// if new value is PersistentStream, try subscribe to events
		// (this call can throw exception (in case of multiple
		// subscriptions), so execute it before owner call)
		subscribe_to( value, true );
		try {
			// call parent method to perform addition processing
			_owner->on_change( key, old, value );
		} catch( Exception^ ) {
			// unsubscribe from PersistentStream events
			subscribe_to( value, false );
			// restore exception
			throw;
		}
	} catch( Exception ^ ) {
		// restore subscription to old PersistentStream value
		if( exists ) subscribe_to( old, true );
		// restore exception
		throw;
	}

	// if this is new property add 'new' record to log
	if( !exists ) log_record[key] = STATE::New;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after inserting a new
/// property into the ObjectProperties instance.
/// </summary><remarks><para>
/// Just adds log record.
/// </para></remarks>
//-------------------------------------------------------------------
void PersistentObject::
ObjectProperties::OnInsertComplete( String ^key, ValueBox value )
{
	// write 'new' log record
	log_record[key] = STATE::New;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after removing a property
/// from the ObjectProperties instance.
/// </summary><remarks><para>
/// 1. If removed property is PersistentStream, unsubscribe this from
/// on_change event handler. </para><para>
/// 2. Add log record.
/// </para></remarks>
//-------------------------------------------------------------------
void PersistentObject::
ObjectProperties::OnRemoveComplete( String ^key, ValueBox value )
{
	// if value is PersistentStream, unsubscribe from events
	subscribe_to( value, false );

	// write 'deleted' log record
	log_record[key] = STATE::Deleted;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after setting a value to
/// some property from the ObjectProperties instance.
/// </summary><remarks>
/// Just adds log record.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::
ObjectProperties::OnSetComplete( String ^key, ValueBox value )
{
	// write 'changed' log record
	log_record[key] = STATE::Changed;
}


//-------------------------------------------------------------------
/// <summary>
/// Create ObjectProperties instance initialized with parent object.
/// </summary><remarks>
/// Stores parent object to give posibility of state checking and
/// event manipulations.
/// </remarks>
//-------------------------------------------------------------------
PersistentObject::
ObjectProperties::ObjectProperties( PersistentObject ^owner ): \
	_owner(owner)
{
	// check for initialized reference
	if( owner == nullptr ) throw gcnew ArgumentNullException("owner");

	// create change log
	m_log = gcnew Map<String^, STATE>();
}


//-------------------------------------------------------------------
/// <summary>
/// Create ObjectProperties instance initialized with all KeyValue
/// pairs items in the given collection and store parent object.
/// </summary><remarks>
/// If pairs in collection have not unique keys then only the last
/// value will be stored. All null references will be ignored and
/// no events will be faired.
/// </remarks>
//-------------------------------------------------------------------
PersistentObject::
ObjectProperties::ObjectProperties(									   \
					PersistentObject ^owner,						   \
					IEnumerable<KeyValuePair<String^, ValueBox>> ^e ): \
	_owner(owner)
{
	// check for initialized references
	if( owner == nullptr ) throw gcnew ArgumentNullException("owner");
	if( e == nullptr ) throw gcnew ArgumentNullException("e");

	// create change log
	m_log = gcnew Map<String^, STATE>();

	// fill instance by specified collection
	fill_by( e );
}


//-------------------------------------------------------------------
/// <summary>
/// Class disposer.
/// </summary><remarks>
/// Must be called if collection doesn't needed anymore. Main goal is
/// to unsubscribe from PersistentStream events.
/// </remarks>
//-------------------------------------------------------------------
PersistentObject::
ObjectProperties::~ObjectProperties( void )
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
ObjectProperties::IsChanged::get( void )
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
/// Accept all changes of content in this ObjectProperties instance.
/// </summary><remarks>
/// This function is used in 'Save' request to mark poperties as
/// up-to-date.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::
ObjectProperties::Accept( void )
{
	// clear log
	m_log->Clear();

	for each( KeyValuePair<String^, ValueBox> pair in this ) {
		// fill log by None states
		m_log[pair.Key] = STATE::None;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Returns properties that are in one of the specified states.
/// </summary><remarks>
/// If requested state is 'Deleted', then return properties with
/// DBNull:Value values.
/// </remarks>
//-------------------------------------------------------------------
PersistentProperties^ PersistentObject::
ObjectProperties::Get( STATE states )
{
	// create empty properties map
	PersistentProperties	^props = gcnew PersistentProperties();

	// look through all pairs in log
	for each( KeyValuePair<String^, STATE> pair in m_log ) {
		// and if states are equals
		if( (states & pair.Value) != STATE::None ) {
			// add property to result collection
			props[pair.Key] = (pair.Value != STATE::Deleted) ?
							  this[pair.Key] : DBNull::Value;
		}
	}
	return props;
}


//-------------------------------------------------------------------
/// <summary>
/// Fill current ObjectProperties instance by specified collection
/// of properties.
/// </summary><remarks>
/// This function is used in 'Retrieve' request to update object
/// properties.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::
ObjectProperties::Reload( IEnumerable<KeyValuePair<String^, ValueBox>> ^e )
{
	// check for null reference
	if( e == nullptr ) throw gcnew ArgumentNullException( "e" );

	// remove all properties
	clear();
	// and fill instance by given enumeration
	fill_by( e );
}
