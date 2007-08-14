/****************************************************************************/
/*																			*/
/*	Project:	RPL Server													*/
/*																			*/
/*	Module:		CrossDomainService.cpp										*/
/*																			*/
/*	Content:	Definition and implementation of CrossDomainService class	*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/*																			*/
/*	The CrossDomainService implements the ICrossDomainService interface		*/
/*	method Marshal(IMessage^ msg). The Message object is forwarded into a	*/
/*	custom proxy which finally resolves the call to needed function. Class	*/
/*	is presented as template, so you have to specify class for each it will	*/
/*	provide cross domain marshaling. Also, you have to specify life time of	*/
/*	the the marshaler in seconds.											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "ICrossDomainService.h"

using namespace System;
using namespace System::Security::Permissions;
using namespace System::Runtime::Remoting::Proxies;


template<class T, int t>
private ref class CCrossDomainService: T, ICrossDomainService
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
	int			m_timeout;
	Proxy		^m_proxy;

public:
	CCrossDomainService( void );

	virtual IMessage^	Marshal( IMessage ^msg ) = ICrossDomainService::Marshal;
	virtual Object^		InitializeLifetimeService( void ) override;
};



//----------------------------------------------------------------------------
//						CCrossDomainService::Proxy
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Create instance of the CCrossDomainService::Proxy class.
//
//-------------------------------------------------------------------
template<class T, int t>
[PermissionSet(SecurityAction::LinkDemand)]
CCrossDomainService<T, t>::Proxy::Proxy( MarshalByRefObject ^obj )
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
[SecurityPermissionAttribute(SecurityAction::LinkDemand, Flags = SecurityPermissionFlag::Infrastructure)]
IMessage^ CCrossDomainService<T, t>::Proxy::Invoke( IMessage ^msg )
{
	msg->Properties["__Uri"] = m_uri;
			
	return ChannelServices::SyncDispatchMessage( msg );
}


//----------------------------------------------------------------------------
//							CCrossDomainService
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Create instance of the CCrossDomainService class.
//
//-------------------------------------------------------------------
template<class T, int t>
CCrossDomainService<T, t>::CCrossDomainService( void )
{
	m_timeout = t;
	m_proxy = gcnew Proxy( this );
}


//-------------------------------------------------------------------
//
// Obtains a lifetime service object to control the lifetime policy
// for this instance.
//
//-------------------------------------------------------------------
template<class T, int t>
Object^ CCrossDomainService<T, t>::InitializeLifetimeService( void )
{
	ILease^ lease = dynamic_cast<ILease^>
					(MarshalByRefObject::InitializeLifetimeService());

	if ( lease->CurrentState == LeaseState::Initial ) {
		// initialize to specified timeout
		lease->InitialLeaseTime = TimeSpan::FromSeconds( m_timeout );
		lease->RenewOnCallTime = TimeSpan::FromSeconds( m_timeout );
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
IMessage^ CCrossDomainService<T, t>::Marshal( IMessage ^msg )
{
	return m_proxy->Invoke( msg );
}