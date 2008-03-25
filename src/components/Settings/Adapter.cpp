/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		Adapter.cpp													*/
/*																			*/
/*	Content:	Implementation of Adapter class								*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright @ 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "ValueBox.h"
#include "Nodes.h"
#include "Item.h"
#include "Adapter.h"

using namespace _SETTINGS;
using namespace _SETTINGS::Adapters;


//----------------------------------------------------------------------------
//						Toolkit::Settings::Adapter
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Convert string from node full path format to adapter path format.
//
// Node full path must starts with name of current adapter that is
// bounded by two delimeters.
//
//-------------------------------------------------------------------
String^ Adapter::path_to_adp( String ^fullpath )
{
	// check for valid symbols in the path (fullpath
	// cann't contain adapter delimeter)
	if( (_delimeter != _adp->Delimeter) &&
		fullpath->Contains( _adp->Delimeter ) ) {
		// throw argument exception
		throw gcnew ArgumentException(
			"Adapter '" + _name + 
			"' doesn't accept any of the following strings in the path: " +
			"'" + _adp->Delimeter + "'.");
	}
	
	// compose prefix for valid fullpath (it consists of
	// current adapter name that is bounded by two delimeters)
	String^	const prefix = _delimeter + _name + _delimeter;
	
	// check that specified full path starts with prefix
	if( !fullpath->StartsWith( prefix ) ) {
		// throw argument exception
		throw gcnew ArgumentException(
			"String '" + fullpath + "' is not valid full path: " +
			"must starts with '" + prefix + "'.");
	}
	
	// replace prefix at the begining by delimeter
	String	^adppath = _delimeter + fullpath->Remove( 0, prefix->Length );
	
	// replace node delimeter by adapter delimeter
	return adppath->Replace( _delimeter, _adp->Delimeter );
}


//-------------------------------------------------------------------
//
// Converts string from adapter path format to node path format.
//
// Adapter path cann't contain node's delimeter.
//
//-------------------------------------------------------------------
String^ Adapter::adp_to_path( String ^adppath )
{
	// check for valid symbols in the path (adapter
	// path cann't contains node's delimeter)
	if( (_delimeter != _adp->Delimeter) &&
		adppath->Contains( _delimeter ) ) {
		// throw argument exception
		throw gcnew ArgumentException(
			"An adapter path cann't contain any of the following strings: " + 
			"'" + _delimeter + "'.");
	}

	// compose node path
	return adppath->Replace( _adp->Delimeter, _delimeter );
}


//-------------------------------------------------------------------
//
// Performs additional custom processes before setting a new parent
// to the current Adapter instance.
//
// Adapter node cann't be added or removed through Nodes accessors,
// so this function throw exception only.
//
//-------------------------------------------------------------------
void Adapter::OnSetParent( Node ^parent )
{
	// throw invalid parent type exception
	throw gcnew InvalidOperationException(
		"Adapter cann't be added/removed through Child's routines. " + 
		"Use Manager::Add and Manager::Remove instead.");
}


//-------------------------------------------------------------------
//
// Create new instance of the Adapter class using specified IAdapter
// interface as parameter.
//
//-------------------------------------------------------------------
Adapter::Adapter( Node ^parent, IAdapter ^adapter ) : \
	Node(adapter->Name, parent), _adp(adapter)
{
	// check name of node (name cann't be empty string)
	// parent constructor was already called, so check
	// initialized const name
	if( _name == "" ) throw gcnew ArgumentException(
		"Name of the adapter cann't be empty string.");
	// check adapter interface for null reference
	if( adapter == nullptr ) throw gcnew ArgumentNullException("adapter");
}


//-------------------------------------------------------------------
//
// Sets a new value to the item that is located by specified
// relative path.
//
// If item is not found by the specified path - it will be created,
// such as all subitems.
//
//-------------------------------------------------------------------
void Adapter::default::set( String ^path, ValueBox value )
{ENTER_WRITE(_lock)

	// check for null reference
	if( path == nullptr ) throw gcnew ArgumentNullException("path");

	// search apropriate node
	Node	^node = Find( path );

	// if specified item was not found,
	// build local subtree by tail of path
	if( path != "" ) {
		// create chunk of items (with value in the leaf) and attach
		// it to the last located node (i use this huge contruction
		// to implement transaction approach)
		node->Childs->Add( Item::create_chain( path, value ) );
	} else {
		// store item value to founded item
		node->Value = value;
	}

EXIT_WRITE(_lock)}


//-------------------------------------------------------------------	
//
// Gets or sets value of adapter.
//
// This property is not supported for Adapter node. So getter returns
// ValueBox with null reference and setter throws exception.
//
//-------------------------------------------------------------------
ValueBox Adapter::Value::get( void )
{
	return nullptr;
}

void Adapter::Value::set( ValueBox value )
{
	throw gcnew InvalidOperationException(
		"Operation is not allowed for this type of node: '" +
		this->GetType()->ToString() + "'.");
}


//-------------------------------------------------------------------
//
// Gets value from adapter that's located on specified path.
//
// Function does not accept relative path.
//
//-------------------------------------------------------------------
ValueBox Adapter::GetValue( String ^fullpath )
{
	// check for null reference
	if( fullpath == nullptr )
		throw gcnew ArgumentNullException("fullpath");

	// convert path to adapter format and return value
	return ValueBox(_adp[path_to_adp( fullpath )]);
}


//-------------------------------------------------------------------
//
// Puts value to adapter using specified full path.
//
// Function does not accept relative path.
//
//-------------------------------------------------------------------
void Adapter::SetValue( String ^fullpath, ValueBox value )
{
	// check for null reference
	if( fullpath == nullptr )
		throw gcnew ArgumentNullException("fullpath");

	// convert path to adapter format and set new value
	_adp[path_to_adp( fullpath )] = value.ToObject();
}


//-------------------------------------------------------------------
//
// Reload node structure from adapter.
//
// This function creates chunks for childs and return it's
// enumerator. Function does not support relative path.
//
//-------------------------------------------------------------------
IEnumerable<Node^>^ Adapter::Load( String ^fullpath )
{
	// check for null reference

	if( fullpath == nullptr ) throw gcnew ArgumentNullException("fullpath");

	// create list of subitems pathes 
	List<String^>	^subPathes = gcnew List<String^>();
	// and load all subitems (pathes) to it
	for each( String ^s in _adp->Reload( path_to_adp( fullpath ) ) ) {
		// convert adapter path to node path
		subPathes->Add( adp_to_path( s ) );
	}

	// to future processing we need only deepest pathes, so
	// sort array to have grouped blocks (path and subpathes)
	subPathes->Sort();
	// and remove all upper pathes
	for( int i = 0; i < subPathes->Count; i++ ) {
		// process as O(n^2)
		for( int j = i+1; j < subPathes->Count; j++ ) {
			// if upper path or same path was found
			if( (subPathes[j] + _delimeter)->StartsWith(
					 subPathes[i] + _delimeter ) ) {
				// remove it from list and move iterator back
				subPathes->RemoveAt( i-- );
				// return to main cycle
				break;
			}
		}
	}

	// create list with results
	List<Node^>	^newChilds = gcnew List<Node^>();
	// reference to current processing child
	Item		^child = nullptr;
	// now, when we have sorted array and must
	// create childs collection with all subchilds
	for( int i = 0; i < subPathes->Count; i++ ) {
		// save next subpath
		String	^s = subPathes[i];
		// check to be able add next chain of
		// subitems to already created child
		if( (child == nullptr) || (child->Name != ParsePath( s )) ) {
			// have different roots: so, create new chain
			child = Item::create_chain( nullptr, subPathes[i] );
			// and add to childs collection
			newChilds->Add( child );
		} else {
			// they are brothers: add to existing child
			Item::create_chain( child, s );
		}
	}

	return newChilds;
}


//-------------------------------------------------------------------
//
// Send flush request to adapter.
//
// Function does not accept relative path.
//
//-------------------------------------------------------------------
void Adapter::Save( String ^fullpath )
{
	// check for null reference
	if( fullpath == nullptr )
		throw gcnew ArgumentNullException("fullpath");

	// convert path to adapter format and notify
	// adapter about flush request
	_adp->Flush( path_to_adp( fullpath ) );
}


//-------------------------------------------------------------------
//
// Removes from adapter item that is located by specified path.
//
// Function does not accept relative path.
//
//-------------------------------------------------------------------
void Adapter::Remove( String ^fullpath )
{
	// check for null reference
	if( fullpath == nullptr )
		throw gcnew ArgumentNullException("fullpath");

	// convert path to adapter format and call 'Remove'
	String	^adppath = path_to_adp( fullpath );
	
	if( !_adp->Remove( adppath ) ) {
		// ignore result, because this is not so important request
		// TODO: output to debug
	}
}


//-------------------------------------------------------------------
//
// Reload all data from source to internal adapter.
//
//-------------------------------------------------------------------
void Adapter::Load( void )
{ENTER_WRITE(_lock)

	// dispose all childs
	for each( Node ^node in _childs ) delete node;
	// and removes it from collection
	_childs->Clear( true );

	// pass call to Load function, that creates subnodes
	for each( Node ^node in Load( RootTraverse( nullptr ) + _delimeter ) ) {
		// attach this nodes to current item
		_childs->Add( node );
	}

EXIT_WRITE(_lock)}


//-------------------------------------------------------------------
//
// Flash all data from internal adapter to source.
//
//-------------------------------------------------------------------
void Adapter::Save( void )
{ENTER_WRITE(_lock)

	// pass call to Save function
	Save( RootTraverse( nullptr ) + _delimeter );

EXIT_WRITE(_lock)}