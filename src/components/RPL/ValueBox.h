/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		ValueBox.h													*/
/*																			*/
/*	Content:	Definition of ValueBox class								*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008 Alexey Tkachuk								*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"

using namespace System;
using namespace System::IO;
using namespace System::Security::Permissions;
using namespace System::Runtime::Serialization;


_RPL_BEGIN
ref class PersistentStream;

/// <summary>
/// This class incapsulate type check for property value.
/// </summary><remarks>
/// Value is wrapper arroud Object handle and provide runtime
/// type check, implicit and explicit cast operators, Equal
/// comparison.
/// Now, the following types are supported: bool, int, double,
/// DateTime, String, PersistentStream and DBNull.
/// </remarks>
[Serializable]
public value class ValueBox : IEquatable<ValueBox>, ISerializable
{
private:
	Object	^m_value;

// ISerializable
private:
	[SecurityPermission(SecurityAction::Demand,SerializationFormatter=true)]
	ValueBox( SerializationInfo ^info, StreamingContext context );

	[SecurityPermission(SecurityAction::Demand,SerializationFormatter=true)]
	virtual void get_object_data( SerializationInfo ^info,
								  StreamingContext context ) sealed =
		ISerializable::GetObjectData;

public:
	explicit ValueBox( Object ^o );

	static operator ValueBox( bool b );
	static operator ValueBox( int i );
	static operator ValueBox( double f );
	static operator ValueBox( DateTime dt );
	static operator ValueBox( String ^s );
	static operator ValueBox( PersistentStream ^stream );
	static operator ValueBox( DBNull ^null );

	static explicit operator bool( ValueBox box );
	static explicit operator int( ValueBox box );
	static explicit operator double( ValueBox box );
	static explicit operator DateTime( ValueBox box );
	static explicit operator String^( ValueBox box );
	static explicit operator PersistentStream^( ValueBox box );
	static explicit operator DBNull^( ValueBox box );

	static explicit operator char( ValueBox box );
	static explicit operator unsigned char( ValueBox box );
	static explicit operator short( ValueBox box );
	static explicit operator unsigned short( ValueBox box );
	static explicit operator unsigned int( ValueBox box );
	static explicit operator long long( ValueBox box );
	static explicit operator unsigned long long( ValueBox box );
	static explicit operator float( ValueBox box );

	static bool operator ==( ValueBox value1, ValueBox box );
	static bool operator !=( ValueBox value1, ValueBox box );

	virtual bool Equals( ValueBox box );
	virtual bool Equals( Object ^o ) override;

	virtual Object^ ToObject( void );
	virtual String^ ToString( void ) override;
};
_RPL_END
