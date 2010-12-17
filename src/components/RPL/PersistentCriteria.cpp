/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentCriteria.cpp										*/
/*																			*/
/*	Content:	Implementation of PersistentCriteria class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include ".\Factories\PersistenceBroker.h"
#include "PersistentCriteria.h"

using namespace _RPL;
using namespace _RPL::Factories;


//-----------------------------------------------------------------------------
//						Toolkit::RPL::PersistentCriteria
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Create new instance of the PersistentCriteria class for given
/// object type.
/// </summary>
//-------------------------------------------------------------------
PersistentCriteria::PersistentCriteria( String ^type ):					\
	_type(type), m_bottom(0), m_count(Int32::MaxValue), m_countFound(0)
{
	dbgprint( String::Format( "-> {0}\n{1}", 
							  this->GetType(), type ) );

	// check for initialized reference
	if( type == nullptr ) throw gcnew ArgumentNullException("type");

	dbgprint( String::Format( "<- {0}", this->GetType() ) );
}


//-------------------------------------------------------------------
/// <summary>
/// Clear search result and reset criteria to it initial state.
/// </summary><remarks>
/// This function must be called from setters that is used to compose
/// SQL request (those, that can change search result). Derived class
/// can override this function to process reset request.
/// </remarks>
//-------------------------------------------------------------------
void PersistentCriteria::Reset( void )
{
	m_list.Clear();
	m_countFound = 0;
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
/// Performs additional custom processes before removing all objects
/// from the PersistentCriteria instance.
/// </summary><remarks>
/// This sealed method raises error to notify about read-only
/// collection.
/// </remarks>
//-------------------------------------------------------------------
void PersistentCriteria::OnClear( void )
{
	throw gcnew InvalidOperationException(String::Format(
	ERR_SUPPORTED_OPERATION, this->GetType()->ToString() ));
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before removing an element
/// from the PersistentCriteria instance.
/// </summary><remarks>
/// This sealed method raises error to notify about read-only
/// collection.
/// </remarks>
//-------------------------------------------------------------------
void PersistentCriteria::OnRemove( PersistentObject ^obj )
{
	throw gcnew InvalidOperationException(String::Format(
	ERR_SUPPORTED_OPERATION, this->GetType()->ToString() ));
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before inserting a new object
/// into the PersistentCriteria instance.
/// </summary><remarks>
/// This sealed method raises error to notify about read-only
/// collection.
/// </remarks>
//-------------------------------------------------------------------
void PersistentCriteria::OnInsert( PersistentObject ^obj )
{
	throw gcnew InvalidOperationException( String::Format(
	ERR_SUPPORTED_OPERATION, this->GetType()->ToString() ));
}


//-------------------------------------------------------------------
/// <summary>
/// Gets object type this criteria was created for.
/// </summary>
//-------------------------------------------------------------------
String^ PersistentCriteria::Type::get( void )
{
	return _type;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets SQL WHERE clause.
/// </summary>
//-------------------------------------------------------------------
::Where^ PersistentCriteria::Where::get( void )
{
	return m_where;
}

void PersistentCriteria::Where::set( ::Where ^value )
{
	// clear content to prevent request-result collisions
	Reset();
	// store WHERE clause
	m_where = value;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets SQL ORDER BY clause.
/// </summary>
//-------------------------------------------------------------------
::OrderBy ^ PersistentCriteria::OrderBy::get( void )
{
	return m_orderBy;
}

void PersistentCriteria::OrderBy::set( ::OrderBy ^value )
{
	// clear content to prevent request-result collisions
	Reset();
	// store ORDER BY clause
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
	if( value < 0 ) throw gcnew ArgumentException(ERR_LESS_THEN_ZERRO);
	// clear content to prevent request-result collisions
	Reset();

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
	if( value < 0 ) throw gcnew ArgumentException(ERR_LESS_THEN_ZERRO);
	// clear content to prevent request-result collisions
	Reset();

	m_count = value;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets number of found objects using specified InnerQuery request
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
/// This operation is atomic and performs in transaction context.
/// </remarks>
//-------------------------------------------------------------------
void PersistentCriteria::Perform( void )
{
	// array to store search results
	array<HEADER>	^headers = nullptr;

	// lock storage for one executable thread
	Monitor::Enter( PersistenceBroker::Storage );
	try {
		// perform storage search request and set CountFound property
		m_countFound = PersistenceBroker::Storage->Search(
							_type,
							m_where, m_orderBy, m_bottom, m_count,
							headers );

		// first of all clear objects list
		m_list.Clear();
		// look through all founded object headers
		for each( HEADER header in headers ) {
			// get object from cache by header
			PersistentObject	^obj = PersistenceBroker::Cache[header];
			// disable add null references
			if( obj == nullptr ) continue;
			// and add object to the list
			m_list.Add( obj );
		}

		// process addition action depend on type of criteria
		try {
			// notify about search complete
			OnPerformComplete();
		} catch( Exception^ ) {
			// clear collection
			Reset();
			// and restore exception
			throw;
		}
	} finally {
		// unlock storage in any case
		Monitor::Exit( PersistenceBroker::Storage );
	}
}
