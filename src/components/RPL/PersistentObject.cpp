/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentObject.cpp										*/
/*																			*/
/*	Content:	Implementation of PersistentObject class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include ".\Factories\PersistenceBroker.h"
#include "PersistentObject.ObjectLinks.h"
#include "PersistentObject.ObjectProperties.h"
#include "PersistentObject.h"

using namespace _RPL;
using namespace _RPL::Factories;


//-----------------------------------------------------------------------------
//						Toolkit::RPL::PersistentObject
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Check for object not to be in specified states.
//
// If there is any correspondence then raise exception with specific
// message.
//
//-------------------------------------------------------------------
void PersistentObject::check_state( bool notNew, bool notDelete, bool notProxy )
{
	String	^s = nullptr;
	// get a string representation of object state
	if( notNew && (m_id == 0) ) {
		// raise for new state
		s = "new object";
	} else if( notDelete && (m_id < 0) ) {
		// raise for deleted state
		s = "deleted object";
	} else if( notProxy && (m_state == STATE::Proxy) ) {
		// raise for proxy
		s = "proxy object";
	} else {
		// all right
		return;
	}
	throw gcnew InvalidOperationException( String::Format(
	ERR_PERFORM_OPERATION, s ));
}


//-------------------------------------------------------------------
//
// Handler routine to catch object's name change.
//
// Check for right object state, call event routine and mark object
// as changed.
//
//-------------------------------------------------------------------
void PersistentObject::on_change( String ^oldName, String ^newName )
{
	// check object state
	check_state( false, true, false );
	// notify about name change
	OnChange( oldName, newName );
	// mark as changed
	m_changed = true;
}


//-------------------------------------------------------------------
//
// Handler routine to catch link change event.
//
// Check for right object state, call event routine and mark object
// as changed.
//
//-----------------------------------------------------------------
void PersistentObject::on_change( PersistentObject ^obj )
{
	// check object state
	check_state( false, true, true );
	// fair OnChange event
	OnChange( obj );
	// mark as changed
	m_changed = true;
}


//-------------------------------------------------------------------
//
// Handler routine to catch property change.
//
// Check for right object state, call event routine and mark object
// as changed.
//
//-------------------------------------------------------------------
void PersistentObject::on_change( String ^prop,							 \
								  ValueBox oldValue, ValueBox newValue )
{
	// check object state
	check_state( false, true, true );
	// fair OnChange event
	OnChange( prop, oldValue, newValue );
	// mark as changed
	m_changed = true;
}


//-------------------------------------------------------------------
//
// ITransaction::Begin implementation.
//
// Stores all object's data until trans_commit will be called to have
// ability to restore data by trans_rollback.
//
//-------------------------------------------------------------------
void PersistentObject::trans_begin( void )
{
	// addition processing before transaction begin
	OnTransactionBegin();

	// create backup record
	RESTORE_POINT	point;
	// and fill it by current data
	point._id = m_id;
	point._stamp = m_stamp;
	point._name = m_name;
	point._state = m_state;
	point._changed = m_changed;
	
	// call to collections about transaction begin
	static_cast<ITransaction^>( _links )->Begin();
	static_cast<ITransaction^>( _props )->Begin();

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
void PersistentObject::trans_commit( void )
{
	// addition processing before transaction commit
	OnTransactionCommit();

	// remove top record from stack
	backup.Pop();

	// call to collections about successful transaction
	static_cast<ITransaction^>( _links )->Commit();
	static_cast<ITransaction^>( _props )->Commit();
}


//-------------------------------------------------------------------
//
// ITransaction::Rollback implementation.
//
// Transaction fails, so rollback object to previous state.
//
//-------------------------------------------------------------------
void PersistentObject::trans_rollback( void )
{
	// addition processing before transaction rollback
	OnTransactionRollback();

	// get top record from stack
	RESTORE_POINT	point = backup.Pop();
	// normalize objects cache
	if( (point._id == 0) && (m_id > 0) ) {
		// object was added, so remove from cache
		PersistenceBroker::Cache[HEADER(Type, m_id,
										m_stamp, m_name)] = nullptr;
	} else if( (point._id > 0) && (m_id < 0) ) {
		// object was deleted, so add to cache
		PersistenceBroker::Cache[HEADER(Type, point._id,
										point._stamp, point._name)] = this;
	}
	// restore previous state
	m_id = point._id;
	m_stamp = point._stamp;
	m_name = point._name;
	m_state = point._state;
	m_changed = point._changed;

	// call to collections about transaction rollback
	static_cast<ITransaction^>( _links )->Rollback();
	static_cast<ITransaction^>( _props )->Rollback();
}


//-------------------------------------------------------------------
/// <summary>
/// Submit hardcoded SQL statements to the persistence.
/// </summary><remarks>
/// This is critical feature that allows you to embed SQL in your
/// application code.
/// </remarks>
//-------------------------------------------------------------------
DataSet^ PersistentObject::ProcessSQL( String ^sql )
{
	// lock storage for one executable thread
	Monitor::Enter( PersistenceBroker::Storage );
	try{
		// retrieve DataSet from storage
		return PersistenceBroker::Storage->ProcessSQL( sql );
	} finally {
		// unlock storage in any case
		Monitor::Exit( PersistenceBroker::Storage );
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Default class constructor builds new persistent object.
/// </summary><remarks>
/// For the new objects ID will be set to 0 and IsProxy to false.
/// </remarks>
//-------------------------------------------------------------------
PersistentObject::PersistentObject( void ):	\
	m_id(0), m_stamp(), m_name(""),			\
	m_state(STATE::Full), m_changed(false)
{
	dbgprint( "-> " + this->GetType()->ToString() );
	
	// create empty collections
	_links = gcnew ObjectLinks(this);
	_props = gcnew ObjectProperties(this);

	dbgprint( "<- " + this->GetType()->ToString() );
}


//-------------------------------------------------------------------
/// <summary>
/// Creates proxy PersistentObject instance.
/// </summary><remarks><para>
/// Such constructor must provide each derived class.</para><para>
/// For this type of object Propertis and Links will be empty and no
/// operations are allowed for it.
/// </para></remarks>
//-------------------------------------------------------------------
PersistentObject::PersistentObject( int id, DateTime stamp, String ^name ): \
	m_id(id), m_stamp(stamp), m_name(name),									\
	m_state(STATE::Proxy), m_changed(false)
{
	dbgprint( String::Format( 
	"-> {0}\n{1}, {2}, {3}", this->GetType(), id, stamp, name ) );

	// check for initialized reference
	if( name == nullptr ) throw gcnew ArgumentNullException("name");

	// create empty collections
	_links = gcnew ObjectLinks(this);
	_props = gcnew ObjectProperties(this);

	dbgprint( "<- " + this->GetType()->ToString() );
}


//-------------------------------------------------------------------
/// <summary>
/// Class destructor.
/// </summary><remarks>
/// Switch object to proxy state. This disposer clears transaction
/// stack, mark object as proxy, dispose links and properties.
/// </remarks>
//-------------------------------------------------------------------
PersistentObject::~PersistentObject( void )
{
	// first of all clear transaction stack
	backup.Clear();

	// now, mark object as proxy
	m_state = STATE::Proxy;
	delete _links;
	delete _props;
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
PersistentObjects^ PersistentObject::Links::get( void )
{
	return _links;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets collection of object's properties.
/// </summary><remarks><para>
/// It's recomended to implement all properties in derived classes
/// as native routines to provide more deep control of busines
/// logic.</para><para>
/// You can setup new property by using default setter.
/// </para></remarks>
//-------------------------------------------------------------------
PersistentProperties^ PersistentObject::Properties::get( void )
{
	return _props;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes when transaction starts.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to create it's own save point. But
/// be shure that no exceptions is raised.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::OnTransactionBegin( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes for successfull transaction.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to delete it's own save point that
/// was created by "OnTransactionBegin" early.  But be shure that no
/// exceptions is raised.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::OnTransactionCommit( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes when transaction fails.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to restore it's state to previous,
/// saved by "OnTransactionBegin". But be shure that no exceptions is
/// raised.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::OnTransactionRollback( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before retrieve up-to-date
/// DB data of PersistentObject instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// object is retrieved from persistance storage.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::OnRetrieve( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after retrieve up-to-date DB
/// data of PersistentObject instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// object is retrieved from persistance storage.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::OnRetrieveComplete( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before save properties and
/// links of PersistentObject instance to persistence mechanism.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// object's properties and links are saved to persistance storage.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::OnSave( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after save properties and
/// links of PersistentObject instance to persistence mechanism.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// object's properties and links is saved to persistance storage.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::OnSaveComplete( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before delete instance of
/// the PersistentObject class from persistence mechanism.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// object is deleted from persistance storage.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::OnDelete( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after delete instance of the
/// PersistentObject class from persistence mechanism.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// object is deleted from persistance storage.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::OnDeleteComplete( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before change object name.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// object name is changeded.
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::OnChange( String ^oldName, String ^newName )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before add/delete new object
/// link to the PersistentObject instance. 
/// </summary><remarks><para>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// new object's link is added/deleted. Event occurs before action,
/// so you must check for this link exists already to determine type
/// of action.</para><para>
/// If "obj" parameter is null reference then content of the links
/// collection will be cleared.
/// </para></remarks>
//-------------------------------------------------------------------
void PersistentObject::OnChange( PersistentObject ^obj )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before add/delete/change
/// persistent property of the PersistantObject instance.
/// </summary><remarks><para>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// object's persistent property is added/deleted/changed. If this
/// is add event, then oldValue will be set to DBNull. If this is
/// delete event, then newValue will be set to nullptr.</para><para>
/// If "prop" parameter is null reference then content of properties
/// will be cleared.
/// </para></remarks>
//-------------------------------------------------------------------
void PersistentObject::OnChange( String ^prop,							\
								 ValueBox oldValue, ValueBox newValue )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Gets identifier that represent object in persistence storage.
/// </summary><remarks>
/// This ID may be not unique in persistance storage, but combination
/// of object type and ID must be unique.
/// </remarks>
//-------------------------------------------------------------------
int PersistentObject::ID::get( void )
{
	return m_id;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets a value indicating that the current object is proxy.
/// </summary>
//-------------------------------------------------------------------
bool PersistentObject::IsProxy::get( void )
{
	return (m_state == STATE::Proxy);
}


//-------------------------------------------------------------------
/// <summary>
/// Gets a time of the last object's modification.
/// </summary>
//-------------------------------------------------------------------
DateTime PersistentObject::Stamp::get( void )
{
	return m_stamp;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets a value indicating that the current object is changed.
/// </summary>
//-------------------------------------------------------------------
bool PersistentObject::IsChanged::get( void )
{
	return m_changed;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets object's short name.
/// </summary><remarks>
/// This property can be overriden in derived classes to be composed
/// from one or more object properties.
/// </remarks>
//-------------------------------------------------------------------
String^ PersistentObject::Name::get( void )
{
	return m_name;
}

void PersistentObject::Name::set( String ^value )
{
	// fire OnChange event
	on_change( m_name, value );
	// set new value
	m_name = value;
}


//-------------------------------------------------------------------
/// <summary>
/// Reload proxy/full object from persistance mechanism making it
/// up-to-date.
/// </summary>
/// <param name="upgrade">
/// Used to build full object by proxy.
/// </param><remarks>
/// Two events will be raised during processing: OnRetrieve before
/// and OnRetrieveComplete after operation. Default implementation
/// checks for the next object states: "new" and "deleted".
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::Retrieve( bool upgrade )
{
	// check object state
	check_state( true, true, false );

	// lock storage for one executable thread
	Monitor::Enter( PersistenceBroker::Storage );
	try {
		// fire OnRetrieve event
		OnRetrieve();

		// have to ignore stamp check in some cases
		bool ignore = (_links->IsChanged || _props->IsChanged || 
					   (upgrade && (m_state == STATE::Proxy)));
		// if no timestamp check is needed pass initial DateTime
		HEADER			header(Type, m_id, (ignore ? DateTime() : m_stamp), m_name);
		array<LINK>		^links = nullptr;
		array<PROPERTY>	^props = nullptr;

		// depend on current state select retreive request
		if( upgrade || (m_state == STATE::Full) ) {
			// full retreive request
			PersistenceBroker::Storage->Retrieve( header, links, props );
		} else {
			// retreive header only
			PersistenceBroker::Storage->Retrieve( header );
		}

		// update object
		m_stamp = header.Stamp;
		m_name = header.Name;
		m_changed = false;
		m_state = (upgrade ? STATE::Full : m_state);
		// update links if needed
		if( links != nullptr ) {
			// create list of new links
			PersistentObjects		^newlinks = gcnew PersistentObjects;
			// look through each link in received array
			for each( LINK link in links ) {
				// request object frome cache and add it to list
				newlinks->Add( PersistenceBroker::Cache[link.Header] );
			}
			_links->Reload( newlinks );
		}
		// update properties if needed
		if( props != nullptr ) {
			// create list of new properties
			PersistentProperties	^newprops = gcnew PersistentProperties;
			// look through each property in received array
			for each( PROPERTY prop in props ) {
				// add it to the list
				newprops->Add( prop.Name, prop.Value );
			}
			_props->Reload( newprops );
		}	

		// notify about complete
		OnRetrieveComplete();
	} finally {
		// unlock storage in any case
		Monitor::Exit( PersistenceBroker::Storage );
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Save object to persistance mechanism.
/// </summary><remarks>
/// For the proxy object name will be stored in DB only. After this
/// operation timestamp will be modified and for the new objects auto
/// generated ID will be assign. Two events will be raised: OnSave
/// before and OnSaveComplete after operation. Default implementation
/// check for the next object states: "deleted".
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::Save( void )
{
	// check for object state
	check_state( false, true, false );

	// lock storage for one executable thread
	Monitor::Enter( PersistenceBroker::Storage );
	try {
		// fire OnSave event
		OnSave();

		HEADER	header(Type, m_id, m_stamp, m_name);
		List<LINK>		^links = gcnew List<LINK>;
		List<PROPERTY>	^props = gcnew List<PROPERTY>;
		array<LINK>		^mlinks = nullptr;
		array<PROPERTY>	^mprops = nullptr;

		// compose list of changed links
		for each( PersistentObject ^obj in
				  _links->Get(ObjectLinks::STATE::New) ) {
			links->Add( 
				LINK(HEADER(obj->Type, obj->m_id, obj->m_stamp, obj->m_name),
				LINK::STATE::New ) );
		}
		for each( PersistentObject ^obj in
				  _links->Get(ObjectLinks::STATE::Deleted) ) {
			links->Add( 
				LINK(HEADER(obj->Type, obj->m_id, obj->m_stamp, obj->m_name),
				LINK::STATE::Deleted ) );
		}
		// compose list of changed properties
		for each( KeyValuePair<String^, ValueBox> prop in
				  _props->Get(ObjectProperties::STATE::New) ) {
			props->Add(
				PROPERTY(prop.Key, prop.Value, PROPERTY::STATE::New) );
		}
		for each( KeyValuePair<String^, ValueBox> prop in
				  _props->Get(ObjectProperties::STATE::Deleted) ) {
			props->Add(
				PROPERTY(prop.Key, prop.Value, PROPERTY::STATE::Deleted) );
		}
		for each( KeyValuePair<String^, ValueBox> prop in
				  _props->Get(ObjectProperties::STATE::Changed) ) {
			props->Add(
				PROPERTY(prop.Key, prop.Value, PROPERTY::STATE::Changed) );
		}

		// request storage to save changes
		PersistenceBroker::Storage->Save( header,
										  links->ToArray(), props->ToArray(),
										  mlinks, mprops );
		// if this is new object - add to cache
		if( m_id == 0 ) PersistenceBroker::Cache[header] = this;

		// update object
		m_id = header.ID;
		m_stamp = header.Stamp;
		m_name = header.Name;
		m_changed = false;
		// update links if needed
		if( (mlinks != nullptr) && (mlinks->Length > 0) ) {
			// duplicate existing links
			PersistentObjects		^newlinks = gcnew PersistentObjects(_links);
			// look through all changes
			for each( LINK link in mlinks ) {
				// depend on action
				switch( link.State ) {
					// create new link
					case LINK::STATE::New:
						newlinks->Add( PersistenceBroker::Cache[link.Header] );
					break;
					// delete existing link
					case LINK::STATE::Deleted:
						newlinks->Remove( PersistenceBroker::Cache[link.Header] );
					break;
				}
			}
			_links->Reload( newlinks );
		} else {
			// just accept links changes
			_links->Accept();
		}
		// update properties if needed
		if( mprops != nullptr && mprops->Length > 0 ) {
			// duplicate existing properties
			PersistentProperties	^newprops = gcnew PersistentProperties(_props);
			// look through all changes
			for each( PROPERTY prop in mprops ) {
				// depend on action
				switch( prop.State ) {
					// create new property
					case PROPERTY::STATE::New:
						newprops->Add( prop.Name, prop.Value );
					break;
					// delete existing property
					case PROPERTY::STATE::Deleted:
						newprops->Remove( prop.Name );
					break;
					// change property value
					case PROPERTY::STATE::Changed:
						newprops[prop.Name] = prop.Value;
					break;
				}
			}
			_props->Reload( newprops );
		} else {
			// just accept property changes
			_props->Accept();
		}

		// notify about complete
		OnSaveComplete();
	} finally {
		// unlock storage in any case
		Monitor::Exit( PersistenceBroker::Storage );
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Delete object from persistance mechanism permanently.
/// </summary><remarks>
/// After this operation object will be proxy with ID(-1), stamp(0)
/// and it's own name. No links and properties will be accessed after
/// operation. Default implementation check for the next object
/// states: "new" and "deleted".
/// </remarks>
//-------------------------------------------------------------------
void PersistentObject::Delete( void )
{
	// check for object state
	check_state( true, true, false );

	// lock storage for one executable thread
	Monitor::Enter( PersistenceBroker::Storage );
	try {
		// fair OnDelete event
		OnDelete();

		HEADER	header(Type, m_id, m_stamp, m_name);

		// perform storage delete request
		PersistenceBroker::Storage->Delete( header );
		// remove from cache
		PersistenceBroker::Cache[header] = nullptr;

		// clear object
		m_id = -1;
		m_stamp = DateTime();
		m_changed = false;
		m_state = STATE::Proxy;
		// dispose object links and properties
		delete _links;
		delete _props;

		// notify about complete
		OnDeleteComplete();
	} finally {
		// unlock storage in any case
		Monitor::Exit( PersistenceBroker::Storage ); 
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Returns hash code for the current PersistentObject.
/// </summary><remarks>
/// If object is stored in DB then function returns hash code for
/// string composed from object Type and ID.
/// </remarks>
//-------------------------------------------------------------------
int PersistentObject::GetHashCode( void )
{
	if( m_id > 0) {
		// compose unique string for this instance
		return (Type + m_id)->GetHashCode();
	}

	// return base method
	return Object::GetHashCode();
}


//-------------------------------------------------------------------
/// <summary>
/// Returns a String that represents the current Object.
/// </summary><remarks>
/// Override standart object method ToString(): returns object name
/// in string representation.
/// </remarks>
//-------------------------------------------------------------------
String^ PersistentObject::ToString( void )
{
	return Name;
}