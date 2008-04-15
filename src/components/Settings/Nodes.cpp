﻿/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		Nodes.cpp													*/
/*																			*/
/*	Content:	Implementation of Nodes class								*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright @ 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "Nodes.h"

using namespace _SETTINGS;


//----------------------------------------------------------------------------
//						Toolkit::Settings::Nodes
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Performs additional custom processes before clearing the Nodes
// instance.
//
// Each node must be notified about removing from collection.
//
//-------------------------------------------------------------------
void Nodes::OnClear( void )
{
	// remove parent reference for all nodes
	for each( Node ^node in this ) node->set_parent( nullptr );
}


//-------------------------------------------------------------------
//
// Performs additional custom processes before inserting a node to
// the Nodes instance.
//
// Notify node about adding to collection (this is equal to set
// node's parent reference).
//
//-------------------------------------------------------------------
void Nodes::OnInsert( Node ^node )
{
	// set parent reference
	node->set_parent( _parent );
}


//-------------------------------------------------------------------
//
// Performs additional custom processes before removing a node from
// the Nodes instance.
//
// Notify node about removing from collection (this is equal to
// remove node's parent reference).
//
//-------------------------------------------------------------------
void Nodes::OnRemove( Node ^node )
{
	// remove parent reference
	node->set_parent( nullptr );
}


//-------------------------------------------------------------------
//
// Creates new instance of the Nodes class for specified parent.
//
// This collection is used as childs/subnodes for specified parent
// node.
//
//-------------------------------------------------------------------
Nodes::Nodes( Node ^parent ) : \
	_parent(parent)
{
	// check for the null reference
	if( parent == nullptr ) 
		throw gcnew ArgumentNullException("parent");
}


//-------------------------------------------------------------------
//
// Creates new instance of the Nodes class for specified parent and
// init themself by one child.
//
//-------------------------------------------------------------------
Nodes::Nodes( Node ^parent, Node ^child ) : \
	KeyedMap(child), _parent(parent)
{
	// check for the null reference
	if( parent == nullptr ) 
		throw gcnew ArgumentNullException("parent");

}


//-------------------------------------------------------------------
//
// Creates new instance of the Nodes class for specified parent and
// init themself by collection of childs.
//
//-------------------------------------------------------------------
Nodes::Nodes( Node ^parent, IEnumerable<Node^> ^childs ) : \
	KeyedMap(childs), _parent(parent)
{
	// check for the null reference
	if( parent == nullptr ) 
		throw gcnew ArgumentNullException("parent");

}


//-------------------------------------------------------------------
//
// Find node that is located on specified relative path.
//
// This is overriden KeyedMap accessor. If specified path doesn't
// contain node delimeter, then search in this collection instance.
// In other case pass call to parent.
//
//-------------------------------------------------------------------
Node^ Nodes::default::get( String ^path )
{
	// check for the null reference
	if( path == nullptr ) throw gcnew ArgumentNullException("path");

	// if path contains node delimeter
	if( path->Contains( Node::Delimeter ) ) {
		// pass this call to parent
		Node	^node = _parent->get_child( path );
		// check for succeeded request
		if( node == nullptr ) {
			throw gcnew ArgumentException(
				"Path '" + path + "' not found.");
		}
		return node;
	} else {
		// try find node in this collection
		return KeyedMap::default[path];
	}
}


//-------------------------------------------------------------------
//
// Find node that is located on specified relative path.
//
// This is overriden KeyedMap accessor. If specified path doesn't
// contain node delimeter, then search in this collection instance.
// In other case pass call to parent.
//
//-------------------------------------------------------------------
bool Nodes::Contains( String ^path )
{
	// check for the null reference
	if( path == nullptr ) throw gcnew ArgumentNullException("path");

	// if path contains node delimeter
	if( path->Contains( Node::Delimeter ) ) {
		// pass search request to parent
		return (_parent->get_child( path ) != nullptr);
	} else {
		// try find node in this collection
		return KeyedMap::Contains( path );
	}
}


//-------------------------------------------------------------------
//
// Adds a node into the collection.
//
// If force parameter is set to false then this function is similar
// to parent Add, in other case no addition handlers will be called:
// OnInsert and OnInsertComplete.
//
//-------------------------------------------------------------------
void Nodes::Add( Node ^node, bool force )
{
	// validate node
	if( node == nullptr ) throw gcnew ArgumentNullException("node");
	
	// if force parameter is false
	if( !force ) {
		// call parrent method
		Add( node );
	} else {
		// use protected function to force clear
		// without any additional processing
		if( !Insert( node->Name, node, false ) ) {
			// raise exception
			throw gcnew ArgumentException(
				"An item with the same key already exists.");
		}
	}
}

	
//-------------------------------------------------------------------
//
// Clears the content of the Nodes instance.
//
// If force parameter is set to false then this function is similar
// to parent Clear, in other case no addition handlers will be 
// called: OnClear and OnClearComplete.
//
//-------------------------------------------------------------------
void Nodes::Clear( bool force )
{
	// if force parameter is false
	if( !force ) {
		// call parrent method
		Clear();
	} else {
		// use protected function to force clear
		// without any additional processing
		DeleteAll();
	}
}


//-------------------------------------------------------------------
//
// Removes node with the specified name from the collection.
//
// If force parameter is set to false then this function is similar
// to parent Remove, in other case no addition handlers will be 
// called: OnRemove and OnRemoveComplete.
//
//-------------------------------------------------------------------
bool Nodes::Remove( String ^name, bool force )
{
	// validate name
	if( name == nullptr ) throw gcnew ArgumentNullException("name"); 

	// if not force is set
	if( !force ) {
		// call parent function
		return Remove( name );
	} else {
		// use protected functions to force delete
		// node without any additional processing
		Node		^node;
		// find node with specified name (in case
		// of search failed return false)
		if( !Find( name, node ) ) return false;

		// remove from collection by name as key
		Delete( name );

		return true;
	}
}


//-------------------------------------------------------------------
//
// Cancel last operation and restore Nodes to previous state.
//
// I need this function to undo operations without 'set_parent' call.
//
//-------------------------------------------------------------------
void Nodes::Revert( void )
{
	Undo();
}