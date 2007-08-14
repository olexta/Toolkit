/****************************************************************************/
/*																			*/
/*	Project:	RPL Server													*/
/*																			*/
/*	Module:		ICrossDomainService.h										*/
/*																			*/
/*	Content:	Definition of ICrossDomainService interface					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/*																			*/
/*	This interface encapsulates	the behavior needed for cross domain		*/
/*	communication. Of course, object must be derived from MBR also.			*/
/*																			*/
/****************************************************************************/

#pragma once

using namespace System;
using namespace System::Runtime::Remoting::Messaging;


public interface class ICrossDomainService
{
	IMessage^ Marshal( IMessage ^msg );
};