/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Collections											*/
/*																			*/
/*	Module:		RedBlackTree.cpp											*/
/*																			*/
/*	Content:	Implementation of BinaryTree::RedBlackTree class			*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "RedBlackTree.h"

using namespace System::Threading;
using namespace _BINARY_TREE;


//-----------------------------------------------------------------------------
// Toolkit::Collections::BinaryTree::RedBlackTree<TKey, TValue>::RedBlackNode
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Creates Red-Black tree leaf.
//
// In Red-Black tree all leafs is sentitel and mark as black nodes.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
RedBlackTree<TKey, TValue>::		   \
RedBlackNode::RedBlackNode( void ):	   \
	Node(KeyValuePair<TKey, TValue>())
{
	m_left = this;
	m_right = this;
	m_color = COLOR::Black;
}


//-------------------------------------------------------------------
//
// Creates new Red-Black tree node.
//
// This node have initialized data, childs and color (RED). Left and
// right childs are pointed to leafs. Parent is undefined.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
RedBlackTree<TKey, TValue>::								 \
RedBlackNode::RedBlackNode( KeyValuePair<TKey, TValue> data, \
							RedBlackNode ^leaf ) :			 \
	Node(data), m_color(COLOR::Red)
{
	// set childs to leafs
	m_left = leaf;
	m_right = leaf;
}


//-------------------------------------------------------------------
//
// Provide implicit Parent property conversion from BinaryTree::Node
// to RedBlackNode.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
RedBlackTree<TKey, TValue>::RedBlackNode^ RedBlackTree<TKey, TValue>:: \
RedBlackNode::Parent::get( void )
{
	return static_cast<RedBlackNode^>( Node::Parent );
}


//-------------------------------------------------------------------
//
// Provide implicit Left property conversion from BinaryTree::Node to
// RedBlackNode.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
RedBlackTree<TKey, TValue>::RedBlackNode^ RedBlackTree<TKey, TValue>:: \
RedBlackNode::Left::get( void )
{
	return static_cast<RedBlackNode^>( Node::Left );
}


//-------------------------------------------------------------------
//
// Provide implicit Right property conversion from BinaryTree::Node
// to RedBlackNode.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
RedBlackTree<TKey, TValue>::RedBlackNode^ RedBlackTree<TKey, TValue>:: \
RedBlackNode::Right::get( void )
{
	return static_cast<RedBlackNode^>( Node::Right );
}


//-------------------------------------------------------------------
//
// Gets/sets node color.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
RedBlackTree<TKey, TValue>::RedBlackNode::COLOR RedBlackTree<TKey, TValue>:: \
RedBlackNode::Color::get( void )
{
	return m_color;
}

generic<typename TKey, typename TValue>
void RedBlackTree<TKey, TValue>:: \
RedBlackNode::Color::set( COLOR value  )
{
	m_color = value;
}


//-----------------------------------------------------------------------------
//Toolkit::Collections::BinaryTree::RedBlackTree<TKey, TValue>::RedBlackVisitor
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Check that tree was modified during enumeration.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void RedBlackTree<TKey, TValue>:: \
RedBlackVisitor::OnCheckState( void )
{	
	if( _fnGetStamp() != _stamp ) {
		// tree was changed, next iteration may be unpredictable
		throw gcnew InvalidOperationException(ERR_ENUM_EXEC);
	}
}


//-------------------------------------------------------------------
//
// Creates new instance of the RedBlackVisitor class for specified
// red-black tree.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
RedBlackTree<TKey, TValue>::									 \
RedBlackVisitor::RedBlackVisitor( RedBlackTree ^rbt ) :			 \
	Visitor(rbt->m_root, Visitor::TRAVERSE::Infix, rbt->_leaf),	 \
	_fnGetStamp(gcnew GET_STAMP(rbt, &RedBlackTree::get_stamp)), \
	_stamp(rbt->m_stamp)
{
	// do nothing
}


//-----------------------------------------------------------------------------
//			Toolkit::Collections::BinaryTree::RedBlackTree<TKey, TValue>
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Return current tree stamp.
//
// This function is used by Enumerator to check tree modification.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
long long RedBlackTree<TKey, TValue>::get_stamp( void )
{
	return Interlocked::Read( m_stamp );
}


//-------------------------------------------------------------------
//
// Store information (depending on current action) to provide future
// restoration procedure.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
bool RedBlackTree<TKey, TValue>::backup( KeyValuePair<TKey, TValue> data,
										 RedBlackTree::RESTORE_POINT::ACTION action )
{
	// save information about action
	m_backup._data = data;
	m_backup._action = action;
	m_backup._count = m_count;
	m_backup._root = m_root;
	
	// return true if tree can be reverted to
	// previous state
	return (action != RESTORE_POINT::ACTION::None);
}


//-------------------------------------------------------------------
//
// Rotate tree node x to left.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void RedBlackTree<TKey, TValue>::rotate_left( RedBlackNode ^x )
{
	RedBlackNode	^y = x->Right;

	// establish x->right link
	x->Right = y->Left;
	if( y->Left != _leaf ) y->Left->Parent = x;

	// establish y->parent link
	if( y != _leaf ) y->Parent = x->Parent;
	if( x->Parent != nullptr ) {
		if( x == x->Parent->Left ) {
			x->Parent->Left = y;
		} else {
			x->Parent->Right = y;
		}
	} else {
		m_root = y;
	}

	// link x and y
	y->Left = x;
	if( x != _leaf ) x->Parent = y;
}


//-------------------------------------------------------------------
//
// Rotate tree node x to right.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void RedBlackTree<TKey, TValue>::rotate_right( RedBlackNode ^x )
{
	RedBlackNode	^y = x->Left;

	// establish x->left link
	x->Left = y->Right;
	if( y->Right != _leaf ) y->Right->Parent = x;

	// establish y->parent link
	if( y != _leaf ) y->Parent = x->Parent;
	if( x->Parent != nullptr ) {
		if( x == x->Parent->Right ) {
			x->Parent->Right = y;
		} else {
			x->Parent->Left = y;
		}
	} else {
		m_root = y;
	}

	// link x and y
	y->Right = x;
	if( x != _leaf ) x->Parent = y;
}


//-------------------------------------------------------------------
//
// Maintain Red-Black tree balance after inserting node x.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void RedBlackTree<TKey, TValue>::insert_fixup( RedBlackNode ^x )
{
	// check Red-Black properties
	while( x != m_root && x->Parent->Color == RedBlackNode::COLOR::Red ) {
	    // look for node above
		if( x->Parent == x->Parent->Parent->Left ) {
			// we have a violation
			RedBlackNode	^y = x->Parent->Parent->Right;

			if( y->Color == RedBlackNode::COLOR::Red ) {
				// uncle is Red
				x->Parent->Color = RedBlackNode::COLOR::Black;
				y->Color = RedBlackNode::COLOR::Black;
				x->Parent->Parent->Color = RedBlackNode::COLOR::Red;
				x = x->Parent->Parent;
			} else {
				// uncle is Black
				if( x == x->Parent->Right ) {
					// make x a Left child
					x = x->Parent;
					rotate_left( x );
				}
				// reColor and rotate
				x->Parent->Color = RedBlackNode::COLOR::Black;
				x->Parent->Parent->Color = RedBlackNode::COLOR::Red;
				rotate_right( x->Parent->Parent );
			}
		} else {
			// mirror image of above code
			RedBlackNode	^y = x->Parent->Parent->Left;
	        
			if( y->Color == RedBlackNode::COLOR::Red ) {
				// uncle is Red
				x->Parent->Color = RedBlackNode::COLOR::Black;
				y->Color = RedBlackNode::COLOR::Black;
				x->Parent->Parent->Color = RedBlackNode::COLOR::Red;
				x = x->Parent->Parent;
			} else {
				// uncle is Black
				if( x == x->Parent->Left ) {
	                // make x a Right child
					x = x->Parent;
					rotate_right( x );
				}
				// recolor and rotate
				x->Parent->Color = RedBlackNode::COLOR::Black;
				x->Parent->Parent->Color = RedBlackNode::COLOR::Red;
				rotate_left( x->Parent->Parent );
			}
		}
	}
	m_root->Color = RedBlackNode::COLOR::Black;
}


//-------------------------------------------------------------------
//
// Maintain Red-Black tree balance after deleting node x.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void RedBlackTree<TKey, TValue>::delete_fixup( RedBlackNode ^x )
{
	// check Red-Black properties
	while( x != m_root && x->Color == RedBlackNode::COLOR::Black ) {

		if( x == x->Parent->Left ) {

			RedBlackNode	^w = x->Parent->Right;
			
			if( w->Color == RedBlackNode::COLOR::Red ) {
				
				w->Color = RedBlackNode::COLOR::Black;
				x->Parent->Color = RedBlackNode::COLOR::Red;
				rotate_left( x->Parent );
				w = x->Parent->Right;
			}
			if( w->Left->Color == RedBlackNode::COLOR::Black &&
				w->Right->Color == RedBlackNode::COLOR::Black ) {
			
				w->Color = RedBlackNode::COLOR::Red;
				x = x->Parent;
			} else {

				if( w->Right->Color == RedBlackNode::COLOR::Black ) {
					
					w->Left->Color = RedBlackNode::COLOR::Black;
					w->Color = RedBlackNode::COLOR::Red;
					rotate_right( w );
					w = x->Parent->Right;
				}
				w->Color = x->Parent->Color;
				x->Parent->Color = RedBlackNode::COLOR::Black;
				w->Right->Color = RedBlackNode::COLOR::Black;
				rotate_left( x->Parent );
				x = m_root;
			}
		} else {
			
			RedBlackNode	^w = x->Parent->Left;

			if( w->Color == RedBlackNode::COLOR::Red ) {
				
				w->Color = RedBlackNode::COLOR::Black;
				x->Parent->Color = RedBlackNode::COLOR::Red;
				rotate_right( x->Parent );
				w = x->Parent->Left;
			}
			if( w->Right->Color == RedBlackNode::COLOR::Black && 
				w->Left->Color == RedBlackNode::COLOR::Black ) {

				w->Color = RedBlackNode::COLOR::Red;
				x = x->Parent;
			} else {
				
				if( w->Left->Color == RedBlackNode::COLOR::Black ) {
					
					w->Right->Color = RedBlackNode::COLOR::Black;
					w->Color = RedBlackNode::COLOR::Red;
					rotate_left( w );
					w = x->Parent->Left;
				}
				w->Color = x->Parent->Color;
				x->Parent->Color = RedBlackNode::COLOR::Black;
				w->Left->Color = RedBlackNode::COLOR::Black;
				rotate_right( x->Parent );
				x = m_root;
			}
		}
	}
	x->Color = RedBlackNode::COLOR::Black;
}


//-------------------------------------------------------------------
//
// Add node to tree.
//
// If node with the same key already exists in the tree than new
// value will be set.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void RedBlackTree<TKey, TValue>::insert_node( RedBlackNode ^x )
{
	RedBlackNode	^parent = nullptr;
	RedBlackNode	^current = nullptr;

	// find where node belongs
	current = m_root;
	parent = nullptr;
	while( current != _leaf ) {
		// in case of containing the item with specified key
		if( x->Data.Key->CompareTo(current->Data.Key ) == 0 ) {
			// set new value in association
			current->Data = x->Data;
			// and return
			return;
		}
		parent = current;
		current = (x->Data.Key->CompareTo(current->Data.Key) < 0) ? \
				  current->Left : current->Right;
	}
	x->Parent = parent;

	// insert node in the tree
	if( parent != nullptr ) {
		// check for node place
		if( x->Data.Key->CompareTo(parent->Data.Key) < 0 ) {
			// new node must be left child for founded parent
			parent->Left = x;
		} else {
			// new node must be right child for founded parent
			parent->Right = x;
		}
	} else {
		// tree is empty, so this node will be root
		m_root = x;
	}

	// balance RB tree
	insert_fixup( x );
}


//-------------------------------------------------------------------
//
// Delete specified node from the tree.
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void RedBlackTree<TKey, TValue>::delete_node( RedBlackNode ^x )
{
    RedBlackNode	^y = nullptr;
	RedBlackNode	^z = nullptr;

    if( (x->Left == _leaf) || (x->Right == _leaf) ) {
        // z has a NIL node as a child
        z = x;
    } else {
        // find tree successor with a NIL node as a child
        z = x->Right;

        while( z->Left != _leaf ) z = z->Left;
    }

    // y is z's only child
	if( z->Left != _leaf ) {
		// set y by left child
		y = z->Left;
	} else {
		// set y by right child
		y = z->Right;
	}

    // remove z from the parent chain 
    y->Parent = z->Parent;

	if( z->Parent != nullptr ) {
		// check what child z is
		if( z == z->Parent->Left ) {
			// z is left child
            z->Parent->Left = y;
		} else {
			// z is right child
            z->Parent->Right = y;
		}
	} else {
		// make y as root
        m_root = y;
	}

	if( z != x ) x->Data = z->Data;

	// balance RB tree after node delete
	if( z->Color == RedBlackNode::COLOR::Black ) delete_fixup( y );
}


//-------------------------------------------------------------------
//
// Find node containing data specified item.
//
// This search process last as O(log N).
//
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
RedBlackTree<TKey, TValue>::
RedBlackNode^ RedBlackTree<TKey, TValue>::find_node( TKey key )
{
	RedBlackNode	^x = m_root;

	while( x != _leaf ) {
		// check for equal keys
		if( key->CompareTo( x->Data.Key ) == 0 ) {
			// return current node
			return x;
		} else {
			// prepare for next iteration
			x = key->CompareTo( x->Data.Key ) < 0 ? x->Left : x->Right;
		}
	}
	// node with specified key was not found
	return nullptr;
}


//-------------------------------------------------------------------
/// <summary>
/// Default class constructor.
/// </summary><remarks>
/// Create a synchronization objects and initialize class members.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
RedBlackTree<TKey, TValue>::RedBlackTree( void ):		   \
	_leaf(gcnew RedBlackNode()), m_count(0), m_root(_leaf)
{
	// do nothing
}


//-------------------------------------------------------------------
/// <summary>
/// Find item by specified key.
/// </summary><remarks>
/// Returns search success result: if specified key was found returns
/// true, in other case returs false.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
bool RedBlackTree<TKey, TValue>::Find( TKey key, TValue %value )
{
	// check for initialized key
	if( key == nullptr ) throw gcnew ArgumentNullException("key");

	// attempt to find node with specified key
	RedBlackNode	^x = find_node( key );

	// return result
	return (x != nullptr) ? value = x->Data.Value, true : false;
}


//-------------------------------------------------------------------
/// <summary>
/// Allocate node for data and insert in tree.
/// </summary><remarks>
/// If "overwrite" flag is set to true, founded item with specified
/// key (if it exists) will be overwriten.
/// Function returns INSERT result: if new node was created it
/// returns true, in other case - false.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
bool RedBlackTree<TKey, TValue>::Insert( TKey key, TValue value, bool overwrite )
{
	// check for initialized key
	if( key == nullptr ) throw gcnew ArgumentNullException("key");

	RedBlackNode	^x = find_node( key );

	// in case of containing the item with specified key
	if( x != nullptr ) {
		// check for overwrite flag and replace data
		if( overwrite ) {
			// mark tree as modified
			Interlocked::Increment( m_stamp );
			// backup current state
			backup( x->Data, RESTORE_POINT::ACTION::Set );
			// set new value in association
			x->Data = KeyValuePair<TKey, TValue>(key, value);
		}
		// return non insert result
		return false;
	}

	// setup new node
	x = gcnew RedBlackNode(KeyValuePair<TKey, TValue>(key, value), _leaf);

	// mark tree as modified
	Interlocked::Increment( m_stamp );
	// backup current state
	backup( x->Data, RESTORE_POINT::ACTION::Insert );

	// insert node to the tree (this call make double search for node
	// with specified key: look for first before, but i do this for
	// more integriti function logic structure)
	insert_node( x );
	// change dictionary properties
	m_count++;

	return true;
}


//-------------------------------------------------------------------
/// <summary>
/// Delete an item with specified key from the tree.
/// </summary><remarks>
/// Function return deletion result: if item was found it returns
/// true, in other case - false.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
bool RedBlackTree<TKey, TValue>::Delete( TKey key )
{
	// check for initialized key
	if( key == nullptr ) throw gcnew ArgumentNullException("key");

    RedBlackNode	^x = find_node( key );	// find node for specified key

	if( (x == nullptr) || (x == _leaf) ) return false;

	// mark tree as modified
	Interlocked::Increment( m_stamp );
    // backup current state
	backup( x->Data, RESTORE_POINT::ACTION::Delete );
	
	// delete founded node
	delete_node( x );
	// modyfy dictionary properties
	m_count--;

	return true;
}


//-------------------------------------------------------------------
/// <summary>
/// Clears the content of the RedBlackTree instance.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
void RedBlackTree<TKey, TValue>::DeleteAll( void )
{
	// mark tree as modified
	Interlocked::Increment( m_stamp );
	// backup data
	backup( KeyValuePair<TKey, TValue>(), RESTORE_POINT::ACTION::DeleteAll );

	// clear tree: this is delete reference to root only
	m_root = _leaf;
	m_count = 0;
}


//-------------------------------------------------------------------
/// <summary>
/// Cancel last operation and restore RedBlackTree to previous state. 
/// </summary><remarks>
/// Only last operation can be canceled. If cancel was succeeded
/// function returns true, in other case returns false.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
bool RedBlackTree<TKey, TValue>::Undo( void )
{
	bool			res = false;
	RedBlackNode	^x = nullptr;

	// if no action was stored then restoration procedure is unavialable
	if( m_backup._action == RESTORE_POINT::ACTION::None ) return false;

	// depend on action type proccess different
	// restoration procedure
	switch( m_backup._action ) {
		
		case RESTORE_POINT::ACTION::Insert:
			// find node for specified key
			if( (x = find_node( m_backup._data.Key )) == nullptr ) break;

			// delete node from the tree
			delete_node( x );
			// save successful result 
			res = true;
		break;

		case RESTORE_POINT::ACTION::Set:
			// find node for specified key
			if( (x = find_node( m_backup._data.Key )) == nullptr ) break;

			// set stored data
			x->Data = m_backup._data;
			// save successful result 
			res = true;
		break;

		case RESTORE_POINT::ACTION::Delete:
			// check for existing node
			if( find_node( m_backup._data.Key ) != nullptr ) break;

			// add new node with stored data
			insert_node( gcnew RedBlackNode(m_backup._data, _leaf) );
			// save successful result
			res = true;
		break;
		
		case RESTORE_POINT::ACTION::DeleteAll:
			// check for empty tree
			if( m_root != _leaf ) break;

			// restore m_root pointer
			m_root = m_backup._root;
			// save successful result
			res = true;
		break;

		default: return false;
	}
	if( res ) m_count = m_backup._count;

	// clear backup data
	m_backup._action = RESTORE_POINT::ACTION::None;

	return res;
}


//-------------------------------------------------------------------
/// <summary>
/// Returns the number of items contained in the instance of the tree.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
int RedBlackTree<TKey, TValue>::Size( void )
{
	return m_count;
}