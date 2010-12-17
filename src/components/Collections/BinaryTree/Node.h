/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Collections											*/
/*																			*/
/*	Module:		Node.h														*/
/*																			*/
/*	Content:	Definition of BinaryTree::Node class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "..\Collections.h"

using namespace System;


_BINARY_TREE_BEGIN
/// <summary>
/// This class provide common definition for binary tree node.
/// </summary><remarks>
/// All specialized tree node structures must be derived from this class. 
/// </remarks>
generic<typename T>
[Serializable]
private ref class Node
{
protected:
	Node		^m_parent;	// parent
	Node		^m_left;	// left child
	Node		^m_right;	// right child
	T			m_data;		// data stored in the node

public:
	Node( void );
	Node( T data );

	property Node^ Parent {
		Node^ get( void );
		void set( Node ^value );
	}
	property Node^ Left {
		Node^ get( void );
		void set( Node ^value );
	}
	property Node^ Right {
		Node^ get( void );
		void set( Node ^value );
	};
	property T Data {
		T get( void );
		void set( T value );
	}
};
_BINARY_TREE_END
