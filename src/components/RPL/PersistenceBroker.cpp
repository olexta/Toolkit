/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistenceBroker.cpp										*/
/*																			*/
/*	Content:	Implementation of CPersistenceBroker class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "IBrokerFactory.h"
#include "IPersistenceStorage.h"
#include "ITransactionSupport.h"
#include "ObjectLinks.h"
#include "ObjectProperties.h"
#include "PersistentObject.h"
#include "PersistentCriteria.h"
#include "PersistentTransaction.h"
#include "PersistenceBroker.h"

using namespace System::Threading;
using namespace System::Data::Common;
using namespace _RPL;
using namespace _RPL::Storage;


// Define lock macroses
#define ENTER(lock)		try { Monitor::Enter( lock );
#define EXIT(lock)		} finally { Monitor::Exit( lock ); }

// Define macros to ignore exceptions
#define TRY(expr)		try { expr; } catch( Exception^ ) {};


//----------------------------------------------------------------------------
//					C O M M O N   D E C L A R A T I O N S
//----------------------------------------------------------------------------

//
// Default timeout to clear cache inaccessible
// weak references. Default value is 600 sec.
//
#define CLEAR_TIMEOUT		0x000927C0
//
// This value identify wait timeout for thread aborting.
// Default value is 10 sec.
//
#define WAIT_TIMEOUT		0x000003E8

//
// This is delegate that provide access to internal
// cache function Clear. 
//
delegate void CLEAR_CACHE( bool bInaccessibleOnly );


//----------------------------------------------------------------------------
//						C O M M O N   F U N C T I O N S
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Thread function that provide clearing cache functionality. I pass
// delegate as parameter, that is pointer to internal cache function
// Clear.
//
//-------------------------------------------------------------------
void thread_clear_cache( Object ^param )
{
	CLEAR_CACHE		^fnClear = nullptr;


	// check for null reference
	if( param == nullptr ) throw gcnew ArgumentNullException("param");

	// check for right object passed to thread function
	fnClear = dynamic_cast<CLEAR_CACHE^>( param );
	if( fnClear == nullptr ) {

		throw gcnew ArgumentException("Specified parameter is not CLEAR_CACHE delegate!");
	}

	// this is clearing cycle
	while( true ) {

		Thread::Sleep( CLEAR_TIMEOUT );

		// call clear to cache
		fnClear( true );
	}
}



//----------------------------------------------------------------------------
//						CPersistenceBroker::CBrokerCache
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Build string from object's ID and Type for using as key in hash
// table.
//
//-------------------------------------------------------------------
String^	CPersistenceBroker::CBrokerCache::key( int id, String ^type )
{
	if( type == nullptr ) throw gcnew ArgumentNullException("type");
	
	return (type + "_" + id.ToString());
}


//-------------------------------------------------------------------
//
// Build string from object's IID interface for using as key in hash
// table.
//
//-------------------------------------------------------------------
String^ CPersistenceBroker::CBrokerCache::key( IID ^iid )
{
	if( iid == nullptr ) throw gcnew ArgumentNullException("iid");

	return key( iid->ID, iid->Type );
}


//-------------------------------------------------------------------
//
// Create new cache. I pass here delegate to IPersistenceStorage
// getter (from parent), so, i will have up-to-date interface after
// base class Connect called.
//
//-------------------------------------------------------------------
CPersistenceBroker::CBrokerCache::CBrokerCache(	GET_STORAGE ^fnStorage,
											    ADD_TO_TRANS ^fnAddToTrans ): \
	m_fnStorage(fnStorage), m_fnAddToTrans(fnAddToTrans), \
	m_rw_lock(gcnew ReaderWriterLock())
{
	// create thread instance to clear current
	// cache from inaccessible weak references
	m_clear_thread = gcnew Thread(gcnew ParameterizedThreadStart(thread_clear_cache));
	// and start it
	m_clear_thread->Start( gcnew CLEAR_CACHE(this, &CBrokerCache::Clear) );
}


//-------------------------------------------------------------------
//
// Class disposer. I must use it to clear internal storage and
// terminate clearing thread (in other case this object will never be
// destroyed because of object-thread cross references).
//
//-------------------------------------------------------------------
CPersistenceBroker::CBrokerCache::~CBrokerCache( void )
{
	m_rw_lock->AcquireReaderLock( Timeout::Infinite );
	try {
		// check for disposer was not called
		if( m_clear_thread != nullptr ) {
			// clear internal storage (i use public function
			// having it's own thread-safe implementation, so
			// i not modify type of lock)
			Clear( false );
			
			LockCookie lc = m_rw_lock->UpgradeToWriterLock( Timeout::Infinite );
			try {
				// attempt to abort thread
				m_clear_thread->Abort();
				// and wait for terminating by specified period
				m_clear_thread->Join( WAIT_TIMEOUT );
				// and prevent next call
				m_clear_thread = nullptr;
			} finally {
				// ensure that the lock is released
				m_rw_lock->DowngradeFromWriterLock( lc );
			}
		}
	} finally {
		// ensure that the lock is released
		m_rw_lock->ReleaseReaderLock();
	}
}


//-------------------------------------------------------------------
//
// Add object to cache. I not check for existing weak references to
// object because of it can contain empty reference. So i create new
// weak reference always. And this prevent more than one same objects
// collision if there are errors in IPersistenceStorage.
//
//-------------------------------------------------------------------
void CPersistenceBroker::CBrokerCache::Add( CPersistentObject ^obj )
{
	m_rw_lock->AcquireReaderLock( Timeout::Infinite );
	try {
		// check for disposed state
		if( m_clear_thread == nullptr ) throw gcnew ObjectDisposedException("BrokerCache");

		LockCookie lc = m_rw_lock->UpgradeToWriterLock( Timeout::Infinite );
		try {
			// create week references to object
			m_cache[key(obj)] = gcnew WeakReference(obj);
		} finally {
			// ensure that the lock is released
			m_rw_lock->DowngradeFromWriterLock( lc );
		}
	} finally {
		// ensure that the lock is released
		m_rw_lock->ReleaseReaderLock();
	}
}


//-------------------------------------------------------------------
//
// Add collection of objects to cache.
//
//-------------------------------------------------------------------
void CPersistenceBroker::CBrokerCache::Add( IEnumerable<CPersistentObject^> ^objs )
{
	m_rw_lock->AcquireReaderLock( Timeout::Infinite );
	try {
		// check for disposed state
		if( m_clear_thread == nullptr ) throw gcnew ObjectDisposedException("BrokerCache");

		// pass through all objects in collection and add each of it
		// to cache (i use here public method "Add" that has it's own
		// thread-safe implementation, so i not modify lock state)
		for each( CPersistentObject ^obj in objs ) Add( obj );
	} finally {
		// ensure that the lock is released
		m_rw_lock->ReleaseReaderLock();
	}	
}


//-------------------------------------------------------------------
//
// Clear cache of objects. I dispose collections of links and
// properties to free tempory resources (i not define object disposer
// to prevent of accessing it by clients).
//
//-------------------------------------------------------------------
void CPersistenceBroker::CBrokerCache::Clear( bool bInaccessibleOnly )
{
	m_rw_lock->AcquireReaderLock( Timeout::Infinite );
	try {
		// check for disposed state
		if( m_clear_thread == nullptr ) throw gcnew ObjectDisposedException("BrokerCache");

		LockCookie lc = m_rw_lock->UpgradeToWriterLock( Timeout::Infinite );
		try {
			// remove inaccessible weak references
			List<String^>	^keys = gcnew List<String^>(m_cache.Keys);
			for each( String^ key in keys ) {
				// check for null references
				if( m_cache[key]->Target == nullptr ) m_cache.Remove( key );
			}

			// and dispose all objects if needed
			if( !bInaccessibleOnly ) {
				// pass through all references
				for each( WeakReference ^wr in m_cache.Values ) {
					// check for object exists
					CPersistentObject ^obj = dynamic_cast<CPersistentObject^>( wr->Target );
					if( obj != nullptr ) {
						// and dispose links and properties
						delete ((IIPersistentObject^) obj)->Links;
						delete ((IIPersistentObject^) obj)->Properties;
					}
				}
				m_cache.Clear();
			}
		} finally {
			// ensure that the lock is released
			m_rw_lock->DowngradeFromWriterLock( lc );
		}
	} finally {
		// ensure that the lock is released
		m_rw_lock->ReleaseReaderLock();
	}	
}


//-------------------------------------------------------------------
//
// Search cache for object with specified Type and ID. If object not
// found or not exsists already nullptr will be returned.
//
//-------------------------------------------------------------------
CPersistentObject^ CPersistenceBroker::CBrokerCache::Search( int id, String ^type )
{
	m_rw_lock->AcquireReaderLock( Timeout::Infinite );
	try {
		// check for disposed state
		if( m_clear_thread == nullptr ) throw gcnew ObjectDisposedException("BrokerCache");

		// check for weak reference for object exists
		if( m_cache.ContainsKey( key(id, type) ) ) {
			// and return target object
			return static_cast<CPersistentObject^>( m_cache[key(id, type)]->Target );
		}
	} finally {
		// ensure that the lock is released
		m_rw_lock->ReleaseReaderLock();
	}
	return nullptr;
}


//-------------------------------------------------------------------
//
// Search cache for object with specified Type and ID. This routine
// return up-to-date object: it check specified date stamp and
// changed state of object. If it must be updated, then call Retrieve
// from persistent storage.
//
//-------------------------------------------------------------------
CPersistentObject^ CPersistenceBroker::CBrokerCache::Search( 
						int id, String ^type, DateTime stamp, String ^name )
{
	IEnumerable<CPersistentObject^>		^links = nullptr;
	IEnumerable<CPersistentProperty^>	^props = nullptr;


	m_rw_lock->AcquireReaderLock( Timeout::Infinite );
	try {
		// check for disposed state
		if( m_clear_thread == nullptr ) throw gcnew ObjectDisposedException("BrokerCache");

		// search cached object
		CPersistentObject ^obj = Search( id, type );
		if( obj != nullptr ) {
			// add to transaction if it exists
			m_fnAddToTrans( obj );
		
			// update proxy and check that retrieve is needed
			if( !((IIPersistentObject^) obj)->on_retrieve( id, stamp, name ) ) {
				// request for new links and properties
				m_fnStorage()->Retrieve( obj, links, props );
				// and update object by it
				((IIPersistentObject^) obj)->on_retrieve( links, props );
			}
			return obj;
		}
	} finally {
		// ensure that the lock is released
		m_rw_lock->ReleaseReaderLock();
	}
	return nullptr;
}


//----------------------------------------------------------------------------
//								CPersistenceBroker
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Check for curent state and raise exception if it is invalid.
//
//-------------------------------------------------------------------
void CPersistenceBroker::check_state( void )
{
	// check for object disposed
	if( m_cache == nullptr ) throw gcnew ObjectDisposedException("PersistenceBroker");
	// check for disconnected state
	if( !IsConnected ) throw gcnew InvalidOperationException( 
							"Operation is not allowed while in disconnected state!");
}


//-------------------------------------------------------------------
//
// Mark object as passed through transaction. This means that the
// object create save point (by calling TransactionBegin) and after
// transaction complete must call TransactionCommit to commit changes
// or TransactionRollback to return to state of save point. Save
// point is created only once: by first object access.
//
//-------------------------------------------------------------------
void CPersistenceBroker::add_to_trans( ITransactionSupport ^tobj )
{
	// check for started transaction
	if( m_trans_stack == nullptr ) return;

	// check for object is in transaction already 
	if( !m_trans_stack->Contains( tobj ) ) {
		// create save point for object
		tobj->TransactionBegin();
		// and add to transaction stack
		m_trans_stack->Add( tobj );
	}
}


//-------------------------------------------------------------------
//
// Get's IPersistentStorage interface. This function is used to
// emulate some getter interface: i must have routine to access last
// set by Connect interface of storage. Of course, ideal method is
// using tracking reference to m_storage, but i can't use it due to
// CLI restrictions. So, i implement this function to pass it as
// delegate to CBrokerCache instance.
//
//-------------------------------------------------------------------
IPersistenceStorage^ CPersistenceBroker::get_storage( void )
{
	return m_storage;
}


//-------------------------------------------------------------------
/// <summary>
/// Retrieve object links and properties from persistence storage.
/// </summary><remarks>
/// This function process in two steps: first of all i get proxy
/// properties (name and stamp) and update it with call on_retrieve
/// to object. on_retrieve check internal state and return value that
/// indicate need of retrieve properties. If it return false - i get
/// links and properties and next call of on_retrieve will update it.
/// </remarks>
//-------------------------------------------------------------------
void CPersistenceBroker::RetrieveObject( CPersistentObject ^obj )
{ENTER(_lock_obj)

	DateTime		stamp;
	String			^name = nullptr;


	// check connection state
	check_state();
	// add object to transaction stack if exists
	add_to_trans( obj );

	// retrieve proxy properties from storage
	m_storage->Retrieve( obj, stamp, name );

	// update proxy and check of needed links
	// and properties update 
	if( !((IIPersistentObject^) obj)->on_retrieve( obj->ID, stamp, name ) ) {

		IEnumerable<CPersistentObject^>		^links = nullptr;
		IEnumerable<CPersistentProperty^>	^props = nullptr;

		// retrieve new links and properties
		m_storage->Retrieve( obj, links, props );
		// and fill object by it
		((IIPersistentObject^) obj)->on_retrieve( links, props );
	}

EXIT(_lock_obj)}


//-------------------------------------------------------------------
/// <summary>
/// Save object into persistence storage.
/// </summary><remarks>
/// Some properties can be updated by DB triggers or persistence
/// schema, so i need update all values: proxy, links and properties.
/// </remarks>
//-------------------------------------------------------------------
void CPersistenceBroker::SaveObject( CPersistentObject ^obj )
{ENTER(_lock_obj)

	int				id = 0;
	DateTime		stamp;
	String			^name = nullptr;
	IEnumerable<CPersistentObject^>		^links = nullptr;
	IEnumerable<CPersistentProperty^>	^props = nullptr;	

	
	// check connection state
	check_state();
	// add object to transaction stack if exists
	add_to_trans( obj );

	// save specified object into storage
	m_storage->Save( obj, id, stamp, name,
					 ((IIPersistentObject^) obj)->Links,
					 ((IIPersistentObject^) obj)->Properties,
					 links, props );

	// update proxy values and check for retrieve links
	// and properties needed (in this case on_retrieve
	// always return false because of object stamb will be
	// changed by Save, but i write such code for customization)
	if( !((IIPersistentObject^) obj)->on_retrieve( id, stamp, name ) ) {
		// update links and properties
		((IIPersistentObject^) obj)->on_retrieve( links, props );
	}

EXIT(_lock_obj)}


//-------------------------------------------------------------------
/// <summary>
/// Delete object from persistence storage.
/// </summary><remarks>
/// This function call storage interface function to update data and
/// set new proxy values to mark object as deleted (ID(-1), stamp(0)
/// and it's own name). I not delete instance from cache and it still
/// store object with it's old ID. This can be usefull by transaction
/// rollback operation. On the other hand, object's ID is incremental
/// index, so no collision will happen and instance will be removed
/// from cache by clear thread.
/// </remarks>
//-------------------------------------------------------------------
void CPersistenceBroker::DeleteObject( CPersistentObject ^obj )
{ENTER(_lock_obj)

	DateTime	stamp;
	String		^name = nullptr;


	// check connection state
	check_state();
	// add object to transaction stack if exists
	add_to_trans( obj );
	
	// after object deletion it's name will be
	// present to provide abylity to identify it
	name = obj->Name;

	// delete object from persistence storage
	m_storage->Delete( obj );

	// and update proxy properties
	((IIPersistentObject^) obj)->on_retrieve( -1, stamp, name );

EXIT(_lock_obj)}


//-------------------------------------------------------------------
/// <summary>
/// Process specified PersistentCriteria.
/// </summary><remarks>
/// I must search for objects that satisfy criteria conditions and
/// than call internal criteria function "on_perform". All aditional
/// processing coresponding to criteria type will be done by itself.
/// </remarks>
//-------------------------------------------------------------------
void CPersistenceBroker::Process( CPersistentCriteria ^crit )
{ENTER(_lock_crit)

	int		count;
	IEnumerable<CPersistentObject^>	^objs = nullptr;

	
	// check connection state
	check_state();

	// add criteria to transaction stack (as transaction object)
	add_to_trans( crit );

	// find objects by criteria request
	count = m_storage->Search( crit, objs );

	// mark objects from collection as passed through transaction.
	// i need store object state here, because of update criteria
	// can change some options before Save will be called, so it's
	// possible to lost data
	for each( ITransactionSupport ^tobj in objs ) add_to_trans( tobj );

	// and add objects to cache
	m_cache->Add( objs );

	// make aditional criteria processing
	((IIPersistentCriteria^) crit)->on_perform( count, objs );

EXIT(_lock_crit)}


//-------------------------------------------------------------------
/// <summary>
/// Process specified CPersistentTransaction.
/// </summary><remarks>
/// This feature provide ability to process set of actions with
/// objects as atomic operation (in DB context and in business logic).
/// </remarks>
//-------------------------------------------------------------------
void CPersistenceBroker::Process( CPersistentTransaction ^trans )
{ENTER(_lock_trans)

	// check connection state
	check_state();

	// start a local transaction.
	m_storage->TransactionBegin();
	// create transaction stack
	m_trans_stack = gcnew List<ITransactionSupport^>();

	try {
		// process transaction
		((IIPersistentTransaction^) trans)->on_process();
		// attempt to commit the transaction.
		m_storage->TransactionCommit();

		for each( ITransactionSupport ^tobj in m_trans_stack ) {
			// commit object changes (delete save point for object)
			tobj->TransactionCommit();
		}
	} catch( Exception^ ) {
		// attempt to roll back the transaction in DB
		TRY( m_storage->TransactionRollback() );
		
		for each( ITransactionSupport ^tobj in m_trans_stack ) {
			// and restore from savepoint
			tobj->TransactionRollback();
		}
		throw;
	} finally {
		// free transaction stack (this means exit from
		// transaction scope)
		m_trans_stack = nullptr;
	}

EXIT(_lock_trans)}


//-------------------------------------------------------------------
/// <summary>
/// Execute some SQL request for DB and return result: IDataReader.
/// </summary><remarks>
/// I cann't pass disconnected DataAdapter because of using common DB
/// access. So, i return IDataReader with opened connection and
/// specify close it after using this interface.
/// </remarks>
//-------------------------------------------------------------------
IDataReader^ CPersistenceBroker::Process( String ^SQL )
{
	// check connection state
	check_state();

	// create command to execute
	IDbCommand	^cmd = m_cnn->CreateCommand();
	cmd->CommandText = SQL;
	cmd-> CommandType = CommandType::Text;

	// open connection
	if( m_cnn->State == ConnectionState::Closed ) m_cnn->Open();
	
	// process SQL request (i must close connection after recordset
	// reading, so i set specified command behavior)
	return cmd->ExecuteReader( CommandBehavior::CloseConnection );
}


//-------------------------------------------------------------------
/// <summary>
/// Gets access to the internal members of the CPersistenceBroker
/// class.
/// </summary>
//-------------------------------------------------------------------
IIPersistenceBroker^ CPersistenceBroker::Broker::get( void )
{
	return Instance;
}


//-------------------------------------------------------------------
/// <summary>
/// Default class constructor.
/// </summary><remarks>
/// I must specify it to provide singleton pattern.
/// </remarks>
//-------------------------------------------------------------------
CPersistenceBroker::CPersistenceBroker( void ): \
	m_cnn(nullptr), m_storage(nullptr), m_trans_stack(nullptr), \
	_lock_obj(gcnew Object), _lock_crit(gcnew Object), _lock_trans(gcnew Object)
{
	dbgprint( "-> [" + AppDomain::CurrentDomain->FriendlyName + "]" );
	
	// create cache of objects in this session
	m_cache = gcnew CBrokerCache(
				gcnew GET_STORAGE(this, &CPersistenceBroker::get_storage),
				gcnew ADD_TO_TRANS(this, &CPersistenceBroker::add_to_trans));

	// store instance in static variable (this is needed
	// for reflection creation of object)
	m_instance = this;

	dbgprint( "<- [" + AppDomain::CurrentDomain->FriendlyName + "]" );
}


//-------------------------------------------------------------------
/// <summary>
/// Dispose CPersistenceBroker instance.
/// </summary><remarks>
/// YOU MUST CALL THIS FUNCTION BEFORE END OF WORK. This routine free
/// all allocated resources and stop all running threads (if you'll
/// not call this function than application domain will not be
/// unloaded because of cross references in some threads). I not
/// implement finalizer, because of singleton pattern and specific
/// cache implementation.
/// </remarks>
//-------------------------------------------------------------------
CPersistenceBroker::~CPersistenceBroker( void )
{	
	dbgprint( "-> [" + AppDomain::CurrentDomain->FriendlyName + "]" );

	if( m_cache != nullptr ) {
		// dispose cache of objects
		delete m_cache;
		// and prevent from future calls
		m_cache = nullptr;
	}

	dbgprint( "<- [" + AppDomain::CurrentDomain->FriendlyName + "]" );
}


//-------------------------------------------------------------------
/// <summary>
/// Sets interface that provide custom object creation. Null means
/// using of default creation algorithm.
/// </summary>
//-------------------------------------------------------------------
void CPersistenceBroker::Factory::set( IBrokerFactory^ factory )
{
	m_factory = factory;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets handle to the singleton instance of the CPersistenceBroker
/// class.
/// </summary>
//-------------------------------------------------------------------
CPersistenceBroker^ CPersistenceBroker::Instance::get( void )
{
	if( m_instance == nullptr ) {
		// now we must create new object instance (constructor
		// of this object init m_instance themself, so i can
		// pass store of it)
		if( m_factory != nullptr ) {
			// use factory for custom creation
			m_instance = m_factory->CreateInstance();
		} else {
			// use default constructor to create object
			m_instance = gcnew CPersistenceBroker();
		}
	}
	return m_instance;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets curent state of connection to persistence storage.
/// </summary>
//-------------------------------------------------------------------
bool CPersistenceBroker::IsConnected::get( void )
{
	// check for object disposed
	if( m_cache == nullptr ) throw gcnew ObjectDisposedException("PersistenceBroker");

	return !(m_storage == nullptr);
}


//-------------------------------------------------------------------
/// <summary>
/// Connects to persistent storage using specified interface.
/// </summary>
//-------------------------------------------------------------------
void CPersistenceBroker::Connect( IDbConnection ^cnn, IPersistenceStorage ^storage )
{ENTER(_lock_trans) ENTER(_lock_crit) ENTER(_lock_obj)

	// check for object disposed
	if( m_cache == nullptr ) throw gcnew ObjectDisposedException("PersistenceBroker");
	
	if( cnn == nullptr ) throw gcnew ArgumentNullException("cnn");
	if( storage == nullptr ) throw gcnew ArgumentNullException("storage");

	// disconnect if is connected now
	if( IsConnected ) Disconnect();

	// open persistent storage connection
	storage->Open( cnn, m_cache );

	// save interfaces
	m_cnn = cnn;
	m_storage = storage;

EXIT(_lock_obj) EXIT(_lock_crit) EXIT(_lock_trans)}


//-------------------------------------------------------------------
/// <summary>
/// Disconnects from persistent storage.
/// </summary><remarks>
/// After this function call all operations with objects will be
/// failed, but objects instances will not be destroyed. So you will
/// be able use it after reconnect.
/// I not need close cnn because of using thread synchronization and
/// connection will be closed after operation completition.
/// </remarks>
//-------------------------------------------------------------------
void CPersistenceBroker::Disconnect( void )
{ENTER(_lock_trans) ENTER(_lock_crit) ENTER(_lock_obj)

	// check for object disposed
	if( m_cache == nullptr ) throw gcnew ObjectDisposedException("PersistenceBroker");	

	if( IsConnected ) m_storage->Close();

	m_cnn = nullptr;
	m_storage = nullptr;

EXIT(_lock_obj) EXIT(_lock_crit) EXIT(_lock_trans)}