/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		UpdateCriteria.cpp											*/
/*																			*/
/*	Content:	Implementation of UpdateCriteria class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentProperty.h"
#include "ObjectProperties.h"
#include "PersistentObject.h"
#include "UpdateCriteria.h"

using namespace _RPL;


//-----------------------------------------------------------------------------
//							Toolkit::RPL::UpdateCriteria
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes when transaction starts.
/// </summary><remarks>
/// I must store all object data (create object's save point) until
/// "OnTransactionComplete" will be called to have ability to restore
/// data by "OnTransactionRollback".
/// </remarks>
//-------------------------------------------------------------------
void UpdateCriteria::OnTransactionBegin( void )
{
	// create new struct to store backup data
	BACKUP_STRUCT ^backup = gcnew BACKUP_STRUCT();
	
	// copy properties
	backup->_props = gcnew PersistentProperties(m_props);
	
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
void UpdateCriteria::OnTransactionCommit( void )
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
void UpdateCriteria::OnTransactionRollback( void )
{
	// get sored backup data
	BACKUP_STRUCT ^backup = m_trans_stack.Pop();
	
	// restore properties by simple copy reference
	m_props = backup->_props;
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after filling collection by
/// proxies.
/// </summary><remarks>
/// Update objects in the persistence storage. I can't simply create
/// UPDATE SQL request to prevent data lost without business logic
/// checking. So, objects retrieve with all properties and links. For
/// each object set new property values and call Save method to store
/// new properties in persistence mechanism. If no errors was raised
/// criteria will contain the set of updated objects.
/// </remarks>
//-------------------------------------------------------------------
void UpdateCriteria::OnPerformComplete( void )
{
	// now, i set all needed properties to founded objects
	for each( PersistentObject ^obj in m_list ) {
		// pass throught all properties to modify
		for each( PersistentProperty ^prop in m_props ) {
			// retrieve object from storage (now it's proxy)
			obj->Retrieve();
			// set property value
			*((IIPersistentObject^) obj)->Properties[prop->Name] = prop->Value;
		}
		// and save object
		obj->Save();
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the UpdateCriteria class to update objects of
/// given type.
/// </summary>
//-------------------------------------------------------------------
UpdateCriteria::UpdateCriteria( String ^type ): \
	PersistentCriteria(type)
{
	m_props = gcnew PersistentProperties();
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the UpdateCriteria class to update objects of
/// given type that satisfy spicified WHERE clause.
/// </summary>
//-------------------------------------------------------------------
UpdateCriteria::UpdateCriteria( String ^type, String ^sWhere ): \
	PersistentCriteria(type)
{
	Where = sWhere;

	m_props = gcnew PersistentProperties();
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the UpdateCriteria class to update objects of
/// given type that satisfy spicified WHERE and ORDER BY clause.
/// </summary>
//-------------------------------------------------------------------
UpdateCriteria::UpdateCriteria( String ^type, String ^sWhere, String ^orderBy ): \
	PersistentCriteria(type)
{
	Where = sWhere;
	OrderBy = orderBy;

	m_props = gcnew PersistentProperties();
}


//-------------------------------------------------------------------
/// <summary>
/// Create update criteria based on the another PersistentCriteria
/// instance.
/// </summary><remarks>
/// Copy all common clauses only.
/// </remarks>
//-------------------------------------------------------------------
UpdateCriteria::UpdateCriteria( const PersistentCriteria %crit ): \
	PersistentCriteria(crit)
{
	m_props = gcnew PersistentProperties();
}


//-------------------------------------------------------------------
/// <summary>
/// Gets collection of properties to modify.
/// </summary>
//-------------------------------------------------------------------
PersistentProperties^ UpdateCriteria::Properties::get( void )
{
	return m_props;	
}