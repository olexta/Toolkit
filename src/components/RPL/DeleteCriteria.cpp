**************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		DeleteCriteria.cpp											*/
/*																			*/
/*	Content:	Implementation of DeleteCriteria class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentObject.h"
#include "PersistentTransaction.h"
#include "DeleteCriteria.h"

using namespace _RPL;


//-----------------------------------------------------------------------------
//							Toolkit::RPL::DeleteCriteria
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after filling collection by
/// proxies: delete objects in persistence storage.
/// </summary><remarks><para>
/// I can't simply create DELETE SQL request to prevent data lost
/// without business logic checking: then, i use single Delete call
/// to each object to apply business check. But this is atomar
/// operation, so it performs under transactional control.</para><para>
/// If no errors was raised then criteria will contain set of deleted
/// objects.
/// </para></remarks>
//-------------------------------------------------------------------
void DeleteCriteria::OnPerformComplete( void )
{
	// declare stack of changes to emulate transaction
	Stack<ITransaction^>	changes;
	try {
		for each( PersistentObject ^obj in m_list ) {
			// save all object's properties
			safe_cast<ITransaction^>( obj )->Begin();
			// push to stack to future rollback
			changes.Push( obj );

			// now make retrieve request
			obj->Retrieve( true );
		}
		// create new transaction with list of objects
		PersistentTransaction	^trans = gcnew PersistentTransaction();
		trans->Add( %m_list, PersistentTransaction::ACTION::Delete );
		// and process it
		trans->Process();

		// all changes was completed successfuly, save it
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
/// Create "empty" (for all objects) delete criteria instance
/// coresponding to specified object's type.
/// </summary>
//-------------------------------------------------------------------
DeleteCriteria::DeleteCriteria( String ^type ): \
	PersistentCriteria( type )
{
	// do nothing
};


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the DeleteCriteria class for given object's
/// type that contains specified WHERE clause.
/// </summary>
//-------------------------------------------------------------------
DeleteCriteria::DeleteCriteria( String ^type, ::Where ^where ): \
	PersistentCriteria( type )
{
	m_where = where;
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the DeleteCriteria class for given type that
/// contains spicified WHERE and ORDER BY clauses.
/// </summary>
//-------------------------------------------------------------------
DeleteCriteria::DeleteCriteria( String ^type,					    \
								::Where ^where, ::OrderBy ^order ): \
	PersistentCriteria(type)
{
	m_where = where;
	m_orderBy = order;
}
