/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Collections											*/
/*																			*/
/*	Module:		RedBlackTree.h												*/
/*																			*/
/*	Content:	Definition of RedBlackTree class							*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "Collections.h"
#include "Association.h"
#include "BinaryTreeNode.h"
#include "BinaryTreeVisitor.h"

using namespace System;
using namespace System::Threading;
using namespace System::Reflection;
using namespace _COLLECTIONS;


_BINARY_TREE_BEGIN
/// <summary>
/// This class provide storage services (based on Red-Black tree algorithms)
/// that implement fast access for objects by it's keys.
/// </summary><remarks>
/// Access to item by it's name is processed as O(log N). Tree traverse ("for
/// each" language construct) is implemented as iteration algorithm, so it
/// process as O(N).
/// </remarks>
generic<typename TKey, typename TValue> 
	where TKey : IComparable<TKey>
public ref class RedBlackTree abstract : MarshalByRefObject
{
private:
	//
	// Red-Black tree node
	//
	ref class RedBlackNode : Node<Association<TKey, TValue>^>
	{
	public:
		// Red-Black tree node colors
		typedef enum class COLOR {Black, Red};
	
	private:
		static RedBlackNode	^sentitel = nullptr;

		COLOR	m_color;

	public:
		RedBlackNode( Association<TKey, TValue> ^data );

		property RedBlackNode^ NIL {
			static RedBlackNode^ get( void );
		}
		property RedBlackNode^ Parent {
			virtual RedBlackNode^ get( void ) new;
		};
		property RedBlackNode^ Left {
			virtual RedBlackNode^ get( void ) new;
		}
		property RedBlackNode^ Right {
			virtual RedBlackNode^ get( void ) new;
		}
		property COLOR Color {
			COLOR get( void );
			void set( COLOR value );
		}
	};

private protected:
	//
	// Enumerator class that provide centered tree bypass
	//
	ref class RedBlackVisitor : Visitor<Association<TKey, TValue>^>
	{
	private:
		RedBlackTree^ const	_rbt;
		
	public:
		RedBlackVisitor( RedBlackTree ^rbt );
		~RedBlackVisitor( void );
	};

private:
	//
	// Struct contains last action info that is used
	// by class instance to provide undo operation
	//
	value struct RESTORE_POINT {
		// enum of available actions
		typedef enum class ACTION {None, Insert, Set, Delete, DeleteAll};
		// data backup information
		KeyValuePair<TKey, TValue>	_data;
		// other backup information
		ACTION			_action;
		int				_count;
		RedBlackNode	^_root;
	} _backup;

	int				m_count;
	RedBlackNode	^m_root;

	bool backup( Association<TKey, TValue> ^data, RESTORE_POINT::ACTION action );
	void rotate_left( RedBlackNode ^x );
	void rotate_right( RedBlackNode ^x );
	void insert_fixup( RedBlackNode ^x );
	void delete_fixup( RedBlackNode ^x );
	void insert_node( RedBlackNode ^x );
	void delete_node( RedBlackNode ^x );
	RedBlackNode^ find_node( TKey key );

protected:
	ReaderWriterLock^ const	_lock;

	RedBlackTree( void );

	bool Find( TKey key, TValue %value );
	bool Insert( TKey key, TValue value, bool overwrite );
	bool Delete( TKey key );
	void DeleteAll( void );
	bool Undo( void );

public:
	property int Count {
		virtual int get( void ) sealed;
	}
};
_BINARY_TREE_END