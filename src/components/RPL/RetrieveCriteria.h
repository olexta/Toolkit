/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		RetrieveCriteria.h											*/
/*																			*/
/*	Content:	Definition of CRetrieveCriteria class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"
#include "PersistentCriteria.h"

using namespace System;


_RPL_BEGIN
/// <summary>
/// This class encapsulates the behavior needed to search scope of objects.
/// </summary><remarks>
/// It derived from CPersistentCriteria which ncapsulate all properties for
/// building SQL request. This class add Move() routine that you can use to
///	move throught all records.
/// </remarks>
public ref class CRetrieveCriteria sealed : CPersistentCriteria
{
private:
	typedef ref struct _backup_struct {
		bool	_as_proxies;
		int		_pos;
	} BackupStruct;

private:
	bool	m_asProxies;
	int		m_pos;

	Collections::Stack	m_trans_stack;

protected:
	virtual void ResetResults( void ) override;
	virtual void OnTransactionBegin( void ) override;
	virtual void OnTransactionCommit( void ) override;
	virtual void OnTransactionRollback( void ) override;
	virtual void OnPerformComplete( void ) override;

public:
	CRetrieveCriteria( String ^type );
	CRetrieveCriteria( String ^type, String ^sWhere );
	CRetrieveCriteria( String ^type, String ^sWhere, String ^orderBy );
	explicit CRetrieveCriteria( const CPersistentCriteria %crit );

	property bool AsProxies {
		bool get( void );
		void set( bool value );
	}

	bool Move( int count );
};
_RPL_END
