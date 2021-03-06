/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Collections											*/
/*																			*/
/*	Module:		Visitor.h													*/
/*																			*/
/*	Content:	Definition of BinaryTree::Visitor class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2009 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "..\Collections.h"
#include "Node.h"

using namespace System;


_BINARY_TREE_BEGIN
/// <summary>
/// Enumerator class that provide routines to tree bypass using specified
/// traverse mode.
/// </summary><remarks>
/// This class represent three traverse modes:
///		PREFIX: direct bypass (node, left subnode, right subnode);
///		INFIX: centered bypass (left subnode, node, right subnode);
///		POSTFIX: reverse bypass ( left subnode, right subnode, node);
/// Class implements finite-state automation algorithm to traverse binary
/// trees, so it process tree as O(N).
/// </remarks>
generic<typename T>
private ref class Visitor
{
public:
	// define traverse modes
	typedef enum class TRAVERSE {Prefix, Infix, Postfix};

private:
	// define states of automation
	typedef enum class STATE {Start, Left, Right, Parent, Stop};

private:
	Node<T>^	const _root;		// root node
	Node<T>^	const _leaf;		// leaf node (null reference by default)
	TRAVERSE	const _traverse;	// traverse mode
	Node<T>		^m_current;			// current node
	STATE		m_state;			// current automation state

	void check_state( void );
	bool stop_traverse( STATE state );

protected:
	bool		m_disposed;			// flag for disposed state

	virtual void OnCheckState( void );

public:
	Visitor( Node<T> ^root, TRAVERSE traverse );
	Visitor( Node<T> ^root, TRAVERSE traverse, Node<T> ^leaf );
	virtual ~Visitor( void );

	property T Current {
		virtual T get( void ) sealed;
	}

	virtual bool MoveNext( void ) sealed;
	virtual void Reset( void ) sealed;
};
_BINARY_TREE_END
