/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Remoting											*/
/*																			*/
/*	Module:		Remoting.h													*/
/*																			*/
/*	Content:	Common definitions, constants and macroses for this project	*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008 Alexey Tkachuk								*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once


//
// Define error strings
//
#define ERR_CLIENT_ID														\
	"No '" + CLIENT_ID + "' was found in the message context."
#define ERR_MARSHALER_URI													\
	"Marshaler URI cann't be empty string."
#define ERR_SERVICE_URL														\
	"Invalid service URL: '{0}'."
#define ERR_NO_CHANNEL														\
	"No channel was found for the URL '{0}'."
#define ERR_DISPOSE															\
	"ERROR! Object dispose failed '{0}': {1}"


//
// Define namespace macros definition. All public classes
// must be defined in this namespace.
//
#define	_REMOTING				Toolkit::Remoting
#define	_REMOTING_BEGIN			namespace Toolkit{namespace Remoting{
#define	_REMOTING_END			}}

#define	_CROSS_DOMAIN			_REMOTING::CrossDomain
#define _CROSS_DOMAIN_BEGIN		_REMOTING_BEGIN namespace CrossDomain{
#define _CROSS_DOMAIN_END		_REMOTING_END}


//
// Define property name that identifies client request.
//
#define CLIENT_ID	"__ClientID"
