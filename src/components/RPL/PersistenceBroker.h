/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistenceBroker.h											*/
/*																			*/
/*	Content:	Definition of PersistenceBroker class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"
#include "Storage\IBrokerCache.h"
#include "Storage\IPersistenceStorage.h"

using namespace System;
using namespace System::Data;
using namespace System::Threading;
using namespace System::Collections::Generic;
using namespace _COLLECTIONS;
using namespace _RPL::Storage;


_RPL_BEGIN
interface class IID;
interface class IBrokerFactory;
interface class ITransactionSupport;
ref class PersistentObject;
ref class PersistentCriteria;
ref class PersistentTransaction;

/// <sumamry>
/// Provide internal access to the PersistenceBroker class.
/// </sumamry><remarks>
/// This is more flexible realisation of a "internal" access modifier. This
/// interface can be used in .NET Remoting.
/// </remarks>
private interface class IIPersistenceBroker
{
	void SaveObject( PersistentObject ^obj );
	void RetrieveObject( PersistentObject ^obj );
	void DeleteObject( PersistentObject ^obj );

	void Process( PersistentCriteria ^crit );
	void Process( PersistentTransaction ^trans );
	IDataReader^ Process( String ^SQL );
};

/// <summary>
/// Singleton class that provide main functionality to manage persistence
/// layer.
/// </summary><remarks>
///	In many ways this class is the key to the persistence layer. This class
/// follows the Singleton design pattern in that there is only one instance
/// of it in the object space of the application domain. During run time
/// PersistenceBroker maintains connection to persistence mechanism and
/// manages interactions with them. PersistenceBroker effectively acts as a
/// go between for the classes PersistentObject, PersistentCriteria and
/// PersistentTransaction as it is where instances of these classes submit
/// themselves to be processed. An important feature of this class is store
/// founded objects in internal cache, that improve search perfomance.
/// </remarks>
public ref class PersistenceBroker : MarshalByRefObject, IIPersistenceBroker
{
private:
	// "interface" to parent IPersistentStorage and IDbConnection
	// accessing i cann't use traking references due to CLI restrictions
	delegate IPersistenceStorage^ GET_STORAGE( void );
	delegate void ADD_TO_TRANS( ITransactionSupport ^tobj );
	
	//
	// This is cache of objects for some session
	//	
	ref class BrokerCache : IBrokerCache
	{
	private:
		ReaderWriterLock^	const _lock;
		GET_STORAGE^		const _fnStorage;
		ADD_TO_TRANS^		const _fnAddToTrans;
		Thread				^m_clear_thread;

		Map<String^, WeakReference^>	m_cache;
		
		String^	key( int id, String ^type );
		String^ key( IID ^iid );
		void clear( bool bInacecessibleOnly );

	public:
		BrokerCache( GET_STORAGE ^fnStorage, ADD_TO_TRANS ^fnAddToTrans );
		~BrokerCache( void );

		void Add( PersistentObject ^obj );
		void Add( IEnumerable<PersistentObject^> ^objs );
		void Clear( void );

		virtual PersistentObject^ Search( int id, String ^type );
		virtual PersistentObject^ Search( int id, String ^type,
										  DateTime stamp, String ^name );
	};

private:
	Object^						const _lock_obj;
	Object^						const _lock_crit;
	Object^						const _lock_trans;

	static IBrokerFactory		^m_factory = nullptr;
	static PersistenceBroker	^m_instance = nullptr;

	IDbConnection				^m_cnn;
	IPersistenceStorage			^m_storage;
	BrokerCache					^m_cache;
	List<ITransactionSupport^>	^m_trans_stack;

	void check_state( void );
	void add_to_trans( ITransactionSupport ^tobj );
	IPersistenceStorage^ get_storage( void );

// IIPersistenceBroker
private:
	virtual void save_object( PersistentObject ^obj ) sealed =
		IIPersistenceBroker::SaveObject;
	virtual void retrieve_object( PersistentObject ^obj ) sealed =
		IIPersistenceBroker::RetrieveObject;
	virtual void delete_object( PersistentObject ^obj ) sealed =
		IIPersistenceBroker::DeleteObject;

	virtual void process( PersistentCriteria ^crit ) sealed =
		IIPersistenceBroker::Process;
	virtual void process( PersistentTransaction ^trans ) sealed =
		IIPersistenceBroker::Process;
	virtual IDataReader^ process( String ^SQL ) sealed =
		IIPersistenceBroker::Process;

internal:
	property IIPersistenceBroker^ Broker {
		static IIPersistenceBroker^ get( void );
	}

protected:
	PersistenceBroker( void );
	~PersistenceBroker( void );

public:
	property IBrokerFactory^ Factory {
		static void set( IBrokerFactory ^factory );
	}
	property PersistenceBroker^ Instance {
		static PersistenceBroker^ get( void );
	};
	property bool IsConnected {
		bool get( void );
	}

	void Connect( IDbConnection ^cnn, IPersistenceStorage ^storage );
	void Disconnect( void );
};
_RPL_END