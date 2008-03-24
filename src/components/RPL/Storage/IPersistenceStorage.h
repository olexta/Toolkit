/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		IPersistenceStorage.h										*/
/*																			*/
/*	Content:	Definition of Storage::IPersistenceStorage interface.		*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#using <System.Data.dll>
#include "RPL.h"

using namespace System;
using namespace System::Data;
using namespace System::Collections::Generic;


_RPL_BEGIN
interface class IID;
ref class PersistentProperty;
ref class PersistentObject;
ref class ObjectLinks;
ref class ObjectProperties;
ref class PersistentCriteria;

namespace Storage {
	interface class IBrokerCache;

	/// <summary>
	/// Encapsulates the behavior needed for low-level working with persistence
	/// storage.
	/// </summary><remarks>
	/// Storage representation must search objects in broker cache, to avoid
	/// dublicate data. For this issue interface IBrokerCache is passed.
	/// </remarks>
	public interface class IPersistenceStorage
	{
		void Open( IDbConnection ^db, IBrokerCache ^cache );
		void Close( void );

		void TransactionBegin( void );
		void TransactionCommit( void );
		void TransactionRollback( void );

		int Search( PersistentCriteria ^crit,
					IEnumerable<PersistentObject^>^ %objs );
		void Retrieve( IID ^iid, DateTime %stamp, String^ %name );
		void Retrieve( IID ^iid,
					   IEnumerable<PersistentObject^>^ %links,
					   IEnumerable<PersistentProperty^>^ %props );
		void Save( PersistentObject ^obj, 
				   int %id, DateTime %stamp, String^ %name,
				   ObjectLinks ^links, ObjectProperties ^props,
				   IEnumerable<PersistentObject^>^ %newlinks,
				   IEnumerable<PersistentProperty^>^ %newprops );
		void Delete( IID ^iid );
	};
}_RPL_END