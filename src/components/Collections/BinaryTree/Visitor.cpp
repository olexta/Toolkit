/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Collections											*/
/*																			*/
/*	Module:		Visitor.cpp													*/
/*																			*/
/*	Content:	Implementation of BinaryTree::Visitor class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "Visitor.h"

using namespace _BINARY_TREE;


//-----------------------------------------------------------------------------
//				Toolkit::Collections::BinaryTree::Visitor<T>
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Function checks visitor to be in invalid state.
//
//-------------------------------------------------------------------
generic<typename T>
void Visitor<T>::check_state( void )
{
	// check for disposed object
	if( m_disposed ) {
		// throw disposed exception using class as object name
		throw gcnew ObjectDisposedException(this->ToString());
	}

	// check binary tree state
	OnCheckState();
}


//-------------------------------------------------------------------
//
// Determine bypass need to be stopped regarding specified state.
//
//-------------------------------------------------------------------
generic<typename T>
bool Visitor<T>::stop_traverse( STATE state )
{
	// switch by specified state
	switch( state ) {
		case STATE::Left:
			return (_traverse == TRAVERSE::Prefix);
		break;

		case STATE::Right:
			return (_traverse == TRAVERSE::Infix);
		break;

		case STATE::Parent:
			return (_traverse == TRAVERSE::Postfix);
		break;
	}
	return false;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before any tree traverse.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// traverse is started.
/// </remarks>
//-------------------------------------------------------------------
generic<typename T>
void Visitor<T>::OnCheckState( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the Visitor class for specified binary
/// tree root node and traverse mode.
/// </summary><remarks>
/// Create a synchronization objects and initialize class members.
/// </remarks>
//-------------------------------------------------------------------
generic<typename T>
Visitor<T>::Visitor( Node<T> ^root, TRAVERSE traverse ): \
	_root(root), _leaf(nullptr), _traverse(traverse),	 \
	m_disposed(false)
{
	// check for null reference
	if( root == nullptr ) throw gcnew ArgumentNullException("root");

	// initialize members
	m_current = root;
	m_state = STATE::Start;
}


//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the Visitor class for specified binary
/// tree root node, traverse mode and leaf node.
/// </summary><remarks>
/// Create a synchronization objects and initialize class members.
/// Some binary trees (for example Red-Black trees) use special Node
/// instances instead of nullptr to mark leaf nodes. So, i need use
/// this reference to determine tree leafs.
/// </remarks>
//-------------------------------------------------------------------
generic<typename T>
Visitor<T>::Visitor( Node<T> ^root, TRAVERSE traverse, Node<T> ^leaf ) : \
	_root(root), _leaf(leaf), _traverse(traverse), m_disposed(false)
{
	// check for null reference
	if( root == nullptr ) throw gcnew ArgumentNullException("root");

	// initialize members
	m_current = root;
	m_state = (root != leaf) ? STATE::Start : STATE::Stop;
}


//-------------------------------------------------------------------
/// <summary>
/// Clear all managed resources and set automation to undefined
/// state.
/// </summary>
//-------------------------------------------------------------------
generic<typename T>
Visitor<T>::~Visitor( void )
{
	if( !m_disposed ) {
		// reset enumerator state
		m_state = STATE::Stop;
		m_current = _root;
		// set state to disposed
		m_disposed = true;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Returns data stored in the node that iterator in current state is
/// pointed on.
/// </summary><remarks>
/// In case of enumeration has not be started or has already finished
/// throw InvalidOperationException.
/// </remarks>
//-------------------------------------------------------------------
generic<typename T>
T Visitor<T>::Current::get( void )
{
	// check enumerator state
	check_state();

	// check for automation state: we haven't to be in
	// initial and finish states
	if( (m_state == STATE::Start) || (m_state == STATE::Stop) ) {
		// throw exception
		throw gcnew InvalidOperationException(ERR_ENUM_NOT_STARTED);
	}
	return m_current->Data;
}


//-------------------------------------------------------------------
/// <summary>
/// Advances the enumerator to the next element in the tree.
/// </summary><remarks>
/// This algorithm implements non recursive tree traverse based on
/// finite-state machine. It process tree as O(N).
/// </remarks>
//-------------------------------------------------------------------
generic<typename T>
bool Visitor<T>::MoveNext( void )
{
	// check enumerator state
	check_state();

	// this cycle will be finished by entering automation Stop state
	do {
		// this is a switch by automation states
		// (states represent direction to visit nodes)
		switch( m_state ) {
			// initial state of automation
			case STATE::Start:
				// from initial state select left direction only
				if( stop_traverse( m_state = STATE::Left ) ) return true;
			break;

			case STATE::Left:
				// we have left direction now
				if( m_current->Left != _leaf ) {
					// in case of having left child go down to
					// find minimal element
					m_current = m_current->Left;
					// change state and check for stop
					if( stop_traverse( m_state = STATE::Left ) ) return true;
				} else {
					// node hasn't left child, so we must check
					// right subnode
					if( stop_traverse( m_state = STATE::Right) ) return true;
				}
			break;

			case STATE::Right:
				// we have right direction now
				if( m_current->Right != _leaf) {
					// node has right subnode, so we change current
					// node and enter LEFT state to find minimal
					// element of this subnode
					m_current = m_current->Right;
					// change state and check for stop
					if( stop_traverse( m_state = STATE::Left ) ) return true;
				} else {
					// select UP direction
					if( stop_traverse( m_state = STATE::Parent ) ) return true;
				}
			break;

			case STATE::Parent:
				// check for root element
				if( m_current != _root ) {
					// determine from what child we are looking up
					if( m_current->Parent->Left == m_current ) {
						// from left child we must select right direction
						m_current = m_current->Parent;
						if( stop_traverse( m_state = STATE::Right ) ) return true;
					} else {
						// in other case look for parent 
						m_current = m_current->Parent;
						if( stop_traverse( m_state = STATE::Parent) ) return true;
					}
				} else {
					// enter finish state to exit loop
					m_state = STATE::Stop;
				}
			break;
		}
	} while( m_state != STATE::Stop );

	return false;
}


//-------------------------------------------------------------------
/// <summary>
/// Sets the enumerator to it's initial position, which is before the
/// first element in the tree traverse.
/// </summary>
//-------------------------------------------------------------------
generic<typename T>
void Visitor<T>::Reset( void )
{
	// check enumerator state
	check_state();

	// reset automation state and current node
	m_state = (_root != _leaf) ? STATE::Start : STATE::Stop;
	m_current = _root;
}
