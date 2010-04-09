/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistenObject.ObjectProperties.h							*/
/*																			*/
/*	Content:	Definition of PersistenObject::ObjectProperties class		*/
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
#include "PersistentProperties.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace Toolkit::Collections;


_RPL_BEGIN
ref class PersistentStream;

/// <summary>
/// Store object properties.
/// </summary><remarks>
/// This class derived from PersistentProperties and was developed
/// to link properties with it's owner object.
/// </remarks>
ref class PersistentObject::
ObjectProperties : PersistentProperties, ITransaction
{
public:
	[Flags]
	enum class STATE {None = 0, New = 1, Deleted = 2, Changed = 4};

private:
	PersistentObject^		const _owner;
	Map<String^, STATE>		^m_log;

	property STATE log_record[String^] {
		STATE get( String ^key );
		void set( String ^key, STATE action );
	}
	bool subscribe_to( ValueBox %value, bool subscribe );
	void on_ps_change( PersistentStream ^sender );
	void fill_by( IEnumerable<KeyValuePair<String^, ValueBox>> ^e );
	void clear( void );

// ITransaction
private:
	value class RESTORE_POINT {
	public:
		PersistentProperties	^_props;
		Map<String^, STATE>		^_log;
	};
	Stack<RESTORE_POINT>	backup;

	virtual void trans_begin( void ) sealed = ITransaction::Begin;
	virtual void trans_commit( void ) sealed = ITransaction::Commit;
	virtual void trans_rollback( void ) sealed = ITransaction::Rollback;

protected:
	virtual void OnClear( void ) override;
	virtual void OnInsert( String ^key, ValueBox value ) override;
	virtual void OnRemove( String ^key, ValueBox value ) override;
	virtual	void OnSet( String ^key, ValueBox value ) override;
	virtual void OnInsertComplete( String ^key, ValueBox value ) override;
	virtual void OnRemoveComplete( String ^key, ValueBox value ) override;
	virtual void OnSetComplete( String ^key, ValueBox value ) override;

public:
	ObjectProperties( PersistentObject ^owner );
	ObjectProperties( PersistentObject ^owner,
					  IEnumerable<KeyValuePair<String^, ValueBox>> ^e );
	~ObjectProperties( void );

	property bool IsChanged {
		bool get( void );
	}

	void Accept( void );
	PersistentProperties^ Get( STATE states );
	void Reload( IEnumerable<KeyValuePair<String^, ValueBox>> ^e );
};
_RPL_END
