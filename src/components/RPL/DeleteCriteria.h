/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		DeleteCriteria.h											*/
/*																			*/
/*	Content:	Definition of DeleteCriteria class							*/
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
/// This class encapsulates the behavior needed to delete scope of objects.
/// </summary><remarks>
/// It derived from PersistentCriteria which incapsulate all properties
/// for building SQL request. To perform this operations all objects retrieve
/// from storage with it properties and links. Then check business logic and
/// delete objects from storage. After completed opertation criteria will be
/// filled by deleted objects.
/// </remarks>
public ref class DeleteCriteria sealed : PersistentCriteria
{
protected:
	virtual void OnPerformComplete( void ) override;

public:
	DeleteCriteria( String ^type );
	DeleteCriteria( String ^type, String ^sWhere );
	DeleteCriteria( String ^type, String ^sWhere, String ^orderBy );
	explicit DeleteCriteria( const PersistentCriteria %crit );
};
_RPL_END