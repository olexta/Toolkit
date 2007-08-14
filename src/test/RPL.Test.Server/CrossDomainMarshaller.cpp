/****************************************************************************/
/*																			*/
/*	Project:	RPL Server													*/
/*																			*/
/*	Module:		CrossDomainMarshaller.cpp									*/
/*																			*/
/*	Content:	Implementation of CrossDomainMarshaller class				*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
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
#define ENTER(lock)		try { Monitor::Enter( lock );
#define EXIT(lock)		} finally { Monitor::Exit( lock ); }


//----------------------------------------------------------------------------
//					CrossDomainMarshaller::CServiceSponsor
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Create instance of the CServiceSponsor class. This class is used
// as Sponsor to manage service lifetime. If service instance was
// created in non default domain, then must be ability to unload this
// domain and delete service slot after client disconnect. All client
// reqests will be called in it's own created domain and
// AppDomain::Unload will be failed. To avoid this limitation i
// create sponsor object in same as CrossDomainMarshaller domain and
// on Renew perform all cleaner actions.
//-------------------------------------------------------------------
CrossDomainMarshaller::CServiceSponsor::CServiceSponsor( String ^clientID ): \
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
[SecurityPermissionAttribute(SecurityAction::LinkDemand,Flags=SecurityPermissionFlag::Infrastructure)]
TimeSpan CrossDomainMarshaller::CServiceSponsor::Renewal( ILease ^lease )
{
	// unregister this instance from sponsorship
	lease->Unregister( this );

	// free service (all work will be done by marshaler)
	CrossDomainMarshaller::Instance->Free( m_clientID );

	// return request to immediate destroy object
	return TimeSpan::FromSeconds(0);
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
CrossDomainMarshaller::CrossDomainMarshaller( void ): m_factory(nullptr)
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
	AppDomain			^domain = nullptr;
	ICrossDomainService	^service = nullptr;


	// create new service instance for client
	m_factory( clientID, domain, service );

	// store in slot per-client created domain and service
	slot->Domain = domain;
	slot->Service = service;

	if ( domain != nullptr ) {
		// create sponsor object
		slot->Sponsor = gcnew CServiceSponsor(clientID);
		// register service object with the server's lease manager.
		ILease ^lease = dynamic_cast<ILease^> (RemotingServices::GetLifetimeService(
							dynamic_cast<MarshalByRefObject^> (service) ));
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
	// call service disposer to free resources
	delete slot->Service;
	// unload domain created for service
	if ( slot->Domain != nullptr ) AppDomain::Unload( slot->Domain );
	// dispose sponsor object
	delete slot->Sponsor;
}


//-------------------------------------------------------------------
//
// Gets handle to the singleton instance of the CrossDomainMarshaller
// class.
//
//-------------------------------------------------------------------
CrossDomainMarshaller^ CrossDomainMarshaller::Instance::get( void )
{
	return m_instance;
}


//-------------------------------------------------------------------
//
// Sets delegate that will be used to process object creation request.
//
//-------------------------------------------------------------------
void CrossDomainMarshaller::Factory::set( FACTORY ^factory )
{
	if ( factory == nullptr ) throw gcnew ArgumentNullException( "factory" );

	m_factory = factory;
}


//-------------------------------------------------------------------
//
// Gets apropriated domain for client by specified ID. If no client
// with this ID was found return nullptr.
//
//-------------------------------------------------------------------
AppDomain^ CrossDomainMarshaller::Domain::get( String ^clientID )
{
	if ( !m_dict.ContainsKey( clientID ) ) return nullptr;

	return m_dict[clientID]->Domain;
}


//-------------------------------------------------------------------
//
// Gets apropriated service for client by specified ID. If no service
// was found then create new using factory delegate.
//
//-------------------------------------------------------------------
ICrossDomainService^ CrossDomainMarshaller::Service::get( String ^clientID )
{
	// check for object state
	if ( m_factory == nullptr ) throw gcnew InvalidOperationException(
						"Object creation factory not specified!" );

	// create an service instance per client only once
	if ( !m_dict.ContainsKey( clientID ) ) {
		// this call must be thread-safe, so i use
		// SyncRoot to lock dictionary
		ENTER( ((Collections::ICollection^) %m_dict)->SyncRoot )
			// create new slot for client 
			m_dict[clientID] = gcnew ServiceSlot();
			// prevent memory leaking
			try {
				// create new service instance for client
				init( clientID, m_dict[clientID] );
			} catch ( Exception^ ) {
				// service was not created correctly,
				// so i delete client's slot only
				m_dict.Remove( clientID );
				// and return exception
				throw;
			}
		EXIT( ((Collections::ICollection^) %m_dict)->SyncRoot );
	}

	return m_dict[clientID]->Service;
}


//-------------------------------------------------------------------
//
// Clear marshaler object by deleting all client services.
//
//-------------------------------------------------------------------
void CrossDomainMarshaller::Free( void )
{
	ENTER( ((Collections::ICollection^) %m_dict)->SyncRoot )
		// pass through all entries in dictionary
		// and free services
		for each ( String ^key in m_dict.Keys ) free( key, m_dict[key] );
		// clear dictionary
		m_dict.Clear();
	EXIT( ((Collections::ICollection^) %m_dict)->SyncRoot )
}


//-------------------------------------------------------------------
//
// Free service instance for specified client, also delete service
// slot (each client session have it's own unique ID, so dictionary
// can store a lot of slots and to increase perfomance i have to
// clear it).
//
//-------------------------------------------------------------------
void CrossDomainMarshaller::Free( String ^clientID )
{
	ENTER( ((Collections::ICollection^) %m_dict)->SyncRoot )
		// check for client's slot exists
		if ( m_dict.ContainsKey( clientID ) ) {
			// free client's service
			free( clientID, m_dict[clientID] );
			// and delete slot
			m_dict.Remove( clientID );
		}
	EXIT( ((Collections::ICollection^) %m_dict)->SyncRoot )
}


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