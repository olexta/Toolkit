/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		Item.cpp													*/
/*																			*/
/*	Content:	Implementation of Item class								*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright @ 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "Adapter.h"
#include "Nodes.h"
#include "Item.h"

using namespace _SETTINGS;


//----------------------------------------------------------------------------
//						Toolkit::Settings::Item
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Check potential parent for Item node (by type) and returns it's
// Online/Offline status.
//
//-------------------------------------------------------------------
Item::STATE Item::check_parent( Node ^parent )
{
	// check for null reference
	if( parent == nullptr ) return STATE::Unknown;
		
	// initial state as Unknown
	STATE	state = STATE::Unknown;
	// get parent type
	Type	^t = parent->GetType();
		
	// for object of the Item class parent may be
	// Adapter or another Item only, so check this
	if( t == Adapter::typeid ) {
		// Adapter node is connected always
		state = STATE::Online;
	} else if( t == Item::typeid ) {
		// if parent is Item class - extract it's state
		state = static_cast<Item^>( parent )->m_state;
	} else {
		// throw invalid parent type exception
		throw gcnew ArgumentException(
			"Object of type '" + t->ToString() +
			"' cann't be parent node to '" +
			this->GetType()->ToString() + "'.");
	}

	// check for capable states
	bool	bErrState = false;
	switch( state ) {
		// Unknown parent accept Unknown child only
		case STATE::Unknown:
			bErrState = (m_state != STATE::Unknown);
		break;
		// Offline parent cann't accept Unknown child 
		case STATE::Offline:
			bErrState = (m_state == STATE::Unknown);
		break;
		// Online parent accept any child state
		case STATE::Online:;
		break;
		// this in only the stab
		default: bErrState = true;
	}
	if( bErrState ) throw gcnew InvalidOperationException(
		"'" + state.ToString() + "' item cann't be parent to " +
		"'" + m_state.ToString() + "' item.");

	return state;
}


//-------------------------------------------------------------------
//
// Returns reference to adapter for current item.
//
// This function doesn't check for current item's state and try to
// get adapter directly from root node. So it can be used in
// 'set_state' without any specific 'm_state' value.
//
//-------------------------------------------------------------------
Adapter^ Item::get_adapter( void )
{
	// get full path to current item
	String	^path = RootTraverse( nullptr );

	// check that local root is "real" root
	if( !path->StartsWith( _delimeter ) ) throw gcnew InvalidOperationException(
		"Cann't locate root node.");

	// and return search for adapter request result
	return static_cast<Adapter^>(
		Find( _delimeter + ParsePath( path->Substring( _delimeter->Length ) ) ) );
}


//-------------------------------------------------------------------
//
// Set specified state to this item and all childs.
//
// If parameter 'sync' set to "true" function synchronize item
// parameter with stored in adapter. Return's result of processing
// this item only (ignore childs).
//
//-------------------------------------------------------------------
bool Item::set_state( STATE state, bool sync )
{
	// check for current state
	if( state == m_state ) return false;

	// in case of synchronization we must
	// check for compatible states
	if( sync ) switch( state ) {
		// have to be in Online state to load values from adapter
		case STATE::Offline:
			if( m_state != STATE::Online ) throw gcnew InvalidOperationException(
				"Cann't synchronize with adapter while switching from " +
				m_state.ToString() + " to " + state.ToString() + " state.");
			
			// load value from adapter and store it
			m_value = get_adapter()->GetValue( RootTraverse( nullptr ) );
		break;

		// have to be Offline state to load values into adapter
		case STATE::Online:
			if ( m_state != STATE::Offline ) throw gcnew InvalidOperationException(
				"Cann't synchronize with adapter while switching from " +
				m_state.ToString() + " to " + state.ToString() + " state.");

			// upload stored value into adapter
			get_adapter()->SetValue( RootTraverse( nullptr ), m_value );
			// and reset
			m_value = nullptr;
		break;

		case STATE::Unknown:
		default: throw gcnew InvalidOperationException(
			"Unsuitable state for items's synchronization with adapter.");
	}

	// store specified state for this items
	m_state = state;
	// and for all childs using recursive call
	for each( Item ^item in _childs ) {
		item->set_state( state, sync );
	}

	return true;
}


//-------------------------------------------------------------------
//
// Creates chain of new items by specified path.
//
// This private constructor is used by Adapter to create hierarchy
// of Items without any additional processing. Signature does not
// contain value parameter because of creating Unknown setting only.
//
//-------------------------------------------------------------------
Item::Item( Item ^parent, String ^path, Item^ *leaf ) : \
	Node(ParsePath(path), parent)
{
	// check name of node (name cann't be empty string)
	// parent constructor was already called, so check
	// initialized const name
	if( _name == "" ) throw gcnew ArgumentException(
		"Name of the item cann't be empty string.");

	// set item state to Unknown
	m_state = STATE::Unknown;

	// check parent type and state only
	check_parent( parent );

	// if tail of path is not empty string
	if( path != "" ) {
		// than use recurse to chunk subitems by tail
		Item	^item = gcnew Item(this, path, leaf);
		// reinitialize childs collection by using
		// other constructor to prevent calling
		// 'set_parent' with it's processing
		_childs = gcnew Nodes(this, item);
	} else {
		// this is las item in chunk, so set leaf to this
		if( leaf != nullptr ) *leaf = this;
	}
}


//-------------------------------------------------------------------
//
// Creates chain of items with Unknown state.
//
// This function creates chain of items by specified path. If parent
// parameter is not null, then function searchs last subitem that
// corresponds path, creates chain by tail of path and attach it to
// founded subitem.
//
// Returns reference to root of created chain.
//
//-------------------------------------------------------------------
Item^ Item::create_chain( Item ^parent, String ^path )
{
	// if no parent specified - just create simple chunk
	if( parent == nullptr ) return gcnew Item(nullptr, path, nullptr);

	// parent is specified, so find last item
	// corresponding to specified path
	Item	^last = dynamic_cast<Item^>( parent->Find( path ) );
	// if dynamic cast failed throw exception:
	// invalid path was specified
	if( last == nullptr ) throw gcnew ArgumentException(
		"Cast operation failed: invalid path.");

	// create simple chain by path tail
	Item	^chain = gcnew Item(nullptr, path, nullptr);
	// and add it to childs of founded item
	last->_childs->Add( chain );

	return chain;
}


//-------------------------------------------------------------------
//
// Creates chain of offline items by given path and initialize leaf
// by specified value.
//
//-------------------------------------------------------------------
Item^ Item::create_chain( String ^path, ValueBox value )
{
	// OK, create chunk of items
	Item	^leaf = nullptr;
	Item	^chain = gcnew Item(nullptr, path, &leaf);

	// set state to Offline
	chain->set_state( STATE::Offline, false );
	// and store value to the leaf
	leaf->m_value = value;

	return chain;
}


//-------------------------------------------------------------------
///	<summary>
///	Performs additional custom processes before setting a new parent
///	to the current Item instance.
///	</summary><remarks>
///	Folowing actions will be done before the specified parent is set:
///	  1. check parent for the proper type
///	  2. cache item and subitems valus from adapter in needed
///	  3. remove current item from adapter
///	</remarks>
//-------------------------------------------------------------------
void Item::OnSetParent( Node ^parent )
{
	// check parent type only (ignore return result)
	check_parent( parent );

	// if this item was attached to adapter
	if( m_state == STATE::Online ) {
		// it (and all subnodes) must cache it's values from
		// adapter while parent not be changed to another
		set_state( STATE::Offline, true );

		// remove this node from adapter
		get_adapter()->Remove( RootTraverse( nullptr ) );
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after setting a new parent
/// to the current Item instance.
/// </summary><remarks>
/// Following actions will be done after the specified parent is set:
///	  1. check this to offline and parent for online states
///	  2. and push stored values
/// </remarks>
//-------------------------------------------------------------------
void Item::OnSetParentComplete( Node ^parent )
{
	// need to be attached to adapter?
	if( check_parent( parent ) == STATE::Online ) {
		// push all stored values to adapter (if m_state is Offline)
		// i not need single function to attach request because of
		// SetValue call in set_state creates all nodes itself
		set_state( STATE::Online, m_state == STATE::Offline );
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Creates new (standalone) instance of the Item class with
/// specified name (value will be set to null reference).
/// </summary>
//-------------------------------------------------------------------
Item::Item( String ^name ) : \
	Node(name)
{
	// check name of node (name cann't be empty string)
	if( name == "" ) throw gcnew ArgumentException(
		"Name of the item cann't be empty string.");

	// set state to offline mode (now we
	// created standalone property)
	m_state = STATE::Offline;
}


//-------------------------------------------------------------------
/// <summary>
/// Creates new (standalone) instance of the Item class with
/// specified name and value.
/// </summary>
//-------------------------------------------------------------------
Item::Item( String ^name, ValueBox value ) : \
	Node(name)
{
	// check name of node (name cann't be empty string)
	if( name == "" ) throw gcnew ArgumentException(
		"Name of the item cann't be empty string.");	
	
	// store value in local variable (now we
	// created standalone property)
	m_value = value;

	// and set state to offline mode
	m_state = STATE::Offline;
}


//-------------------------------------------------------------------
/// <summary>
/// Copy constructor. Creates new (standalone) copy of the spesified
/// item.
/// </summary>
//-------------------------------------------------------------------
Item::Item( Item ^item ) : \
	Node(item != nullptr ? item->Name : nullptr) 
{
	// check for item state
	if( item->m_state == STATE::Unknown )  {
		throw gcnew InvalidOperationException(
		"Cann't duplicate item with state '" + m_state.ToString() +"'.");
	}

	// init object state
	m_state = STATE::Offline;
	// get and store value from specified item
	m_value = item->Value;

	// look for all childs
	for each( Item ^child in item->_childs ) {
		// and duplicate it by recurse call
		_childs->Add( gcnew Item(child) );
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Class disposer.
/// </summary><remarks>
/// Set state of item to Unknown and set value to null.
/// </remarks>
//-------------------------------------------------------------------
Item::~Item( void )
{ENTER_WRITE(_lock)

	m_state = STATE::Unknown;
	m_value = nullptr;

EXIT_WRITE(_lock)}


//-------------------------------------------------------------------
/// <summary>
/// Gets connection state of current setting's item.
/// </summary><remarks>
/// Setting has two states: Online and Offline. The first means to be
/// connected to some adapter (you can traverse from this to adapter
/// node) and second means having cached value .
/// </remarks>
//-------------------------------------------------------------------
Item::STATE Item::State::get( void )
{	
	return m_state;
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
void Item::default::set( String ^path, ValueBox value )
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
		node->Childs->Add( create_chain( path, value ) );
	} else {
		// store item value to founded item
		node->Value = value;
	}

EXIT_WRITE(_lock)}


//-------------------------------------------------------------------	
/// <summary>
/// Gets or sets value of current setting's item.
/// </summary><remarks>
/// If item state is Online - request will be passed to coresponding
/// adapter directly. In other case, value will be cached to the
/// moment when item will be connected to adapter.
/// </remarks>
//-------------------------------------------------------------------
Node::ValueBox Item::Value::get( void )
{ENTER_READ(_lock)

	// depend on current state returns
	// following results
	switch( m_state ) {
		// setting in cached state, return stored value
		case STATE::Offline:
			return m_value;
		break;
		// request value from adapter
		case STATE::Online:
			return get_adapter()->GetValue( RootTraverse( nullptr ) );
		break;
		// undepricable results
		default: throw gcnew InvalidOperationException(
					 "Invalid state of Item: '" + m_state.ToString() + "'." );
	}

EXIT_READ(_lock)}

void Item::Value::set( ValueBox value )
{ENTER_WRITE(_lock)

	// depend on current state value setting may
	// be processed in the following ways
	switch( m_state ) {
		// setting in cached state, cache value
		case STATE::Offline:
			m_value = value;
		break;
		// pass value to adapter
		case STATE::Online:
			return get_adapter()->SetValue( RootTraverse( nullptr ), value );
		break;
		// undepricable results
		default: throw gcnew InvalidOperationException(
					 "Invalid state of Item: '" + m_state.ToString() + "'." );
	}

EXIT_WRITE(_lock)}


//-------------------------------------------------------------------	
/// <summary>
/// Reload subitems tree from source.
/// </summary><remarks>
/// This function can be called for Online item only.
/// </remarks>
//-------------------------------------------------------------------
void Item::Load( void )
{ENTER_WRITE(_lock)

	// check for corresponding state
	if( m_state != STATE::Online ) throw gcnew InvalidOperationException(
		"Invalid state of Item: '" + m_state.ToString() + "'." );
	
	// dispose all childs
	for each( Node ^node in _childs ) delete node;
	// and removes it from collection
	_childs->Clear( true );

	// pass call to adapter, that creates subnodes
	for each( Node ^node in get_adapter()->Load( RootTraverse( nullptr ) ) ) {
		// attach this nodes to current item
		_childs->Add( node );
	}

EXIT_WRITE(_lock)}


//-------------------------------------------------------------------	
/// <summary>
/// Flush subitems to source.
/// </summary><remarks>
/// This function requests force data flush from adapter to source.
/// It can be called for Online item only.
/// </remarks>
//-------------------------------------------------------------------
void Item::Save( void )
{ENTER_WRITE(_lock)

	// check for corresponding state
	if( m_state != STATE::Online ) throw gcnew InvalidOperationException(
		"Invalid state of Item: '" + m_state.ToString() + "'." );
	
	// call flush for this node
	get_adapter()->Save( RootTraverse( nullptr ) );

EXIT_WRITE(_lock)}