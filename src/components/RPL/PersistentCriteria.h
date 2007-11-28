/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentCriteria.h										*/
/*																			*/
/*	Content:	Definition of PersistentCriteria class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"
#include "ITransactionSupport.h"
#include "PersistentObjects.h"

using namespace System;
using namespace System::Collections::Generic;


_RPL_BEGIN
/// <sumamry>
/// Provide internal access to the PersistentCriteria class.
/// </sumamry><remarks>
/// This is more flexible realisation of a "internal" access modifier. This
/// interface can be used in .NET Remoting.
/// </remarks>
private interface class IIPersistentCriteria
{
	void OnPerform( int found, IEnumerable<PersistentObject^> ^objs );
};

/// <summary>
/// This abstract class encapsulates the common behavior needed to search,
/// delete and update scope of persistent objects.
/// </summary><remarks>
/// This is class from which all search classes inherit from. Class derived
/// from PersistentObjects. You can set WHERE and ORDER BY clauses to limit
/// count of objects to	retrieve. Also, you can use InnerQuery property to set
/// additional SQL request for which WHERE and ORDER BY clause will be applied
/// (note, that not all classes maps implement this feature). BottomLimit and
/// CountLimit provide limits for scope of objects represented by recordset.
/// </remarks>
public ref class PersistentCriteria abstract : PersistentObjects,
											   ITransactionSupport,
											   IIPersistentCriteria
{
private:
	typedef ref struct BACKUP_STRUCT_ {
		String		^_type;
		String		^_inner_query;
		String		^_where;
		String		^_order_by;
		int			_bottom;
		int			_count;
		int			_count_found;
		PersistentObjects	^_objs;
	} BACKUP_STRUCT;

private:
	String					^m_type;
	String					^m_innerQuery;
	String					^m_where;
	String					^m_orderBy;
	int						m_countFound;

	Stack<BACKUP_STRUCT^>	m_trans_stack;

// IIPersistentCriteria
private:
	virtual void on_perform( int found,
							 IEnumerable<PersistentObject^> ^objs ) sealed =
		IIPersistentCriteria::OnPerform;

// ITransactionSupport
private:
	virtual void trans_begin( void ) sealed =
		ITransactionSupport::TransactionBegin;
	virtual void trans_commit( void ) sealed =
		ITransactionSupport::TransactionCommit;
	virtual void trans_rollback( void ) sealed =
		ITransactionSupport::TransactionRollback;

protected:
	int		m_bottom;
	int		m_count;

	PersistentCriteria( String ^type );
	explicit PersistentCriteria( const PersistentCriteria %crit );
	
	virtual void ResetResults( void );
	virtual void OnTransactionBegin( void );
	virtual void OnTransactionCommit( void );
	virtual void OnTransactionRollback( void );
	virtual void OnPerformComplete( void );

	virtual void OnRemove( PersistentObject ^obj ) override sealed;
	virtual void OnInsert( PersistentObject ^obj ) override sealed;

public:
	property String^ Type {
		String^ get( void );
	}
	property String^ InnerQuery {
		String^ get( void );
		void set( String ^value );
	}
	property String^ Where {
		String^ get( void );
		void set( String ^value );
	}
	property String^ OrderBy {
		String^ get( void );
		void set( String ^value );
	}
	property int BottomLimit {
		int get( void );
		void set( int value );
	}
	property int CountLimit {
		int get( void );
		void set( int value );
	}
	property int CountFound {
		int get( void );
	}
	property PersistentObject^ default[int] {
		PersistentObject^ get( int index );
	}

	int IndexOf( PersistentObject ^obj );
	void Perform( void );
};
_RPL_END