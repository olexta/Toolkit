/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		RPL.Test.Server.cpp											*/
/*																			*/
/*	Content:	This is the main executable file.							*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#using "..\..\..\..\bin\Toolkit.RPL.dll"
#using "..\..\..\..\bin\Toolkit.RPL.Storage.dll"

#include "CrossDomainService.h"
#include "CrossDomainMarshaller.h"

using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::Remoting;
using namespace System::Runtime::Remoting::Channels;
using namespace Toolkit::RPL::Factories;
using namespace Toolkit::RPL::Storage;


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
#define SERVER_PORT			12345

//
// Connection string to SQL database
//
#define CNN_STR				"Data Source=.;									\
							 Initial Catalog=Toolkit.RPL.Storage.ODB;		\
							 Integrated Security=SSPI;						\
							 Persist Security Info=False"


//----------------------------------------------------------------------------
//									Broker
//----------------------------------------------------------------------------

//
// Define special class that will implement service
// functionality (define trough CCrossDomainService template)
//
private ref class Broker : CrossDomainService<PersistenceBroker, CLIENT_TIMEOUT>
{
private:
	static void Init( void );

public:
	static void Create( String ^clientID, \
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
void Broker::Init( void )
{
	// put initialization code here (this function
	// will be called in per-client context)
	PersistenceBroker::Connect( gcnew ODB(CNN_STR) );
}


//-------------------------------------------------------------------
//
// Create new instance of service by specified unique client ID. It
// can create instance in new domain or in curent main domain.
//
//-------------------------------------------------------------------
void Broker::Create( String ^clientID,									 \
					 AppDomain^ %domain, ICrossDomainService^ %service )
{
	// create domain using unique clientID
	domain = AppDomain::CreateDomain( clientID );

	try {
		// create service instance in new created domain
		service = dynamic_cast<Broker^>(
					domain->CreateInstanceAndUnwrap(
						Assembly::GetExecutingAssembly()->GetName()->Name,
						Broker::typeid->ToString() ) );

		// initialize broker in created context
		domain->DoCallBack( gcnew CrossAppDomainDelegate(&Broker::Init) );
	} catch( Exception^ ) {
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
	ChannelServices::RegisterChannel( gcnew Tcp::TcpServerChannel(SERVER_PORT),
									  false );
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
		CrossDomainMarshaller::Instance->Factory = gcnew FACTORY(&Broker::Create);
		CrossDomainMarshaller::Instance->Marshal( "Toolkit.RPL.Test.Server.rem" );

		// and wait for user reques to unload
		Console::WriteLine( "Press ENTER to exit..." );
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
