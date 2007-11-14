/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Collections											*/
/*																			*/
/*	Module:		Association.cpp												*/
/*																			*/
/*	Content:	Implementation of Association class							*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "Association.h"

using namespace _COLLECTIONS;


//----------------------------------------------------------------------------
//				Toolkit::Collections::Association<TKey, TValue>
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Create new instance of the Association class using specified key.
/// </summary><remarks>
/// By using this constructor you can create association with
/// undefined value.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
Association<TKey, TValue>::Association( TKey key ) : \
	_key(key)
{
	if( _key == nullptr ) throw gcnew ArgumentNullException("key");
}


//-------------------------------------------------------------------
/// <summary>
/// Create new instance of the Association class using specified key
/// and value.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
Association<TKey, TValue>::Association( TKey key, TValue value ) : \
	_key(key), m_value(value)
{
	if( _key == nullptr ) throw gcnew ArgumentNullException("key");
}
	

//-------------------------------------------------------------------
/// <summary>
/// Create new instance of the Association class using specified
/// KeyValuePair struct.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
Association<TKey, TValue>::Association( KeyValuePair<TKey, TValue> pair ) : \
	_key(pair.Key), m_value(pair.Value)
{
	if( _key == nullptr ) throw gcnew ArgumentNullException("key");
}


//-------------------------------------------------------------------
/// <summary>
/// Operator (KeyValuePair). Return pair representation of current
/// association.
/// </summary><remarks>
/// To get pair representation more easy i overload Implicit
/// operator. This is create new instance of KeyValuePair only.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
Association<TKey, TValue>::operator KeyValuePair<TKey, TValue>( void )
{
	return KeyValuePair<TKey, TValue>(TKey(_key), m_value);
}


//-------------------------------------------------------------------
/// <summary>
/// Gets key for this accociation.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
TKey Association<TKey, TValue>::Key::get( void )
{
	return TKey(_key);
}


//-------------------------------------------------------------------
/// <summary>
/// Gets/Sets value for this association.
/// </summary>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
TValue Association<TKey, TValue>::Value::get( void )
{
	return m_value;
}

generic<typename TKey, typename TValue>
void Association<TKey, TValue>::Value::set( TValue value )
{
	m_value = value;
}


//-------------------------------------------------------------------
/// <summary>
/// Returns a String that represents the current Object.
/// </summary><remarks>
/// Override standart object method ToString(): must provide special
/// processing for representing key and value.
/// </remarks>
//-------------------------------------------------------------------
generic<typename TKey, typename TValue>
String^ Association<TKey, TValue>::ToString( void )
{
	return "[" + _key->ToString() + ", " +
			(m_value == nullptr ? "null" : m_value->ToString()) + "]";
}