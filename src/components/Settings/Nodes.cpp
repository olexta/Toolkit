/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		Nodes.cpp													*/
/*																			*/
/*	Content:	Implementation of Node::Nodes class							*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008-2009 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "Nodes.h"

using namespace _SETTINGS;


//----------------------------------------------------------------------------
//						Toolkit::Settings::Node::Nodes
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before clearing the Nodes
/// instance.
/// </summary><remarks>
/// Each node must be notified about removing from collection.
/// </remarks>
//-------------------------------------------------------------------
void Node::Nodes::OnClear( void )
{
	// remove parent reference for all nodes
	for each( Node ^node in this ) node->set_parent( nullptr );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before inserting a node to
/// the Nodes instance.
/// </summary><remarks>
/// Notify node about adding to collection (this is equal to set
/// node's parent reference).
/// </remarks>
//-------------------------------------------------------------------
void Node::Nodes::OnInsert( Node ^node )
{
	// set parent reference
	node->set_parent( _parent );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before removing a node from
/// the Nodes instance.
/// </summary><remarks>
/// Notify node about removing from collection (this is equal to
/// remove node's parent reference).
/// </remarks>
//-------------------------------------------------------------------
void Node::Nodes::OnRemove( Node ^node )
{
	// remove parent reference
	node->set_parent( nullptr );
}


//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the Nodes class for specified parent.
/// </summary><remarks>
/// This collection is used as childs/subnodes for specified parent
/// node.
/// </remarks>
//-------------------------------------------------------------------
Node::Nodes::Nodes( Node ^parent ) : \
	_parent(parent)
{
	// check for the null reference
	if( parent == nullptr ) throw gcnew ArgumentNullException("parent");
}


//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the Nodes class for specified parent and
/// init themself by one child.
/// </summary>
//-------------------------------------------------------------------
Node::Nodes::Nodes( Node ^parent, Node ^child ) : \
	KeyedMap(child), _parent(parent)
{
	// check for the null reference
	if( parent == nullptr ) throw gcnew ArgumentNullException("parent");

}


//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the Nodes class for specified parent and
/// init themself by collection of childs.
/// </summary>
//-------------------------------------------------------------------
Node::Nodes::Nodes( Node ^parent, IEnumerable<Node^> ^childs ) : \
	KeyedMap(childs), _parent(parent)
{
	// check for the null reference
	if( parent == nullptr ) throw gcnew ArgumentNullException("parent");

}


//-------------------------------------------------------------------
/// <summary>
/// Find node that is located on specified relative path.
/// </summary><remarks>
/// This is overriden KeyedMap accessor. If specified path doesn't
/// contain relative elements, then searchs in this collection items.
/// In other case pass call to parent.
/// </remarks>
//-------------------------------------------------------------------
Node^ Node::Nodes::default::get( String ^path )
{
	// check for the null reference
	if( path == nullptr ) throw gcnew ArgumentNullException("path");

	// if path contains relative elements
	if( (path == _parent->_cur_path) || 
		(path == _parent->_par_path) ||
		path->Contains( _parent->_delimeter ) ) {
		// pass this call to parent
		Node	^node = _parent->get_child( path );
		// check for succeeded request
		if( node == nullptr ) {
			throw gcnew ArgumentException(String::Format(
			ERR_PATH_NOT_FOUND, path ));
		}
		return node;
	} else {
		// try find node in this collection
		return KeyedMap::default[path];
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Find node that is located on specified relative path.
/// </summary><remarks>
/// This is overriden KeyedMap accessor. If specified path doesn't
/// contain relative elements, then searchs in this collection items.
/// In other case pass call to parent.
/// </remarks>
//-------------------------------------------------------------------
bool Node::Nodes::Contains( String ^path )
{
	// check for the null reference
	if( path == nullptr ) throw gcnew ArgumentNullException("path");

	// if path contains relative elements
	if( (path == _parent->_cur_path) || 
		(path == _parent->_par_path) ||
		path->Contains( _parent->_delimeter ) ) {
		// pass search request to parent
		return (_parent->get_child( path ) != nullptr);
	} else {
		// try find node in this collection
		return KeyedMap::Contains( path );
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Adds a node into the collection.
/// </summary><remarks>
/// If force parameter is set to false then this function is similar
/// to parent Add, in other case no addition handlers will be called:
/// OnInsert and OnInsertComplete.
/// </remarks>
//-------------------------------------------------------------------
void Node::Nodes::Add( Node ^node, bool force )
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
			throw gcnew ArgumentException(ERR_ITEM_EXISTS);
		}
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Clears the content of the Nodes instance.
/// </summary><remarks>
/// If force parameter is set to false then this function is similar
/// to parent Clear, in other case no addition handlers will be 
/// called: OnClear and OnClearComplete.
/// </remarks>
//-------------------------------------------------------------------
void Node::Nodes::Clear( bool force )
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
/// <summary>
/// Removes node with the specified name from the collection.
/// </summary><remarks>
/// If force parameter is set to false then this function is similar
/// to parent Remove, in other case no addition handlers will be 
/// called: OnRemove and OnRemoveComplete.
/// </remarks>
//-------------------------------------------------------------------
bool Node::Nodes::Remove( String ^name, bool force )
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
/// <summary>
/// Canceles last operation and restore Nodes to previous state.
/// </summary><remarks>
/// I need this function to undo operations without 'set_parent'
/// call.
/// </remarks>
//-------------------------------------------------------------------
void Node::Nodes::Revert( void )
{
	Undo();
}