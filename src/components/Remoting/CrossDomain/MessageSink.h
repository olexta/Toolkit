/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Remoting											*/
/*																			*/
/*	Module:		MessageSink.h												*/
/*																			*/
/*	Content:	Definition of CrossDomain::MessageSink class				*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "..\Remoting.h"

using namespace System;
using namespace System::Runtime::Remoting::Messaging;


_CROSS_DOMAIN_BEGIN
/// <summary>
/// This class implements IMessageSink interface.
/// </summary><remarks>
/// Implementing a CrossDomainMarshaller as a ContextBoundObject requires me
/// also to implement a MessageSink object. That offers me the opportunity to
/// intercept the Message before it enters the point of reconstruction to the
/// ultimate method call.
/// </remarks>
private ref class MessageSink : IMessageSink
{
private:
	MarshalByRefObject^	const _object;
	IMessageSink^		const _nextSink;

// IMessageSink
private:
	virtual IMessage^ sync_process( IMessage ^msg ) sealed = 
		IMessageSink::SyncProcessMessage;
	virtual IMessageCtrl^ async_process( IMessage ^msg, 
										 IMessageSink ^replySink ) sealed =
		IMessageSink::AsyncProcessMessage;
	virtual IMessageSink^ get_next_sink( void ) sealed = 
		IMessageSink::NextSink::get;

public:
	MessageSink( MarshalByRefObject ^obj, IMessageSink ^nextSink );
};
_CROSS_DOMAIN_END
