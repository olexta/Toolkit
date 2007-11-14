/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Collections											*/
/*																			*/
/*	Module:		BinaryTreeNode.cpp											*/
/*																			*/
/*	Content:	Implementation of BinaryTree::Node class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "BinaryTreeNode.h"

using namespace _BINARY_TREE;


//----------------------------------------------------------------------------
//				Toolkit::Collections::BinaryTree::Node<T>
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Create new instance of the BinaryTree::Node class that contains
/// default data value.
/// </summary><remarks>
/// Created node have not parent and childs.
/// </remarks>
//-------------------------------------------------------------------
generic<typename T>
Node<T>::Node( void ) :												  \
	m_parent(nullptr), m_left(nullptr), m_right(nullptr), m_data(T())
{
}


//-------------------------------------------------------------------
/// <summary>
/// Create new instance of the BinaryTree::Node class that contains
/// specified data.
/// </summary><remarks>
/// Created node have not parent and childs.
/// </remarks>
//-------------------------------------------------------------------
generic<typename T>
Node<T>::Node( T data ) :											   \
	m_parent(nullptr), m_left(nullptr), m_right(nullptr), m_data(data)
{
}


//-------------------------------------------------------------------
/// <summary>
/// Gets/Sets parent node for this.
/// </summary>
//-------------------------------------------------------------------
generic<typename T>
Node<T>^ Node<T>::Parent::get( void )
{
	return m_parent;
}

generic<typename T>
void Node<T>::Parent::set( Node^ value )
{
	m_parent = value;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets/Sets left subnode for this.
/// </summary>
//-------------------------------------------------------------------
generic<typename T>
Node<T>^ Node<T>::Left::get( void )
{
	return m_left;
}

generic<typename T>
void Node<T>::Left::set( Node^ value )
{
	m_left = value;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets/Sets right subnode for this.
/// </summary>
//-------------------------------------------------------------------
generic<typename T>
Node<T>^ Node<T>::Right::get( void )
{
	return m_right;
}

generic<typename T>
void Node<T>::Right::set( Node^ value )
{
	m_right = value;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets/Sets a data that current node contains.
/// </summary>
//-------------------------------------------------------------------
generic<typename T>
T Node<T>::Data::get( void )
{
	return m_data;
}

generic<typename T>
void Node<T>::Data::set( T value )
{
	m_data = value;
}