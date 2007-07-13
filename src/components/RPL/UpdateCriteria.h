/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		UpdateCriteria.h											*/
/*																			*/
/*	Content:	Definition of CUpdateCriteria class							*/
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
ref class CPersistentProperties;

/// <summary>
/// This class encapsulates the behavior needed to update scope of objects.
/// </summary><remarks>
/// It derived from CPersistentCriteria which ncapsulate all properties for
/// building SQL request.To perform this operations all	objects retrieve from
/// storage with it properties and links. Then set new property values, check
/// business logic and store in persistent mechanism. After successfull
/// operation criteria will be filled  by updated objects.
/// </remarks>
public ref class CUpdateCriteria sealed : CPersistentCriteria
{
private:
	typedef ref struct _backup_struct {
		CPersistentProperties	^_props;
	} BackupStruct;

private:
	CPersistentProperties	^m_props;

	Collections::Stack		m_trans_stack;

protected:
	virtual void OnTransactionBegin( void ) override;
	virtual void OnTransactionCommit( void ) override;
	virtual void OnTransactionRollback( void ) override;
	virtual void OnPerformComplete( void ) override;

public:
	CUpdateCriteria( String ^type );
	CUpdateCriteria( String ^type, String ^sWhere );
	CUpdateCriteria( String ^type, String ^sWhere, String ^orderBy );
	explicit CUpdateCriteria( const CPersistentCriteria %crit );

	property CPersistentProperties^ Properties {
		CPersistentProperties^ get( void );
	}
};
_RPL_END