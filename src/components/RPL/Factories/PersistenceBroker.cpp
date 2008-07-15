		// TODO: сделать статический диспозе
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistenceBroker.cpp										*/
/*																			*/
/*	Content:	Implementation of PersistenceBroker class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "..\PersistentObject.h"
#include "IBrokerFactory.h"
#include "IObjectFactory.h"
#include "PersistenceBroker.h"

using namespace _RPL;
using namespace _RPL::Factories;


//-----------------------------------------------------------------------------
//					C O M M O N   D E C L A R A T I O N S
//-----------------------------------------------------------------------------

//
// Clear thread sleep timeout. Default value is 0.512 sec.
//
#define SLEEP_TIMEOUT		0x00000200

//
// Default timeout to clear cache inaccessible
// weak references. Default value is 600 sec.
//
#define CLEAR_TIMEOUT		0x000927C0


//----------------------------------------------------------------------------
//						PersistenceBroker::BrokerCache
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Thread function that provide clearing cache functionality.
//
// This function removes inaccessible objects only.
//
//-------------------------------------------------------------------
void PersistenceBroker::		  \
BrokerCache::thread_clean( void )
{
	// initialize value to current time
	DateTime time = DateTime::Now;
			
	try {
		// check for thread must be terminated
		while( !m_disposed ) {
			// sleep for specified time
			Thread::Sleep( SLEEP_TIMEOUT );
			// if time spent enought
			if ( time.AddSeconds( CLEAR_TIMEOUT ) < DateTime::Now ) {
				// save time of clearing
				time = DateTime::Now;
				
				ENTER_WRITE(_lock)
				// remove inaccessible weak references
				for each( String ^key in m_cache.Keys ) {
					// check for null references
					if( m_cache[key]->Target == nullptr ) m_cache.Remove( key );
				}
				EXIT_WRITE(_lock)
			}
		}
	} catch( Exception ^e ) {
		// output error message
		dbgprint( "[ERROR]: " + e->Message );
	}
}


//-------------------------------------------------------------------
//
// Build string from object's Type and ID for using as key in the
// map.
//
//-------------------------------------------------------------------
String^	PersistenceBroker::				 \
BrokerCache::key( String ^type, int id )
{
	if( type == nullptr ) throw gcnew ArgumentNullException("type");
	
	return (type + "_" + id.ToString());
}


//-------------------------------------------------------------------
//
// Create new cache instance.
//
//-------------------------------------------------------------------
PersistenceBroker::									   \
BrokerCache::BrokerCache( void ):					   \
	_lock(gcnew ReaderWriterLock()), m_disposed(false)
{
	// create thread instance to clear current
	// cache from inaccessible weak references
	(gcnew Thread(
		gcnew ThreadStart(this, &BrokerCache::thread_clean))
	)->Start();
}


//-------------------------------------------------------------------
//
// Class disposer.
//
// Has to use it to clear internal storage and stop clearing thread
// (in other case this object will never be destroyed because of
// object-thread cross references).
//
//-------------------------------------------------------------------
PersistenceBroker::				  \
BrokerCache::~BrokerCache( void )
{
	// check for disposer was not called
	if( !m_disposed ) {
		// prevent future cache usage
		m_disposed = true;
		
		ENTER_WRITE(_lock)
		// pass through all references
		for each( WeakReference ^wr in m_cache.Values ) {
			// dispose object
			delete wr->Target;
		}
		m_cache.Clear();
		EXIT_WRITE(_lock)
	}
}


//-------------------------------------------------------------------
//
// Gets or sets object in the cache.
//
// If object not found nullptr will be returned.
// Setter doesn't check for existing weak references to object: just
// create new weak reference.
// 
//-------------------------------------------------------------------
PersistentObject^ PersistenceBroker::			  \
BrokerCache::default::get( String ^type, int id )
{ENTER_READ(_lock)

	// check for disposed state
	if( m_disposed ) throw gcnew ObjectDisposedException(
		this->GetType()->ToString());

	WeakReference	^wr = nullptr;
	// try get weak reference for object
	if( m_cache.TryGetValue( key( type, id ), wr ) ) {
		// and return target object if succeeded
		return static_cast<PersistentObject^>( wr->Target );
	}
	return nullptr;

EXIT_READ(_lock)}

void PersistenceBroker::												 \
BrokerCache::default::set( String ^type, int id, PersistentObject ^obj )
{ENTER_WRITE(_lock)

	// check for disposed state
	if( m_disposed ) throw gcnew ObjectDisposedException(
		this->GetType()->ToString());

	// create week references to object
	m_cache[key( type, id )] = gcnew WeakReference(obj);

EXIT_WRITE(_lock)}


//-----------------------------------------------------------------------------
//								PersistenceBroker
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Check for curent state and raise exception if it is invalid.
//
//-------------------------------------------------------------------
void PersistenceBroker::check_state( void )
{
	// check for object disposed
	if( s_disposed ) throw gcnew ObjectDisposedException(
		this->GetType()->ToString());

	// check for disconnected state
	if( s_storage == nullptr ) throw gcnew InvalidOperationException( 
		ERR_DISCONNECTED);
}


//-------------------------------------------------------------------		
//
// IIRemoteStorage::TransactionBegin implementation.
//
// Starts a storage transaction.
//
//-------------------------------------------------------------------
void PersistenceBroker::trans_begin( void )
{
	// check current state
	check_state();

	// call to real storage
	s_storage->TransactionBegin();
}


//-------------------------------------------------------------------
//
// IIRemoteStorage::TransactionCommit implementation.
//
// Commits the storage transaction.
//
//-------------------------------------------------------------------
void PersistenceBroker::trans_commit( void )
{
	// check current state
	check_state();

	// call to real storage
	s_storage->TransactionCommit();
}


//-------------------------------------------------------------------
//
// IIRemoteStorage::TransactionRollback implementation.
//
// Rolls back a transaction from a pending state.
//
//-------------------------------------------------------------------
void PersistenceBroker::trans_rollback( void )
{
	// check current state
	check_state();

	// call to real storage
	s_storage->TransactionRollback();
}


//-------------------------------------------------------------------
//
// IIRemoteStorage::Search implementation.
//
// Search storage for persistent objects that satisfy specified
// conditions.
//
//-------------------------------------------------------------------
int PersistenceBroker::
search( String ^type, String ^query, String ^where,
		String ^order, int bottom, int count,
		[Out] array<HEADER>^ %headers )
{
	// check current state
	check_state();

	// call to real storage
	return s_storage->Search( type,
							  query, where, order, bottom, count,
							  headers );
}


//-------------------------------------------------------------------
//
// IIRemoteStorage::Retrieve implementation.
//
// Retrieve object header from storage.
//
//-------------------------------------------------------------------
void PersistenceBroker::
retrieve( HEADER %header )
{
	// check current state
	check_state();

	// call to real storage
	s_storage->Retrieve( header );
}


//-------------------------------------------------------------------
//
// IIRemoteStorage::Retrieve implementation.
//
// Retrieve object header, links and properties from storage.
//
//-------------------------------------------------------------------
void PersistenceBroker::
retrieve( HEADER %header, 
		  [Out] array<LINK>^ %links, [Out] array<PROPERTY>^ %props )
{
	// check current state
	check_state();

	// call to real storage
	s_storage->Retrieve( header, links, props );
}


//-------------------------------------------------------------------
//
// IIRemoteStorage::Save implementation.
//
// Save object header, links and properties to storage.
//
//-------------------------------------------------------------------
void PersistenceBroker::
save( HEADER %header, 
	  [In] array<LINK> ^links, [In] array<PROPERTY> ^props,
	  [Out] array<LINK>^ %mlinks, [Out] array<PROPERTY>^ %mprops )
{
	// check current state
	check_state();

	// call to real storage
	s_storage->Save( header, links, props, mlinks, mprops );
}


//-------------------------------------------------------------------
//
// IIRemoteStorage::Delete implementation.
//
// Delete object with specified header from storage.
//
//-------------------------------------------------------------------
void PersistenceBroker::
remove( HEADER header )
{
	// check current state
	check_state();

	// call to real storage
	s_storage->Delete( header );
}


//-------------------------------------------------------------------
//
// IIRemoteStorage::ProcessSQL implementation.
//
// Submit hardcoded SQL statements to the persistence.
//
//-------------------------------------------------------------------
DataSet^ PersistenceBroker::
process_sql( String ^sql )
{
	// check current state
	check_state();

	// call to real storage
	return s_storage->ProcessSQL( sql );
}


//-------------------------------------------------------------------
/// <summary>
/// Gets internal access to the storage.
/// </summary><remarks>
/// Access to the members from this interface can acts through .NET
/// Remoting.
/// </remarks>
//-------------------------------------------------------------------
IIRemoteStorage^ PersistenceBroker::Storage::get( void )
{
	// check for object disposed
	if( s_disposed ) throw gcnew ObjectDisposedException(
		PersistenceBroker::typeid->ToString());

	if( s_instance == nullptr ) {
		// now we must create new object instance
		if( s_brokerFactory != nullptr ) {
			// use factory for custom creation
			s_instance = s_brokerFactory->CreateInstance();
		} else {
			// use default constructor to create object
			s_instance = gcnew PersistenceBroker();
		}
	}
	return s_instance;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets object by specified header.
/// </summary><remarks>
/// Getter searchs for object in cache and if unsuccessful creates it
/// using factory.
/// </remarks>
//-------------------------------------------------------------------
PersistentObject^ PersistenceBroker::Cache::get( HEADER header )
{
	// create cache instance by first access
	if( s_cache == nullptr ) s_cache = gcnew BrokerCache();

	// check for right object header
	if( (header.Type == nullptr) || (header.ID <= 0) ||
		(header.Stamp == DateTime()) || (header.Name == nullptr) ) {
		// throw argument exception
		throw gcnew ArgumentException(ERR_OBJECT_HEADER, "header");
	}
	
	// lock cache access
	Monitor::Enter( s_cache );
	try {
		// search for object already exists
		PersistentObject	^obj = s_cache[header.Type, header.ID];
		if( obj == nullptr ) {
			// check for factory has been initialized already
			if( s_objectFactory == nullptr ) {
				// throw exception
				throw gcnew InvalidOperationException(ERR_OBJECT_FACTORY);
			}
			// create object through factory
			obj = s_objectFactory->CreateInstance( header.Type, header.ID,
												   header.Stamp, header.Name );
			// check for succeded object creation
			if( obj == nullptr) {
				throw gcnew InvalidOperationException(String::Format(
				ERR_OBJECT_CREATION, header.Type) );
			}
			// and push new object to cache
			s_cache[header.Type, header.ID] = obj;
		}
		return obj;
	} finally {
		// unlock cache access
		Monitor::Exit( s_cache );
	}
}

void PersistenceBroker::Cache::set( HEADER header, PersistentObject ^obj )
{
	// create cache instance by first access
	if( s_cache == nullptr ) s_cache = gcnew BrokerCache();

	// check for right object header
	if( (header.Type == nullptr) || (header.ID <= 0) ||
		(header.Stamp == DateTime()) || (header.Name == nullptr) ) {
		// throw argument exception
		throw gcnew ArgumentException(ERR_OBJECT_HEADER, "header");
	}
	
	// lock cache access
	Monitor::Enter( s_cache );
	try {
		// and push new object to cache
		s_cache[header.Type, header.ID] = obj;
	} finally {
		// unlock cache access
		Monitor::Exit( s_cache );
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Default class constructor.
/// </summary><remarks>
/// Specify it to provide singleton pattern.
/// </remarks>
//-------------------------------------------------------------------
PersistenceBroker::PersistenceBroker( void )
{
	dbgprint( "-> [" + AppDomain::CurrentDomain->FriendlyName + "]" );

	// store instance in static variable (for correct
	// creation of singleton object through reflection)
	s_instance = this;

	dbgprint( "<- [" + AppDomain::CurrentDomain->FriendlyName + "]" );
}


//-------------------------------------------------------------------
/// <summary>
/// Sets interface that provide custom broker creation. Null means
/// using of default creation algorithm.
/// </summary>
//-------------------------------------------------------------------
void PersistenceBroker::BrokerFactory::set( IBrokerFactory ^factory )
{
	s_brokerFactory = factory;
}


//-------------------------------------------------------------------
/// <summary>
/// Sets interface that provide persistent object creation.
/// </summary><remarks>
/// This interface must be set on the client but is not needed on the
/// server side.
/// </remarks>
//-------------------------------------------------------------------
void PersistenceBroker::ObjectFactory::set( IObjectFactory ^factory )
{
	// check for null reference
	if( factory == nullptr ) throw gcnew ArgumentNullException( "factory" );

	// save to static field
	s_objectFactory = factory;
}


//-------------------------------------------------------------------
/// <summary>
/// Connects to persistent storage using specified interface.
/// </summary><remarks><para>
/// Use on the server side to connect to storage.</para><para>
/// This function define as static that prevent access to server
/// real routine through .NET Remoting.
/// </para></remarks>
//-------------------------------------------------------------------
void PersistenceBroker::Connect( IPersistenceStorage ^storage )
{
	// check for null reference
	if( storage == nullptr ) throw gcnew ArgumentNullException("storage");
	
	// save storage interface
	s_storage = storage;
}


//-------------------------------------------------------------------
/// <summary>
/// Disconnects from persistent storage.
/// </summary><remarks><para>
/// After this function call all operations with objects will be
/// failed, but objects instances will not be destroyed. So you will
/// be able use it after reconnect.</para><para>
/// This function define as static that prevent access to server
/// real routine through .NET Remoting.
/// </para></remarks>
//-------------------------------------------------------------------
void PersistenceBroker::Disconnect( void )
{
	// dispose storage interface
	delete s_storage;

	s_storage = nullptr;
}
//-------------------------------------------------------------------
/// <summary>
/// Close PersistenceBroker.
/// </summary><remarks>
/// YOU MUST CALL THIS FUNCTION BEFORE END OF WORK. This routine free
/// all allocated resources and stop all running threads (if you'll
/// not call this function than application domain will not be
/// unloaded because of cross references in some threads).
/// </remarks>
//-------------------------------------------------------------------
void PersistenceBroker::Close( void )
{	
	dbgprint( "-> [" + AppDomain::CurrentDomain->FriendlyName + "]" );

	if( !s_disposed ) {
		// prevent from future calls
		s_disposed = true;
		
		// dispose storage if specified
		delete s_storage;
		// dispose cache of objects
		delete s_cache;
		
		s_storage = nullptr;
		s_brokerFactory = nullptr;
		s_objectFactory = nullptr;
	}

	dbgprint( "<- [" + AppDomain::CurrentDomain->FriendlyName + "]" );
}