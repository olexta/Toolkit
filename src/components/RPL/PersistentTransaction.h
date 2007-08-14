/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentTransaction.h										*/
/*																			*/
/*	Content:	Definition of CPersistentTransaction class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"

using namespace System;
using namespace System::Collections::Generic;


_RPL_BEGIN
ref class CPersistentObject;
ref class CPersistentCriteria;

/// <sumamry>
/// Provide internal access to the CPersistentTransaction class.
/// </sumamry><remarks>
/// This is more flexible realisation of a "internal" access modifier. This
/// interface can be used in .NET Remoting.
/// </remarks>
private interface class IIPersistentTransaction
{
	void on_process( void );
};

/// <summary>
/// This class encapsulates the behavior needed to use transactions.
/// </summary><remarks>
/// Only you need is add criteria or object (with asked action) using Add
/// routine and perform transaction by calling Process(). If transaction will
/// not be succeded some error (from persistence storage) will be raised.
/// </remarks>
public ref class CPersistentTransaction sealed : MarshalByRefObject,
												 IIPersistentTransaction
{
public:
	enum class Actions { actNone, actRetrieve, actSave, actDelete };

private:
	value class Task
	{
	public:
		Object		^Obj;
		Actions		Act;
		
		Task(Object^ obj, Actions act ): Obj(obj), Act(act) {};
	};

private:
	List<Task>		m_tasks;
	Object			^_lock_this;

// IIPersistentTransaction
private:
	virtual void on_process( void ) sealed =
		IIPersistentTransaction::on_process;

public:
	CPersistentTransaction( void );

	void Add( CPersistentCriteria ^crit );
	void Add( CPersistentObject ^obj, Actions act );

	void Process( void );
};
_RPL_END