/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Remoting											*/
/*																			*/
/*	Module:		CrossDomainProxy.h											*/
/*																			*/
/*	Content:	Definition of CrossDomainProxy class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "Remoting.h"

using namespace System;
using namespace System::Runtime::Remoting::Proxies;
using namespace System::Runtime::Remoting::Messaging;


_REMOTING_BEGIN
/// <summary>
/// This class represents client side of the cross domain communication.
/// </summary><remarks>
/// The CrossDomainProxy allows to create and then transmit a unique client
/// identifier with every method call providing the means to identify each
/// client on the server's side.
/// </remarks>
generic<typename T>
	where T: MarshalByRefObject
public ref class CrossDomainProxy sealed : RealProxy
{
private:
	String^		const _url;
	String^		const _clientID;

	initonly IMessageSink^	_sink;

public:
	CrossDomainProxy( String ^URL, String ^clientID );

	static operator T( CrossDomainProxy<T> ^proxy );

	virtual IMessage^ Invoke( IMessage ^msg ) override;
};
_REMOTING_END