/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentProperty.cpp										*/
/*																			*/
/*	Content:	Implementation of PersistentProperty class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "StreamWrapper.h"
#include "PersistentProperty.h"

using namespace System::IO;
using namespace _RPL;


//
// Define macros to ignore exceptions
//
#define TRY(expr)		try { expr; } catch( Exception^ ) {};


//-----------------------------------------------------------------------------
//						Toolkit::RPL::PersistentProperty
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Sets new value for the property instance.
//
// I need have spetsial processing for stream wrapper class.
//
//-------------------------------------------------------------------
void PersistentProperty::set_value( Object ^value )
{
	IIStreamWrapper	^sw = dynamic_cast<IIStreamWrapper^>( m_value );
	
	// check for existing value is StreamWrapper
	if( sw != nullptr ) {
		// unsubscribe from wrapper events
		sw->OnChange -= gcnew SW_CHANGE(this, &PersistentProperty::stream_change);
	}

	// check for new value is Stream
	if( dynamic_cast<Stream^>( value ) != nullptr ) {
		// create wrapper for stream
		sw = gcnew StreamWrapper(dynamic_cast<Stream^>( value ));
		// subscribe to wrapper events
		sw->OnChange += gcnew SW_CHANGE(this, &PersistentProperty::stream_change);
		// change value
		value = sw;
	}

	// store new value
	m_value = value;
}


//-------------------------------------------------------------------
//
// Event handler to catch stream change events.
//
// Handler occurs after successfull stream change. So i must notify
// parent only.
//
//-------------------------------------------------------------------
void PersistentProperty::stream_change( StreamWrapper ^sender )
{
	// mark property as changed
	m_changed = true;

	// fair OnChange event
	TRY( on_change( this, sender, sender ) );
}


//-------------------------------------------------------------------
//
// OnChange event implementation.
//
// I must override the default access on the add, remove, and raise
// events methods to present event as delegate (i must be able copy
// our subscribers in copy constructor).
//
//-------------------------------------------------------------------
void PersistentProperty::on_change::add( PP_CHANGE ^d )
{
	// property is corpuskular object and can belong
	// to only one object, so i need to clear list
	// of subsribers
	Delegate::RemoveAll( m_on_change, m_on_change );

	m_on_change += d;
}

void PersistentProperty::on_change::remove( PP_CHANGE ^d )
{
	m_on_change -= d;
}

void PersistentProperty::on_change::raise( PersistentProperty ^sender, \
										   Object ^oldValue, Object ^newValue )
{
	if( m_on_change != nullptr ) {
		// perform all delegates in event
		m_on_change->Invoke( sender, oldValue, newValue );
	}
}


//-------------------------------------------------------------------
//
// Sets IsChanged property to specified value.
//
// Internal assembly classes must have access to set or reset
// IsChanged property in case of loading properties from server and
// adding new property instances to object.
//
//-------------------------------------------------------------------
void PersistentProperty::set_changed( bool value )
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
void PersistentProperty::OnValidate( Object ^value )
{
	// check for initialized reference
	if( value == nullptr ) throw gcnew ArgumentNullException("value");

	Type	^t = value->GetType();

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
PersistentProperty::PersistentProperty( String ^name ): \
	m_value(DBNull::Value), m_changed(false)
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
PersistentProperty::PersistentProperty( String ^name, Object ^value ): \
	m_changed(false)
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
/// Create persistent property based on another PersistentProperty
/// instance.
/// </summary><remarks>
/// Copy all internal data (deep coping to dublicate object).
/// </remarks>
//-------------------------------------------------------------------
PersistentProperty::PersistentProperty( const PersistentProperty %prop )
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
PersistentProperty::~PersistentProperty( void )
{
	// i don't know why, but for classes inherited
	// from Stream destructor not call throught
	// operator delete, so i use force Close call
	Stream ^stream = dynamic_cast<Stream^>( m_value );
	// close stream	
	if( stream != nullptr ) stream->Close();
	
	// dispose value
	delete m_value;
}


//-------------------------------------------------------------------
/// <summary>
/// Operator =. Set new given value as property value.
/// </summary><remarks>
/// To set value more easy i overload this Assign operator. It only
/// provide Value property setter functionality.
/// </remarks>
//-------------------------------------------------------------------
PersistentProperty% PersistentProperty::operator=( Object ^value )
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
/// Operator =. Assign another instance of PersistentProperty to
/// this.
/// </summary><remarks>
/// Property is atomic object (such as value class), so i must copy
/// data only.
/// </remarks>
//-------------------------------------------------------------------
PersistentProperty% PersistentProperty::operator=( const PersistentProperty %prop )
{
	// check for same instance
	if( this == %prop ) return *this;
	
	// check for equal names of properties
	if( m_name != prop.m_name ) {

		throw gcnew ArgumentException("Property name differ from this!");
	}
	// return instance reference to support all brakets placement
	return (*this = prop.m_value);
}


//-------------------------------------------------------------------
/// <summary>
/// Operator ==. Check for another instance of PersistentProperty to
/// be equal to this.
/// </summary><remarks>
/// I check for name and value equivalence.
/// </remarks>
//-------------------------------------------------------------------
bool PersistentProperty::operator==( const PersistentProperty %prop )
{
	return ((m_name == prop.m_name) &&
			Object::Equals( m_value, prop.m_value ));
}


//-------------------------------------------------------------------
/// <summary>
/// Operator !=. Check for another instance of PersistentProperty to
/// be not equal to this.
/// </summary><remarks>
/// I use = (Equal) operator in implementation.
/// </remarks>
//-------------------------------------------------------------------
bool PersistentProperty::operator!=( const PersistentProperty %value )
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
PersistentProperty::operator String^( void )
{
	return ToString();
}


//-------------------------------------------------------------------
/// <summary>
/// Gets name of the property.
/// </summary>
//-------------------------------------------------------------------
String^ PersistentProperty::Name::get( void )
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
Object^ PersistentProperty::Value::get( void )
{
	return m_value;
}

void PersistentProperty::Value::set( Object ^value )
{
	// validate value
	OnValidate( value );

	// fair OnChange event
	on_change( this, m_value, value );
	
	// check for same objects (we cann't use == operator
	// in case of reference values)
	if( Object::Equals( m_value, value ) ) return;
	
	// store new value
	set_value( value );

	// mark property as changed
	m_changed = true;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets value indicating that property was changed.
/// </summary><remarks>
/// This property is false by default. In case of Value setter use it
/// will be set to true.
/// </remarks>
//-------------------------------------------------------------------
bool PersistentProperty::IsChanged::get( void )
{
	return m_changed;
}


//-------------------------------------------------------------------
/// <summary>
/// Determines whether the specified Object is equal to the current
/// PersistentProperty instance.
/// </summary><remarks>
/// It uses Equal operator in implementation.
/// </remarks>
//-------------------------------------------------------------------
bool PersistentProperty::Equals( Object ^obj )
{
	// attempt object cast to PersistentProperty
	PersistentProperty	^prop = dynamic_cast<PersistentProperty^>( obj );
	// in case of unsuccessful return false
	if( prop == nullptr ) return false;

	// use Equal operator
	return (*this == *prop);
}


//-------------------------------------------------------------------
/// <summary>
/// Returns a String that represents the current Object.
/// </summary><remarks>
/// Override standart object method ToString(): must provide special
/// processing for string representation of internal data.
/// </remarks>
//-------------------------------------------------------------------
String^ PersistentProperty::ToString( void )
{
	return m_value->ToString();
}