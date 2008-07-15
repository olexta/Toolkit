/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentTransaction.cpp									*/
/*																			*/
/*	Content:	Implementation of PersistentTransaction class				*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include ".\Factories\PersistenceBroker.h"
#include "PersistentObject.h"
#include "PersistentTransaction.h"

using namespace _RPL;
using namespace _RPL::Factories;


//----------------------------------------------------------------------------
//				Toolkit::RPL::PersistentTransaction::Task
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Create new instance of the PersistentTransaction::Task object and
// initialise it with specified object's action.
//
//-------------------------------------------------------------------
PersistentTransaction::Task::Task( PersistentObject ^obj, ACTION act ) : \
	m_obj(obj), m_act(act)
{
	// check for the null reference
	if( obj == nullptr ) throw gcnew ArgumentNullException("obj");
}


//-------------------------------------------------------------------
//
// Perform specified operation.
//
// This function performs atomar action, so it returns ITransaction
// interface to be able rollback changes later.
//
//-------------------------------------------------------------------
ITransaction^ PersistentTransaction::Task::Perform( void )
{
	// save all internal object's data
	static_cast<ITransaction^>( m_obj )->Begin();

	try {
		// check for action and call appropriate object method
		switch( m_act ) {
			case ACTION::Retrieve:
				m_obj->Retrieve( false );
			break;

			case ACTION::Upgrade:
				m_obj->Retrieve( true );
			break;
			
			case ACTION::Save:
				m_obj->Save();
			break;
			
			case ACTION::Delete:
				m_obj->Delete();
			break;
		}
	} catch( Exception^ ) {
		// rollback last changes that was make at object
		static_cast<ITransaction^>( m_obj )->Rollback();
		// and restore exception
		throw;
	}
	// if operation succeded than return ITransaction
	// interface to object to be able revert operation later
	return m_obj;
}


//----------------------------------------------------------------------------
//					Toolkit::RPL::PersistentTransaction
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Default constructor. Create instance of the PersistentTransaction
/// class.
/// </summary>
//-------------------------------------------------------------------
PersistentTransaction::PersistentTransaction( void ) : \
	_tasks(gcnew Queue<Task>)
{
	// do nothing
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the PersistentTransaction class and fill it by
/// specified action at persistent object.
/// </summary>
//-------------------------------------------------------------------
PersistentTransaction::PersistentTransaction( PersistentObject ^obj, \
											  ACTION action ) :		 \
	_tasks(gcnew Queue<Task>)
{
	// check for initialized reference
	if( obj == nullptr ) throw gcnew ArgumentNullException("obj");

	// add specified task to queue
	_tasks->Enqueue( Task( obj, action ) );
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the PersistentTransaction class and fill it by
/// specified action at the collection of persistent objects.
/// </summary><remarks>
/// All null references in the collection will be ignored.
/// </remarks>
//-------------------------------------------------------------------
PersistentTransaction::
PersistentTransaction( IEnumerable<PersistentObject^> ^objs, ACTION action ) : \
	_tasks(gcnew Queue<Task>)
{
	// check for initialized reference
	if( objs == nullptr ) throw gcnew ArgumentNullException("objs");

	// add specified tasks to queue
	for each( PersistentObject ^obj in objs  ) {
		// ignore null references
		if( obj != nullptr ) _tasks->Enqueue( Task( obj, action ) );
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Add some object manipulation request to transaction.
/// </summary>
//-------------------------------------------------------------------
void PersistentTransaction::Add( PersistentObject ^obj, ACTION action )
{
	// check for the null reference
	if( obj == nullptr ) throw gcnew ArgumentNullException("obj");

	// add to list of tasks
	_tasks->Enqueue( Task(obj, action) );
}


//-------------------------------------------------------------------
/// <summary>
/// Add some objects manipulation request to transaction.
/// </summary><remarks>
/// All null references in the collection will be ignored.
/// </remarks>
//-------------------------------------------------------------------
void PersistentTransaction::Add( IEnumerable<PersistentObject^> ^objs, \
								 ACTION action )
{
	// check for the null reference
	if( objs == nullptr ) throw gcnew ArgumentNullException("objs");

	// add specified tasks to queue
	for each( PersistentObject ^obj in objs  ) {
		// ignore null references
		if( obj != nullptr ) _tasks->Enqueue( Task( obj, action ) );
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Process transaction.
/// </summary><remarks>
/// Exposes internal content of transaction.
/// </remarks>
//-------------------------------------------------------------------
void PersistentTransaction::Process( void )
{
	Stack<ITransaction^>	changes;

	// lock storage for one executable thread 
	Monitor::Enter( PersistenceBroker::Storage );
	try {
		// begin storage transaction
		PersistenceBroker::Storage->TransactionBegin();
		try {
			// process all tasks in right order
			while( _tasks->Count > 0 ) {
				// extract from queue
				Task	task = _tasks->Dequeue();
				// and push to stack ITransaction interface
				// (this adds posibility of the future rollback)
				changes.Push( task.Perform() );
			}
			// all operations comleted successfuly: commit storage
			PersistenceBroker::Storage->TransactionCommit();
			// and object changes
			while( changes.Count > 0 ) changes.Pop()->Commit();
		} catch( Exception^ ) {
			// rollback all object changes
			while( changes.Count > 0 ) changes.Pop()->Rollback();
			// and storage changes
			PersistenceBroker::Storage->TransactionRollback();
			// and restore exception
			throw;
		}
	} finally {
		// unlock storage in any case
		Monitor::Exit( PersistenceBroker::Storage );
	}
}