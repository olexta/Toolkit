**************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		Query.cpp													*/
/*																			*/
/*	Content:	Implementation of Where and OrderBy classes					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008-2009 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "Query.h"

using namespace _RPL;


//-----------------------------------------------------------------------------
//						Toolkit::RPL::OrderBy::Clause
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Creates a new instance of the OrderBy.Clause class that uses
/// ascending sorting order for specified operand.
/// </summary>
//-------------------------------------------------------------------
OrderBy::Clause::Clause( String ^opd ) : \
	_opd(opd), _sort(SORT::ASC)
{
	// check for the null reference
	if( opd == nullptr ) throw gcnew ArgumentNullException("opd");
}


//-------------------------------------------------------------------
/// <summary>
/// Creates a new instance of the OrderBy.Clause class with specified
/// operand and sorting order.
/// </summary>
//-------------------------------------------------------------------
OrderBy::Clause::Clause( String ^opd, SORT sort ) : \
	_opd(opd), _sort(sort)
{
	// check for the null reference
	if( opd == nullptr ) throw gcnew ArgumentNullException("opd");
	// check for the valid sort value
	if( !safe_cast<System::Collections::IList^>(
			Enum::GetValues( SORT::typeid ) )->Contains( sort ) ) {
		// throw exception
		throw gcnew ArgumentOutOfRangeException("sort");
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Gets order operand.
/// </summary>
//-------------------------------------------------------------------
String^ OrderBy::Clause::OPD::get( void )
{
	return _opd;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets sorting order.
/// </summary>
//-------------------------------------------------------------------
OrderBy::Clause::SORT OrderBy::Clause::Sort::get( void )
{
	return _sort;
}


//-----------------------------------------------------------------------------
//							Toolkit::RPL::OrderBy
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Returns an enumerator that iterates through a collection.
//
//-------------------------------------------------------------------
System::Collections::IEnumerator^ OrderBy::get_enumerator( void )
{
	// enumerator to stored args
	return _args->GetEnumerator();
}


//-------------------------------------------------------------------
/// <summary>
/// Creates a new instance of the OrderBy class using specified set
/// of clauses.
/// </summary>
//-------------------------------------------------------------------
OrderBy::OrderBy( ... array<Clause^> ^args )
{
	// check for the null reference
	if( args == nullptr ) throw gcnew ArgumentNullException("args");
	// clone arguments list
	_args = safe_cast<array<Clause^>^>( args->Clone() );
}


//-------------------------------------------------------------------
/// <summary>
/// Implicit cast operator from OrderBy.Clause value to OrderBy. 
/// </summary>
//-------------------------------------------------------------------
OrderBy::operator OrderBy^( OrderBy::Clause ^clause )
{
	return gcnew OrderBy( clause );
}


//-----------------------------------------------------------------------------
//							Toolkit::RPL::Where::Clause
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Creates a new instance of the Where.Clause class (predicate)
/// where specified operand is equal to the specified value.
/// </summary>
//-------------------------------------------------------------------
Where::Clause::Clause( String ^opd, ValueBox value ) : \
	_opd(opd), _op(OP::EQ), _value(value)
{
	// check for the null reference
	if( opd == nullptr ) throw gcnew ArgumentNullException("opd");
}


//-------------------------------------------------------------------
/// <summary>
/// Creates a new instance of the Where.Clause class (predicate) 
/// consists of specified operand, comparison operator and value.
/// </summary>
//-------------------------------------------------------------------
Where::Clause::Clause( String ^opd, OP op, ValueBox value ) : \
	_opd(opd), _op(op), _value(value)
{
	// check for the null reference
	if( opd == nullptr ) throw gcnew ArgumentNullException("opd");
	// check for the valid operator value
	if( !safe_cast<System::Collections::IList^>(
			Enum::GetValues( OP::typeid ) )->Contains( op ) ) {
		// throw exception
		throw gcnew ArgumentOutOfRangeException("op");
	}
	if( (value == DBNull::Value) && !((op == OP::EQ) || (op ==OP::NE))) {
		// only "Equal" or "Not equal" comparison
		// for value of type DBNull is allowed
		throw gcnew ArgumentException(String::Format(
		ERR_COMPARISON_TYPE, String::Format( "{0} or {1}", OP::EQ, OP::NE ),
							 DBNull::typeid ), "op" );
	};
}


//-------------------------------------------------------------------
/// <summary>
/// Gets predicate operand.
/// </summary>
//-------------------------------------------------------------------
String^ Where::Clause::OPD::get( void )
{
	return _opd;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets predicate comparison operator.
/// </summary>
//-------------------------------------------------------------------
Where::Clause::OP Where::Clause::Operator::get( void )
{
	return _op;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets predicate value.
/// </summary>
//-------------------------------------------------------------------
ValueBox Where::Clause::Value::get( void )
{
	return _value;
}


//-----------------------------------------------------------------------------
//					Toolkit::RPL::Where::Operation::Or
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Class constructor. Creates combination of two predicates that uses
// the logical operation OR.
//
//-------------------------------------------------------------------
Where::Operation::Or::Or( Where ^left, Where ^right ) :	\
	_left(left), _right(right)
{
	// check for the null references
	if( left == nullptr ) throw gcnew ArgumentNullException("left");
	if( right == nullptr ) throw gcnew ArgumentNullException("right");
}


//-------------------------------------------------------------------
/// <summary>
/// Gets left search condition OR operator is applied to.
/// </summary>
//-------------------------------------------------------------------
Where^ Where::Operation::Or::LeftWhere::get( void )
{
	return _left;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets right search condition OR operator is applied to.
/// </summary>
//-------------------------------------------------------------------
Where^ Where::Operation::Or::RightWhere::get( void )
{
	return _right;
}


//-----------------------------------------------------------------------------
//					Toolkit::RPL::Where::Operation::And
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Class constructor. Creates combination of two predicates that uses
// the logical operation AND.
//
//-------------------------------------------------------------------
Where::Operation::And::And( Where ^left, Where ^right ) : \
	_left(left), _right(right)
{
	// check for the null references
	if( left == nullptr ) throw gcnew ArgumentNullException("left");
	if( right == nullptr ) throw gcnew ArgumentNullException("right");
}


//-------------------------------------------------------------------
/// <summary>
/// Gets left search condition AND operator is applied to.
/// </summary>
//-------------------------------------------------------------------
Where^ Where::Operation::And::LeftWhere::get( void )
{
	return _left;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets right search condition AND operator is applied to.
/// </summary>
//-------------------------------------------------------------------
Where^ Where::Operation::And::RightWhere::get( void )
{
	return _right;
}


//-----------------------------------------------------------------------------
//					Toolkit::RPL::Where::Operation::Not
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Class constructor. Creates agregation of the predicate that uses
// the logical operation NOT.
//
//-------------------------------------------------------------------
Where::Operation::Not::Not( Where ^sub ) : \
	_sub(sub)
{
	// check for the null reference
	if( sub == nullptr ) throw gcnew ArgumentNullException("sub");
}


//-------------------------------------------------------------------
/// <summary>
/// Gets search expression NOT operation is applied to.
/// </summary>
//-------------------------------------------------------------------
Where^ Where::Operation::Not::SubWhere::get( void )
{
	return _sub;
}


//-----------------------------------------------------------------------------
//							Toolkit::RPL::Where
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Creates combination of two predicates that uses the logical
/// operation OR.
/// </summary>
//-------------------------------------------------------------------
Where::Operation^ Where::operator |( Where ^left, Where ^right )
{
	return gcnew Where::Operation::Or(left, right);
}


//-------------------------------------------------------------------
/// <summary>
/// Creates combination of two predicates that uses the logical
/// operation AND.
/// </summary>
//-------------------------------------------------------------------
Where::Operation^ Where::operator &( Where ^left, Where ^right )
{
	return gcnew Where::Operation::And(left, right);
}


//-------------------------------------------------------------------
/// <summary>
/// Creates agregation of the predicate that uses the logical
/// operation NOT.
/// </summary>
//-------------------------------------------------------------------
Where::Operation^ Where::operator !( Where ^sub )
{
	return gcnew Where::Operation::Not(sub);
}
