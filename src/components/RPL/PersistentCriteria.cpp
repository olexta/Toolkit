/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentCriteria.cpp										*/
/*																			*/
/*	Content:	Implementation of PersistentCriteria class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistenceBroker.h"
#include "PersistentCriteria.h"

using namespace _RPL;


// Define macros to ignore exceptions
#define TRY(expr)		try { expr; } catch( Exception^ ) {};


//----------------------------------------------------------------------------
//						Toolkit::RPL::PersistentCriteria
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// This internal function is needed for PersistentBroker to fill our
// collection by result. To make PersistentBroker interface more easy
// i implement this routine. So, this function receive proxy objects
// and fill collection by it. To make addition processing you must
// override it.
//
//-------------------------------------------------------------------
void PersistentCriteria::on_perform( int found, IEnumerable<PersistentObject^> ^objs )
{
	// store CountFound property
	m_countFound = found;
	// first of all clear current list
	m_list.Clear();
	// and add elements to the end of it
	for each( PersistentObject ^obj in objs ) {
		// disable add null references
		if( obj == nullptr ) continue;
		// and add object if it is not
		// in collection already
		if( !Contains( obj ) ) m_list.Add( obj );
	}
	
	// now we must process all addition actions
	// that depends on type of criteria 
	try {
		// notify about perfom complete
		OnPerformComplete();
	} catch( Exception^ ) {
		// clear collection
		ResetResults();
		// and rerise exception
		throw;
	}
}


//-------------------------------------------------------------------
//
// I must store all object data until trans_commit will be called to
// have ability to restore data by trans_rollback.
//
//-------------------------------------------------------------------
void PersistentCriteria::trans_begin( void )
{
	// addition processing before transaction begin
	TRY( OnTransactionBegin() )

	// make object copy
	BACKUP_STRUCT ^backup = gcnew BACKUP_STRUCT();
	// copy simple object attributes
	backup->_type = m_type;
	backup->_inner_query = m_innerQuery;
	backup->_where = m_where;
	backup->_order_by = m_orderBy;
	backup->_bottom = m_bottom;
	backup->_count = m_count;
	backup->_count_found = m_countFound;
	// copy content
	backup->_objs = gcnew PersistentObjects(%m_list);

	// store backup in transaction stack
	m_trans_stack.Push( backup );
}


//-------------------------------------------------------------------
//
// Transaction was completed successfuly, then we must free resources
// for transaction support.
//
//-------------------------------------------------------------------
void PersistentCriteria::trans_commit( void )
{
	// addition processing before transaction commit
	TRY( OnTransactionCommit() )

	// remove stored backup data
	m_trans_stack.Pop();
}


//-------------------------------------------------------------------
//
// Transaction failed, so we need to rollback object to previous
// state.
//
//-------------------------------------------------------------------
void PersistentCriteria::trans_rollback( void )
{
	// addition processing before transaction rollback
	TRY( OnTransactionRollback() )
	
	// get sored backup data
	BACKUP_STRUCT ^backup = m_trans_stack.Pop();

	// restore simple object attributes
	m_type = backup->_type;
	m_innerQuery = backup->_inner_query;
	m_where = backup->_where;
	m_orderBy = backup->_order_by;
	m_bottom = backup->_bottom;
	m_count = backup->_count;
	m_countFound = backup->_count_found;
	// restore content
	m_list.Clear();
	m_list.AddRange( backup->_objs );
}


//-------------------------------------------------------------------
/// <summary>
/// Create new instance of the PersistentCriteria class for given
/// object type.
/// </summary>
//-------------------------------------------------------------------
PersistentCriteria::PersistentCriteria( String ^type ): \
	m_innerQuery(""), m_where(""), m_orderBy(""),		\
	m_bottom(0), m_count(Int32::MaxValue), m_countFound(0)
{
	dbgprint( String::Format( "-> {0}\n{1}", 
							  this->GetType(), type ) );

	// check for initialized reference
	if( type == nullptr ) throw gcnew ArgumentNullException("type");

	m_type = type;

	dbgprint( String::Format( "<- {0}", this->GetType() ) );
}


//-------------------------------------------------------------------
/// <summary>
/// Create persistent criteria based on another PersistentCriteria
/// instance.
/// </summary><remarks>
/// Copy only common clauses (content of the criteria will not be
/// copied).
/// </remarks>
//-------------------------------------------------------------------
PersistentCriteria::PersistentCriteria( const PersistentCriteria %crit )
{
	// first of all clear current content
	m_list.Clear();
	
	// and copy all criterias
	m_type = crit.m_type;
	m_innerQuery = crit.m_innerQuery;
	m_where = crit.m_where;
	m_orderBy = crit.m_orderBy;
	m_bottom = crit.m_bottom;
	m_count = crit.m_count;
	
	// we create empty collection
	m_countFound = 0;
}


//-------------------------------------------------------------------
/// <summary>
/// Clear all results of collection processing.
/// </summary><remarks>
/// This function must be called by derived class property setters
/// that is used to create SQL requet (that can change search result).
/// </remarks>
//-------------------------------------------------------------------
void PersistentCriteria::ResetResults( void )
{
	m_list.Clear();
	m_countFound = 0;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes when transaction starts.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to create it's own save point.
/// </remarks>
//-------------------------------------------------------------------
void PersistentCriteria::OnTransactionBegin( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes for successfull transaction.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to delete it's own save point that
/// was created by "OnTransactionBegin" early.
/// </remarks>
//-------------------------------------------------------------------
void PersistentCriteria::OnTransactionCommit( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes when transaction fails.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to restore it's state to previous,
/// saved by "OnTransactionBegin".
/// </remarks>
//-------------------------------------------------------------------
void PersistentCriteria::OnTransactionRollback( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after filling collection by
/// proxies.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action depend on
/// criteria type.
/// </remarks>
//-------------------------------------------------------------------
void PersistentCriteria::OnPerformComplete( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before removing an element
/// from the PersistentCriteria instance.
/// </summary><remarks>
/// This sealed method raises error to notify about read-only object.
/// </remarks>
//-------------------------------------------------------------------
void PersistentCriteria::OnRemove( PersistentObject ^obj )
{
	throw gcnew InvalidOperationException("The collection is read-only!");
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before inserting a new object
/// into the PersistentCriteria instance.
/// </summary><remarks>
/// This sealed method raises error to notify about read-only object.
/// </remarks>
//-------------------------------------------------------------------
void PersistentCriteria::OnInsert( PersistentObject ^obj )
{
	throw gcnew InvalidOperationException("The collection is read-only!");
}


//-------------------------------------------------------------------
/// <summary>
/// Gets object type this criteria was created for.
/// </summary>
//-------------------------------------------------------------------
String^ PersistentCriteria::Type::get( void )
{
	return m_type;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets additional SQL request.
/// </summary><remarks>
/// This is SQL request to which WHERE and ORDER BY clauses will be
/// applied. You have to avoid using of this approach, because in
/// this case you use internal DB structure that can be modified. Of
/// course, not all criterias can have this property. In this case it
/// must be overriden by derived class to throw exception.
/// </remarks>
//-------------------------------------------------------------------
String^ PersistentCriteria::InnerQuery::get( void )
{
	return m_innerQuery;
}

void PersistentCriteria::InnerQuery::set( String ^value )
{
	// check for initialized reference
	if( value == nullptr ) throw gcnew ArgumentNullException("value");
	// clear content to prevent request-result collisions
	ResetResults();

	m_innerQuery = value;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets SQL WHERE clause.
/// </summary><remarks>
/// This clause presents in all criterias, but for more capability i
/// provide it as virtual property to give ability to override it if
/// needed.
/// </remarks>
//-------------------------------------------------------------------
String^ PersistentCriteria::Where::get( void )
{
	return m_where;
}

void PersistentCriteria::Where::set( String ^value )
{	
	// check for initialized reference
	if( value == nullptr ) throw gcnew ArgumentNullException("value");
	// clear content to prevent request-result collisions
	ResetResults();

	m_where = value;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets SQL ORDER BY clause.
/// </summary><remarks>
/// Not all criterias can have this property. In this case it must be
/// overriden by derived class to throw exception.
/// </remarks>
//-------------------------------------------------------------------
String^ PersistentCriteria::OrderBy::get( void )
{	
	return m_orderBy;
}

void PersistentCriteria::OrderBy::set( String ^value )
{
	// check for initialized reference
	if( value == nullptr ) throw gcnew ArgumentNullException("value");
	// clear content to prevent request-result collisions
	ResetResults();
	
	m_orderBy = value;
}



//-------------------------------------------------------------------
/// <summary>
/// Gets or sets bottom limit in criteria request.
/// </summary><remarks>
/// This is 0-based index of the recordset's row retrieved from DB.
/// By default is set to 0.
/// </remarks>
//-------------------------------------------------------------------
int PersistentCriteria::BottomLimit::get( void )
{
	return m_bottom;
}

void PersistentCriteria::BottomLimit::set( int value )
{
	if( value < 0 ) throw gcnew ArgumentException("Incorrect Bottom value!");
	
	// clear content to prevent request-result collisions
	ResetResults();
	
	m_bottom = value;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets count limit in criteria request.
/// </summary><remarks>
/// This is the number of rows that will be retrieved. By default it
/// is set to int::MaxValue.
/// </remarks>
//-------------------------------------------------------------------
int PersistentCriteria::CountLimit::get( void )
{
	return m_count;
}

void PersistentCriteria::CountLimit::set( int value )
{
	if( value < 0 ) {

		throw gcnew ArgumentException("Incorrect CountLimit value!");
	}
	// clear content to prevent request-result collisions
	ResetResults();

	m_count = value;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets number of founded objects using specified InnerQuery request
/// with current WHERE clause.
/// </summary><remarks>
/// To get count of objects that was processed by this criteria
/// request (with specified "BottomLimit" and "CountLimit" values) 
/// use "Count" property.
/// </remarks>
//-------------------------------------------------------------------
int PersistentCriteria::CountFound::get( void )
{
	return m_countFound;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets the object at the specified index.
/// </summary><remarks>
/// This is default indexed property.
/// </remarks>
//-------------------------------------------------------------------
PersistentObject^ PersistentCriteria::default::get( int index )
{
	return m_list[index];
}


//-------------------------------------------------------------------
/// <summary>
/// Determines the index of a specific object in the collection. 
/// </summary>
//-------------------------------------------------------------------
int PersistentCriteria::IndexOf( PersistentObject ^obj )
{
	return m_list.IndexOf( obj );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs criteria operation.
/// </summary><remarks>
/// After searching of objects "on_perform" will be called to fill
/// collection by result (proxy objects) and to make addition
/// processing depend on criteria type. If you want to use perform
/// operation as atomic, it must be called in transaction context.
/// </remarks>
//-------------------------------------------------------------------
void PersistentCriteria::Perform( void )
{
	PersistenceBroker::Broker->Process( this );
}