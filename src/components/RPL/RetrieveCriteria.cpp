/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		RetrieveCriteria.cpp										*/
/*																			*/
/*	Content:	Implementation of CRetrieveCriteria class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentObject.h"
#include "RetrieveCriteria.h"

using namespace System::Threading;
using namespace RPL;


// Define lock macroses
#define ENTER(lock)		try { Monitor::Enter( lock );
#define EXIT(lock)		} finally { Monitor::Exit( lock ); }

// Define macro for MIN and MAX
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))


//----------------------------------------------------------------------------
//								CRetrieveCriteria
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Clear all results of collection processing.
/// </summary><remarks>
/// Call base class implementation and reset current cursor position
/// that could be modified by using cursor routines: "Move".
/// </remarks>
//-------------------------------------------------------------------
void CRetrieveCriteria::ResetResults( void )
{
	// call to base method
	CPersistentCriteria::ResetResults();
	// and reset position
	m_pos = -1;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes when transaction starts.
/// </summary><remarks>
/// I must store all object data (create object's save point) until
/// "OnTransactionComplete" will be called to have ability to restore
/// data by "OnTransactionRollback".
/// </remarks>
//-------------------------------------------------------------------
void CRetrieveCriteria::OnTransactionBegin( void )
{
	// create new struct to store backup data
	BACKUP_STRUCT ^backup = gcnew BACKUP_STRUCT();

	// copy simple object attributes
	backup->_as_proxies = m_asProxies;
	backup->_pos = m_pos;

	// store backup in transaction stack
	m_trans_stack.Push( backup );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes for successfull transaction.
/// </summary><remarks>
/// Transaction was completed successfuly, so i must free resources
/// for transaction support (delete object's save point).
/// </remarks>
//-------------------------------------------------------------------
void CRetrieveCriteria::OnTransactionCommit( void )
{
	// remove stored backup data
	m_trans_stack.Pop();
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes when transaction fails.
/// </summary><remarks>
/// Transaction was failed, so i need to rollback object to previous
/// state (object's save point).
/// </remarks>
//-------------------------------------------------------------------
void CRetrieveCriteria::OnTransactionRollback( void )
{
	// get sored backup data
	BACKUP_STRUCT ^backup = static_cast<BACKUP_STRUCT^>( m_trans_stack.Pop() );
	
	// restore simple object attributes
	m_asProxies = backup->_as_proxies;
	m_pos = backup->_pos;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after filling collection by
/// proxies.
/// </summary><remarks>
/// Retrieves full objects from persistence storage. Because of many
/// custom processings exists for each criteria i not retrieve full
/// objects in CPersistentBroker (even if this option is active for
/// retrieve criteria). There are a lot of class schemas that can
/// have it's own implementation, so i simple call Retrieve for each
/// object to get it's links and properties from persistent storage.
/// </remarks>
//-------------------------------------------------------------------
void CRetrieveCriteria::OnPerformComplete( void )
{
	// we got objects as proxies already
	if( !m_asProxies ) {
		// pass througght all founded objects
		for each( CPersistentObject ^obj in m_list ) {
			// and retrieve links and properties
			obj->Retrieve();
		}
	}
}

//-------------------------------------------------------------------
/// <summary>
/// Create instance of the CRetrieveCriteria class to retrieve the
/// objects of given type.
/// </summary>
//-------------------------------------------------------------------
CRetrieveCriteria::CRetrieveCriteria( String ^type ): \
	CPersistentCriteria(type), m_asProxies(false), m_pos(-1)
{
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the CRetrieveCriteria class to retrieve the
/// objects of given type that satisfy spicified WHERE clause.
/// </summary>
//-------------------------------------------------------------------
CRetrieveCriteria::CRetrieveCriteria( String ^type, String ^sWhere ): \
	CPersistentCriteria(type), m_asProxies(false), m_pos(-1)
{
	Where = sWhere;
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the CRetrieveCriteria class to retrieve the
/// objects of given type that sutisfy spicified WHERE and ORDER BY
/// clauses.
/// </summary>
//-------------------------------------------------------------------
CRetrieveCriteria::CRetrieveCriteria( String ^type, String ^sWhere, String ^orderBy ): \
	CPersistentCriteria(type), m_asProxies(false), m_pos(-1)
{
	Where = sWhere;
	OrderBy = orderBy;
}


//-------------------------------------------------------------------
/// <summary>
/// Create retrieve criteria based on another CPersistentCriteria
/// instance.
/// </summary><remarks>
/// Copy all internal data: base class will copy common propertis
/// except collection content.
/// </remarks>
//-------------------------------------------------------------------
CRetrieveCriteria::CRetrieveCriteria( const CPersistentCriteria %crit ): \
	CPersistentCriteria(crit), m_asProxies(false), m_pos(-1)
{
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
bool CRetrieveCriteria::AsProxies::get( void )
{	
	return m_asProxies;
}


void CRetrieveCriteria::AsProxies::set( bool value )
{ENTER(_lock_this)

	m_asProxies = value;

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Read next/previous count of objects from recordset.
/// </summary><remarks>
/// Collection content will be cleared and filled by next/previous
/// new data. Positive sign of count means using "next" request and
/// negative means "previous". Using of zero means clearing the
/// collection and fill only common properties such as CountFound.
/// Function return bool value that indicates not emty content of
/// collection.
/// </remarks>
//-------------------------------------------------------------------
bool CRetrieveCriteria::Move( int count )
{ENTER(_lock_this)

	int	oldBottom = m_bottom;
	int oldCount = m_count;


	// init cursor position for first request.
	// m_pos identify current cursor location for
	// request without top and count limits
	if( m_pos < 0 ) m_pos = m_bottom;

	// set new bottom and count limits according
	// to current cursor position and requested offset
	if( count > 0 ) {

		m_count = MIN(count, m_bottom + m_count - m_pos);
		m_bottom = MAX(m_bottom, m_pos);
	} else {

		m_bottom = MAX(m_bottom, m_pos + count);
		m_count = m_pos - m_bottom;
	}

	// try perform operation. we must restore old bottom
	// and count limits by fail, so in use finally block
	try {
		Perform();

		// set new cursor location. if after last request count
		// of elements was changed, then for "previous" offset
		// i set new position directly (not throught returned
		// value: Count)
		m_pos += (count > 0 ? Count : -m_count);
	} finally {
		// restore bottom and top limits
		m_bottom = oldBottom;
		m_count = oldCount;
	}

	return (Count > 0);

EXIT(_lock_this)}