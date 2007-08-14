/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentProperty.cpp										*/
/*																			*/
/*	Content:	Implementation of CPersistentProperty class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "StreamWrapper.h"
#include "PersistentProperty.h"

using namespace System::IO;
using namespace System::Threading;
using namespace RPL;


// Define lock macroses
#define ENTER(lock)		try { Monitor::Enter( lock );
#define EXIT(lock)		} finally { Monitor::Exit( lock ); }

// Define macros to ignore exceptions
#define TRY(expr)		try { expr; } catch( Exception^ ) {};


//----------------------------------------------------------------------------
//							CPersistentProperty
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// This routine set new value for this property. I need have spetsial
// processing for stream wrapper class.
//
//-------------------------------------------------------------------
void CPersistentProperty::set_value( Object ^value )
{
	IIStreamWrapper	^sw = nullptr;


	// check for existing value is StreamWrapper
	sw = dynamic_cast<IIStreamWrapper^>( m_value );
	if( sw != nullptr ) {
		// unsubscribe from wrapper events
		sw->OnChange -= gcnew SW_CHANGE(this, &CPersistentProperty::on_change);
	}

	// check for new value is Stream
	if( dynamic_cast<Stream^>( value ) != nullptr ) {
		// create wrapper for stream
		sw = gcnew StreamWrapper(dynamic_cast<Stream^>( value ));
		// subscribe to wrapper events
		sw->OnChange += gcnew SW_CHANGE(this, &CPersistentProperty::on_change);
		// change value
		value = sw;
	}

	// store new value
	m_value = value;
}


//-------------------------------------------------------------------
//
// This is event handler to catch stream change events. Handler
// occurs after successfull stream change. So i must notify parent
// only.
//
//-------------------------------------------------------------------
void CPersistentProperty::on_change( StreamWrapper ^sender )
{
	// mark property as changed
	m_changed = true;

	// fair OnChange event
	TRY( OnChange( this, sender, sender ) );
}


//-------------------------------------------------------------------
//
// I must override the default access on the add, remove, and raise
// events methods to present event as delegate (i must be able copy
// our subscribers in copy constructor).
//
//-------------------------------------------------------------------
void CPersistentProperty::OnChange::add( PP_CHANGE ^d )
{
	// property is corpuskular object and can belong
	// to only one object, so i need to clear list
	// of subsribers
	Delegate::RemoveAll( m_on_change, m_on_change );

	m_on_change += d;
}


void CPersistentProperty::OnChange::remove( PP_CHANGE ^d )
{
	m_on_change -= d;
}


void CPersistentProperty::OnChange::raise( CPersistentProperty ^sender,
										   Object ^oldValue, Object ^newValue )
{
	if( m_on_change != nullptr ) {
		// perform all delegates in event
		m_on_change->Invoke( sender, oldValue, newValue );
	}
}


//-------------------------------------------------------------------
//
// Set IsChanged property to specified value. Internal asembly 
// classes must have access to set or reset IsChanged property in
// case of loading properties from server and adding new property
// instances to object.
//
//-------------------------------------------------------------------
void CPersistentProperty::SetChanged( bool value )
{
	m_changed = value;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes when validating a value.
/// </summary><remarks>
/// In default implementation check value of followed types: bool,
/// int, double, DateTime, String, Stream and DBNull. In case of
/// other type to pass - the ArgumentException will be raised. Also
/// check for initialized reference.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentProperty::OnValidate( Object^ value )
{
	Type	^t;


	// check for initialized reference
	if( value == nullptr ) throw gcnew ArgumentNullException("value");

	t = value->GetType();

	// test for all supported types
	if( (t == bool::typeid) || (t == int::typeid) ||
		(t == double::typeid) || (t == DateTime::typeid) ||
		(t == String::typeid) || (t == DBNull::typeid) ) {

		// this is system type
		return;
	} else if( dynamic_cast<Stream^>( value ) != nullptr ) {

		//this is binary stream
		return;
	} else {

		//we cann't support this type
		throw gcnew ArgumentException( \
			"Type '" + t->ToString() + "' not supported!");
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Create property instance with given name that store DBNull value.
/// </summary><remarks>
/// It can be useful to declare object scheme.
/// </remarks>
//-------------------------------------------------------------------
CPersistentProperty::CPersistentProperty( String ^name ): \
	m_value(DBNull::Value), m_changed(false), _lock_this(gcnew Object())
{
	dbgprint( "-> " + name );

	// check for initialized reference
	if( name == nullptr ) throw gcnew ArgumentNullException("name");

	// store property name
	m_name = name;

	dbgprint( "<- " + name );
}


//-------------------------------------------------------------------
/// <summary>
/// Create property instance with given name and initialize it with
/// specified value.
/// </summary><remarks>
/// "Changed" property will be set to false. In attempt of set value
/// that are not supported by current implementation (incompability
/// type) the ArgumentException will be raised.
/// </remarks>
//-------------------------------------------------------------------
CPersistentProperty::CPersistentProperty( String ^name, Object ^value ): \
	m_changed(false), _lock_this(gcnew Object())
{
	dbgprint( String::Format( "-> {0}\n{1}", name, value ) );

	// check for initialized reference
	if( name == nullptr ) throw gcnew ArgumentNullException("name");

	// store property name
	m_name = name;
	
	// validate value
	OnValidate( value );
	// store value
	set_value( value );

	dbgprint( "<- " + name );
}


//-------------------------------------------------------------------
/// <summary>
/// Create persistent property based on another CPersistentProperty
/// instance.
/// </summary><remarks>
/// Copy all internal data (deep coping to dublicate object).
/// </remarks>
//-------------------------------------------------------------------
CPersistentProperty::CPersistentProperty( const CPersistentProperty %prop ): \
	_lock_this(gcnew Object())
{
	dbgprint( String::Format( "-> {0} (copy constructor)\n{1}",
							  prop.m_name, prop.m_value ) );

	// store property name
	m_name = prop.m_name;
	// store value
	set_value( prop.m_value );
	// store state of property
	m_changed = prop.m_changed;
	// copy event's subscribers
	m_on_change += prop.m_on_change;

	dbgprint( "<- " + prop.m_name + " (copy constructor)" );
}


//-------------------------------------------------------------------
/// <summary>
/// Class disposer.
/// </summary><remarks>
/// This is only Dispose call, so i have to call it if internal data
/// implement IDisposable only. But .NET check this interface and i
/// can use delete operator to try call it.
/// </remarks>
//-------------------------------------------------------------------
CPersistentProperty::~CPersistentProperty( void )
{ENTER(_lock_this)

	// i don't know why, but for classes inherited
	// from Stream destructor not call throught
	// operator delete, so i use force Close call
	Stream ^stream = dynamic_cast<Stream^>( m_value );
	// close stream	
	if( stream != nullptr ) stream->Close();
	
	// dispose value
	delete m_value;

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Operator =. Set new given value as property value.
/// </summary><remarks>
/// To set value more easy i overload this Assign operator. It only
/// provide Value property setter functionality.
/// </remarks>
//-------------------------------------------------------------------
CPersistentProperty% CPersistentProperty::operator=( Object^ value )
{
	// check for initialized reference
	if( value == nullptr ) throw gcnew ArgumentNullException("value");
	
	// try set value
	Value = value;
	
	// return instance reference to support all brakets placement
	return *this;
}


//-------------------------------------------------------------------
/// <summary>
/// Operator =. Assign another instance of CPersistentProperty to
/// this.
/// </summary><remarks>
/// Property is atomic object (such as value class), so i must copy
/// data only. To make property treadsafe lock another instance while
/// operation will not be completed.
/// </remarks>
//-------------------------------------------------------------------
CPersistentProperty% CPersistentProperty::operator=( const CPersistentProperty %prop )
{ENTER(_lock_this)

	// check for same instance
	if( this == %prop ) return *this;
	
	// check for equal names of properties
	if( m_name != prop.m_name ) {

		throw gcnew ArgumentException("Property name differ from this!");
	}
	// return instance reference to support all brakets placement
	return (*this = prop.m_value);

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Operator ==. Check for another instance of CPersistentProperty to
/// be equal to this.
/// </summary><remarks>
/// I check for name and value equivalence.
/// </remarks>
//-------------------------------------------------------------------
bool CPersistentProperty::operator==( const CPersistentProperty %prop )
{
	return ((m_name == prop.m_name) &&
			Object::Equals( m_value, prop.m_value ));
}


//-------------------------------------------------------------------
/// <summary>
/// Operator !=. Check for another instance of CPersistentProperty to
/// be not equal to this.
/// </summary><remarks>
/// I use = (Equal) operator in implementation.
/// </remarks>
//-------------------------------------------------------------------
bool CPersistentProperty::operator!=( const CPersistentProperty %value )
{
	return !(*this == value);
}


//-------------------------------------------------------------------
/// <summary>
/// Operator (String^). Return string representation of property
/// value.
/// </summary><remarks>
/// To get value string representation more easy i overload Implicit
/// operator. This is only ToString call.
/// </remarks>
//-------------------------------------------------------------------
CPersistentProperty::operator String^( void )
{
	return ToString();
}


//-------------------------------------------------------------------
/// <summary>
/// Gets name of the property.
/// </summary>
//-------------------------------------------------------------------
String^ CPersistentProperty::Name::get( void )
{	
	return m_name;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets value of the property.
/// </summary><remarks>
/// Changing this value sets "Changed" property to true. In case of 
/// attempt incapability type to set - the ArgumentException will be
/// raised.
/// </remarks>
//-------------------------------------------------------------------
Object^ CPersistentProperty::Value::get( void )
{
	return m_value;
}


void CPersistentProperty::Value::set( Object ^value )
{ENTER(_lock_this)

	// validate value
	OnValidate( value );

	// fair OnChange event
	OnChange( this, m_value, value );
	
	// check for same objects (we cann't use == operator
	// in case of reference values)
	if( Object::Equals( m_value, value ) ) return;
	
	// store new value
	set_value( value );

	// mark property as changed
	m_changed = true;

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Gets value indicating that property was changed.
/// </summary><remarks>
/// This property is false by default. In case of Value setter use it
/// will be set to true.
/// </remarks>
//-------------------------------------------------------------------
bool CPersistentProperty::IsChanged::get( void )
{
	return m_changed;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets an object that can be used to synchronize access to the
/// CPersistentProperty instance.
/// </summary><remarks>
/// The most common case is using property that contains boxed value.
/// But in case of using reference class such as Stream you must lock
/// property access while wiorking with. So, locking trought SyncRoot
/// will suspend all access to this property.
/// </remarks>
//-------------------------------------------------------------------
Object^ CPersistentProperty::SyncRoot::get( void )
{
	return _lock_this;
}


//-------------------------------------------------------------------
/// <summary>
/// Returns a String that represents the current Object.
/// </summary><remarks>
/// Override standart object method ToString(): must provide special
/// processing for Stream type.
/// </remarks>
//-------------------------------------------------------------------
String^ CPersistentProperty::ToString( void )
{
	// check for stream type
	if( dynamic_cast<Stream^>( m_value ) != nullptr ) {
		
		return "<binary data>";
	} else {

		return m_value->ToString();
	}
}