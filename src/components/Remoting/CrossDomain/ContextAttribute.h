/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Remoting											*/
/*																			*/
/*	Module:		ContextAttribute.h											*/
/*																			*/
/*	Content:	Definition of CrossDomain::Context attribute				*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "..\Remoting.h"

using namespace System;
using namespace System::Runtime::Remoting;
using namespace System::Runtime::Remoting::Messaging;
using namespace System::Runtime::Remoting::Activation;


_CROSS_DOMAIN_BEGIN
/// <summary>
/// Define attribute that will provide new context creating for the new object
/// request.
/// </summary><remarks>
/// This allows me intercept object creation on the server.
/// </remarks>
[AttributeUsage(AttributeTargets::Class)]
private ref class ContextAttribute : Contexts::ContextAttribute
{
private:
	//
	// Implements context property. The main goal of this property is adding a 
	// new sink to the requested object chain.
	//
	ref class ContextProperty : Contexts::IContextProperty, 
								Contexts::IContributeObjectSink
	{
	private:
		static String^	const _name = "CrossDomain";

	// IContextProperty
	private:
		virtual String^ get_name( void ) sealed =
			Contexts::IContextProperty::Name::get;
		virtual bool is_new_context_ok( Contexts::Context ^newContext ) sealed =
			Contexts::IContextProperty::IsNewContextOK;
		virtual void freeze( Contexts::Context ^newContext ) sealed =
			Contexts::IContextProperty::Freeze;

	// IContributeObjectSink
	private:
		virtual IMessageSink^ get_object_sink( MarshalByRefObject ^obj,
											   IMessageSink ^nextSink ) sealed =
			Contexts::IContributeObjectSink::GetObjectSink;
	};

private:
	static String^	const _name = "Interception";

public:
	ContextAttribute( void );

	virtual bool IsContextOK( Contexts::Context ^context, 
							  IConstructionCallMessage ^ctor ) override;
	virtual void GetPropertiesForNewContext( 
					IConstructionCallMessage ^ctor ) override;
};
_CROSS_DOMAIN_END
