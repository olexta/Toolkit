/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentObject.h											*/
/*																			*/
/*	Content:	Definition of PersistentObject class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#using <System.Data.dll>

#include "RPL.h"
#include "ITransaction.h"
#include "ValueBox.h"

using namespace System;
using namespace System::Data;
using namespace System::Collections::Generic;


_RPL_BEGIN
ref class PersistentObjects;
ref class PersistentProperties;

/// <summary>
/// This class encapsulates the behavior needed to make a single object
/// persistent and is the class from which all classes in your business domain
/// inherit from.
/// </summary><remarks>
/// All inherited classes must provide same constructors (except default).
/// Methods Retrive, Save and Delete can be overriden, but you must call parent
/// routines to store object in persistence mechanism. There are two types of
/// object: full and proxy. Full object is a normal object, having properties
/// and links to other objects, in contrast to proxy that have type, id and
/// name only. Full object can be constructed from proxy by calling Retrive.
/// But proxy also accept all operations: retrieve, save and delete.
/// </remarks>
public ref class PersistentObject abstract : MarshalByRefObject,
											 ITransaction
{
private:
	typedef enum class STATE{ Proxy, Full };

	ref class ObjectProperties;
	ref class ObjectLinks;

private:
	initonly ObjectLinks		^_links;
	initonly ObjectProperties	^_props;

	int					m_id;
	DateTime			m_stamp;
	String				^m_name;
	STATE				m_state;
	bool				m_changed;

	void check_state( bool notNew, bool notDelete, bool notProxy );

	void on_change( String ^oldName, String ^newName );
	void on_change( PersistentObject ^obj );
	void on_change( String ^prop, ValueBox oldValue, ValueBox newValue );

// ITransaction
private:
	value class RESTORE_POINT {
	public:
		int					_id;
		DateTime			_stamp;
		String				^_name;
		STATE				_state;
		bool				_changed;
	};
	Stack<RESTORE_POINT>	backup;

	virtual void trans_begin( void ) sealed = ITransaction::Begin;
	virtual void trans_commit( void ) sealed = ITransaction::Commit;
	virtual void trans_rollback( void ) sealed = ITransaction::Rollback;

protected:
	static DataSet^ ProcessSQL( String ^sql, ... array<Object^> ^params );

	PersistentObject( void );
	PersistentObject( int id, DateTime stamp, String ^name );
	~PersistentObject( void );

	property PersistentObjects^ Links {
		PersistentObjects^ get( void );
	}
	property PersistentProperties^ Properties {
		PersistentProperties^ get( void );
	}

	virtual void OnTransactionBegin( void );
	virtual void OnTransactionCommit( void );
	virtual void OnTransactionRollback( void );
	virtual void OnRetrieve( void );
	virtual void OnRetrieveComplete( void );
	virtual void OnSave( void );
	virtual void OnSaveComplete( void );
	virtual void OnDelete( void );
	virtual void OnDeleteComplete( void );
	virtual void OnChange( String ^oldName, String ^newName );
	virtual void OnChange( PersistentObject ^obj );
	virtual void OnChange( String ^prop,
						   ValueBox oldValue, ValueBox newValue );

public:
	property int ID {
		int get( void );
	}
	property bool IsProxy {
		bool get( void );
	}
	property DateTime Stamp {
		DateTime get( void );
	}
	property bool IsChanged {
		bool get( void );
	}
	property String^ Type {
		virtual String^ get( void ) = 0;
	}
	property String^ Name {
		virtual String^ get( void );
		virtual void set( String ^value );
	}

	virtual void Retrieve( bool upgrade );
	virtual void Save( void );
	virtual void Delete( void );

	virtual int GetHashCode( void ) override;
	virtual String^ ToString( void ) override;
};
_RPL_END
