/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		RetrieveCriteria.cpp										*/
/*																			*/
/*	Content:	Implementation of RetrieveCriteria class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentObject.h"
#include "RetrieveCriteria.h"

using namespace _RPL;


//
// Define macro for determine MIN and MAX values
//
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))


//-----------------------------------------------------------------------------
//						Toolkit::RPL::RetrieveCriteria
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Clear search result.
/// </summary><remarks>
/// Call base class implementation and reset current cursor position
/// that could be modified by using cursor routine "Next".
/// </remarks>
//-------------------------------------------------------------------
void RetrieveCriteria::Reset( void )
{
	// call to base method
	PersistentCriteria::Reset();
	// and reset position
	m_pos = -1;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after filling collection by
/// proxies: make objects up-to-date and retrieves full objects from
/// persistence storage if needed.
/// </summary><remarks>
/// This is atomar operation, so it performs under transactional
/// control.
/// </remarks>
//-------------------------------------------------------------------
void RetrieveCriteria::OnPerformComplete( void )
{
	// declare stack of changes to emulate transaction
	Stack<ITransaction^>	changes;
	try {
		for each( PersistentObject ^obj in m_list ) {
			// save all object's properties
			safe_cast<ITransaction^>( obj )->Begin();
			// add push to stack to future rollback
			changes.Push( obj );

			// now make request based on type of retrieve criteria
			// (if no full retrieve is needed then make object
			// (proxy or full) up-to-date only)
			obj->Retrieve( !m_asProxies );
		}
		// retrieve operations was completed
		// successfuly, now commit object changes 
		while( changes.Count > 0 ) changes.Pop()->Commit();
	} catch( Exception^ ) {
		// revert all modified objects to previous state
		while( changes.Count > 0 ) changes.Pop()->Rollback();
		// and restore exception
		throw;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the RetrieveCriteria class to retrieve the
/// objects of given type.
/// </summary>
//-------------------------------------------------------------------
RetrieveCriteria::RetrieveCriteria( String ^type ): \
	PersistentCriteria(type), m_asProxies(false), m_pos(-1)
{
	// do nothing
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the RetrieveCriteria class to retrieve the
/// objects of given type that satisfy spicified WHERE clause.
/// </summary>
//-------------------------------------------------------------------
RetrieveCriteria::RetrieveCriteria( String ^type, ::Where ^where ): \
	PersistentCriteria(type), m_asProxies(false), m_pos(-1)
{
	m_where = where;
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the RetrieveCriteria class to retrieve the
/// objects of given type that sutisfy specified WHERE and ORDER BY
/// clauses.
/// </summary>
//-------------------------------------------------------------------
RetrieveCriteria::RetrieveCriteria( String ^type,						\
									::Where ^where, ::OrderBy ^order ):	\
	PersistentCriteria(type), m_asProxies(false), m_pos(-1)
{
	m_where = where;
	m_orderBy = order;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets retrieving method.
/// </summary><remarks>
/// If this property is set to true, then proxy objects will be
/// retrieved. This feature can be usefull to network traffic reduce
/// or to increase perfomance. By default is set to false.
/// </remarks>
//-------------------------------------------------------------------
bool RetrieveCriteria::AsProxies::get( void )
{
	return m_asProxies;
}

void RetrieveCriteria::AsProxies::set( bool value )
{
	m_asProxies = value;
}


//-------------------------------------------------------------------
/// <summary>
/// Read next count of objects from the recordset.
/// </summary><remarks>
/// Collection content will be cleared and filled by next new data.
/// Using of zero means clearing the collection and fill only common
/// properties such as CountFound.
/// Function return bool value that indicates not emty content of
/// resulting collection.
/// </remarks>
//-------------------------------------------------------------------
bool RetrieveCriteria::Next( unsigned short count )
{
	int oldBottom = m_bottom;
	int oldCount = m_count;

	// init cursor position for first request.
	// m_pos identify current cursor location for
	// request without top and count limits
	if( m_pos < 0 ) m_pos = m_bottom;

	// set new bottom and count limits according
	// to current cursor position and requested offset
	m_count = MIN(count, m_bottom + m_count - m_pos);
	m_bottom = MAX(m_bottom, m_pos);

	// try perform operation: we must restore old bottom
	// and count limits in any case, so use finally block
	try {
		Perform();

		// set new cursor location
		m_pos += Count;
	} finally {
		// restore bottom and top limits
		m_bottom = oldBottom;
		m_count = oldCount;
	}

	return (Count > 0);
}
