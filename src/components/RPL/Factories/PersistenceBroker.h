/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistenceBroker.h											*/
/*																			*/
/*	Content:	Definition of Factories::PersistenceBroker class			*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "..\RPL.h"
#include "..\Storage\IPersistenceStorage.h"

using namespace System;
using namespace System::Threading;
using namespace Toolkit::Collections;
using namespace _RPL;
using namespace _RPL::Storage;


_RPL_BEGIN
ref class PersistentObject;

namespace Factories {
	interface class IBrokerFactory;
	interface class IObjectFactory;

	/// <summary>
	/// Interface that provides internal access to the storage.
	/// </summary><remarks><para>
	/// Calls to members of this interface can be passed through .NET
	/// Remoting. So, you should not use any persistence objects
	/// access in implementation (in other case remote client request
	/// will be used).</para><para>
	/// Now, this interface duplicate IPersistenceStorage interface
	/// but have private access modifier. This is done to protect
	/// storage from implementer access. But still have one bug:
	/// implicit cast to IPersistenceStorage remove access
	/// restrictions. To avoid this just duplicate IPersistenceStorage
	/// code here.
	/// </para></remarks>
	private interface class IIRemoteStorage : IPersistenceStorage
	{
		// no additional members
	};


	/// <summary>
	/// Singleton class that provides main functionality to manage
	/// persistence layer.
	/// </summary><remarks><para>
	///	In many ways this class is the key to the persistence layer.
	/// This class follows the Singleton design pattern in that there
	/// is only one instance of it in the object space of the
	/// application domain. Also, this is main/ goal to manage
	/// client-server configuration:</para><para>
	/// 1. Broker and objects are at the client side. This is "fat
	/// client" approach. In this case just create
	/// IPersistenceStorage interface and connect it by using Connect
	/// function.</para><para>
	/// 2. Broker and objects are at the server side. This is "thin
	/// client" approach. Configure .NET remoting to create all
	/// persistent objects (business objects, criterias and
	/// transactions) at the server side and just use it.</para><para>
	/// 3. Persistent objects are at client, but Broker is at the
	/// server side (default). Configure .NET remoting to create
	/// Broker on the server. All communication between client and
	/// server will be passed through IIRemoteStorage interface.
	/// </para></remarks>
	public ref class PersistenceBroker : MarshalByRefObject,
										 IIRemoteStorage
	{
	private:
		//
		// Cache of objects for some session.
		//
		ref class BrokerCache
		{
		private:
			ReaderWriterLock^				const _lock;
			
			bool							m_disposed;
			Map<String^, WeakReference^>	m_cache;

			void thread_clean( void );
			String^	key( String ^type, int id );
			
		public:
			BrokerCache( void );
			~BrokerCache( void );

			property PersistentObject^ default[String^, int] {
				PersistentObject^ get( String ^type, int id );
				void set( String ^type, int id, PersistentObject ^obj );
			}
		};

	private:
		static Object^				const _lock = gcnew Object();

		static bool					s_disposed = false;
		static IBrokerFactory		^s_brokerFactory = nullptr;
		static IObjectFactory		^s_objectFactory = nullptr;
		static PersistenceBroker	^s_instance = nullptr;
		static BrokerCache			^s_cache = nullptr;
		static IPersistenceStorage	^s_storage = nullptr;
		
		void check_state( void );

	// IIRemoteStorage
	private:
		virtual void trans_begin( void ) sealed =
			IIRemoteStorage::TransactionBegin;
		virtual void trans_commit( void ) sealed =
			IIRemoteStorage::TransactionCommit;
		virtual void trans_rollback( void ) sealed =
			IIRemoteStorage::TransactionRollback;
		
		virtual int search( String^, String^, String^, String^, int, int,
							[Out] array<HEADER>^% ) sealed =
			IIRemoteStorage::Search;

		virtual void retrieve( HEADER% ) sealed =
			IIRemoteStorage::Retrieve;
		virtual void retrieve( HEADER%, [Out] array<LINK>^%,
							   [Out] array<PROPERTY>^% ) sealed =
			IIRemoteStorage::Retrieve;
		virtual void save( HEADER%, [In] array<LINK>^, [In] array<PROPERTY>^,
						   [Out] array<LINK>^%, [Out] array<PROPERTY>^% ) sealed =
			IIRemoteStorage::Save;
		virtual void remove( HEADER ) sealed =
			IIRemoteStorage::Delete;
		
		virtual DataSet^ process_sql( String^ ) sealed =
			IIRemoteStorage::ProcessSQL;

	internal:
		property IIRemoteStorage^ Storage {
			static IIRemoteStorage^ get( void );
		}
		property PersistentObject^ Cache[HEADER] {
			static PersistentObject^ get( HEADER header );
			static void set( HEADER header, PersistentObject ^obj );
		}

	protected:
		PersistenceBroker( void );
		~PersistenceBroker( void );

	public:
		property IBrokerFactory^ BrokerFactory {
			static void set( IBrokerFactory ^factory );
		}
		property IObjectFactory^ ObjectFactory {
			static void set( IObjectFactory ^factory );
		}

		static void Connect( IPersistenceStorage ^storage );
		static void Disconnect( void );
		static void Close( void );
	};
}_RPL_END
