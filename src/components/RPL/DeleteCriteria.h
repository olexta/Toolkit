/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		DeleteCriteria.h											*/
/*																			*/
/*	Content:	Definition of DeleteCriteria class							*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
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
/// </summary><remarks><para>
/// This criterial request means action on persistent mechanism. So, it
/// proceses under transactial control in following steps: all objects retrieve
/// from storage it's properties and links, then delete request will be
/// performed to each object.</para><para>
/// After operation complete, criteria will be filled by deleted objects.
/// </para></remarks>
public ref class DeleteCriteria sealed : PersistentCriteria
{
protected:
	virtual void OnPerformComplete( void ) override;

public:
	DeleteCriteria( String ^type );
	DeleteCriteria( String ^type, RPL::Where ^where );
	DeleteCriteria( String ^type, RPL::Where ^where, RPL::OrderBy ^order );
};
_RPL_END
