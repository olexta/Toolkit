/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentTransaction.h										*/
/*																			*/
/*	Content:	Definition of PersistentTransaction class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"

using namespace System;


_RPL_BEGIN
ref class PersistentObject;
ref class PersistentCriteria;

/// <sumamry>
/// Provide internal access to the PersistentTransaction class.
/// </sumamry><remarks>
/// This is more flexible realisation of a "internal" access modifier. This
/// interface can be used in .NET Remoting.
/// </remarks>
private interface class IIPersistentTransaction
{
	void OnProcess( void );
};

/// <summary>
/// This class encapsulates the behavior needed to use transactions.
/// </summary><remarks>
/// Only you need is add criteria or object (with asked action) using Add
/// routine and perform transaction by calling Process(). If transaction will
/// not be succeded some error (from persistence storage) will be raised.
/// </remarks>
public ref class PersistentTransaction sealed : MarshalByRefObject,
												IIPersistentTransaction
{
public:
	typedef enum class Actions { actNone, actRetrieve, actSave, actDelete };

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

// IIPersistentTransaction
private:
	virtual void on_process( void ) sealed =
		IIPersistentTransaction::OnProcess;

public:
	PersistentTransaction( void );

	void Add( PersistentCriteria ^crit );
	void Add( PersistentObject ^obj, Actions act );

	void Process( void );
};
_RPL_END