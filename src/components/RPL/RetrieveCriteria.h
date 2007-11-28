/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		RetrieveCriteria.h											*/
/*																			*/
/*	Content:	Definition of RetrieveCriteria class						*/
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
/// It derived from PersistentCriteria which ncapsulate all properties for
/// building SQL request. This class add Move() routine that you can use to
///	move throught all records.
/// </remarks>
public ref class RetrieveCriteria sealed : PersistentCriteria
{
private:
	typedef ref struct BACKUP_STRUCT_ {
		bool	_as_proxies;
		int		_pos;
	} BACKUP_STRUCT;

private:
	bool					m_asProxies;
	int						m_pos;

	Stack<BACKUP_STRUCT^>	m_trans_stack;

protected:
	virtual void ResetResults( void ) override;
	virtual void OnTransactionBegin( void ) override;
	virtual void OnTransactionCommit( void ) override;
	virtual void OnTransactionRollback( void ) override;
	virtual void OnPerformComplete( void ) override;

public:
	RetrieveCriteria( String ^type );
	RetrieveCriteria( String ^type, String ^sWhere );
	RetrieveCriteria( String ^type, String ^sWhere, String ^orderBy );
	explicit RetrieveCriteria( const PersistentCriteria %crit );

	property bool AsProxies {
		bool get( void );
		void set( bool value );
	}

	bool Move( int count );
};
_RPL_END
