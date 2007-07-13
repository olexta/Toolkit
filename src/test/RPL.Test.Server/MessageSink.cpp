/****************************************************************************/
/*																			*/
/*	Project:	RPL Server													*/
/*																			*/
/*	Module:		MessageSink.cpp												*/
/*																			*/
/*	Content:	Implementation of CMessageSink class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/*																			*/
/*	Implementing a CCrossDomainMarshaller as a ContextBoundObject requires	*/
/*	me also to implement a CMessageSink object. That offers me the			*/
/*	opportunity to intercept the Message before it enters the point of		*/
/*	reconstruction to the ultimate method call.	This class implements		*/
/*	IMessageSink interface.													*/
/*																			*/
/****************************************************************************/

#include "ICrossDomainService.h"
#include "CrossDomainMarshaller.h"
#include "MessageSink.h"


//----------------------------------------------------------------------------
//								CMessageSink
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Create instance of the CMessageSink class. I store next message
// sink in the sink chain here.
//
//-------------------------------------------------------------------
CMessageSink::CMessageSink( IMessageSink ^nextSink )
{
	m_nextSink = nextSink;
}


//-------------------------------------------------------------------
//
// Synchronously processes the given message. Here is where we must
// intercept redirect the client's method call.
// 
//-------------------------------------------------------------------
IMessage^ CMessageSink::SyncProcessMessage( IMessage ^msg )
{
	if ( msg->Properties["__Uri"] == nullptr ) {

		return m_nextSink->SyncProcessMessage( msg );
	}

	// get client ID from context data
	LogicalCallContext ^callContext = dynamic_cast<LogicalCallContext^> 
										(msg->Properties["__CallContext"]);
	String ^clientID = dynamic_cast<String^> (callContext->GetData("__ClientID"));

	if ( clientID != nullptr ) {
		// get associated service interface
		return CrossDomainMarshaller::Instance->Service[clientID]->Marshal( msg );
	} else {
		// no client ID in message, throw exception
		return gcnew ReturnMessage( gcnew ApplicationException("No __ClientID"), 
									dynamic_cast <IMethodCallMessage^> (msg) );
	}
}


//-------------------------------------------------------------------
//
// Asynchronously processes the given message.
//
//-------------------------------------------------------------------
IMessageCtrl^ CMessageSink::AsyncProcessMessage( IMessage ^msg, 
												 IMessageSink ^replySink )
{
	return m_nextSink->AsyncProcessMessage( msg, replySink );
}


//-------------------------------------------------------------------
//
// Gets the next message sink in the sink chain. 
//
//-------------------------------------------------------------------
IMessageSink^ CMessageSink::NextSink::get( void )
{
	return m_nextSink;
}