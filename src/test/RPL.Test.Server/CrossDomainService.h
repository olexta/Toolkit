/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		CrossDomainService.cpp										*/
/*																			*/
/*	Content:	Definition and implementation of CrossDomainService class	*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/*																			*/
/*	The CrossDomainService implements the ICrossDomainService interface		*/
/*	method Marshal(IMessage^ msg). The Message object is forwarded into a	*/
/*	custom proxy which finally resolves the call to needed function. Class	*/
/*	is presented as template, so you have to specify class for each it		*/
/*	will provide cross domain marshaling. Also, you have to specify life	*/
/*	time of the the marshaler in seconds.									*/
/*																			*/
/****************************************************************************/

#pragma once
#include "ICrossDomainService.h"

using namespace System;
using namespace System::Security::Permissions;
using namespace System::Runtime::Remoting::Proxies;


template<class T, int t>
private ref class CrossDomainService: T, ICrossDomainService
{
private:
	ref class Proxy : RealProxy
	{
	private:
		String	^m_uri;

	public:
		Proxy( MarshalByRefObject ^obj );

		virtual IMessage^ Invoke( IMessage ^msg ) override;
	};

private:
	Proxy		^m_proxy;

// ICrossDomaainService
private:
	virtual IMessage^ Marshal( IMessage ^msg ) sealed =
		ICrossDomainService::Marshal;

public:
	CrossDomainService( void );

	virtual Object^ InitializeLifetimeService( void ) override;
};



//----------------------------------------------------------------------------
//						CrossDomainService::Proxy
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Create instance of the CrossDomainService::Proxy class.
//
//-------------------------------------------------------------------
template<class T, int t>
[PermissionSet(SecurityAction::LinkDemand)]
CrossDomainService<T, t>::Proxy::Proxy( MarshalByRefObject ^obj )
{
	m_uri = RemotingServices::Marshal( obj )->URI;
}


//-------------------------------------------------------------------
//
// Invokes the method that is specified in the provided IMessage on
// the remote object that is represented by the current instance.
//
//-------------------------------------------------------------------
template<class T, int t>
[SecurityPermissionAttribute(SecurityAction::LinkDemand,\
							 Flags=SecurityPermissionFlag::Infrastructure)]
IMessage^ CrossDomainService<T, t>::Proxy::Invoke( IMessage ^msg )
{
	msg->Properties["__Uri"] = m_uri;
			
	return ChannelServices::SyncDispatchMessage( msg );
}


//----------------------------------------------------------------------------
//							CrossDomainService
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Create instance of the CrossDomainService class.
//
//-------------------------------------------------------------------
template<class T, int t>
CrossDomainService<T, t>::CrossDomainService( void )
{
	m_proxy = gcnew Proxy(this);
}


//-------------------------------------------------------------------
//
// Obtains a lifetime service object to control the lifetime policy
// for this instance.
//
//-------------------------------------------------------------------
template<class T, int t>
Object^ CrossDomainService<T, t>::InitializeLifetimeService( void )
{
	ILease^	lease = dynamic_cast<ILease^>( 
					MarshalByRefObject::InitializeLifetimeService() ) ;

	if( lease->CurrentState == LeaseState::Initial ) {
		// initialize to specified timeout
		lease->InitialLeaseTime = TimeSpan::FromSeconds( t );
		lease->RenewOnCallTime = TimeSpan::FromSeconds( t );
		// sponsor locate in this process, so left defult value
	}
	return lease;
}


//-------------------------------------------------------------------
//
// Marshal message using internal stored proxy (this will marshal
// message to real service).
//
//-------------------------------------------------------------------
template<class T, int t>
IMessage^ CrossDomainService<T, t>::Marshal( IMessage ^msg )
{
	return m_proxy->Invoke( msg );
}
