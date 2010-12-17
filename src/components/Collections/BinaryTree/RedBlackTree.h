/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Collections											*/
/*																			*/
/*	Module:		RedBlackTree.h												*/
/*																			*/
/*	Content:	Definition of BinaryTree::RedBlackTree class				*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "..\Collections.h"
#include "Node.h"
#include "Visitor.h"

using namespace System;
using namespace System::Collections::Generic;
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
[Serializable]
public ref class RedBlackTree abstract
{
private:
	//
	// Red-Black tree node
	//
	[Serializable]
	ref class RedBlackNode : Node<KeyValuePair<TKey, TValue>>
	{
	public:
		// Red-Black tree node colors
		typedef enum class COLOR {Black, Red};

	private:
		COLOR	m_color;

	internal:
		RedBlackNode( void );

	public:
		RedBlackNode( KeyValuePair<TKey, TValue> data, RedBlackNode ^leaf );

		property RedBlackNode^ Parent {
			RedBlackNode^ get( void );
		};
		property RedBlackNode^ Left {
			RedBlackNode^ get( void );
		}
		property RedBlackNode^ Right {
			RedBlackNode^ get( void );
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
	ref class RedBlackVisitor : Visitor<KeyValuePair<TKey, TValue>>
	{
	private:
		// get tree state stamp
		delegate long long GET_STAMP( void );

	private:
		long long	const _stamp;
		GET_STAMP^	const _fnGetStamp;

	protected:
		virtual void OnCheckState( void ) override;

	public:
		RedBlackVisitor( RedBlackTree ^rbt );
	};

private:
	RedBlackNode^	const _leaf;

	//
	// Struct contains last action info that is used
	// by class instance to provide undo operation
	//
	[Serializable]
	value struct RESTORE_POINT {
		// enum of available actions
		typedef enum class ACTION {None, Insert, Set, Delete, DeleteAll};
		// data backup information
		KeyValuePair<TKey, TValue>	_data;
		// other backup information
		ACTION			_action;
		int				_count;
		RedBlackNode	^_root;
	};
	RESTORE_POINT	m_backup;

	int				m_count;
	RedBlackNode	^m_root;
	long long		m_stamp;

	long long get_stamp( void );
	bool backup( KeyValuePair<TKey, TValue> data, RESTORE_POINT::ACTION action );

	void rotate_left( RedBlackNode ^x );
	void rotate_right( RedBlackNode ^x );
	void insert_fixup( RedBlackNode ^x );
	void delete_fixup( RedBlackNode ^x );
	void insert_node( RedBlackNode ^x );
	void delete_node( RedBlackNode ^x );

	RedBlackNode^ find_node( TKey key );

protected:
	RedBlackTree( void );

	bool Find( TKey key, TValue %value );
	bool Insert( TKey key, TValue value, bool overwrite );
	bool Delete( TKey key );
	void DeleteAll( void );
	bool Undo( void );
	int Size( void );
};
_BINARY_TREE_END
