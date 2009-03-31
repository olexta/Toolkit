/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentTransaction.h										*/
/*																			*/
/*	Content:	Definition of PersistentTransaction class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2009 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"

using namespace System;
using namespace System::Collections::Generic;


_RPL_BEGIN
interface class ITransaction;
ref class PersistentObject;

/// <summary>
/// This class encapsulates the behavior that is needed to use transactions.
/// </summary><remarks><para>
/// Only you need, this is add object or collection of objects with asked
/// action to transaction instance (use "Add" method) and perform operation
/// by calling Process().</para><para>
/// If transaction will be not succeded, then some error (from persistence
/// storage) will be raised and all changes will be rolled back.
/// </para></remarks>
public ref class PersistentTransaction sealed : MarshalByRefObject
{
public:
	typedef enum class ACTION { None, Retrieve, Upgrade, Save, Delete };

private:
	value class Task
	{
	private:
		PersistentObject	^m_obj;
		ACTION				m_act;

	public:
		Task( PersistentObject ^obj, ACTION act );

		void Perform( void );
	};

private:
	static Stack<ITransaction^>^	s_stack = nullptr;

	Queue<Task>^	const _tasks;

public:
	// TODO: понять как будет работать даное решение при тонком клиенте
	// (объект PersistentObjects находится на сервере + мы добавляем объект
	// MarshalByRef, который находится на том-же сервере: вопрос, не будут-ли
	// запросы от PersistentObjects на объект проходить через клиент)
	PersistentTransaction( void );
	PersistentTransaction( PersistentObject ^obj, ACTION action );
	PersistentTransaction( IEnumerable<PersistentObject^> ^objs,
						   ACTION action );

	void Add( PersistentObject ^obj, ACTION action );
	void Add( IEnumerable<PersistentObject^> ^objs, ACTION action );

	void Process( void );
};
_RPL_END
