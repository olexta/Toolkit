/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentObject.h											*/
/*																			*/
/*	Content:	Definition of CPersistentObject class						*/
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
ref class CPersistentObject;
ref class CPersistentProperty;
ref class CObjectProperties;
ref class CObjectLinks;

/// <sumamry>
/// Provide internal access to the CPersistentObject class.
/// </sumamry><remarks>
/// This is more flexible realisation of a "internal" access modifier. This
/// interface can be used in .NET Remoting.
/// </remarks>
private interface class IIPersistentObject
{
	property CObjectLinks^ Links {
		CObjectLinks^ get( void );
	}
	property CObjectProperties^ Properties {
		CObjectProperties^ get( void );
	}
	property Object^ SyncRoot {
		Object^ get( void );
	}

	void on_change( void );
	void on_change( CPersistentObject ^sender );
	void on_change( CPersistentProperty ^sender,
					Object ^oldValue, Object ^newValue );
	bool on_retrieve( int id, DateTime stamp, String ^name );
	void on_retrieve( IEnumerable<CPersistentObject^> ^links,
					  IEnumerable<CPersistentProperty^> ^props );
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
public ref class CPersistentObject abstract : MarshalByRefObject, IID,
											  ITransactionSupport,
											  IIPersistentObject
{
private:
	enum class STATE{ proxy, filling, full };

	ref struct BACKUP_STRUCT {
		int					_id;
		STATE				_state;
		DateTime			_stamp;
		String				^_name;
		bool				_changed;

		CObjectLinks		^_links;
		CObjectProperties	^_props;
	};

private:
	int					m_id;
	STATE				m_state;
	DateTime			m_stamp;
	String				^m_name;
	bool				m_changed;

	CObjectLinks		^m_links;
	CObjectProperties	^m_props;
	
	Object				^_lock_this;
	Collections::Stack	m_trans_stack;

	void check_state( bool notNew, bool notDelete, bool notProxy );

// IIPersistentObject
private:
	virtual void on_change( void ) sealed =
		IIPersistentObject::on_change;
	virtual void on_change( CPersistentObject ^sender ) sealed = 
		IIPersistentObject::on_change;
	virtual void on_change( CPersistentProperty ^sender,
					Object ^oldValue, Object ^newValue ) sealed =
		IIPersistentObject::on_change;
	virtual bool on_retrieve( int id, DateTime stamp, String ^name ) sealed =
		IIPersistentObject::on_retrieve;
	virtual void on_retrieve( IEnumerable<CPersistentObject^> ^links,
							  IEnumerable<CPersistentProperty^> ^props ) sealed =
		IIPersistentObject::on_retrieve;

// ITransactionSupport
private:
	virtual void TransactionBegin( void ) sealed =
		ITransactionSupport::TransactionBegin;
	virtual void TransactionCommit( void ) sealed =
		ITransactionSupport::TransactionCommit;
	virtual void TransactionRollback( void ) sealed =
		ITransactionSupport::TransactionRollback;

protected:
	CPersistentObject( void );
	CPersistentObject( int id, DateTime stamp, String ^name );
	CPersistentObject( int id, DateTime stamp, String ^name,
					   IEnumerable<CPersistentObject^> ^links,
					   IEnumerable<CPersistentProperty^> ^props);

	property CObjectLinks^ Links {
		virtual CObjectLinks^ get( void ) = IIPersistentObject::Links::get;
	}
	property CObjectProperties^ Properties {
		virtual CObjectProperties^ get( void ) = IIPersistentObject::Properties::get;
	}
	property Object^ SyncRoot {
		virtual Object^ get( void ) = IIPersistentObject::SyncRoot::get;
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
	virtual void OnChange( CPersistentObject^ obj );
	virtual void OnChange( CPersistentProperty^ prop,
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
		virtual void set( String^ value );
	}
	
	virtual void Retrieve( void );
	virtual void Save( void );
	virtual void Delete( void );

	virtual String^ ToString( void ) override;
};
_RPL_END