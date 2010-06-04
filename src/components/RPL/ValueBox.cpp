/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		ValueBox.cpp												*/
/*																			*/
/*	Content:	Implementation of ValueBox class							*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008 Alexey Tkachuk								*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentStream.h"
#include "ValueBox.h"

using namespace _RPL;


//
// Define double safe cast operation
//
#define SCAST_(to, from, value)												\
safe_cast<to>( safe_cast<from>( value ) )

//
// Define macro for implicit cast operator from
// specified type to ValueBox.
//
#define OP_IMP_FROM_(type)													\
ValueBox::operator ValueBox( type v )										\
{																			\
	return ValueBox( v );													\
}

//
// Define macro for explicit cast operator from
// ValueBox to specified native type.
//
#define OP_EXP_TO_(type)													\
ValueBox::operator type( ValueBox box )										\
{																			\
	Object	^val = box.ToObject();											\
	Type	^r = val->GetType();											\
	Type	^t = type::typeid;												\
																			\
	if( r == t ) return safe_cast<type>( val );								\
																			\
	throw gcnew InvalidCastException(String::Format(						\
	ERR_CAST_FROM_TO, r->ToString(), t->ToString() ));						\
}

//
// Define macro for explicit cast operator from ValueBox
// (being in 'from' type) to specified 'to' type.
//
#define OP_EXP_(to, from)													\
ValueBox::operator to( ValueBox box )										\
{																			\
	Object	^val = box.ToObject();											\
	Type	^r = val->GetType();											\
	Type	^f = from::typeid;												\
	Type	^t = to::typeid;												\
																			\
	if( r == f ) return SCAST_(to, from, val );								\
																			\
	throw gcnew InvalidCastException(String::Format(						\
	ERR_CAST_FROM_TO, r->ToString(), t->ToString() ));						\
}


//----------------------------------------------------------------------------
//							Toolkit::PRL::ValueBox
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Creates a new instance of the ValueBox class using serialization
// info.
//
//-------------------------------------------------------------------
ValueBox::ValueBox( SerializationInfo ^info, StreamingContext context )
{
	// check for null reference
	if( info == nullptr ) throw gcnew ArgumentNullException("info");

	// restore value from serialization info
	m_value = info->GetValue( "value", Object::typeid );
}


//-------------------------------------------------------------------
//
// ISerializable::GetObjectData implementation.
//
// Populates a System.Runtime.Serialization.SerializationInfo with
// the data needed to serialize the target object.
//
//-------------------------------------------------------------------
void ValueBox::get_object_data( SerializationInfo ^info,
								StreamingContext context )
{
	// check for null reference
	if( info == nullptr ) throw gcnew ArgumentNullException("info");

	// save serialization data
	info->AddValue( "value", m_value );
}


//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the ValueBox class containing specified
/// object as internal data.
/// </summary><remarks>
/// Now following types are supported as native: bool, int, double,
/// DateTime, String, PersistentStream and DBNull. And char, unsigned
/// char, short, unsigned short, unsigned int, float are convertible
/// to native.
/// </remarks>
//-------------------------------------------------------------------
ValueBox::ValueBox( Object ^o ): \
	m_value(DBNull::Value)
{
	// check for null reference
	if( o == nullptr ) throw gcnew ArgumentNullException("o");

	// get type of value
	Type	^type = o->GetType();

	// check this type through all supported types (native 
	// types checks first, so it minimize processing of
	// cast operators, because of they convert values to
	// such types, then process all convertible)
	if( type == bool::typeid ) {
		// native bool
		m_value = safe_cast<bool>( o );
	} else if( type == int::typeid ) {
		// native int
		m_value = safe_cast<int>( o );
	} else if( type == double::typeid ) {
		// native double
		m_value = safe_cast<double>( o );
	} else if( type == DateTime::typeid ) {
		// native DateTime
		m_value = safe_cast<DateTime>( o );
	} else if( type == String::typeid ) {
		// native String
		m_value = safe_cast<String^>( o );
	} else if( type == PersistentStream::typeid ) {
		// native PersistentStream
		m_value = safe_cast<PersistentStream^>( o );
	} else if( type == DBNull::typeid ) {
		// native DBNull
		// do nothing: m_value already initialized with it
	} else if( type == char::typeid ) {
		// convertible to int
		m_value = SCAST_(int, char, o);
	} else if( type == unsigned char::typeid ) {
		// convertible to int
		m_value = SCAST_(int, unsigned char, o);
	} else if( type == short::typeid ) {
		// convertible to int
		m_value = SCAST_(int, short, o);
	} else if( type == unsigned short::typeid ) {
		// convertible to int
		m_value = SCAST_(int, unsigned short, o);
	} else if( type == unsigned int::typeid) {
		// convertible to int
		m_value = SCAST_(int, unsigned int, o);
	} else if( type == float::typeid ) {
		// convertible to double
		m_value = SCAST_(double, float, o );
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
/// Implicit cast operator from PersistentStream value to ValueBox. 
/// </summary>
//-------------------------------------------------------------------
OP_IMP_FROM_(PersistentStream^)


//-------------------------------------------------------------------
/// <summary>
/// Implicit cast operator from DBNull value to ValueBox. 
/// </summary>
//-------------------------------------------------------------------
OP_IMP_FROM_(DBNull^)


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
ValueBox::operator String^( ValueBox box )
{
	return box.ToString();
}


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to PersistentStream value.
/// </summary><remarks>
/// If internal data is not PersistentStream value then
/// InvalidCastException will be raised.
/// </remarks>
//-------------------------------------------------------------------
ValueBox::operator PersistentStream^( ValueBox box )
{
	// get real value
	Object	^val = box.ToObject();

	PersistentStream	^ps = dynamic_cast<PersistentStream^>( val );
	if( ps == nullptr ) {
		// stored value is not a Stream
		throw gcnew InvalidCastException(String::Format(
		ERR_CAST_FROM_TO, val->GetType()->ToString(),
						  PersistentStream::typeid->ToString() ));
	}
	return ps;
}


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to DBNull value (native).
/// </summary><remarks>
/// If internal data is not DBNull::Value or null reference then
/// InvalidCastException will be raised.
/// </remarks>
//-------------------------------------------------------------------
ValueBox::operator DBNull^( ValueBox box )
{
	// get real value
	Object	^val = box.ToString();

	if( val->GetType() == DBNull::typeid ) return DBNull::Value;

	// stored value is not a DBNull/null
	throw gcnew InvalidCastException(String::Format(
	ERR_CAST_FROM_TO, val->GetType()->ToString(),
					  DBNull::typeid->ToString() ));
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
/// Checks internal values to be equivalent.
/// </remarks>
//-------------------------------------------------------------------
bool ValueBox::operator ==( ValueBox box1, ValueBox box2 )
{
	return Object::Equals( box1.ToObject(), box2.ToObject() );
}


//-------------------------------------------------------------------
/// <summary>
/// Operator !=. Check for two instances of ValueBox to be not equal.
/// </summary><remarks>
/// It uses Equal operator in implementation.
/// </remarks>
//-------------------------------------------------------------------
bool ValueBox::operator !=( ValueBox box1, ValueBox box2 )
{
	return !(box1 == box2);
}


//-------------------------------------------------------------------
/// <summary>
/// Returns a value indicating whether this instance and a specified
/// ValueBox object represent the same value.
/// </summary><remarks>
/// It uses Equal operator in implementation.
/// </remarks>
//-------------------------------------------------------------------
bool ValueBox::Equals( ValueBox box )
{
	return (*this == box);
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
bool ValueBox::Equals( Object ^object )
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
/// </summary><remarks>
/// This function returns modified internal data: if box contains
/// null reference than it returns DBNull::Value. This functionality
/// prevents from null accessing errors. ToObject is used in all
/// functions instead of direct m_value access.
/// </remarks>
//-------------------------------------------------------------------
Object^ ValueBox::ToObject( void )
{
	return (m_value != nullptr) ? m_value : DBNull::Value;
}


//-------------------------------------------------------------------
/// <summary>
/// Returns a String that represents the current ValueBox.
/// </summary><remarks><para>
/// This function returns: </para><para>
/// 1. "&lt;null&gt;" for null reference or DBNull </para><para>
/// 2. and result of internal data ToString() call in other cases 
/// </para></remarks>
//-------------------------------------------------------------------
String^ ValueBox::ToString( void )
{
	Object	^val = this->ToObject();

	// this is empty value box
	if( val == DBNull::Value ) return "<null>";

	// returns call to internal data
	return val->ToString();
}
