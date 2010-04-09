/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		Manager.cpp													*/
/*																			*/
/*	Content:	Implementation of Manager class								*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008-2009 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "Nodes.h"
#include "Item.h"
#include "Adapter.h"
#include "Manager.h"

using namespace _SETTINGS;


//----------------------------------------------------------------------------
//						Toolkit::Settings::Manager
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before setting a new parent
/// to the current Manager instance.
/// </summary><remarks>
/// Root node cann't have any parent, so this function throw
/// exception only.
/// </remarks>
//-------------------------------------------------------------------
void Manager::OnSetParent( Node ^parent )
{
	throw gcnew InvalidOperationException(ERR_ROOT_PARENT);
}


//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the Manager class that is empty or is
/// initialized with specified list of adapter interfaces.
/// </summary>
//-------------------------------------------------------------------
Manager::Manager( ... array<IAdapter^> ^adapters ) : \
	Node("")
{
	// check name of node (name must be empty string)
	// parent constructor was already called, so check
	// initialized const name
	if( _name != "" ) throw gcnew ArgumentException(ERR_ROOT_NAME);

	// create list of nodes that will contain childs
	List<Node^>	^childs = gcnew List<Node^>();
	// fill list of childs by created adapter nodes
	for each( IAdapter ^adp in adapters )
		childs->Add( gcnew Adapter(this, adp) );

	// and reinitialize _childs member by creating collection
	// using another constructor (this prevent unneeded
	// 'set_parent' calls)
	_childs = gcnew Nodes(this, childs);
}


//-------------------------------------------------------------------
/// <summary>
/// Sets a new value to the item that is located by specified
/// relative path.
/// </summary><remarks>
/// If item is not found by the specified path - it will be created,
/// such as all subitems.
/// </remarks>
//-------------------------------------------------------------------
void Manager::default::set( String ^path, ValueBox value )
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
/// <summary>
/// Gets or sets value of Manager.
/// </summary><remarks>
/// This property is not supported for root node. So getter returns
/// ValueBox with null reference and setter throws exception.
/// </remarks>
//-------------------------------------------------------------------
Node::ValueBox Manager::Value::get( void )
{
	return nullptr;
}

void Manager::Value::set( ValueBox value )
{
	throw gcnew InvalidOperationException(String::Format(
	ERR_NODE_OPERATION, this->GetType()->ToString() ));
}


//-------------------------------------------------------------------
/// <summary>
/// Add new adapter with specified interface to root and returns node
/// that contains it.
/// </summary><remarks>
/// It is only one way to add new adapter interface to root node:
/// using standard way ('Add' method of Childs) will raise exception.
/// So, root and adapters means as a single whole.
/// </remarks>
//-------------------------------------------------------------------
Node^ Manager::Add( IAdapter ^adapter )
{ENTER_WRITE(_lock)

	// create new adapter node by specified interface
	Node	^node = gcnew Adapter(this, adapter);

	// and add it to collection of childs (use force
	// method to add adapter: prevent 'set_parent' call)
	_childs->Add( node, true );

	return node;

EXIT_WRITE(_lock)}


//-------------------------------------------------------------------
/// <summary>
/// Removes adapter with specified name.
/// </summary><remarks>
/// It is only one way to remove existing adapter interface from root
/// node: using standard way ('Remove' method of Childs) will raise
/// exception. So, root and adapters means as a single whole.
/// </remarks>
//-------------------------------------------------------------------
void Manager::Remove( String ^adapter )
{ENTER_WRITE(_lock)

	// check for null reference
	if( adapter == nullptr ) throw gcnew ArgumentNullException("adapter");

	// dispose adapter with all it childs
	delete _childs[adapter];
	// and force remove from collection of childs
	// (prevent 'set_parent' call)
	_childs->Remove( adapter, true );

EXIT_WRITE(_lock)}


//-------------------------------------------------------------------
/// <summary>
/// Reload all data from sources to internal adapters.
/// </summary><remarks>
/// Really, this call rebuild tree of nodes for each adapter (of
/// course, adapter can perform any additional processing).
/// </remarks>
//-------------------------------------------------------------------
void Manager::Load( void )
{ENTER_WRITE(_lock)

	// pass this call to each adapter
	for each( Node ^node in _childs ) node->Load();

EXIT_WRITE(_lock)}


//-------------------------------------------------------------------
/// <summary>
/// Flash all data from internal adapters to sources.
/// </summary><remarks>
/// This function requests force data flush from all adapters to
/// sources. For realtime adapters this function is used rarely.
/// </remarks>
//-------------------------------------------------------------------
void Manager::Save( void )
{ENTER_WRITE(_lock)

	// pass this call to each adapter
	for each( Node ^node in _childs ) node->Save();

EXIT_WRITE(_lock)}
