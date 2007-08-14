/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentCriteria.h										*/
/*																			*/
/*	Content:	Definition of CPersistentCriteria class						*/
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
/// Provide internal access to the CPersistentCriteria class.
/// </sumamry><remarks>
/// This is more flexible realisation of a "internal" access modifier. This
/// interface can be used in .NET Remoting.
/// </remarks>
private interface class IIPersistentCriteria
{
	void on_perform( int found, IEnumerable<CPersistentObject^> ^objs );
};

/// <summary>
/// This abstract class encapsulates the common behavior needed to search,
/// delete and update scope of persistent objects.
/// </summary><remarks>
/// This is class from which all search classes inherit from. Class derived
/// from CPersistentObjects. You can set WHERE and ORDER BY clauses to limit
/// count of objects to	retrieve. Also, you can use InnerQuery property to set
/// additional SQL request for which WHERE and ORDER BY clause will be applied
/// (note, that not all classes maps implement this feature). BottomLimit and
/// CountLimit provide limits for scope of objects represented by recordset.
/// </remarks>
public ref class CPersistentCriteria abstract : CPersistentObjects,
												ITransactionSupport,
												IIPersistentCriteria
{
private:
	ref struct BACKUP_STRUCT {
		String		^_type;
		String		^_inner_query;
		String		^_where;
		String		^_order_by;
		int			_bottom;
		int			_count;
		int			_count_found;
		CPersistentObjects	^_objs;
	};

private:
	String	^m_type;
	String	^m_innerQuery;
	String	^m_where;
	String	^m_orderBy;
	int		m_countFound;

	Collections::Stack	m_trans_stack;

// IIPersistentCriteria
private:
	virtual void on_perform( int found,
							 IEnumerable<CPersistentObject^> ^objs ) sealed =
		IIPersistentCriteria::on_perform;

// ITransactionSupport
private:
	virtual void TransactionBegin( void ) sealed =
		ITransactionSupport::TransactionBegin;
	virtual void TransactionCommit( void ) sealed =
		ITransactionSupport::TransactionCommit;
	virtual void TransactionRollback( void ) sealed =
		ITransactionSupport::TransactionRollback;

protected:
	int		m_bottom;
	int		m_count;

	CPersistentCriteria( String ^type );
	explicit CPersistentCriteria( const CPersistentCriteria %crit );
	
	virtual void ResetResults( void );
	virtual void OnTransactionBegin( void );
	virtual void OnTransactionCommit( void );
	virtual void OnTransactionRollback( void );
	virtual void OnPerformComplete( void );

	virtual void OnClear( void ) override sealed;
	virtual void OnRemove( CPersistentObject ^obj ) override sealed;
	virtual void OnInsert( CPersistentObject ^obj ) override sealed;

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
	property CPersistentObject^ default[int] {
		CPersistentObject^ get( int index );
	}

	int IndexOf( CPersistentObject ^obj );
	void Perform( void );
};
_RPL_END