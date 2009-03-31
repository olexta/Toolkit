/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		ValueBox.h													*/
/*																			*/
/*	Content:	Definition of Node::ValueBox class							*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008-2009 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "Settings.h"
#include "Node.h"

using namespace System;


_SETTINGS_BEGIN
/// <summary>
/// This class incapsulates type check for setting value.
/// </summary><remarks>
/// ValueBox is wrapper arroud Object handle and provide runtime
/// type check, implicit and explicit cast operators, Equal
/// comparison.
/// Now, the following types are supported: bool, int, double,
/// DateTime and String.
/// </remarks>
value class Node::ValueBox : IEquatable<ValueBox>
{
private:
	Object	^m_value;

public:
	explicit ValueBox( Object ^o );

	static operator ValueBox( bool b );
	static operator ValueBox( int i );
	static operator ValueBox( double f );
	static operator ValueBox( DateTime dt );
	static operator ValueBox( String ^s );

	static explicit operator bool( ValueBox box );
	static explicit operator int( ValueBox box );
	static explicit operator double( ValueBox box );
	static explicit operator DateTime( ValueBox box );
	static explicit operator String^( ValueBox box );

	static explicit operator char( ValueBox box );
	static explicit operator unsigned char( ValueBox box );
	static explicit operator short( ValueBox box );
	static explicit operator unsigned short( ValueBox box );
	static explicit operator unsigned int( ValueBox box );
	static explicit operator long long( ValueBox box );
	static explicit operator unsigned long long( ValueBox box );
	static explicit operator float( ValueBox box );

	static bool operator ==( ValueBox value1, ValueBox value2 );
	static bool operator !=( ValueBox value1, ValueBox value2 );

	virtual bool Equals( ValueBox value );
	virtual bool Equals( Object ^object ) override;

	virtual Object^ ToObject( void );
	virtual String^ ToString( void ) override;
};
_SETTINGS_END
