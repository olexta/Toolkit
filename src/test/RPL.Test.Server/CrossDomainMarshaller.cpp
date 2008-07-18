/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		CrossDomainMarshaller.cpp									*/
/*																			*/
/*	Content:	Implementation of CrossDomainMarshaller class				*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/*																			*/
/*	The clients can never directly connect to the actual service objects	*/
/*	that are instantiated in separate AppDomains. The object they will		*/
/*	connect to must be some sort of a cross domain marshaller that can		*/
/*	intercept a method call and forward it to the appropriate AppDomain. A	*/
/*	CrossDomainMarshaller that is a ContextBoundObject provide this feature	*/
/*	and must be installed as an interceptor. It intercepts all messages		*/
/*	send by all clients and redirects it to apropriated services.			*/
/*																			*/
/****************************************************************************/

#include "ICrossDomainService.h"
#include "CrossDomainMarshaller.h"

using namespace System::Threading;
using namespace System::Runtime::Remoting;
using namespace System::Security::Permissions;


// Define lock macroses
#define ENTER_(lock)	try { Monitor::Enter(								\
							static_cast<Collections::ICollection^>(			\
							lock )->SyncRoot);
#define EXIT_(lock)		} finally { Monitor::Exit(							\
							static_cast<Collections::ICollection^>(			\
							lock )->SyncRoot);}


//----------------------------------------------------------------------------
//					CrossDomainMarshaller::ServiceSponsor
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Create instance of the ServiceSponsor class. This class is used
// as Sponsor to manage service lifetime. If service instance was
// created in non default domain, then must be ability to unload this
// domain and delete service slot after client disconnect. All client
// reqests will be called in it's own created domain and
// AppDomain::Unload will be failed. To avoid this limitation i
// create sponsor object in same as CrossDomainMarshaller domain and
// on Renew perform all cleaner actions.
//-------------------------------------------------------------------
CrossDomainMarshaller::								\
ServiceSponsor::ServiceSponsor( String ^clientID ): \
	m_clientID(clientID)
{
	// do nothing
};


//-------------------------------------------------------------------
//
// Requests a sponsoring client to renew the lease for the specified
// object.
//
//-------------------------------------------------------------------
[SecurityPermission(SecurityAction::LinkDemand,
					Flags=SecurityPermissionFlag::Infrastructure)]
TimeSpan CrossDomainMarshaller:: \
ServiceSponsor::Renewal( ILease ^lease )
{
	// unregister this instance from sponsorship
	lease->Unregister( this );

	// free service (all work will be done by marshaler)
	CrossDomainMarshaller::Instance->Free( m_clientID );

	// return request to immediate destroy object
	return TimeSpan::FromSeconds( 0 );
};


//----------------------------------------------------------------------------
//							CrossDomainMarshaller
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Default class constructor. I must specify it to provide singleton
// pattern.
//
//-------------------------------------------------------------------
CrossDomainMarshaller::CrossDomainMarshaller( void ) : \
	m_factory(nullptr)
{
	// do nothing
}


//-------------------------------------------------------------------
//
// Initialize service instance for specified service slot.
//
//-------------------------------------------------------------------
void CrossDomainMarshaller::init( String ^clientID, ServiceSlot ^slot )
{
	// check for initialized factory 
	if( m_factory == nullptr ) throw gcnew InvalidOperationException(
		"Object creation factory is not specified!" );

	AppDomain			^domain = nullptr;
	ICrossDomainService	^service = nullptr;

	// create new service instance for client
	m_factory( clientID, domain, service );

	// store in slot per-client created domain and service
	slot->Domain = domain;
	slot->Service = service;

	if( domain != nullptr ) {
		// create sponsor object
		slot->Sponsor = gcnew ServiceSponsor(clientID);
		// register service object with the server's lease manager.
		ILease ^lease = dynamic_cast<ILease^>(
						RemotingServices::GetLifetimeService(
							dynamic_cast<MarshalByRefObject^>( service ) ) );
		lease->Register( slot->Sponsor );
	}
}


