/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentObject.h											*/
/*																			*/
/*	Content:	Definition of PersistentObject class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"
#include "IID.h"
#include "ITransactionSupport.h"

using namespace System;
using namespace System::Collections::Generic;


_RPL_BEGIN
ref class PersistentObject;
ref class PersistentProperty;
ref class ObjectProperties;
ref class ObjectLinks;

/// <sumamry>
/// Provide internal access to the PersistentObject class.
/// </sumamry><remarks>
/// This is more flexible realisation of a "internal" access modifier. This
/// interface can be used in .NET Remoting.
/// </remarks>
private interface class IIPersistentObject
{
	property ObjectLinks^ Links {
		ObjectLinks^ get( void );
	}
	property ObjectProperties^ Properties {
		ObjectProperties^ get( void );
	}

	void OnChange( void );
	void OnChange( PersistentObject ^sender );
	void OnChange( PersistentProperty ^sender,
				   Object ^oldValue, Object ^newValue );
	bool OnRetrieve( int id, DateTime stamp, String ^name );
	void OnRetrieve( IEnumerable<PersistentObject^> ^links,
					 IEnumerable<PersistentProperty^> ^props );
};

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
/// Call Save for proxy object update name and timestamp in persistence
/// mechanism.
/// </remarks>
public ref class PersistentObject abstract : MarshalByRefObject, IID,
											 ITransactionSupport,
											 IIPersistentObject
{
private:
	typedef enum class STATE{ proxy, filling, full };

	typedef ref struct BACKUP_STRUCT_ {
		int					_id;
		STATE				_state;
		DateTime			_stamp;
		String				^_name;
		bool				_changed;

		ObjectLinks			^_links;
		ObjectProperties	^_props;
	} BACKUP_STRUCT;

private:
	int						m_id;
	STATE					m_state;
	DateTime				m_stamp;
	String					^m_name;
	bool					m_changed;

	ObjectLinks				^m_links;
	ObjectProperties		^m_props;
	
	Stack<BACKUP_STRUCT^>	m_trans_stack;

	void check_state( bool notNew, bool notDelete, bool notProxy );

// IIPersistentObject
private:
	virtual void on_change( void ) sealed =
		IIPersistentObject::OnChange;
	virtual void on_change( PersistentObject ^sender ) sealed = 
		IIPersistentObject::OnChange;
	virtual void on_change( PersistentProperty ^sender,
					Object ^oldValue, Object ^newValue ) sealed =
		IIPersistentObject::OnChange;
	virtual bool on_retrieve( int id, DateTime stamp, String ^name ) sealed =
		IIPersistentObject::OnRetrieve;
	virtual void on_retrieve( IEnumerable<PersistentObject^> ^links,
							  IEnumerable<PersistentProperty^> ^props ) sealed =
		IIPersistentObject::OnRetrieve;

// ITransactionSupport
private:
	virtual void trans_begin( void ) sealed =
		ITransactionSupport::TransactionBegin;
	virtual void trans_commit( void ) sealed =
		ITransactionSupport::TransactionCommit;
	virtual void trans_rollback( void ) sealed =
		ITransactionSupport::TransactionRollback;

protected:
	PersistentObject( void );
	PersistentObject( int id, DateTime stamp, String ^name );
	PersistentObject( int id, DateTime stamp, String ^name,
					  IEnumerable<PersistentObject^> ^links,
					  IEnumerable<PersistentProperty^> ^props );

	property ObjectLinks^ Links {
		virtual ObjectLinks^ get( void ) = IIPersistentObject::Links::get;
	}
	property ObjectProperties^ Properties {
		virtual ObjectProperties^ get( void ) = IIPersistentObject::Properties::get;
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
	virtual void OnChange( void );
	virtual void OnChange( PersistentObject ^obj );
	virtual void OnChange( PersistentProperty ^prop,
						   Object ^oldValue, Object ^newValue );

public:
	operator String^( void );

	property int ID {
		virtual int get( void ) sealed;
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
	
	virtual void Retrieve( void );
	virtual void Save( void );
	virtual void Delete( void );

	virtual String^ ToString( void ) override;
};
_RPL_END