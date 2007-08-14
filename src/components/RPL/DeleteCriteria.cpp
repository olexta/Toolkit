/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		DeleteCriteria.cpp											*/
/*																			*/
/*	Content:	Implementation of CDeleteCriteria class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentObject.h"
#include "DeleteCriteria.h"

using namespace RPL;


//----------------------------------------------------------------------------
//								CDeleteCriteria
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Perform delete operation in persistence storage.
/// </summary><remarks>
/// I can't simply create DELETE SQL request to prevent data lost
/// without business logic checking. So, i use single call of Delete
/// to retrieve full object and apply business check. If no errors
/// was raised then criteria will contain set of deleted objects.
/// </remarks>
//-------------------------------------------------------------------
void CDeleteCriteria::OnPerformComplete( void )
{
	// pass throught all founded objects
	for each( CPersistentObject ^obj in m_list ) {
		// call method of deletion
		obj->Delete();
	}
}

//-------------------------------------------------------------------
/// <summary>
/// Create "empty" (for all objects) delete criteria instance
/// coresponding to specified object's type.
/// </summary>
//-------------------------------------------------------------------
CDeleteCriteria::CDeleteCriteria( String ^type ): \
	CPersistentCriteria( type )
{
};


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the CDeleteCriteria class for given object's
/// type that contains specified WHERE clause.
/// </summary>
//-------------------------------------------------------------------
CDeleteCriteria::CDeleteCriteria( String ^type, String ^sWhere ): \
	CPersistentCriteria( type )
{
	Where = sWhere;
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the CDeleteCriteria class for given type that
/// contains spicified WHERE and ORDER BY clauses.
/// </summary>
//-------------------------------------------------------------------
CDeleteCriteria::CDeleteCriteria( String ^type, String ^sWhere, String ^orderBy ): \
	CPersistentCriteria(type)
{
	Where = sWhere;
	OrderBy = orderBy;
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the CDeleteCriteria class based on another
/// CPersistentCriteria instance.
/// </summary><remarks>
/// Copy common clauses only from the specified instance.
/// </remarks>
//-------------------------------------------------------------------
CDeleteCriteria::CDeleteCriteria( const CPersistentCriteria %crit ): \
	CPersistentCriteria(crit)
{}