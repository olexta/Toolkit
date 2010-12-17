**************************************************************************/
/*																			*/
/*	Project:	Toolkit Remoting											*/
/*																			*/
/*	Module:		CrossDomainMarshaler.cpp									*/
/*																			*/
/*	Content:	Implementation of CrossDomainMarshaler class				*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "CrossDomain\Service.h"
#include "CrossDomainMarshaler.h"

using namespace System::Reflection;
using namespace _REMOTING;


// Define lock macroses
#define ENTER_(lock)	try { Monitor::Enter(								\
							safe_cast<Collections::ICollection^>(			\
							lock )->SyncRoot);
#define EXIT_(lock)		} finally { Monitor::Exit(							\
							safe_cast<Collections::ICollection^>(			\
							lock )->SyncRoot);}


//----------------------------------------------------------------------------
//				Toolkit::CrossDomainMarshaler::ServiceSlot
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// This function is called when service lifetime expired.
//
//-------------------------------------------------------------------
void CrossDomainMarshaler::ServiceSlot::on_timer( Object ^state )
{ENTER_(state)

	// unload service
	delete this;

	// remove this instance from parent collection
	safe_cast<Collections::IDictionary^>( state )->Remove( _clientID );

EXIT_(state)}


//-------------------------------------------------------------------
//
// Creates new instance of the ServiceSlot class.
//
// This class incapsulates service management and is used to manage
// service lifetime.
//
//-------------------------------------------------------------------
CrossDomainMarshaler::													   \
ServiceSlot::ServiceSlot( String ^clientID, CrossDomainMarshaler ^owner ): \
	_clientID(clientID), _owner(owner), m_disposed(false)
{
	// create domain using unique clientID
	m_domain = AppDomain::CreateDomain( _clientID ); 

	try {
		// initialize contructor parameters for CrossDomain::Service class
		array<Object^>	^params = gcnew array<Object^>(2);
		params[0] = _clientID;
		params[1] = _owner->_factory;
		// create service instance in the new created domain
		m_service = dynamic_cast<CrossDomain::IService^>(
					m_domain->CreateInstanceAndUnwrap(
						Assembly::GetExecutingAssembly()->GetName()->Name,
						CrossDomain::Service::typeid->ToString(),
						false, BindingFlags::Default, nullptr, params,
						nullptr, nullptr, nullptr ) );

		// create timer for lifetime monitoring but in suspend mode
		_timer = gcnew Timer(
					gcnew TimerCallback(this, &ServiceSlot::on_timer),
					%_owner->m_dict,
					TimeSpan::FromMilliseconds( Timeout::Infinite ),
					TimeSpan::FromMilliseconds( Timeout::Infinite ));
	} catch( Exception^ ) {
		// unload domain
		AppDomain::Unload( m_domain );
		// and restore exception
		throw;
	}
};


//-------------------------------------------------------------------
//
// Class disposer.
//
// Dispose service instance and unload appropriated domain.
//
//-------------------------------------------------------------------
CrossDomainMarshaler::ServiceSlot::~ServiceSlot( void )
{
	if( !m_disposed ) {
		// dispose lifetime timer
		delete _timer;
		// service disposer can raise exception
		try{
			// dispose service
			delete m_service;
		} catch( Exception ^e ) {
			// output exception info
			Diagnostics::Trace::WriteLine( String::Format( 
			ERR_DISPOSE, m_service->GetType(), e->Message ) );
		} finally {
			// unload domain created for service
			AppDomain::Unload( m_domain );
			// prevent from future cals
			m_disposed = true;
		}
	}
}


//-------------------------------------------------------------------
//
// Gets CrossDomain::IService interface from apropriated service.
//
//-------------------------------------------------------------------
CrossDomain::IService^ CrossDomainMarshaler::ServiceSlot::Service::get( void )
{
	// check for the disposed object
	if( m_disposed ) throw gcnew ObjectDisposedException(
		this->GetType()->ToString());

	// this is service request, so reset lifetime monitoring timer
	_timer->Change( _owner->_timeout,
					TimeSpan::FromMilliseconds( Timeout::Infinite ) );

	return m_service;
}


//----------------------------------------------------------------------------
//							CrossDomainMarshaler
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Gets apropriated service for the client with specified ID.
//
// If no service was found then new instance will be created.
//
//-------------------------------------------------------------------
CrossDomain::IService^ CrossDomainMarshaler::default::get( String ^clientID )
{ENTER_(%m_dict)

	// check for the disposed object
	if( m_disposed ) throw gcnew ObjectDisposedException(
		this->GetType()->ToString());

	ServiceSlot	^slot = nullptr;
	// create an service instance per client only once
	if( !m_dict.TryGetValue( clientID, slot ) ) {
		// create new slot for the client 
		slot = gcnew ServiceSlot(clientID, this);
		// and save it in the map
		m_dict[clientID] = slot;
	}

	 return slot->Service;

EXIT_(%m_dict) }


//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the CrossDomainMarshaler class.
/// </summary>
/// <param name="URI">The specified URI with which to initialize the
/// new CrossDomainMarshaler.</param>
/// <param name="factory">Factory delegate that is used to create
/// per client service.</param>
/// <param name="timeout"><para>
/// The amount of time by which a client domain will be unloaded in
/// lack of service requests.</para><para>
/// Specify negative one (-1) milliseconds to disable lifetime
/// monitoring.
/// </para></param>
//------------------------------------------------------------------
CrossDomainMarshaler::CrossDomainMarshaler( String ^URI,			  \
										    SERVICE_FACTORY ^factory, \
											TimeSpan timeout ) :	  \
	_factory(factory), _timeout(timeout), m_disposed(false)
{
	// chek for the valid URI
	if( URI == nullptr ) throw gcnew ArgumentNullException("URI");
	if( URI == "" ) throw gcnew ArgumentException(ERR_MARSHALER_URI, "URI");
	// check for the valid factory
	if( factory == nullptr ) throw gcnew ArgumentNullException("factory");

	// register this object with the specified URI
	RemotingServices::Marshal( this, URI );
}


//-------------------------------------------------------------------
/// <summary>
/// Class disposer.
/// </summary><remarks>
/// Clear marshaler object by deleting all services.
/// </remarks>
//-------------------------------------------------------------------
CrossDomainMarshaler::~CrossDomainMarshaler( void )
{ENTER_(%m_dict)

	if( !m_disposed ) {
		// dispose all services
		for each( ServiceSlot ^slot in m_dict.Values ) delete slot;
		// and clear dictionary
		m_dict.Clear();

		// prevent from future cals
		m_disposed = true;
	}

EXIT_(%m_dict)}


//-------------------------------------------------------------------
/// <summary>
/// Gives the instance an infinite lifetime by preventing a lease
/// from being created.
/// </summary>
//-------------------------------------------------------------------
Object^ CrossDomainMarshaler::InitializeLifetimeService( void )
{
	return nullptr;
}
