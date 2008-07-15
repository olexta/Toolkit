/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		IPersistenceStorage.h										*/
/*																			*/
/*	Content:	Definition of Storage::IPersistenceStorage interface.		*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#using <System.Data.dll>

#include "..\RPL.h"
#include "..\ValueBox.h"

using namespace System;
using namespace System::Data;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;


_RPL_BEGIN
namespace Storage {
	/// <summary>
	/// Encapsulates persistent object header.
	///	</summary>
	[Serializable]
	public value class HEADER
	{
	private:
		String		^m_type;
		int			m_id;
		DateTime	m_stamp;
		String		^m_name;

	public:
		HEADER( String ^type, int id, DateTime stamp, String ^name ): \
			m_type(type), m_id(id), m_stamp(stamp), m_name(name) {}
		
		/// <summary>
		/// Gets type of the object.
		/// </summary>
		property String^ Type {
			String^ get( void ) {return m_type;}
		}
		/// <summary>
		/// Gets identifier that represent object in persistence storage.
		/// </summary>
		property int ID {
			int get( void ) {return m_id;}
		}
		/// <summary>
		/// Gets a time of the last object's modification.
		/// </summary>
		property DateTime Stamp {
			DateTime get( void ) {return m_stamp;}
		}
		/// <summary>
		/// Gets object's short name.
		/// </summary>
		property String^ Name {
			String^ get( void ) {return m_name;}
		}
	};


	/// <summary>
	/// Encapsulates persistent object link.
	///	</summary>
	[Serializable]
	public value class LINK
	{
	public:
		/// <summary>
		/// Encapsulates persistent link action.
		/// </summary>
		[Serializable]
		enum class STATE {None, New, Deleted};

	private:
		HEADER	m_header;
		STATE	m_state;

	public:
		LINK( HEADER header, STATE state ): \
			m_header(header), m_state(state) {}

		/// <summary>
		/// Gets header of the object this link is pointed to.
		/// </summary>
		property HEADER Header {
			HEADER get( void ) {return m_header;}
		}
		/// <summary>
		/// Gets action this link is performed.
		/// </summary>
		property STATE State {
			STATE get( void ) {return m_state;}
		}
	};


	/// <summary>
	/// Encapsulates persistent object property.
	///	</summary>
	[Serializable]
	public value class PROPERTY
	{
	public:
		/// <summary>
		/// Encapsulates persistent property action.
		/// </summary>
		[Serializable]
		enum class STATE {None, New, Changed, Deleted};

	private:
		String		^m_name;
		ValueBox	m_value;
		STATE		m_state;

	public:
		PROPERTY( String ^name, ValueBox value, STATE state ): \
			m_name(name), m_value(value), m_state(state) {}

		/// <summary>
		/// Gets property name.
		/// </summary>
		property String^ Name {
			String^ get( void ) {return m_name;}
		}
		/// <summary>
		/// Gets property value.
		/// </summary>
		property ValueBox Value {
			ValueBox get( void ) {return m_value;}
		}
		/// <summary>
		/// Gets action this property is performed.
		/// </summary>
		property STATE State {
			STATE get( void ) {return m_state;}
		}
	};


	/// <summary>
	/// Encapsulates the behavior needed for low-level working with persistence
	/// storage.
	/// </summary>
	public interface class IPersistenceStorage
	{
		/// <summary>
		/// Starts a storage transaction.
		/// </summary><remarks>
		/// RPL supports nested transaction. If storage doesn't, it must expose
		/// it by itself.
		/// </remarks>
		void TransactionBegin( void );
		/// <summary>
		/// Commits the storage transaction.
		/// </summary><remarks>
		/// RPL supports nested transaction. If storage doesn't, it must expose
		/// it by itself.
		/// </remarks>
		void TransactionCommit( void );
		/// <summary>
		/// Rolls back a transaction from a pending state.
		/// </summary><remarks>
		/// RPL supports nested transaction. If storage doesn't, it must expose
		/// it by itself.
		/// </remarks>
		void TransactionRollback( void );
		
		/// <summary>
		/// Search storage for persistent objects that satisfy specified conditions.
		/// </summary>
		/// <param name="type">Objects type.</param>
		/// <param name="query">Additional SQL request.</param>
		/// <param name="where">SQL WHERE clause.</param>
		/// <param name="order">SQL ORDER BY clause.</param>
		/// <param name="bottom">Bottom limit in the request.</param>
		/// <param name="count">Count limit in the request.</param>
		/// <param name="headers">Array of found object headers.</param>
		/// <returns>
		/// Number of objects found
		/// </returns>
		int Search( String ^type, String ^query, String ^where,
					String ^order, int bottom, int count,
					[Out] array<HEADER>^ %headers );

		/// <summary>
		/// Retrieve object header from storage.
		/// </summary>
		/// <param name="header">In/Out header value.</param>
		/// <remarks>
		/// Type and object ID must be specified while call request.
		/// </remarks>
		void Retrieve( HEADER %header );
		/// <summary>
		/// Retrieve object header, links and properties from storage.
		/// </summary>
		/// <param name="header">In/Out header value.</param>
		/// <param name="links">Array of object links.</param>
		/// <param name="props">Array of object properties.</param>
		/// <remarks><para>
		/// Type and object ID must be specified while call request.</para><para>
		/// Persistence storage has to check specified date and return links
		/// and properties if object is not up-to-date.
		/// </para></remarks>
		void Retrieve( HEADER %header, 
					   [Out] array<LINK>^ %links,
					   [Out] array<PROPERTY>^ %props );
		/// <summary>
		/// Save object header, links and properties to storage.
		/// </summary>
		/// <param name="header">In/Out header value.</param>
		/// <param name="links">Array of modified object links.</param>
		/// <param name="props">Array of modified object properties.</param>
		/// <param name="mlinks">Array of new object links.</param>
		/// <param name="mprops">Array of new object properties.</param>
		/// <remarks><para>
		/// Type and object ID must be specified while call request.</para><para>
		/// Storage has to check specified date and throw exception if DB
		/// object has newer modification stamp.</para><para> 
		/// After save some links or properties may be updated by storage. So
		/// such objects have to be returned through mlinks and mprops params.
		/// </para></remarks>
		void Save( HEADER %header, 
				   [In] array<LINK> ^links,
				   [In] array<PROPERTY> ^props,
				   [Out] array<LINK>^ %mlinks,
				   [Out] array<PROPERTY>^ %mprops );
		/// <summary>
		/// Delete object with specified header from storage.
		/// </summary>
		/// <param name="header">Header value.</param>
		/// <remarks><para>
		/// Type and object ID must be specified while call request.</para><para>
		/// Storage has to check specified date and throw exception if DB
		/// object has newer modification stamp.
		/// </para></remarks>
		void Delete( HEADER header );
		
		/// <summary>
		/// Submit hardcoded SQL statements to the persistence.
		/// </summary>
		/// <param name="sql">SQL request to be processed.</param>
		/// <returns>
		/// Disconnected an in-memory cache of data.
		/// </returns>
		DataSet^ ProcessSQL( String ^sql );
	};
}_RPL_END