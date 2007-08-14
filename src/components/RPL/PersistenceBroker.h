/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistenceBroker.h											*/
/*																			*/
/*	Content:	Definition of CPersistenceBroker class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#using <System.Data.dll>
#include "RPL.h"
#include "IBrokerCache.h"
#include "IPersistenceStorage.h"

using namespace System;
using namespace System::Data;
using namespace System::Threading;
using namespace System::Collections::Generic;
using namespace RPL::Storage; 


_RPL_BEGIN
interface class IID;
interface class IBrokerFactory;
interface class ITransactionSupport;
ref class CPersistentObject;
ref class CPersistentCriteria;
ref class CPersistentTransaction;

/// <sumamry>
/// Provide internal access to the CPersistenceBroker class.
/// </sumamry><remarks>
/// This is more flexible realisation of a "internal" access modifier. This
/// interface can be used in .NET Remoting.
/// </remarks>
private interface class IIPersistenceBroker
{
	void SaveObject( CPersistentObject ^obj );
	void RetrieveObject( CPersistentObject ^obj );
	void DeleteObject( CPersistentObject ^obj );

	void Process( CPersistentCriteria ^crit );
	void Process( CPersistentTransaction ^trans );
	IDataReader^ Process( String ^SQL );
};

/// <summary>
/// Singleton class that provide main functionality to manage persistence
/// layer.
/// </summary><remarks>
///	In many ways this class is the key to the persistence layer. This class
/// follows the Singleton design pattern in that there is only one instance
/// of it in the object space of the application domain. During run time
/// CPersistenceBroker maintains connection to persistence mechanism and
/// manages interactions with them. PersistenceBroker effectively acts as a
/// go between for the classes CPersistentObject, CPersistentCriteria and
/// CPersistentTransaction as it is where instances of these classes submit
/// themselves to be processed. An important feature of this class is store
/// founded objects in internal cache, that improve search perfomance.
/// </remarks>
public ref class CPersistenceBroker : MarshalByRefObject, IIPersistenceBroker
{
private:
	// "interface" to parent IPersistentStorage and IDbConnection
	// accessing i cann't use traking references due to CLI restrictions
	delegate IPersistenceStorage^ GET_STORAGE( void );
	delegate void ADD_TO_TRANS( ITransactionSupport ^tobj );
	
	//
	// This is cache of objects for some session
	//	
	ref class CBrokerCache : IBrokerCache
	{
	private:
		Dictionary<String^, WeakReference^>	m_cache;
		GET_STORAGE							^m_fnStorage;
		ADD_TO_TRANS						^m_fnAddToTrans;
	
		Thread				^m_clear_thread;
		ReaderWriterLock	^m_rw_lock;

		String^	key( int id, String ^type );
		String^ key( IID ^iid );

	public:
		CBrokerCache( GET_STORAGE ^fnStorage, ADD_TO_TRANS ^fnAddToTrans );
		~CBrokerCache( void );

		void Add( CPersistentObject ^obj );
		void Add( IEnumerable<CPersistentObject^> ^objs );
		void Clear( bool bInaccessibleOnly );

		virtual CPersistentObject^ Search( int id, String ^type );
		virtual CPersistentObject^ Search( int id, String ^type,
										   DateTime stamp, String ^name );
	};

private:
	static IBrokerFactory		^m_factory = nullptr;
	static CPersistenceBroker	^m_instance = nullptr;
	Object						^_lock_obj;
	Object						^_lock_crit;
	Object						^_lock_trans;

	IDbConnection				^m_cnn;
	IPersistenceStorage			^m_storage;
	CBrokerCache				^m_cache;
	List<ITransactionSupport^>	^m_trans_stack;

	void check_state( void );
	void add_to_trans( ITransactionSupport ^tobj );
	IPersistenceStorage^ get_storage( void );

// IIPersistenceBroker
private:
	virtual void SaveObject( CPersistentObject ^obj ) sealed =
		IIPersistenceBroker::SaveObject;
	virtual void RetrieveObject( CPersistentObject ^obj ) sealed =
		IIPersistenceBroker::RetrieveObject;
	virtual void DeleteObject( CPersistentObject ^obj ) sealed =
		IIPersistenceBroker::DeleteObject;

	virtual void Process( CPersistentCriteria ^crit ) sealed =
		IIPersistenceBroker::Process;
	virtual void Process( CPersistentTransaction ^trans ) sealed =
		IIPersistenceBroker::Process;
	virtual IDataReader^ Process( String ^SQL ) sealed =
		IIPersistenceBroker::Process;

internal:
	static property IIPersistenceBroker^ Broker {
		IIPersistenceBroker^ get( void );
	}

protected:
	CPersistenceBroker( void );
	~CPersistenceBroker( void );

public:
	static property IBrokerFactory^ Factory {
		void set( IBrokerFactory^ factory );
	}
	static property CPersistenceBroker^ Instance {
		CPersistenceBroker^ get( void );
	};
	property bool IsConnected {
		bool get( void );
	}

	void Connect( IDbConnection ^cnn, IPersistenceStorage ^storage );
	void Disconnect( void );
};
_RPL_END