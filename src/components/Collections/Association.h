/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Collections											*/
/*																			*/
/*	Module:		Association.h												*/
/*																			*/
/*	Content:	Definition of Association class								*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "Collections.h"

using namespace System;
using namespace System::Collections::Generic;


_COLLECTIONS_BEGIN
/// <summary>
/// This class provide same functionality as KeyValuePair, but have
/// setter method for Value.
/// </summary>
generic<typename TKey, typename TValue>
	where TKey : IComparable<TKey>
private ref class Association
{
private:
	TKey		const _key;
	TValue		m_value;

public:
	Association( TKey key );
	Association( TKey key, TValue value );
	Association( KeyValuePair<TKey, TValue> pair );

	operator KeyValuePair<TKey, TValue>( void ); 

	property TKey Key {
		TKey get( void );
	};
	property TValue Value {
		TValue get( void );
		void set( TValue value );
	}

	virtual String^ ToString( void ) override;
};
_COLLECTIONS_END