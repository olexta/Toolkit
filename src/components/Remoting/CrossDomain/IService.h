/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Remoting											*/
/*																			*/
/*	Module:		IService.h													*/
/*																			*/
/*	Content:	Definition of CrossDomain::IService interface				*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "..\Remoting.h"

using namespace System;
using namespace System::Runtime::Remoting::Messaging;


_CROSS_DOMAIN_BEGIN
/// <summary>
/// This interface encapsulates	the behavior needed for cross domain
/// communication.
/// </summary>
private interface class IService
{
	IMessage^ Marshal( IMessage ^msg );
};
_CROSS_DOMAIN_END
