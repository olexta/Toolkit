/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		IPersistenceStorage.h										*/
/*																			*/
/*	Content:	Definition of IPersistenceStorage interface.				*/
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
using namespace System::Collections::Generic;
using namespace System::Data;


_RPL_BEGIN
interface class IID;
ref class CPersistentProperty;
ref class CPersistentObject;
ref class CObjectLinks;
ref class CObjectProperties;
ref class CPersistentCriteria;

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

		int Search( CPersistentCriteria ^crit,
					IEnumerable<CPersistentObject^>^ %objs );
		void Retrieve( IID ^iid, DateTime %stamp, String^ %name );
		void Retrieve( IID ^iid,
					   IEnumerable<CPersistentObject^>^ %links,
					   IEnumerable<CPersistentProperty^>^ %props );
		void Save( CPersistentObject ^obj, 
				   int %id, DateTime %stamp, String^ %name,
				   CObjectLinks ^links, CObjectProperties ^props,
				   IEnumerable<CPersistentObject^>^ %newlinks,
				   IEnumerable<CPersistentProperty^>^ %newprops );
		void Delete( IID ^iid );
	};
}_RPL_END