/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		MessageSink.h												*/
/*																			*/
/*	Content:	Definition of MessageSink class								*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once

using namespace System;
using namespace System::Runtime::Remoting::Messaging;


private ref class MessageSink : IMessageSink
{
private:
	IMessageSink	^m_nextSink;

public:
	MessageSink( IMessageSink ^nextSink );

	virtual IMessage^ SyncProcessMessage( IMessage ^msg );
	virtual IMessageCtrl^ AsyncProcessMessage( IMessage ^msg, 
											   IMessageSink ^replySink );

	virtual property IMessageSink^ NextSink {
		IMessageSink^ get( void );
	}
};
