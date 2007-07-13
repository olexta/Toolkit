/****************************************************************************/
/*																			*/
/*	Project:	RPL Server													*/
/*																			*/
/*	Module:		RPL.Server.cpp												*/
/*																			*/
/*	Content:	This is the main executable file.							*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "CrossDomainService.h"
#include "CrossDomainMarshaller.h"

#using "..\..\..\..\bin\RPL.dll"
#using "..\..\..\..\bin\RPL.Storage.dll"
#using "..\..\..\..\bin\RPL.Test.Objects.dll"

using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::Remoting;
using namespace System::Runtime::Remoting::Channels;
using namespace System::Data::SqlClient;
using namespace RPL;
using namespace RPL::Test;


//----------------------------------------------------------------------------
//					C O M M O N   D E F I N I T I O N S
//----------------------------------------------------------------------------

//
// Define client disconnect timeout
//
#define CLIENT_TIMEOUT		1800
//
// Define server port for comunication
//
#define SERVER_PORT			43667


//----------------------------------------------------------------------------
//								CBroker
//----------------------------------------------------------------------------

//
// Define spetsial class that will implement service
// functionality (define trough CCrossDomainService template)
//
private ref class CBroker : CCrossDomainService<CPersistenceBroker, CLIENT_TIMEOUT>
{
private:
	static void Init( void );

public:
	static void Create( String ^clientID,
						AppDomain^ %domain, ICrossDomainService^ %service );
};


//-------------------------------------------------------------------
//
// Initialize service. After service creation i call this function to
// configure domain properies. This function will be called in
// service context, so i can access to static variables and
// singletone objects.
//
//-------------------------------------------------------------------
void CBroker::Init( void )
{
//=============== Test seting chanel ==============
	// create server sink provider
	BinaryServerFormatterSinkProvider ^srvProv = 
		gcnew BinaryServerFormatterSinkProvider();
	srvProv->TypeFilterLevel = Runtime::Serialization::Formatters::TypeFilterLevel::Full;
	// create client sink provider
	BinaryClientFormatterSinkProvider ^cliProv =
		gcnew BinaryClientFormatterSinkProvider();
	// create properties collection
	Collections::IDictionary ^props = gcnew Collections::Hashtable();
	//props["port"] = 0;
	
	// register TCP chanel
	Tcp::TcpChannel ^chan = gcnew Tcp::TcpChannel( props, cliProv, srvProv );
	ChannelServices::RegisterChannel( chan, false );

	// tcp://localhost:8888/310001/{FE0FD0EB-D975-4c5a-918E-FE5787112726}
	String ^url = Text::RegularExpressions::Regex::Match(
							AppDomain::CurrentDomain->FriendlyName,
							"^tcp://(\\d|\\w|-|_)+:\\d+" )->Value;
	
	RemotingConfiguration::RegisterActivatedClientType( CTestObject::typeid, url );
	RemotingConfiguration::RegisterActivatedClientType( Storage::SqlStream::typeid, url );

//=================================================

	// put initialization code here (this function
	// will be called in per-client context)
	CPersistenceBroker::Instance->Connect( 
		gcnew SqlConnection( "Data Source=CORP;Initial Catalog=LineNBU;Persist Security Info=True;User ID=sa;Password=12345" ),
		gcnew Storage::ODB() );
}


//-------------------------------------------------------------------
//
// Create new instance of service by specified unique client ID. It
// can create instance in new domain or in curent main domain.
//
//-------------------------------------------------------------------
void CBroker::Create( String ^clientID,
					  AppDomain^ %domain, ICrossDomainService^ %service )
{
	// create domain using unique clientID
	domain = AppDomain::CreateDomain( clientID );

	try {
		// create service instance in new created domain
		service = dynamic_cast<CBroker^> (domain->CreateInstanceAndUnwrap(
								Assembly::GetExecutingAssembly()->GetName()->Name,
								CBroker::typeid->ToString() ));

		// initialize broker in created context
		domain->DoCallBack( gcnew CrossAppDomainDelegate( &CBroker::Init ) );
	} catch ( Exception^ ) {

		// free Service, if needed
		delete service;
		// free domain
		AppDomain::Unload( domain );
		// and restore exception
		throw;
	}
}


//----------------------------------------------------------------------------
//						M A I N   F U N C T I O N S
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Configure .NET Remoting subsystem.
//
//-------------------------------------------------------------------
void remoting_config( void )
{
	// create server sink provider
	BinaryServerFormatterSinkProvider ^srvProv = 
		gcnew BinaryServerFormatterSinkProvider();
	srvProv->TypeFilterLevel = Runtime::Serialization::Formatters::TypeFilterLevel::Full;
	// create client sink provider
	BinaryClientFormatterSinkProvider ^cliProv =
		gcnew BinaryClientFormatterSinkProvider();
	// create properties collection
	Collections::IDictionary ^props = gcnew Collections::Hashtable();
	props["port"] = SERVER_PORT;
	
	// register TCP chanel
	Tcp::TcpChannel ^chan = gcnew Tcp::TcpChannel( props, cliProv, srvProv );
	ChannelServices::RegisterChannel( chan, false );
}


//-------------------------------------------------------------------
//
// Main application function. I configure .NET remoting and
// CrossDomainMarshaler.
//
//-------------------------------------------------------------------
int main(array<System::String ^> ^args)
{
	try	{
		// configure .NET remoting
		remoting_config();

		// configure cross domain marshaler
		CrossDomainMarshaller::Instance->Factory = gcnew FACTORY(&CBroker::Create);
		CrossDomainMarshaller::Instance->Marshal( "RPL.Server.rem" );

		// and wait for user reques to unload
		Console::WriteLine("Press ENTER to exit...");
		Console::ReadLine();
		
		// free cross domain marshaler (i need to call this
		// routine to correctly dispose all client services)
		CrossDomainMarshaller::Instance->Free();
	} catch ( Exception ^e ) {
		
		// write error message
		Console::WriteLine( e );
		Console::ReadLine();
	}

	return 0;
}