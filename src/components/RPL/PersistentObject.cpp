/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentObject.cpp										*/
/*																			*/
/*	Content:	Implementation of CPersistentObject class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistenceBroker.h"
#include "ObjectLinks.h"
#include "ObjectProperties.h"
#include "PersistentObject.h"

using namespace System::IO;
using namespace System::Threading;
using namespace _RPL;


// Define lock macroses
#define ENTER(lock)		try { Monitor::Enter( lock );
#define EXIT(lock)		} finally { Monitor::Exit( lock ); }

// Define macros to ignore exceptions
#define TRY(expr)		try { expr; } catch( Exception^ ) {};


//----------------------------------------------------------------------------
//								CPersistentObject
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Check for given states and raise exception with specific message
// if there is any correspondence.
//
//-------------------------------------------------------------------
void CPersistentObject::check_state( bool notNew, bool notDelete, bool notProxy )
{
	String	^s = nullptr;
	// get a string representation of object state
	if( notNew && (m_id == 0) ) {
		// raise for new state
		s = "new object";
	} else if( notDelete && (m_id < 0) ) {
		// raise for deleted state
		s = "deleted object";
	} else if( notProxy && IsProxy ) {
		// raise for proxy
		s = "proxy object";
	}
	if( s != nullptr ) throw gcnew InvalidOperationException( 
							"Cann't perform operation for " + s + "!");
}


//-------------------------------------------------------------------
//
// Handler routine to catch proxy change event. This is post event,
// so checking for right object state was performed already. Mark
// object as changed and call our event routine (ignore exceptions).
//
//-------------------------------------------------------------------
void CPersistentObject::on_change( void )
{
	// mark as changed
	m_changed = true;
	// notify about changes
	TRY( OnChange() );
}


//-------------------------------------------------------------------
//
// Handler routine to catch link change event. Check for right object
// state, call our event routine and mark object as changed.
//
//-----------------------------------------------------------------
void CPersistentObject::on_change( CPersistentObject ^sender )
{
	// check object state
	check_state( false, true, true );
	// fair OnChange event
	OnChange( sender );
	// mark as changed
	m_changed = true;
}


//-------------------------------------------------------------------
//
// Handler routine to catch property change event. This handler more
// dificalt because of having special processing for stream internal
// change events. For this type of events (this is post event
// notifications) i set object state as changed only. For other
// events - check for right object state, call our event routine and
// mark object as changed.
//
//-------------------------------------------------------------------
void CPersistentObject::on_change( CPersistentProperty ^sender,
								   Object ^oldValue, Object ^newValue )
{
	// determine stream content change events
	bool	content = (dynamic_cast<Stream^>( newValue )) && 
					  (newValue == oldValue);
	
	// for stream content change events i must always set
	// state to changed (to provide ability of Retrieve)
	if( content ) m_changed = true;
	
	// check object state
	check_state( false, true, true );
	// fair OnChange event for non stream content change events
	if( !content ) OnChange( sender, oldValue, newValue );
	// mark as changed
	m_changed = true;
}


//-------------------------------------------------------------------
//
// Update existing object with new values. Return true if update can
// be finished (object is up to date) and false in case of links and
// properties must be retrieved. There are several states of object
// before calling this function, so it can be processed in some
// different ways:
// 1. Object is up to date and was not changed: do nothing and return
//    true.
// 2. Object is proxy and not up to date: update needed values and
//    return true.
// 3. Object is full, not up to date or was changed: update needed
//    values and return false.
//
//-------------------------------------------------------------------
bool CPersistentObject::on_retrieve( int id, DateTime stamp, String ^name )
{
	// check for up to date in storage
	if( (m_stamp == stamp) && !m_changed &&
		(m_state != STATE::filling) ) return true;

	// clear links and properties by recreate it.
	// i use Dispose to free tempory resources.
	delete m_links;
	delete m_props;
	m_links = gcnew CObjectLinks(this);
	m_props = gcnew CObjectProperties(this);

	// determine need of OnChange raising (this can be
	// only for existing objects by retrieve request)
	bool changed = ((m_id == id) && (m_stamp != stamp));
	// determine need of links and properties
	// retrieve (if this is proxy, or object was
	// deleted we cann't fill links and properties)
	bool retrieve = !((m_state == STATE::proxy) || (id < 0));

	// update values
	m_id = id;
	m_stamp = stamp;
	m_name = name;
	m_changed = false;

	// i must prevent retrieve by exception in events
	try {
		// raise OnChange if needed
		if( changed ) OnChange();
		// raise OnRetrieve if needed (this is low-lewel function to 
		// work with object, so i cann't use upper-level function
		// Retrieve after it in server, therefore i place here event)
		if( retrieve ) OnRetrieve();
	} catch( Exception^ ) {
		// mark object as proxy and not for future retrieve
		m_state = STATE::proxy;
		retrieve = false;
	}

	return !retrieve;
}


//-------------------------------------------------------------------
//
// This function is called by Broker to fill object by links and
// properties.
//
//-------------------------------------------------------------------
void CPersistentObject::on_retrieve( IEnumerable<CPersistentObject^> ^links,
									 IEnumerable<CPersistentProperty^> ^props )
{
	// recreate links and properties
	m_links = gcnew CObjectLinks(this, links);
	m_props = gcnew CObjectProperties(this, props);
	// set state as full object
	m_state = STATE::full;

	try {
		// notify about object retrieve
		OnRetrieveComplete();
	} catch( Exception^ ) {
		// ignore all exceptions in notification routine
	};
}


//-------------------------------------------------------------------
//
// I must store all object data until trans_commit will be called to
// have ability to restore data by trans_rollback.
//
//-------------------------------------------------------------------
void CPersistentObject::TransactionBegin( void )
{
	// addition processing before transaction begin
	TRY( OnTransactionBegin() );

	// make object copy
	BACKUP_STRUCT ^backup = gcnew BACKUP_STRUCT();
	
	// copy simple object attributes
	backup->_id = m_id;
	backup->_state = m_state;
	backup->_stamp = m_stamp;
	backup->_name = m_name;
	backup->_changed = m_changed;
	// dublicate properties and links
	// using copy constructor
	backup->_links = gcnew CObjectLinks(*m_links);
	backup->_props = gcnew CObjectProperties(*m_props);

	// store backup in transaction stack
	m_trans_stack.Push( backup );
}


//-------------------------------------------------------------------
//
// Transaction was completed successfuly, then we must free resources
// for transaction support.
//
//-------------------------------------------------------------------
void CPersistentObject::TransactionCommit( void )
{
	// addition processing before transaction commit
	TRY( OnTransactionCommit() );

	// remove stored backup data
	m_trans_stack.Pop();
}


//-------------------------------------------------------------------
//
// Transaction fails, so we need to rollback object to previous state.
//
//-------------------------------------------------------------------
void CPersistentObject::TransactionRollback( void )
{
	// addition processing before transaction rollback
	TRY( OnTransactionRollback() );

	// get sored backup data
	BACKUP_STRUCT ^backup = static_cast<BACKUP_STRUCT^>( m_trans_stack.Pop() );

	// restore simple object attributes
	m_id = backup->_id;
	m_state = backup->_state;
	m_stamp = backup->_stamp;
	m_name = backup->_name;
	m_changed = backup->_changed;
	// restore properties and links by
	// simple coping of references
	m_links = backup->_links;
	m_props = backup->_props;
}


//-------------------------------------------------------------------
/// <summary>
/// Default class constructor to build new persistent object.
/// </summary><remarks>
/// For the new objects ID will be set to 0 and IsProxy to false.
/// </remarks>
//-------------------------------------------------------------------
CPersistentObject::CPersistentObject( void ): \
	m_id(0), m_state(STATE::full), m_stamp(), m_name(""), \
	m_changed(false), _lock_this(gcnew Object())
{
	dbgprint( String::Format( "-> {0}", this->GetType() ) );
	
	// create empty collections
	m_links = gcnew CObjectLinks(this);
	m_props = gcnew CObjectProperties(this);

	dbgprint( String::Format( "<- {0}", this->GetType() ) );
}


//-------------------------------------------------------------------
/// <summary>
/// Create CPersistentObject instance of proxy persistent object.
/// </summary><remarks>
/// For this type of object Propertis and Links collections will be
/// empty and no operations is allowed for it.
/// </remarks>
//-------------------------------------------------------------------
CPersistentObject::CPersistentObject( int id, DateTime stamp, String ^name ): \
	m_id(id), m_state(STATE::proxy), m_stamp(stamp), \
	m_changed(false), _lock_this(gcnew Object())
{
	dbgprint( String::Format( "-> {0}\n{1}, {2}, {3}",
							  this->GetType(), id, stamp, name ) );

	// check for initialized reference
	if( name == nullptr ) throw gcnew ArgumentNullException("name");

	m_name = name;

	// create empty collections
	m_links = gcnew CObjectLinks(this);
	m_props = gcnew CObjectProperties(this);

	dbgprint( String::Format( "<- {0}", this->GetType() ) );
}


//-------------------------------------------------------------------
/// <summary>
/// Create CPersistentObject instance of full persistent object.
/// </summary>
//-------------------------------------------------------------------
CPersistentObject::CPersistentObject( int id, DateTime stamp, String ^name, \
									  IEnumerable<CPersistentObject^> ^links, \
									  IEnumerable<CPersistentProperty^> ^props ): \
	m_id(id), m_state(STATE::full), m_stamp(stamp), \
	m_changed(false), _lock_this(gcnew Object())
{
	dbgprint( String::Format( "-> {0}\n{1}, {2}, {3}, {4}, {5}", 
							  this->GetType(), id, stamp, name, links, props ) );

	// check for initialized references
	if( name == nullptr ) throw gcnew ArgumentNullException("name");
	if( links == nullptr ) throw gcnew ArgumentNullException("links");
	if( props == nullptr ) throw gcnew ArgumentNullException("props");

	m_name = name;

	// create filled collections
	m_links = gcnew CObjectLinks(this, links);
	m_props = gcnew CObjectProperties(this, props);

	dbgprint( String::Format( "<- {0}", this->GetType() ) );
}


//-------------------------------------------------------------------
/// <summary>
/// Gets collection of object's links.
/// </summary><remarks>
/// It's STRONGLY recomended to implement all links in derived
/// classes as native routines to provide more deep type and busines
/// logic control.
/// </remarks>
//-------------------------------------------------------------------
CObjectLinks^ CPersistentObject::Links::get( void )
{
	return m_links;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets collection of object's properties.
/// </summary><remarks>
/// It's recomended to implement all properties in derived classes
/// as native routines to provide more deep control of busines logic.
/// You can access to non existing property - in this case it will be
/// created automatically and added to properties list.
/// </remarks>
//-------------------------------------------------------------------
CObjectProperties^ CPersistentObject::Properties::get( void )
{
	return m_props;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets an object that can be used to synchronize access to the
/// CPersistentObject instance.
/// </summary><remarks>
/// Locking trought SyncRoot will suspend all threads that access to
/// this property.
/// </remarks>
//-------------------------------------------------------------------
Object^ CPersistentObject::SyncRoot::get( void )
{
	return _lock_this;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes when transaction starts.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to create it's own save point.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObject::OnTransactionBegin( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes for successfull transaction.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to delete it's own save point that
/// was created by "OnTransactionBegin" early.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObject::OnTransactionCommit( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes when transaction fails.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to restore it's state to previous,
/// saved by "OnTransactionBegin".
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObject::OnTransactionRollback( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before retrieve properties
/// of CPersistentObject instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// object is retrieved from persistance storage.
/// </remarks>

//-------------------------------------------------------------------
void CPersistentObject::OnRetrieve( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after retrieve properties of
/// CPersistentObject instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// object is retrieved from persistance storage.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObject::OnRetrieveComplete( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before save properties and
/// links of CPersistentObject instance to persistence mechanism.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// object's properties and links are saved to persistance storage.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObject::OnSave( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after save properties and
/// links of CPersistentObject instance to persistence mechanism.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// object's properties and links is saved to persistance storage.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObject::OnSaveComplete( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before delete instance of
/// the CPersistentObject class from persistence mechanism.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// object is deleted from persistance storage.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObject::OnDelete( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after delete instance of the
/// CPersistentObject class from persistence mechanism.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// object is deleted from persistance storage.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObject::OnDeleteComplete( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after change proxy object
/// properties (name, datetime).
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// proxy object properties are changeded.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObject::OnChange( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before add/delete new object
/// link to the CPersistentObject instance. 
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// new object's link is added/deleted. Event occurs before action,
/// so you must check for this link exists already to determine type
/// of action.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObject::OnChange( CPersistentObject^ obj )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before add/delete/change
/// persistent property of the CPersistantObject instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// object's persistent property is added/deleted/changed. If there
/// is add event, then oldValue will be set to nullptr. If there is
/// delete event, then newValue will be set to nullptr.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObject::OnChange( CPersistentProperty^ prop, 
								  Object ^oldValue, Object ^newValue )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Operator (String^). Return string representation of the object.
/// </summary><remarks>
/// To get object string representation more easy i overload Implicit
/// operator. This is only ToString call.
/// </remarks>
//-------------------------------------------------------------------
CPersistentObject::operator String^( void )
{
	return ToString();
}


//-------------------------------------------------------------------
/// <summary>
/// Gets identifier that represent object in persistence storage.
/// </summary><remarks>
/// This ID may be not unique identifier in persistance storage, but
/// combination of object type and ID must be unique.
/// </remarks>
//-------------------------------------------------------------------
int CPersistentObject::ID::get( void )
{
	return m_id;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets a value indicating that the current object is proxy.
/// </summary>
//-------------------------------------------------------------------
bool CPersistentObject::IsProxy::get( void )
{
	return (m_state == STATE::proxy);
}


//-------------------------------------------------------------------
/// <summary>
/// Gets a time of the last object's modification.
/// </summary>
//-------------------------------------------------------------------
DateTime CPersistentObject::Stamp::get( void )
{
	return m_stamp;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets a value indicating that the current object is changed.
/// </summary>
//-------------------------------------------------------------------
bool CPersistentObject::IsChanged::get( void )
{
	return m_changed;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets object's short name.
/// </summary><remarks>
/// This property can be overriden in derived classes to be build
/// from one or more object properties.
/// </remarks>
//-------------------------------------------------------------------
String^ CPersistentObject::Name::get( void )
{
	return m_name;
}


void CPersistentObject::Name::set( String^ value )
{ENTER(_lock_this)

	// check object state
	check_state( false, true, false );
	// set new value
	m_name = value;
	// fire OnChange event
	on_change();

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Retrive object properties and links from persistance mechanism.
/// </summary><remarks>
/// It can be used for build full object by proxy. Two events will be
/// raised: OnRetrieve befor and OnRetrieveComplete after operation
/// (this function not fire events emplicitly, it will be fired by
/// filling this object throught on_retrieve). Default implementation
/// check for the next object states: "new" and "deleted".
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObject::Retrieve( void )
{ENTER(_lock_this);

	// check object state
	check_state( true, true, false );

	// if object is proxy now then
	// mark it for filling
	if( m_state == STATE::proxy ) m_state = STATE::filling;

	// call to Broker
	CPersistenceBroker::Broker->RetrieveObject( this );

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Save object links and properties to persistance mechanism.
/// </summary><remarks>
/// For the proxy object name will be stored in DB only. After this
/// operation timestamp will be modified and for the new objects auto
/// generated ID will be assign. Two events will be raised: OnSave
/// before and OnSaveComplete after operation. Default implementation
/// check for the next object states: "deleted".
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObject::Save( void )
{ENTER(_lock_this)

	// check for object state
	check_state( false, true, false );

	// fire OnSave event
	OnSave();

	//call to PersistenceBroker
	CPersistenceBroker::Broker->SaveObject( this );

	// notify about complete
	OnSaveComplete();

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Delete object from persistance mechanism permanently.
/// </summary><remarks>
/// After this operation object will be proxy with ID(-1), stamp(0)
/// and it's own name. No links and properties will be accessed after
/// operation. Proxy object cann't be deleted to prevent data lost
/// without business check, so i retrieve object before. Default
/// implementation check for the next object states: "new" and
/// "deleted".
/// </remarks>
//-------------------------------------------------------------------
void CPersistentObject::Delete( void )
{ENTER(_lock_this)

	// check for object state
	check_state( true, true, false );

	// retrieve object to get new properties for
	// business check
	Retrieve();

	// fair OnDelete event
	OnDelete();

	//call to PersistenceBroker
	CPersistenceBroker::Broker->DeleteObject( this );

	// notify about complete
	OnDeleteComplete();

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Returns a String that represents the current Object.
/// </summary><remarks>
/// Override standart object method ToString(): return object name as
/// string representation.
/// </remarks>
//-------------------------------------------------------------------
String^ CPersistentObject::ToString( void )
{
	return Name;
}