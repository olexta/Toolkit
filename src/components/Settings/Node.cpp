/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		Node.cpp													*/
/*																			*/
/*	Content:	Implementation of Node class								*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008-2009 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "Nodes.h"
#include "Node.h"
#include "ValueBox.h"

using namespace _SETTINGS;


//----------------------------------------------------------------------------
//						Toolkit::Settings::Node
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Check node name to be in the correct format.
//
//-------------------------------------------------------------------
void Node::check_name( String ^name )
{
	// check for null reference
	if( name == nullptr ) throw gcnew ArgumentNullException("name");

	// check for illegal names ('path to current' and 'path to parent')
	if( (name == _cur_path) || (name == _par_path) ) {
		// throw reserved names exception 
		throw gcnew ArgumentException(String::Format(
		ERR_RESERVED_NAME, _cur_path, _par_path ));
	}

	// check for illegal symbols (name cann't contains delimeter)
	if( name->Contains( _delimeter ) ) {
		// throw illegal symbol exception
		throw gcnew ArgumentException(String::Format(
		ERR_STR_CONTAINS, "A name", _delimeter ));
	}
}


//-------------------------------------------------------------------
//
// Set new parent for current node.
//
// By doing this two events will be raised: 'OnSetParent' and
// 'OnSetParentComplete'. This function is used by Nodes class to
// notify about direct collection actions.
//
//-------------------------------------------------------------------
void Node::set_parent( Node ^parent )
{ENTER_WRITE(_lock)

	// backup old parent reference
	Node	^oldParent = m_parent;

	// in case of having parent and specified parent is
	// not null reference - remove this from parent's
	// child collection
	if( (m_parent != nullptr) && (parent != nullptr) ) {

		m_parent->_childs->Remove( this );
	}

	// now m_parent is null reference, so all actions
	// go through exception control (if error will be
	// raised all changes will be rolled back)
	try {
		// fire event before the action
		OnSetParent( parent );

		// save new value
		m_parent = parent;

		// fire event after the action 
		OnSetParentComplete( parent );
	} catch( Exception^ ) {
		// roll back changes
		m_parent = oldParent;
		// if remove operation was done
		if( (m_parent != nullptr) && (parent != nullptr) ) {
			// roll back remove request
			m_parent->_childs->Revert();
		}
		// restore exception
		throw;
	}

EXIT_WRITE(_lock)}


//-------------------------------------------------------------------
//
// Returns child node that is located on specified relative path.
//
// This function is used by Nodes class to search child if default
// accessor or Contains gets relative path as parameter.
//
//-------------------------------------------------------------------
Node^ Node::get_child( String ^path )
{ENTER_READ(_lock)

	Node	^node = Find( path );

	// if specified item was not found return null reference
	return (path == "" ? node : nullptr);

EXIT_READ(_lock)}


//-------------------------------------------------------------------
/// <summary>
/// Split input string to next subnode and tail.
/// </summary><remarks>
/// Subnode name returns as function result, and tail is returned
/// through input string.
/// </remarks>
//-------------------------------------------------------------------
String^ Node::ParsePath( String^ %path )
{
	// search for delimeter
	int		pos = path->IndexOf( _delimeter, 0 );
	
	// split string by this delimeter
	String	^res = pos < 0 ? path : path->Substring( 0, pos );
	path = pos < 0 ? "" : path->Substring( pos + _delimeter->Length );

	return res;
}


//-------------------------------------------------------------------
/// <summary>
/// Returns path and reference to root node.
/// </summary><remarks>
/// This function proceed only current subnode, so founded node may
/// be not root in common sense (like "/" in UNIX). To determine this
/// situation you can use check for 'Delimeter' at the begining of
/// the returned path ("real" root node have "" as name, so path will
/// start with 'Delimeter').
/// </remarks>
//-------------------------------------------------------------------
String^ Node::RootTraverse( Node^ *root )
{
	// if this node is root - return it's name and
	// reference to this
	if( m_parent == nullptr ) {
		// check for null pointer
		if( root != nullptr ) *root = this;
		// and return path as name
		return _name;
	}

	// use recurse to get path from root (this is not efficient
	// approach, but we don't use deep tree hierarche)
	return m_parent->RootTraverse( root ) + _delimeter + _name;
}


//-------------------------------------------------------------------
/// <summary>
/// Find node that is located on specified relative path.
/// </summary><remarks>
/// In case of unsuccessful result return as deep founded node as
/// possible. Path will contain subpath that was not found.
/// </remarks>
//-------------------------------------------------------------------
Node^ Node::Find( String^ %path )
{
	// recurse stop point
	if( path == "" ) return this;

	Node	^node = nullptr;
	String	^subnode = ParsePath( path );
	
	// select node to recursive request
	if( subnode == "" ) {
		// this is root node request, so search current subnode
		// for root and check that founded local root to be
		// "real" root (have empty name)
		if( RootTraverse( &node ), (node->_name != "") ) {
			// throw exception
			throw gcnew InvalidOperationException(ERR_ROOT_LOCATE);
		}
	} else if( subnode == _cur_path ) {
		// just return this
		node = this;
	} else if( subnode == _par_path ) {
		// return reference to parent, or ignore
		// if we are in the root node
		node = (m_parent == nullptr) ? this : m_parent;
	} else {
		// at the end, return subnode if it exists
		if( _childs->Contains( subnode ) ) node = _childs[subnode];
	}

	// in case of unsuccessful search we must return
	// last founded node and tail of unprocessed path
	if( node == nullptr ) {
		// restore original path
		path = subnode + _delimeter + path;
		// and return this as last founded node
		return this;
	}

	// return recursive request to founded node
	return node->Find( path );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before setting a new parent
/// to the current Node instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// specified parent is set.
/// </remarks>
//-------------------------------------------------------------------
void Node::OnSetParent( Node ^parent )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after setting a new parent
/// to the current Node instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// specified parent is set.
/// </remarks>
//-------------------------------------------------------------------
void Node::OnSetParentComplete( Node ^parent )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the Node class, and initialise it with
/// specified name.
/// </summary>
//-------------------------------------------------------------------
Node::Node( String ^name ) : \
	_name(name), _lock(gcnew ReaderWriterLock())
{
	// check for the correct name
	check_name( name );

	// create subnodes (i use initonly modifier
	// to be able use 'this' pointer)
	_childs = gcnew Nodes(this);

	// initialize reference to parent node
	m_parent = nullptr;
}


//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the Node class and initialise it with
/// specified name and parent.
/// </summary>
//-------------------------------------------------------------------
Node::Node( String ^name, Node ^parent ) : \
	_name(name), _lock(gcnew ReaderWriterLock())
{
	// check for the correct name
	check_name( name );

	// create subnodes (i use initonly modifier
	// to be able use 'this' pointer)
	_childs = gcnew Nodes(this);

	// set reference to parent node
	m_parent = parent;
}


//-------------------------------------------------------------------
/// <summary>
/// Class disposer.
/// </summary><remarks>
/// Set parent reference to null, dispose all childs and removes it
/// from collection.
/// </remarks>
//-------------------------------------------------------------------
Node::~Node( void )
{ENTER_WRITE(_lock)

	// set parent to null reference
	m_parent = nullptr;

	// call destructor to all childs
	for each( Node ^node in _childs ) delete node;
	// and force clear collection
	_childs->Clear( true );

EXIT_WRITE(_lock)}


//-------------------------------------------------------------------
/// <summary>
/// Gets string that is used as delimeter in node path.
/// </summary>
//-------------------------------------------------------------------
String^ Node::Delimeter::get( void )
{
	// return without any thread synchronization
	// because of using static const string
	return _delimeter;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets name of the node.
/// </summary>
//-------------------------------------------------------------------
String^ Node::Name::get( void )
{
	// return without any thread synchronization
	// because of using const string (name is read-only)
	return _name;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets full path to this node.
/// </summary><remarks>
/// Full path include name of this node at the end. This property
/// proceed current subnode only, so root node may be not "root" in
/// common sence (like "/" in UNIX). To determine this situation you
/// can use check for 'Delimeter' at the begining of the returned
/// path ("real" root node have "" as name, so path will start with
/// 'Delimeter').
/// </remarks>
//-------------------------------------------------------------------
String^ Node::Path::get( void )
{ENTER_READ(_lock)

	return RootTraverse( nullptr );

EXIT_READ(_lock)}


//-------------------------------------------------------------------
/// <summary>
/// Gets subnodes collection.
/// </summary><remarks>
/// This collection can be empty, but never nullptr.
/// </remarks>
//-------------------------------------------------------------------
KeyedMap<String^, Node^>^ Node::Childs::get( void )
{
	return _childs;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets node value that is located on specified relative
/// path.
/// </summary><remarks>
/// Default implementation searchs subnode with specified relative
/// path and call apropriate get or set Value handler. In case of
/// unssuccesful search ArgumentException will be raised.
/// </remarks>
//-------------------------------------------------------------------
Node::ValueBox Node::default::get( String ^path )
{ENTER_READ(_lock)

	// check for null reference
	if( path == nullptr ) throw gcnew ArgumentNullException("path");

	// copy path because of, it will be modified by Find request 
	String	^relpath = path;
	// search apropriate node	
	Node	^node = Find( relpath );
	
	// if specified item was not found, throw exception
	if( relpath != "" )
		throw gcnew ArgumentException(String::Format(
		ERR_PATH_NOT_FOUND, path ));

	// all right, pas call to derived class
	return node->Value;

EXIT_READ(_lock)}

void Node::default::set( String ^path, ValueBox value )
{ENTER_WRITE(_lock)
	
	// check for null reference
	if( path == nullptr ) throw gcnew ArgumentNullException("path");

	// copy path because of, it will be modified by Find request 
	String	^relpath = path;
	// search apropriate node	
	Node	^node = Find( relpath );
	
	// if specified item was not found, throw exception
	if( relpath != "" )
		throw gcnew ArgumentException(String::Format(
		ERR_PATH_NOT_FOUND, path ));

	// all right, pas call to derived object
	node->Value = value;

EXIT_WRITE(_lock)}
