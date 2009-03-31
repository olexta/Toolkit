/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Remoting											*/
/*																			*/
/*	Module:		Service.cpp													*/
/*																			*/
/*	Content:	Implementation of CrossDomain::Service class				*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008-2009 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "Service.h"

using namespace System::Runtime::Remoting;
using namespace System::Runtime::Remoting::Channels;
using namespace System::Runtime::Remoting::Lifetime;
using namespace _REMOTING;
using namespace _CROSS_DOMAIN;


//----------------------------------------------------------------------------
//				Toolkit::Remoting::CrossDomain::Service::Proxy
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Creates new instance of the CrossDomain::Service::Proxy class.
//
//-------------------------------------------------------------------
Service::Proxy::Proxy( MarshalByRefObject ^obj )
{
	_uri = RemotingServices::Marshal( obj )->URI;
}


//-------------------------------------------------------------------
//
// Invokes the method that is specified in the provided IMessage on
// the remote object that is represented by the current instance.
//
//-------------------------------------------------------------------
IMessage^ Service::Proxy::Invoke( IMessage ^msg )
{
	msg->Properties["__Uri"] = _uri;
			
	return ChannelServices::SyncDispatchMessage( msg );
}


//----------------------------------------------------------------------------
//					Toolkit::Remoting::CrossDomain::Service
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// CrossDomain::IService implementation.
//
// Marshal message using internal stored proxy (this will marshal
// message to real service).
//
//-------------------------------------------------------------------
IMessage^ Service::marshal( IMessage ^msg )
{
	return _proxy->Invoke( msg );
}


//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the CrossDomain::Service class.
/// </summary>
//-------------------------------------------------------------------
Service::Service( String ^clientID,	\
				  CrossDomainMarshaler::SERVICE_FACTORY ^factory )
{
	// create real service and store proxy to it
	_proxy = gcnew Proxy(factory( clientID ));
}


//-------------------------------------------------------------------
/// <summary>
/// Gives the instance an infinite lifetime by preventing a lease
/// from being created.
/// </summary>
//-------------------------------------------------------------------
Object^ Service::InitializeLifetimeService( void )
{
	return nullptr;
}
