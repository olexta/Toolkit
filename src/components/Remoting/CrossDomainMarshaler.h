/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Remoting											*/
/*																			*/
/*	Module:		CrossDomainMarshaler.h										*/
/*																			*/
/*	Content:	Definition of CrossDomainMarshaler class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "Remoting.h"
#include "CrossDomain\IService.h"
#include "CrossDomain\ContextAttribute.h"

using namespace System;
using namespace System::Threading;
using namespace System::Collections::Generic;
using namespace System::Security::Permissions;
using namespace System::Runtime::Remoting::Lifetime;


_REMOTING_BEGIN
/// <summary>
/// Implements cross domain marshaller that intercepts all messages send by all
/// clients and redirects it to apropriated services.
/// </summary><remarks>
/// The clients can never directly connect to the actual service objects that
/// are instantiated in separate AppDomains. The object they will connect to
/// must be some sort of a cross domain marshaller that can intercept a method
/// call and forward it to the appropriate AppDomain. A CrossDomainMarshaler
/// that is a ContextBoundObject provide this feature and must be installed as
/// an interceptor.
/// </remarks>
[CrossDomain::Context]
public ref class CrossDomainMarshaler sealed : ContextBoundObject
{
public:
	/// <summary>
	/// Represents the factory method that is used by CrossDomainMarshaler to
	/// create per client service.
	/// </summary>
	/// <param name="clientID">Specify unique client name for wich new service
	/// has been created.</param>
	/// <remarks>
	/// This function is called in per client domain.
	/// </remarks>
	delegate MarshalByRefObject^ SERVICE_FACTORY( String ^clientID );

private:
	//
	// Incapsulates all service depend functionality.
	//
	ref class ServiceSlot
	{
	private:
		CrossDomainMarshaler^	const _owner;
		String^					const _clientID;
		initonly Timer^			_timer;

		AppDomain				^m_domain;
		CrossDomain::IService	^m_service;
		bool					m_disposed;

		void on_timer( Object ^state );

	public:
		ServiceSlot( String ^clientID, CrossDomainMarshaler ^owner );
		~ServiceSlot( void );

		property CrossDomain::IService^ Service {
			CrossDomain::IService^ get( void );
		}
	};

private:
	TimeSpan			const _timeout;
	SERVICE_FACTORY^	const _factory;

	Dictionary<String^, ServiceSlot^>	m_dict;
	bool								m_disposed;

internal:
	property CrossDomain::IService^ default[String^] {
		CrossDomain::IService^ get( String ^clientID );
	};

public:
	CrossDomainMarshaler( String ^URI, SERVICE_FACTORY ^factory,
						  TimeSpan timeout );
	~CrossDomainMarshaler( void );

	[SecurityPermissionAttribute(SecurityAction::Demand,Flags=
								 SecurityPermissionFlag::Infrastructure)]
	virtual Object^ InitializeLifetimeService( void ) override;
};
_REMOTING_END
