/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentTransaction.cpp									*/
/*																			*/
/*	Content:	Implementation of PersistentTransaction class				*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistenceBroker.h"
#include "PersistentObject.h"
#include "PersistentCriteria.h"
#include "PersistentTransaction.h"

using namespace _RPL;


//----------------------------------------------------------------------------
//					Toolkit::RPL::PersistentTransaction
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// This function will be called by Broker to expose internal content
// of transaction. I make call to objects or to Broker (for atomic
// operations) only - the transaction context was initialized by
// Process operation.
//
//-------------------------------------------------------------------
void PersistentTransaction::on_process( void )
{
	// pass through all tasks in transaction (i use here
	// track reference to prevent of value class copying)
	for each( Task %task in m_tasks ) {
		// determine type of object in task
		PersistentObject	^obj = dynamic_cast<PersistentObject^>( task.Obj );
		PersistentCriteria	^crit = dynamic_cast<PersistentCriteria^>( task.Obj );

		if( obj != nullptr ) {
			// check for action and call appropriate
			// object method (i cann't call Broker method
			// because of non atomic operation)
			switch( task.Act ) {
				case Actions::actRetrieve:
					obj->Retrieve();
				break;
				case Actions::actSave:
					obj->Save();
				break;
				case Actions::actDelete:
					obj->Delete();
				break;
			}
		} else if( crit != nullptr ) {
			// call to criteria (i cann't call Broker method
			// because of non atomic operation)
			crit->Perform();
		}
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Default constructor. Create instance of PersistentTransaction
/// class.
/// </summary><remarks>
/// Create synhronization object only.
/// </remarks>
//-------------------------------------------------------------------
PersistentTransaction::PersistentTransaction( void )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Add some criteria request to transaction.
/// </summary>
//-------------------------------------------------------------------
void PersistentTransaction::Add( PersistentCriteria ^crit )
{
	if( crit == nullptr ) throw gcnew ArgumentNullException("crit");

	// add to list of tasks
	m_tasks.Add( Task(crit, Actions::actNone) );
}


//-------------------------------------------------------------------
/// <summary>
/// Add some object manipulation request to transaction.
/// </summary>
//-------------------------------------------------------------------
void PersistentTransaction::Add( PersistentObject ^obj, Actions act )
{
	if( obj == nullptr ) throw gcnew ArgumentNullException("obj");

	// add to list of tasks
	m_tasks.Add( Task(obj, act) );
}


//-------------------------------------------------------------------
/// <summary>
/// Process transaction. 
/// </summary>
//-------------------------------------------------------------------
void PersistentTransaction::Process( void )
{
	//call to PersistanceBroker
	PersistenceBroker::Broker->Process( this );
}
