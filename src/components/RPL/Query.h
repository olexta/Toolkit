/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		Query.h														*/
/*																			*/
/*	Content:	Definition of Where and OrderBy classes						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"
#include "ValueBox.h"

using namespace System;
using namespace System::Runtime::InteropServices;


_RPL_BEGIN
/// <summary>
/// Encapsulates the common behavoir to organize search request.
/// </summary>
[Serializable]
public ref class OrderBy : System::Collections::IEnumerable
{
public:
	/// <summary>
	/// Describes how to order objects according specified property.
	/// </summary>
	[Serializable]
	ref class Clause {
	public:
		/// <summary>
		/// Specifies sorting order.
		/// </summary>
		enum class SORT {
			/// <summary>
			/// Specifies that the values of the specified object
			/// property should be sorted in ascending order, from
			/// lowest value to highest value.
			/// </summary>
			ASC=1,			
			/// <summary>
			/// Specifies that the values of the specified object
			/// property should be sorted in descending order, from
			/// highest value to lowest value.
			/// </summary>
			DESC=2
		};

	private:
		String^		const _opd;
		SORT		const _sort;

	public:
		Clause( String ^opd );
		Clause( String ^opd, SORT sort );

		property String^ OPD {
			String^ get( void );
		}
		property SORT Sort {
			SORT get( void );
		}
	};

private:
	initonly array<Clause^>	^_args;

	virtual System::Collections::IEnumerator^ get_enumerator( void ) sealed =
		System::Collections::IEnumerable::GetEnumerator;

public:
	OrderBy( ... array<Clause^> ^args );
};


/// <summary>
/// Specifies a search condition to restrict the objects returned.
/// </summary><remarks>
/// This is abstract class from which all condition classes inherit
/// from.
/// </remarks>
[Serializable]
public ref class Where abstract
{
public:
	ref class Clause;
	ref class Operation;

private:
	Where() {};

public:
	static Operation^ operator |( Where ^left, Where ^right );
	static Operation^ operator &( Where ^left, Where ^right );
	static Operation^ operator !( Where ^sub );
};


/// <summary>
/// Represents simple WHERE clause (predicate).
/// </summary><remarks>
/// Predicate is an expression that returns TRUE or FALSE.
/// </remarks>
[Serializable]
ref class Where::Clause sealed : Where
{
public:
	/// <summary>
	/// Comparison operators.
	/// </summary>
	enum class OP {
		/// <summary> Equal to. </summary>
		EQ=1,
		/// <summary> Not equal to. </summary>
		NE=2,
		/// <summary> Greater than. </summary>
		GT=4,
		/// <summary> Less than. </summary>
		LT=8,
		/// <summary> Greater than or equal to. </summary>
		GE=16,
		/// <summary> Less than or equal to. </summary>
		LE=32
	};

private:
	String^		const _opd;
	OP			const _op;
	ValueBox	const _value;
	
public:
	Clause( String ^opd, ValueBox value );
	Clause( String ^opd, OP op, ValueBox value );

	property String^ OPD {
		String^ get( void );
	}
	property OP Operator {
		OP get( void );
	}
	property ValueBox Value {
		ValueBox get( void );
	}
};


/// <summary>
/// This class represents a combination of one or more predicates
/// that use the logical operations AND, OR, and NOT.
/// </summary>
[Serializable]
ref class Where::Operation abstract : Where
{
public:
	ref class Or;
	ref class And;
	ref class Not;

private:
	Operation() {};
};


/// <summary>
/// Combines two conditions and evaluates to TRUE when either
/// condition is TRUE. 
/// </summary>
[Serializable]
ref class Where::Operation::Or sealed : Operation
{
private:
	Where^	const _left;
	Where^	const _right;

internal:
	Or( Where ^left, Where ^right );

public:
	property Where^	LeftWhere {
		Where^ get( void );
	}
	property Where^ RightWhere {
		Where^ get( void );
	}
};


/// <summary>
/// Combines two conditions and evaluates to TRUE when both of the
/// conditions are TRUE.
/// </summary>
[Serializable]
ref class Where::Operation::And sealed : Operation
{
private:
	Where^	const _left;
	Where^	const _right;

internal:
	And( Where ^left, Where ^right );

public:
	property Where^ LeftWhere {
		Where^ get( void );
	}
	property Where^ RightWhere {
		Where^ get( void );
	}
};


/// <summary>
/// Negates the Boolean expression specified by the predicate.
/// </summary>
[Serializable]
ref class Where::Operation::Not sealed : Operation
{
private:
	Where^	const _sub;

internal:
	Not( Where ^sub );

public:
	property Where^ SubWhere {
		Where^ get( void );
	}
};
_RPL_END