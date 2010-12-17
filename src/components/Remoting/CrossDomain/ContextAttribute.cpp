**************************************************************************/
/*																			*/
/*	Project:	Toolkit Remoting											*/
/*																			*/
/*	Module:		ContextAttribute.cpp										*/
/*																			*/
/*	Content:	Implementation of CrossDomain::Context attribute			*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "MessageSink.h"
#include "ContextAttribute.h"

using namespace _CROSS_DOMAIN;


//----------------------------------------------------------------------------
//		Toolkit::Remoting::CrossDomain::ContextAttribute::ContextProperty
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// IContextProperty::Name::get implementation.
//
// Gets the name of the property under which it will be added to the
// context.
//
//-------------------------------------------------------------------
String^ ContextAttribute:: \
ContextProperty::get_name( void )
{
	return _name;
}


//-------------------------------------------------------------------
//
// IContextProperty::IsNewContextOK implementation.
//
// Returns a Boolean value indicating whether the context property is
// compatible with the new context.
//
//-------------------------------------------------------------------
bool ContextAttribute:: \
ContextProperty::is_new_context_ok( Contexts::Context ^newContext )
{
	return true;
}


//-------------------------------------------------------------------
//
// IContextProperty::Freeze implementation.
//
// Called when the context is frozen.
//
//-------------------------------------------------------------------
void ContextAttribute:: \
ContextProperty::freeze( Contexts::Context ^newContext )
{
	// nothing to do
}


//-------------------------------------------------------------------
//
// IContributeObjectSink::GetObjectSink implementation.
//
// Chains the message sink of the provided server object in front of
// the given sink chain.
//
//-------------------------------------------------------------------
IMessageSink^ ContextAttribute::						   \
ContextProperty::get_object_sink( MarshalByRefObject ^obj, \
								  IMessageSink ^nextSink )
{
	return gcnew MessageSink(obj, nextSink);
}


//----------------------------------------------------------------------------
//				Toolkit::Remoting::CrossDomain::ContextAttribute
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Create instance of the CrossDomain::Context attribute and set
/// it's name to "Interception".
/// </summary>
//-------------------------------------------------------------------
ContextAttribute::ContextAttribute( void ): \
	Contexts::ContextAttribute(_name)
{
	// do nothing
}
        

//-------------------------------------------------------------------
/// <summary>
/// Returns a Boolean value indicating whether the context parameter
/// meets the context attribute's requirements.
/// </summary>
//-------------------------------------------------------------------
bool ContextAttribute::IsContextOK( Contexts::Context ^context, \
									IConstructionCallMessage ^ctor )
{
	// if this is already an intercepting context, it's ok for us
	return (context->GetProperty( _name ) != nullptr);
}


//-------------------------------------------------------------------
/// <summary>
/// Adds the current context property to the given message.
/// </summary>
//-------------------------------------------------------------------
void ContextAttribute::GetPropertiesForNewContext( \
		IConstructionCallMessage ^ctor )
{
	// add the context property which will later create a sink
	ctor->ContextProperties->Add( gcnew ContextProperty() );
}
