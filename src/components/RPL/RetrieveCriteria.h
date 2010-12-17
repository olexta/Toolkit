/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		RetrieveCriteria.h											*/
/*																			*/
/*	Content:	Definition of RetrieveCriteria class						*/
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
/// This class encapsulates the behavior needed to search scope of objects.
/// </summary><remarks><para>
/// This criterial request means action on persistent mechanism. So, it
/// proceses under transactial control.</para><para>
/// Also, it add Move() routine that you can use to move throught all records.
/// </para></remarks>
public ref class RetrieveCriteria sealed : PersistentCriteria
{
private:
	bool	m_asProxies;
	int		m_pos;

protected:
	virtual void Reset( void ) override;
	virtual void OnPerformComplete( void ) override;

public:
	RetrieveCriteria( String ^type );
	RetrieveCriteria( String ^type, RPL::Where ^where );
	RetrieveCriteria( String ^type, RPL::Where ^where, RPL::OrderBy ^order );

	property bool AsProxies {
		bool get( void );
		void set( bool value );
	}

	bool Move( int count );
};
_RPL_END
