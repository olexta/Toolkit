/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Collections											*/
/*																			*/
/*	Module:		Node.h														*/
/*																			*/
/*	Content:	Definition of BinaryTree::Node class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "..\Collections.h"

using namespace System;
using namespace System::Reflection;


_BINARY_TREE_BEGIN
/// <summary>
/// This class provide common definition for binary tree node.
/// </summary><remarks>
/// All specialized tree node structures must be derived from this class. 
/// </remarks>
generic<typename T>
private ref class Node : MarshalByRefObject
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
		virtual Node^ get( void );
		virtual void set( Node^ value );
	}
	property Node^ Left {
		virtual Node^ get( void );
		virtual void set( Node^ value );
	}
	property Node^ Right {
		virtual Node^ get( void );
		virtual void set( Node^ value );
	};
	property T Data {
		virtual T get( void );
		virtual void set( T value );
	}
};
_BINARY_TREE_END