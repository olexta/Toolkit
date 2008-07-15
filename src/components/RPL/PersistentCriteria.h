/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentCriteria.h										*/
/*																			*/
/*	Content:	Definition of PersistentCriteria class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"
#include "PersistentObjects.h"

using namespace System;
using namespace System::Collections::Generic;


_RPL_BEGIN
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
public ref class PersistentCriteria abstract : PersistentObjects
{
protected:
	String^		const _type;

	String		^m_innerQuery;
	String		^m_where;
	String		^m_orderBy;
	int			m_countFound;
	int			m_bottom;
	int			m_count;

	PersistentCriteria( String ^type );
		
	virtual void Reset( void );
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