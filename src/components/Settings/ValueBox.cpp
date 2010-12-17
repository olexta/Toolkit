/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		ValueBox.cpp												*/
/*																			*/
/*	Content:	Implementation of Node::ValueBox class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008-2009 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "ValueBox.h"

using namespace _SETTINGS;


//
// Define double safe cast operation
//
#define SCAST_(to, from, value)													\
safe_cast<to>( safe_cast<from>( value ) )

//
// Define macro for implicit cast operator from specified type to ValueBox.
//
#define OP_IMP_FROM_(type)														\
Node::ValueBox::operator Node::ValueBox( type v )								\
{																				\
	return ValueBox( v );														\
}

//
// Define macro for explicit cast operator from ValueBox to specified
// native type.
//
#define OP_EXP_TO_(type)														\
Node::ValueBox::operator type( ValueBox box )									\
{																				\
	Type	^r = (box.m_value != nullptr ? box.m_value->GetType() : nullptr);	\
	Type	^t = type::typeid;													\
																				\
	if( r == t ) return safe_cast<type>( box.m_value );							\
																				\
	throw gcnew InvalidCastException(String::Format(							\
	ERR_CAST_FROM_TO, (r != nullptr ? r->ToString() : "null"),					\
					  (t != nullptr ? t->ToString() : "null") ));				\
}

//
// Define macro for explicit cast operator from ValueBox (being in 'from'
// type) to specified 'to' type.
//
#define OP_EXP_(to, from)														\
Node::ValueBox::operator to( ValueBox box )										\
{																				\
	Type	^r = (box.m_value != nullptr ? box.m_value->GetType() : nullptr);	\
	Type	^f = from::typeid;													\
	Type	^t = to::typeid;													\
																				\
	if( r == f ) return SCAST_(to, from, box.m_value );							\
																				\
	throw gcnew InvalidCastException(String::Format(							\
	ERR_CAST_FROM_TO, (r != nullptr ? r->ToString() : "null"),					\
					  (t != nullptr ? t->ToString() : "null") ));				\
}


//----------------------------------------------------------------------------
//					Toolkit::Settings::Node::ValueBox
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the ValueBox class containing specified
/// object as internal data.
/// </summary><remarks>
/// Now following types are supported as native: bool, int, double,
/// DateTime and String. And char, unsigned char, short, unsigned
/// short, unsigned int, float are convertible to native.
/// </remarks>
//-------------------------------------------------------------------
Node::ValueBox::ValueBox( Object ^value ): \
	m_value(nullptr)
{
	// m_value was already initialized with null reference
	if( value == nullptr ) return;

	// get type of value
	Type	^type = value->GetType();

	// check this type through all supported types (native 
	// types checks first, so it minimize processing of
	// cast operators, because of they convert values to
	// such types, then process all convertible)
	if( type == bool::typeid ) {
		// native bool
		m_value = safe_cast<bool>( value );
	} else if( type == int::typeid ) {
		// native int
		m_value = safe_cast<int>( value );
	} else if( type == double::typeid ) {
		// native double
		m_value = safe_cast<double>( value );
	} else if( type == DateTime::typeid ) {
		// native DateTime
		m_value = safe_cast<DateTime>( value );
	} else if( type == String::typeid ) {
		// native String
		m_value = safe_cast<String^>( value );
	} else if( type == char::typeid ) {
		// convertible to int
		m_value = SCAST_(int, char, value);
	} else if( type == unsigned char::typeid ) {
		// convertible to int
		m_value = SCAST_(int, unsigned char, value);
	} else if( type == short::typeid ) {
		// convertible to int
		m_value = SCAST_(int, short, value);
	} else if( type == unsigned short::typeid ) {
		// convertible to int
		m_value = SCAST_(int, unsigned short, value);
	} else if( type == unsigned int::typeid) {
		// convertible to int
		m_value = SCAST_(int, unsigned int, value);
	} else if( type == float::typeid ) {
		// convertible to double
		m_value = SCAST_(double, float, value );
	} else {
		//unsupported type: throw exception
		throw gcnew ArgumentException(String::Format(
		ERR_INVALID_TYPE, type->ToString() ));
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Implicit cast operator from bool value to ValueBox. 
/// </summary>
//-------------------------------------------------------------------
OP_IMP_FROM_(bool)


//-------------------------------------------------------------------
/// <summary>
/// Implicit cast operator from int value to ValueBox. 
/// </summary>
//-------------------------------------------------------------------
OP_IMP_FROM_(int)


//-------------------------------------------------------------------
/// <summary>
/// Implicit cast operator from double value to ValueBox. 
/// </summary>
//-------------------------------------------------------------------
OP_IMP_FROM_(double)


//-------------------------------------------------------------------
/// <summary>
/// Implicit cast operator from DateTime value to ValueBox. 
/// </summary>
//-------------------------------------------------------------------
OP_IMP_FROM_(DateTime)


//-------------------------------------------------------------------
/// <summary>
/// Implicit cast operator from String value to ValueBox. 
/// </summary>
//-------------------------------------------------------------------
OP_IMP_FROM_(String^)


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to bool value (native).
/// </summary><remarks>
/// If internal data is not bool value then InvalidCastException will
/// be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_TO_(bool)


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to int value (native).
/// </summary><remarks>
/// If internal data is not int value then InvalidCastException will
/// be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_TO_(int)


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to double value (native).
/// </summary><remarks>
/// If internal data is not double value then InvalidCastException
/// will be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_TO_(double)


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to DateTime value (native).
/// </summary><remarks>
/// If internal data is not DateTime value then InvalidCastException
/// will be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_TO_(DateTime)


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to String value (native).
/// </summary><remarks>
/// This cast operator differs from other: it doesn't requires
/// internal data to be in valid type. Since this is settings, so any
/// setting can be converted to string value. This function is
/// similar to ToString() method.
/// </remarks>
//-------------------------------------------------------------------
Node::ValueBox::operator String^( ValueBox box )
{
	return box.ToString();
}


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to char value.
/// </summary><remarks>
/// If internal data is not int value then InvalidCastException will
/// be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_(char, int)


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to unsigned char value.
/// </summary><remarks>
/// If internal data is not int value then InvalidCastException will
/// be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_(unsigned char, int);


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to short value.
/// </summary><remarks>
/// If internal data is not int value then InvalidCastException will
/// be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_(short, int);


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to unsigned short value.
/// </summary><remarks>
/// If internal data is not int value then InvalidCastException will
/// be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_(unsigned short, int);


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to unsigned int value.
/// </summary><remarks>
/// If internal data is not int value then InvalidCastException will
/// be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_(unsigned int, int);


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to long long value.
/// </summary><remarks>
/// If internal data is not int value then InvalidCastException will
/// be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_(long long, int);


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to unsigned long long value.
/// </summary><remarks>
/// If internal data is not int value then InvalidCastException will
/// be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_(unsigned long long, int);


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to float value.
/// </summary><remarks>
/// If internal data is not double value then InvalidCastException
/// will be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_(float, double);


//-------------------------------------------------------------------
/// <summary>
/// Operator ==. Check for two instances of ValueBox to be equal.
/// </summary><remarks>
/// Checks internal m_value members to be equivalent.
/// </remarks>
//-------------------------------------------------------------------
bool Node::ValueBox::operator ==( ValueBox value1, ValueBox value2 )
{
	return Object::Equals( value1.m_value, value2.m_value );
}


//-------------------------------------------------------------------
/// <summary>
/// Operator !=. Check for two instances of ValueBox to be not equal.
/// </summary><remarks>
/// It uses Equal operator in implementation.
/// </remarks>
//-------------------------------------------------------------------
bool Node::ValueBox::operator !=( ValueBox value1, ValueBox value2 )
{
	return !(value1 == value2);
}


//-------------------------------------------------------------------
/// <summary>
/// Returns a value indicating whether this instance and a specified
/// ValueBox object represent the same value.
/// </summary><remarks>
/// It uses Equal operator in implementation.
/// </remarks>
//-------------------------------------------------------------------
bool Node::ValueBox::Equals( ValueBox value )
{
	return (*this == value);
}


//-------------------------------------------------------------------
/// <summary>
/// Determines whether the specified Object is equal to the current
/// ValueBox instance.
/// </summary><remarks>
/// Try build new instance of the ValueBox class by specified Object
/// parameter. If custructor fails, returns false. In other case uses
/// Equal operator.
/// </remarks>
//-------------------------------------------------------------------
bool Node::ValueBox::Equals( Object ^object )
{
	// try to build new ValueBox instance
	try {
		// if succeeded use Equal operator
		return (*this == ValueBox(object));
	} catch( ArgumentException^ ) {
		// if constructor failed return false
		return false;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Returns a handle to the internal data.
/// </summary>
//-------------------------------------------------------------------
Object^ Node::ValueBox::ToObject( void )
{
	return m_value;
}


//-------------------------------------------------------------------
/// <summary>
/// Returns a String that represents the current ValueBox.
/// </summary><remarks>
/// This function call the internal data ToString() method or return
/// "null" for internal null reference.
/// </remarks>
//-------------------------------------------------------------------
String^ Node::ValueBox::ToString( void )
{
	return (m_value != nullptr ? m_value->ToString() : "null");
}
