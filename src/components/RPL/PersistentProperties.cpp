/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentProperties.cpp									*/
/*																			*/
/*	Content:	Implementation of PersistentProperties class				*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentProperty.h"
#include "PersistentProperties.h"

using namespace _RPL;


//----------------------------------------------------------------------------
//					Toolkit::RPL::PersistentProperties
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Default class constructor.
/// </summary><remarks>
/// It's task is to create the synchronization object only.
/// </remarks>
//-------------------------------------------------------------------
PersistentProperties::PersistentProperties( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the PersistentProperties class initialized
/// with all property objects in the given collection.
/// </summary><remarks>
/// If properties in collection have not unique names then only the
/// last value will be stored. This constructor provide shallow
/// collection coping: properties will be copied with it's own names
/// and values only. All null references will be ignored and no
/// events will be faired.
/// </remarks>
//-------------------------------------------------------------------
PersistentProperties::PersistentProperties( IEnumerable<PersistentProperty^> ^e )
{
	// path through collection
	for each( PersistentProperty ^prop in e ) {
		// prevent errors by null references
		if( prop != nullptr ) {
			// always build new property instance and store
			// it for corespondin key in dictionary
			Insert( prop->Name, gcnew PersistentProperty(prop->Name, prop->Value), true );
		}
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Class disposer. Dispose all properties in collection.
/// </summary>
//-------------------------------------------------------------------
PersistentProperties::~PersistentProperties( void )
{
	// dispose all properties in collection
	for each( PersistentProperty ^prop in this ) delete prop;
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
PersistentProperty^ PersistentProperties::default::get( String ^name )
{
	// check for input values
	if( name == nullptr ) throw gcnew ArgumentNullException("name");

	PersistentProperty	^prop = nullptr;
	
	// attemt to find property by key and
	// add property if it doesn't exists yet
	if( !Find( name, prop ) ) {
		// create empty property with specified name
		prop = gcnew PersistentProperty(name);
		// add to dictionary
		Add( prop );
	}
	// return property reference
	return prop;
}


//-------------------------------------------------------------------
/// <summary>
/// Determines whether the PersistentProperties contains a specific
/// property.
/// </summary><remarks>
/// Property is atomic object such as value type in .NET, so I not
/// need check for reference equal, therefore i must provide deep
/// comparing by using op_Equal.
/// </remarks>
//-------------------------------------------------------------------
bool PersistentProperties::Contains( PersistentProperty ^prop )
{
	// check for initialized value
	if( prop == nullptr ) throw gcnew ArgumentNullException("prop");

	// get stored property by name, if exists 
	if( Contains( prop->Name ) ) return (*prop == *this[prop->Name]);

	// in other case return false
	return false;
}


//-------------------------------------------------------------------
/// <summary>
/// Clears the contents of the PersistentProperties instance. 
/// </summary><remarks>
/// THIS METHOD IS NOT SUPPORTED!
/// </remarks>
//-------------------------------------------------------------------
void PersistentProperties::Clear( void )
{
	throw gcnew NotImplementedException(
		"Method Clear() is not supported in this implementation." );
}