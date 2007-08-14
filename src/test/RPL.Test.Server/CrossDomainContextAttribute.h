/****************************************************************************/
/*																			*/
/*	Project:	RPL Server													*/
/*																			*/
/*	Module:		CrossDomainContextAttribute.h								*/
/*																			*/
/*	Content:	Definition of CrossDomainContext attribute					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once

using namespace System;
using namespace System::Runtime::Remoting::Contexts;
using namespace System::Runtime::Remoting::Messaging;
using namespace System::Runtime::Remoting::Activation;


[AttributeUsage(AttributeTargets::Class)]
private ref class CrossDomainContextAttribute : ContextAttribute
{
private:
	ref class CCrossDomainContextProperty : IContextProperty, IContributeObjectSink
	{
	public:
		virtual bool IsNewContextOK( Context ^newContext );
		virtual void Freeze( Context ^newContext );
		virtual IMessageSink^ GetObjectSink( MarshalByRefObject ^obj, 
											 IMessageSink ^nextSink );

		virtual property String^ Name {
			String^ get( void );
		}
	};

public:
	CrossDomainContextAttribute( void );

	virtual bool IsContextOK( Context ^context, IConstructionCallMessage ^ctor ) override;
	virtual void GetPropertiesForNewContext( IConstructionCallMessage ^ctor ) override;
};