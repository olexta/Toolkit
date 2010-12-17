/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistenObject.ObjectLinks.h								*/
/*																			*/
/*	Content:	Definition of PersistenObject::ObjectLinks class			*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"
#include "ITransaction.h"
#include "PersistentObject.h"
#include "PersistentObjects.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace Toolkit::Collections;


_RPL_BEGIN
/// <summary>
/// Store object relations.
/// </summary><remarks>
/// This class derived from PersistentObjects and was developed
/// to link relations with it's owner object.
/// </remarks>
ref class PersistentObject::
ObjectLinks : PersistentObjects, ITransaction
{
public:
	[Flags]
	enum class STATE {None = 0, New = 1, Deleted = 2};

private:
	PersistentObject^						const _owner;
	Dictionary<PersistentObject^, STATE>	^m_log;

	property STATE log_record[PersistentObject^] {
		STATE get( PersistentObject ^obj );
		void set( PersistentObject ^obj, STATE action );
	}
	void fill_by( IEnumerable<PersistentObject^> ^e );
	void clear( void );

// ITransaction
private:
	value class RESTORE_POINT {
	public:
		PersistentObjects						^_objs;
		Dictionary<PersistentObject^, STATE>	^_log;
	};
	Stack<RESTORE_POINT>	backup;

	virtual void trans_begin( void ) sealed = ITransaction::Begin;
	virtual void trans_commit( void ) sealed = ITransaction::Commit;
	virtual void trans_rollback( void ) sealed = ITransaction::Rollback;

protected:
	virtual void OnClear( void ) override;
	virtual void OnInsert( PersistentObject ^obj ) override;
	virtual void OnRemove( PersistentObject ^obj ) override;
	virtual void OnInsertComplete( PersistentObject ^obj ) override;
	virtual void OnRemoveComplete( PersistentObject ^obj ) override;

public:
	ObjectLinks( PersistentObject ^owner );
	ObjectLinks( PersistentObject ^owner,
				 IEnumerable<PersistentObject^> ^e );
	~ObjectLinks( void );

	property bool IsChanged {
		bool get( void );
	}

	void Accept( void );
	PersistentObjects^ Get( STATE states );
	void Reload( IEnumerable<PersistentObject^> ^e );
};
_RPL_END
