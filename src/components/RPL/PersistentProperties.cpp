/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentProperties.cpp									*/
/*																			*/
/*	Content:	Implementation of CPersistentProperties class				*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentProperty.h"
#include "PersistentProperties.h"

using namespace System::Threading;
using namespace RPL;


// Define lock macroses
#define ENTER(lock)		try { Monitor::Enter( lock );
#define EXIT(lock)		} finally { Monitor::Exit( lock ); }


//----------------------------------------------------------------------------
//						ICollection & ICollection<T>
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Gets a value indicating whether the CPersistentProperties is
// read-only.
//
//-------------------------------------------------------------------
bool CPersistentProperties::IsReadOnly::get( void )
{
	return false;
}


//-------------------------------------------------------------------
//
// Gets a value indicating whether access to the CPersistentProperties
// is synchronized (thread safe).
//
//-------------------------------------------------------------------
bool CPersistentProperties::IsSynchronized::get( void )
{
	return false;
}


//-------------------------------------------------------------------
//
// Gets an object that can be used to synchronize access to the
// CPersistentProperties.
//
//-------------------------------------------------------------------
Object^ CPersistentProperties::SyncRoot::get( void  )
{
	return _lock_this;
}


//-------------------------------------------------------------------
//
// Determines whether the CPersistentProperties contains a specific
// property. Property is atomic object such as value type in .NET, so
// I not need check for reference equal, therefore i must provide
// deep comparing by using op_Equal. Here i can make some perfomance
// tuning by using name as key to get stored property (this is O(1)).
//
//-------------------------------------------------------------------
bool CPersistentProperties::Contains( CPersistentProperty ^prop )
{
	// check for initialized value
	if ( prop == nullptr ) throw gcnew ArgumentNullException( "prop" );

	// get stored property by name, if exists 
	if ( Contains( prop->Name ) ) return (*prop == *m_dict[prop->Name]);

	// in other case return falsd
	return false;
}


//-------------------------------------------------------------------
//
// Copies the elements of the CPersistentProperties to an System.Array,
// starting at a particular System.Array index.
//
//-------------------------------------------------------------------
void CPersistentProperties::CopyTo( Array ^dest, int index )
{
	// get ICollection interface for Values
	Collections::ICollection	^c = m_dict.Values;
	// and copy it content
	c->CopyTo( dest, index );
}


//-------------------------------------------------------------------
//
// Copies the elements of the CPersistentProperties to an array of
// type CPersistentProperty^, starting at the specified array index. 
//
//-------------------------------------------------------------------
void CPersistentProperties::CopyTo( array<CPersistentProperty^> ^dest, int index )
{
	m_dict.Values->CopyTo( dest, index );
}


//-------------------------------------------------------------------
//
// Returns an enumerator that iterates through a collection.
//
//-------------------------------------------------------------------
Collections::IEnumerator^ CPersistentProperties::GetEnumerator( void )
{
	return GetGEnumerator();
}


//-------------------------------------------------------------------
//
// Returns an enumerator that iterates through a generic collection.
//
//-------------------------------------------------------------------
IEnumerator<CPersistentProperty^>^ CPersistentProperties::GetGEnumerator( void )
{	
	return m_dict.Values->GetEnumerator();
}


//-------------------------------------------------------------------
//
// Removes the specific property from the CPersistentProperties.
//
//-------------------------------------------------------------------
bool CPersistentProperties::Remove( CPersistentProperty ^prop )
{ENTER(_lock_this)

	// validate property
	if( !prop ) throw gcnew ArgumentNullException("prop");

	// i cann't use access only by property name to
	// prevent data loss, so check for property exists
	if ( !Contains( prop ) ) return false;

	// fire event before action
	OnRemove( prop );
	// remove from collection using property name as key
	m_dict.Remove( prop->Name );
	// fire event after action
	OnRemoveComplete( prop );

	return true;

EXIT(_lock_this)}


//----------------------------------------------------------------------------
//							CPersistentProperties
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before clearing the contents
/// of the CPersistentProperties instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// collection is cleared.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentProperties::OnClear( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before removing an property
/// from the CPersistentProperties instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// specified property is removed.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentProperties::OnRemove( CPersistentProperty ^prop )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before inserting a new
/// property into the CPersistentProperties instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// specified property is inserted.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentProperties::OnInsert( CPersistentProperty ^prop )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after clearing the contents
/// of the CPersistentProperties instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// collection is cleared.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentProperties::OnClearComplete( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after removing an property
/// from the CPersistentProperties instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// specified property is removed.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentProperties::OnRemoveComplete( CPersistentProperty ^prop )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after inserting a new
/// property into the CPersistentProperties instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// specified property is inserted.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentProperties::OnInsertComplete( CPersistentProperty ^prop )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Default class constructor.
/// </summary><remarks>
/// It's task is to create the synchronization object only.
/// </remarks>
//-------------------------------------------------------------------
CPersistentProperties::CPersistentProperties( void ): \
	_lock_this(gcnew Object())
{
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the CPersistentProperties class initialized
/// with all property objects in the given collection.
/// </summary><remarks>
/// If properties in collection have not unique names then only the
/// last value will be stored. This constructor provide shallow
/// collection coping: properties will be copied with it's own names
/// and values only. All null references will be ignored and no
/// events will be faired.
/// </remarks>
//-------------------------------------------------------------------
CPersistentProperties::CPersistentProperties( IEnumerable<CPersistentProperty^> ^e ): \
	_lock_this(gcnew Object())
{
	// path through collection
	for each ( CPersistentProperty ^prop in e ) {
		// prevent errors by null references
		if ( prop != nullptr ) {
			// always build new property instance and store
			// it for corespondin key in dictionary
			m_dict[prop->Name] = gcnew CPersistentProperty(prop->Name, prop->Value);
		}
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Class disposer. Dispose all properties in collection.
/// </summary>
//-------------------------------------------------------------------
CPersistentProperties::~CPersistentProperties( void )
{ENTER(_lock_this)

	for each ( CPersistentProperty ^prop in m_dict.Values ) {
		// dispose all properties in collection
		delete prop;
	}

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Operator ==. Check for another instance of CPersistentProperties
/// to be equal to this.
/// </summary><remarks>
/// I check content equivalence.
/// </remarks>
//-------------------------------------------------------------------
bool CPersistentProperties::operator==( const CPersistentProperties %props )
{
	CPersistentProperties	%val = const_cast<CPersistentProperties%> (props);


	// check for same instance
	if ( this == %props ) return true;

	// this is shallow check for equal
	if ( m_dict.Count != val.m_dict.Count ) return false;

	// and this is deep check by content
	for each ( CPersistentProperty ^prop in val.m_dict.Values ) {
		// check for exists and for property equivalent.
		// i can use only Contains call because it check
		// for property equivalent stored in collection.
		if ( !Contains( prop ) ) return false;
	}
	return true;
}


//-------------------------------------------------------------------
/// <summary>
/// Operator !=. Check for another instance of CPersistentProperties
/// to be not equal to this.
/// </summary><remarks>
/// I use Equal operator in implementation.
/// </remarks>
//-------------------------------------------------------------------
bool CPersistentProperties::operator!=( const CPersistentProperties %props )
{
	return !(*this == props);
}


//-------------------------------------------------------------------
/// <summary>
/// Operator []. Return reference to property instance by given name.
/// </summary><remarks>
/// Subscript operator can be used only with MC++ compiler. This
/// routine present property as atomic object and you can use it's
/// Assign operator to set value. If property doesn't exists then new
/// instance will be created.
/// </remarks>
//-------------------------------------------------------------------
CPersistentProperty% CPersistentProperties::operator[]( String ^name )
{
	return *Item[name];
}


//-------------------------------------------------------------------
/// <summary>
/// Gets property with the specified name.
/// </summary><remarks>
/// No setter method defined because of dublicate parameters (each
/// property contains it's name that will be used as dictionary key).
/// If property doesn't exists it will be added (to add property i
/// perform Add method).
/// </remarks>
//-------------------------------------------------------------------
CPersistentProperty^ CPersistentProperties::Item::get( String ^name )
{
	// add property if it doesn't exists yet
	if ( !Contains( name ) ) Add( gcnew CPersistentProperty(name) );
	
	// return property reference
	return m_dict[name];
}


//-------------------------------------------------------------------
/// <summary>
/// Gets a collection containing the names of properties in the
/// CPersistentProperties instance.
/// </summary>
//-------------------------------------------------------------------
ICollection<String^>^ CPersistentProperties::Names::get( void )
{
	return m_dict.Keys;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets the number of properties contained in the instance of the
/// CPersistentProperties class.
/// </summary>
//-------------------------------------------------------------------
int CPersistentProperties::Count::get( void )
{
	return m_dict.Count;
}


//-------------------------------------------------------------------
/// <summary>
/// Determines whether the CPersistentProperties contains the property
/// with specified name.
/// </summary>
//-------------------------------------------------------------------
bool CPersistentProperties::Contains( String ^name )
{
	return m_dict.ContainsKey( name );
}


//-------------------------------------------------------------------
/// <summary>
/// Removes the property with the specified name from the collection.
/// </summary>
//-------------------------------------------------------------------
bool CPersistentProperties::Remove( String ^name )
{ENTER(_lock_this)

	// check for property exists
	if ( !Contains( name ) ) return false;

	// get property to use in events
	CPersistentProperty	^prop = Item[name];

	// fire event before action
	OnRemove( prop );
	// remove from collection by name
	m_dict.Remove( name );
	// fire event after action
	try {
		OnRemoveComplete( prop );
	} catch ( Exception^ ) {
		// roll back changes
		m_dict.Add( prop->Name, prop );
		throw;
	}

	return true;

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Adds a property into the collection.
/// </summary>
//-------------------------------------------------------------------
void CPersistentProperties::Add( CPersistentProperty ^prop )
{ENTER(_lock_this)

	// validate property
	if ( prop == nullptr ) throw gcnew ArgumentNullException("prop");
	// check for property exists
	if ( Contains( prop->Name ) ) {

		throw gcnew ArgumentException( "An property with the same name already exists!" );
	}
	//fire event before the action
	OnInsert( prop );
	// add property to dict (using name as key)
	m_dict.Add( prop->Name, prop );
	// fire event after the action (if error will be raised
	// all changes will be rolled back)
	try {
		OnInsertComplete( prop );
	} catch ( Exception^ ) {
		// roll back changes
		m_dict.Remove( prop->Name );
		throw;
	}

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Clears the content of the CPersistentProperties instance.
/// </summary><remarks>
/// If error was raised in OnClearComplete no changes will be rolled
/// back.
/// </remarks>
//-------------------------------------------------------------------
void CPersistentProperties::Clear( void )
{ENTER(_lock_this)

	// fire event before action
	OnClear();
	// clear properties
	m_dict.Clear();
	// fire event after action
	OnClearComplete();

EXIT(_lock_this)}