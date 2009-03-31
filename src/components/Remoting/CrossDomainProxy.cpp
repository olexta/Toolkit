/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Remoting											*/
/*																			*/
/*	Module:		CrossDomainProxy.cpp										*/
/*																			*/
/*	Content:	Implementation of CrossDomainProxy class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008-2009 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "CrossDomainProxy.h"

using namespace System::Runtime::Remoting;
using namespace System::Runtime::Remoting::Channels;
using namespace _REMOTING;


//----------------------------------------------------------------------------
//						Toolkit::CrossDomainProxy<T>
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Initializes a new instance of the CrossDomainProxy class that
/// represents a remote object of the specified generic type.
/// </summary>
/// <param name="URL">The service URL to which the proxy will deliver
/// calls.</param>
/// <param name="clientID">String that will identify client on the
/// server's side.</param>
//-------------------------------------------------------------------
generic<typename T>
CrossDomainProxy<T>::CrossDomainProxy( String ^URL, String ^clientID ) : \
	RealProxy(T::typeid), _url(URL), _clientID(clientID) 
{
	// check for the valid service url
	if( URL == nullptr ) throw gcnew ArgumentNullException("URL");
	if( URL == "" ) throw gcnew ArgumentException(String::Format(
					ERR_SERVICE_URL, URL ));
	// check for the clientID valid value
	if( clientID == nullptr ) throw gcnew ArgumentNullException("clientID");

	// extract protocol name from the URL
	int pos = _url->IndexOf( ":" );
	if( !(pos > 0) ) throw gcnew ArgumentException(String::Format(
					 ERR_SERVICE_URL, _url ));
	String	^prot = URL->Substring( 0, pos )->ToLower();

	// pass through registered chanels
	for each( IChannel ^channel in ChannelServices::RegisteredChannels ) {
		// and try to find chanel, that can be used to
		// create message sink for given url
		IChannelSender	^sender = dynamic_cast<IChannelSender^>( channel );
		if( sender != nullptr ) {
			// check for the messaging protocol
			if( sender->ChannelName == prot ) {
				// try to create message sink for specified uri
				String	^uri = nullptr;
				_sink = sender->CreateMessageSink( _url, nullptr, uri );
				// all right, can return
				if( _sink != nullptr ) break;
			}
		}
	}
	// no channel was found
	if ( _sink == nullptr ) throw gcnew RemotingException(String::Format(
							ERR_NO_CHANNEL, _url ));	
}


//-------------------------------------------------------------------
/// <summary>
/// Implicit cast operator from CrossDomainProxy to the generic proxy
/// type.
/// </summary>
//-------------------------------------------------------------------
generic<typename T>
CrossDomainProxy<T>::operator T( CrossDomainProxy<T> ^proxy )
{
	// return null reference, if proxy is null reference
	// (this is because base of T is MarshalByRefObject)
	if( proxy == nullptr ) return T();

	return static_cast<T>( proxy->GetTransparentProxy() );
}

	
//-------------------------------------------------------------------
/// <summary>
/// Invokes the method that is specified in the provided IMessage on
/// the remote object that is represented by the current instance.
/// </summary>
/// <param name="msg"> A IMessage that contains a IDictionary of
/// information about the method call.</param>
/// <remarks>
/// In this implementation transmits, a unique client identifier with
/// every method call.
/// </remarks>
//-------------------------------------------------------------------
generic<typename T>
IMessage^ CrossDomainProxy<T>::Invoke( IMessage ^msg )
{
	// set message destination
	msg->Properties["__Uri"] = _url;

	// pass the client's unique identifier as part of the call context
	LogicalCallContext	^context = static_cast<LogicalCallContext^>(
								   msg->Properties["__CallContext"] );
	context->SetData( CLIENT_ID, _clientID );

	// and process message with the channel's sink
	return _sink->SyncProcessMessage( msg );
}
