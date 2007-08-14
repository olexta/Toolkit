/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentTransaction.cpp									*/
/*																			*/
/*	Content:	Implementation of CPersistentTransaction class				*/
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

using namespace System::Threading;
using namespace RPL;


// Define lock macroses
#define ENTER(lock)		try { Monitor::Enter( lock );
#define EXIT(lock)		} finally { Monitor::Exit( lock ); }


//----------------------------------------------------------------------------
//							CPersistentTransaction
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// This function will be called by Broker to expose internal content
// of transaction. I make call to objects or to Broker (for atomic
// operations) only - the transaction context was initialized by
// Process operation.
//
//-------------------------------------------------------------------
void CPersistentTransaction::on_process( void )
{
	// pass through all tasks in transaction (i use here
	// track reference to prevent of value class copying)
	for each( Task %task in m_tasks ) {
		// determine type of object in task
		CPersistentObject ^obj =
			dynamic_cast<CPersistentObject^>( task.Obj );
		CPersistentCriteria ^crit =
			dynamic_cast<CPersistentCriteria^>( task.Obj );
		
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
/// Default constructor. Create instance of CPersistentTransaction
/// class.
/// </summary><remarks>
/// Create synhronization object only.
/// </remarks>
//-------------------------------------------------------------------
CPersistentTransaction::CPersistentTransaction( void ): \
	_lock_this(gcnew Object())
{
}


//-------------------------------------------------------------------
/// <summary>
/// Add some criteria request to transaction.
/// </summary>
//-------------------------------------------------------------------
void CPersistentTransaction::Add( CPersistentCriteria ^crit )
{ENTER(_lock_this)

	if( crit == nullptr ) throw gcnew ArgumentNullException("crit");
	
	// add to list of tasks
	m_tasks.Add( Task(crit, Actions::actNone) );

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Add some object manipulation request to transaction.
/// </summary>
//-------------------------------------------------------------------
void CPersistentTransaction::Add( CPersistentObject ^obj, Actions act )
{ENTER(_lock_this)

	if( obj == nullptr ) throw gcnew ArgumentNullException("obj");
	
	// add to list of tasks
	m_tasks.Add( Task(obj, act) );

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Process transaction. 
/// </summary>
//-------------------------------------------------------------------
void CPersistentTransaction::Process( void )
{ENTER(_lock_this)

	//call to PersistanceBroker
	CPersistenceBroker::Broker->Process( this );

EXIT(_lock_this)}