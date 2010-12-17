**************************************************************************/
/*																			*/
/*	Project:	Toolkit Remoting											*/
/*																			*/
/*	Module:		Service.cpp													*/
/*																			*/
/*	Content:	Definition of CrossDomain::Service class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "..\Remoting.h"
#include "..\CrossDomainMarshaler.h"
#include "IService.h"

using namespace System;
using namespace System::Security::Permissions;
using namespace System::Runtime::Remoting::Proxies;


_CROSS_DOMAIN_BEGIN
/// <summary>
/// The CrossDomain::Service implements the CrossDomain::IService interface
/// method Marshal( IMessage^ msg ). The Message object is forwarded into a
/// custom proxy which finally resolves the call to needed function. 
/// </summary>
private ref class Service: MarshalByRefObject, IService
{
private:
	//
	// This proxy stores real service URI and is used to marshal messages to it.
	//
	ref class Proxy : RealProxy
	{
	private:
		initonly String	^_uri;

	public:
		[PermissionSet(SecurityAction::LinkDemand)]
		Proxy( MarshalByRefObject ^obj );

		[SecurityPermissionAttribute(SecurityAction::LinkDemand,Flags= \
									 SecurityPermissionFlag::Infrastructure)]
		virtual IMessage^ Invoke( IMessage ^msg ) override;
	};

private:
	initonly Proxy^	_proxy;

// CrossDomain::IService
private:
	virtual IMessage^ marshal( IMessage ^msg ) sealed =
		IService::Marshal;

public:
	Service( String ^clientID, CrossDomainMarshaler::SERVICE_FACTORY ^factory );

	[SecurityPermissionAttribute(SecurityAction::Demand,Flags=
								 SecurityPermissionFlag::Infrastructure)]
	virtual Object^ InitializeLifetimeService( void ) override;
};
_CROSS_DOMAIN_END