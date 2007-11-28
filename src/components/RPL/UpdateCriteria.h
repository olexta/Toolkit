/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		UpdateCriteria.h											*/
/*																			*/
/*	Content:	Definition of UpdateCriteria class							*/
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
ref class PersistentProperties;

/// <summary>
/// This class encapsulates the behavior needed to update scope of objects.
/// </summary><remarks>
/// It derived from PersistentCriteria which ncapsulate all properties for
/// building SQL request.To perform this operations all	objects retrieve from
/// storage with it properties and links. Then set new property values, check
/// business logic and store in persistent mechanism. After successfull
/// operation criteria will be filled  by updated objects.
/// </remarks>
public ref class UpdateCriteria sealed : PersistentCriteria
{
private:
	typedef ref struct BACKUP_STRUCT_ {
		PersistentProperties	^_props;
	} BACKUP_STRUCT;

private:
	PersistentProperties	^m_props;

	Stack<BACKUP_STRUCT^>	m_trans_stack;

protected:
	virtual void OnTransactionBegin( void ) override;
	virtual void OnTransactionCommit( void ) override;
	virtual void OnTransactionRollback( void ) override;
	virtual void OnPerformComplete( void ) override;

public:
	UpdateCriteria( String ^type );
	UpdateCriteria( String ^type, String ^sWhere );
	UpdateCriteria( String ^type, String ^sWhere, String ^orderBy );
	explicit UpdateCriteria( const PersistentCriteria %crit );

	property PersistentProperties^ Properties {
		PersistentProperties^ get( void );
	}
};
_RPL_END