//-------------------------------------------------------------------
//
// Free service instance stored in specified client's slot.
//
//-------------------------------------------------------------------
void CrossDomainMarshaller::free( String ^clientID, ServiceSlot ^slot )
{
	try {
		// call service disposer to free resources
		delete slot->Service;
	} catch( Exception ^e ) {
		// output exception info
		Diagnostics::Debug::WriteLine( String::Format( 
			"Object dispose failed '{0}': {1}",
			slot->Service->GetType(), e->Message ) );
	} finally {
		// unload domain created for service
		if( slot->Domain != nullptr ) AppDomain::Unload( slot->Domain );
	}
}


//-------------------------------------------------------------------
//
// Gets handle to the singleton instance of the CrossDomainMarshaller
// class.
//
//-------------------------------------------------------------------
CrossDomainMarshaller^ CrossDomainMarshaller::Instance::get( void )
{
	return s_instance;
}


//-------------------------------------------------------------------
//
// Sets delegate that will be used to process object creation request.
//
//-------------------------------------------------------------------
void CrossDomainMarshaller::Factory::set( FACTORY ^factory )
{
	if( factory == nullptr ) throw gcnew ArgumentNullException("factory");

	m_factory = factory;
}


//-------------------------------------------------------------------
//
// Gets apropriated domain for client by specified ID. If no client
// with this ID was found return nullptr.
//
//-------------------------------------------------------------------
AppDomain^ CrossDomainMarshaller::Domain::get( String ^clientID )
{ENTER_(%m_dict)

	ServiceSlot		^slot = nullptr;
	if( m_dict.TryGetValue( clientID, slot ) ) {
		// return client's domain
		return slot->Domain;
	}
	return nullptr;

EXIT_(%m_dict)}


//-------------------------------------------------------------------
//
// Gets apropriated service for client by specified ID. If no service
// was found then create new using factory delegate.
//
//-------------------------------------------------------------------
ICrossDomainService^ CrossDomainMarshaller::Service::get( String ^clientID )
{ENTER_(%m_dict)

	// create an service instance per client only once
	if( !m_dict.ContainsKey( clientID ) ) {
		// create new slot for client 
		ServiceSlot	^slot = gcnew ServiceSlot();
		// create new service instance for client
		init( clientID, slot );
		// save new slot in the dictionary
		m_dict[clientID] = slot;
	}
	return m_dict[clientID]->Service;

EXIT_(%m_dict) }


//-------------------------------------------------------------------
//
// Clear marshaler object by deleting all client services.
//
//-------------------------------------------------------------------
void CrossDomainMarshaller::Free( void )
{ENTER_(%m_dict)

	// pass through all entries in dictionary
	for each( KeyValuePair<String^, ServiceSlot^> pair in m_dict ) {
		// and free services
		free( pair.Key, pair.Value );
	}
	// clear dictionary
	m_dict.Clear();

EXIT_(%m_dict)};


//-------------------------------------------------------------------
//
// Free service instance for specified client, also delete service
// slot (each client session have it's own unique ID, so dictionary
// can store a lot of slots and to increase perfomance i have to
// clear it).
//
//-------------------------------------------------------------------
void CrossDomainMarshaller::Free( String ^clientID )
{ENTER_(%m_dict)

	ServiceSlot		^slot = nullptr;
	// check for client's slot exists
	if( m_dict.TryGetValue( clientID, slot ) ) {
		// free client's service
		free( clientID, slot );
		// and delete slot
		m_dict.Remove( clientID );
	}

EXIT_(%m_dict)}


//-------------------------------------------------------------------
//
// Converts this object into an instance of the ObjRef class with the
// specified URI. 
//
//-------------------------------------------------------------------
void CrossDomainMarshaller::Marshal( String ^URI )
{
	RemotingServices::Marshal( this, URI );
}
	

//-------------------------------------------------------------------
//
// Gives the instance an infinite lifetime by preventing a lease from
// being created.
//
//-------------------------------------------------------------------
Object^ CrossDomainMarshaller::InitializeLifetimeService( void )
{
	return nullptr;
}
