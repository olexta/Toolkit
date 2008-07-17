/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		CrossDomainMarshaller.h										*/
/*																			*/
/*	Content:	Definition of CrossDomainMarshaller class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "CrossDomainContextAttribute.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::Remoting::Lifetime;


interface class ICrossDomainService;
delegate void FACTORY( String ^clientID, AppDomain^ %domain,
					   ICrossDomainService^ %service );

[CrossDomainContext]
private ref class CrossDomainMarshaller : ContextBoundObject
{
private:
	ref class ServiceSponsor : MarshalByRefObject, ISponsor
	{
	private:
		String	^m_clientID;

	// ISponsor
	private:
		virtual TimeSpan Renewal( ILease ^lease ) sealed =
			ISponsor::Renewal;

	public:
		ServiceSponsor( String ^clientID );
	};

private:
	ref class ServiceSlot : MarshalByRefObject
	{
	public:
		property AppDomain^				Domain;
		property ICrossDomainService^	Service;
		property ISponsor^				Sponsor;
	};
	
private:
	static CrossDomainMarshaller^		const s_instance = 
											gcnew CrossDomainMarshaller();

	FACTORY								^m_factory;
	Dictionary<String^, ServiceSlot^>	m_dict;

	CrossDomainMarshaller( void );

	void init( String ^clientID, ServiceSlot ^slot );
	void free( String ^clientID, ServiceSlot ^slot );

public:
	static property CrossDomainMarshaller^ Instance {
		CrossDomainMarshaller^ get( void );
	}
	property FACTORY^ Factory {
		void set( FACTORY ^factory );
	}
	property AppDomain^ Domain[String^] {
		AppDomain^ get( String ^clientID );
	};
	property ICrossDomainService^ Service[String^] {
		ICrossDomainService^ get( String ^clientID );
	};
	
	void Free( void );
	void Free( String ^clientID );
	
	virtual void Marshal( String ^URI );
	virtual Object^ InitializeLifetimeService( void ) override;
};
