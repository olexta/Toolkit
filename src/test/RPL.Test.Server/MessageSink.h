/****************************************************************************/
/*																			*/
/*	Project:	RPL Server													*/
/*																			*/
/*	Module:		MessageSink.h												*/
/*																			*/
/*	Content:	Definition of CMessageSink class							*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once

using namespace System;
using namespace System::Runtime::Remoting::Messaging;


private ref class CMessageSink : IMessageSink
{
private:
	IMessageSink	^m_nextSink;

public:
	CMessageSink( IMessageSink ^nextSink );

	virtual IMessage^ SyncProcessMessage( IMessage ^msg );
	virtual IMessageCtrl^ AsyncProcessMessage( IMessage ^msg, 
											   IMessageSink ^replySink );

	virtual property IMessageSink^ NextSink
	{
		IMessageSink^ get( void );
	}
};