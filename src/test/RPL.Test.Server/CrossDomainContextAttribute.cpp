/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		CrossDomainContextAttribute.cpp								*/
/*																			*/
/*	Content:	Implementation of CrossDomainContext attribute				*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/*																			*/
/*  Define the attribute that will provide new context creating for the new	*/
/*	object request. This allow me intercept object creation on the server.	*/
/*																			*/
/****************************************************************************/

#include "MessageSink.h"
#include "CrossDomainContextAttribute.h"


//----------------------------------------------------------------------------
//			CrossDomainContextAttribute::CCrossDomainContextProperty
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Returns a Boolean value indicating whether the context property is
// compatible with the new context.
//
//-------------------------------------------------------------------
bool CrossDomainContextAttribute::CCrossDomainContextProperty::IsNewContextOK( \
		Context ^newContext )
{
	return true;
}


//-------------------------------------------------------------------
//
// Called when the context is frozen.
//
//-------------------------------------------------------------------
void CrossDomainContextAttribute::CCrossDomainContextProperty::Freeze(
		Context ^newContext )
{
	// nothing to do
}


//-------------------------------------------------------------------
//
// Gets the name of the property under which it will be added to the
// context.
//
//-------------------------------------------------------------------
String^ CrossDomainContextAttribute::CCrossDomainContextProperty::Name::get( void )
{
	return "CrossDomain";
}


//-------------------------------------------------------------------
//
// Chains the message sink of the provided server object in front of
// the given sink chain.
//
//-------------------------------------------------------------------
IMessageSink^ CrossDomainContextAttribute::CCrossDomainContextProperty::GetObjectSink( \
		MarshalByRefObject ^obj, IMessageSink ^nextSink )
{
	return gcnew MessageSink( nextSink );
}


//----------------------------------------------------------------------------
//							CrossDomainContextAttribute
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Create instance of the CrossDomainContext attribute and set it's
// name to "Interception".
//
//-------------------------------------------------------------------
CrossDomainContextAttribute::CrossDomainContextAttribute(): \
	ContextAttribute("Interception")
{
	// do nothing
};
        

//-------------------------------------------------------------------
//
// Returns a Boolean value indicating whether the context parameter
// meets the context attribute's requirements.
//
//-------------------------------------------------------------------
bool CrossDomainContextAttribute::IsContextOK( Context ^context, \
											   IConstructionCallMessage ^ctor )
{
	// if this is already an intercepting context, it's ok for us
	return (context->GetProperty("Interception") != nullptr);
}


//-------------------------------------------------------------------
//
// Adds the current context property to the given message.
//
//-------------------------------------------------------------------
void CrossDomainContextAttribute::GetPropertiesForNewContext( \
		IConstructionCallMessage ^ctor )
{
	// add the context property which will later create a sink
	ctor->ContextProperties->Add( gcnew CCrossDomainContextProperty() );
}