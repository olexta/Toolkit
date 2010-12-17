/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Remoting											*/
/*																			*/
/*	Module:		MessageSink.cpp												*/
/*																			*/
/*	Content:	Implementation of CrossDomain::MessageSink class			*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "..\CrossDomainMarshaler.h"
#include "IService.h"
#include "MessageSink.h"

using namespace _CROSS_DOMAIN;


//----------------------------------------------------------------------------
//				Toolkit::Remoting::CrossDomain::MessageSink
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// IMessageSink::SyncProcessMessage implementation.
//
// Synchronously processes the given message. Here is where we must
// intercept redirect the client's method call.
// 
//-------------------------------------------------------------------
IMessage^ MessageSink::sync_process( IMessage ^msg )
{
	if( msg->Properties["__Uri"] == nullptr ) {
		// redirect message to next sink in the chain
		return _nextSink->SyncProcessMessage( msg );
	}

	// get client ID from context data
	LogicalCallContext	^callContext = dynamic_cast<LogicalCallContext^>( 
									   msg->Properties["__CallContext"] );
	String				^clientID = dynamic_cast<String^>(
									callContext->GetData( CLIENT_ID ) );

	if( clientID != nullptr ) {
		// get marshaler instance
		CrossDomainMarshaler	^cdm = dynamic_cast<CrossDomainMarshaler^>(
									   _object );
		// and process message with associated service interface
		return cdm[clientID]->Marshal( msg );
	} else {
		// no client ID in message, throw exception
		return gcnew ReturnMessage(gcnew ArgumentException(ERR_CLIENT_ID, "msg"), 
								   dynamic_cast<IMethodCallMessage^>( msg ));
	}
}


//-------------------------------------------------------------------
//
// IMessageSink::AsyncProcessMessage implementation.
//
// Asynchronously processes the given message.
//
//-------------------------------------------------------------------
IMessageCtrl^ MessageSink::async_process( IMessage ^msg, 
										  IMessageSink ^replySink )
{
	return _nextSink->AsyncProcessMessage( msg, replySink );
}


//-------------------------------------------------------------------
//
// IMessageSink::NextSink::get implementation.
//
// Gets the next message sink in the sink chain. 
//
//-------------------------------------------------------------------
IMessageSink^ MessageSink::get_next_sink( void )
{
	return _nextSink;
}


//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the MessageSink class.
/// </summary><remarks>
/// I store next message sink in the sink chain here.
/// </remarks>
//-------------------------------------------------------------------
MessageSink::MessageSink( MarshalByRefObject ^obj, IMessageSink ^nextSink ) : \
	_object(obj), _nextSink(nextSink)
{
	// nothing to do
}