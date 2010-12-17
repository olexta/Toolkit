**************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		IPersistenceStorage.h										*/
/*																			*/
/*	Content:	Definition of Storage::IPersistenceStorage interface.		*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#using <System.Data.dll>

#include "..\RPL.h"
#include "..\ValueBox.h"
#include "..\Query.h"

using namespace System;
using namespace System::Data;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;


_RPL_BEGIN
namespace Storage {
	/// <summary>
	/// Encapsulates persistent object header.
	/// </summary>
	[Serializable]
	public value class HEADER
	{
	private:
		String^		const _type;
		int			const _id;
		DateTime	const _stamp;
		String^		const _name;

	public:
		HEADER( String ^type, int id, DateTime stamp, String ^name ): \
			_type(type), _id(id), _stamp(stamp), _name(name) {}

		/// <summary>
		/// Gets type of the object.
		/// </summary>
		property String^ Type {
			String^ get( void ) {return _type;}
		}
		/// <summary>
		/// Gets identifier that represent object in persistence storage.
		/// </summary>
		property int ID {
			int get( void ) {return _id;}
		}
		/// <summary>
		/// Gets a time of the last object's modification.
		/// </summary>
		property DateTime Stamp {
			DateTime get( void ) {return _stamp;}
		}
		/// <summary>
		/// Gets object's short name.
		/// </summary>
		property String^ Name {
			String^ get( void ) {return _name;}
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
		HEADER	const _header;
		STATE	const _state;

	public:
		LINK( HEADER header, STATE state ): \
			_header(header), _state(state) {}

		/// <summary>
		/// Gets header of the object this link is pointed to.
		/// </summary>
		property HEADER Header {
			HEADER get( void ) {return _header;}
		}
		/// <summary>
		/// Gets action this link is performed.
		/// </summary>
		property STATE State {
			STATE get( void ) {return _state;}
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
		String^		const _name;
		ValueBox	const _value;
		STATE		const _state;

	public:
		PROPERTY( String ^name, ValueBox value, STATE state ): \
			_name(name), _value(value), _state(state) {}

		/// <summary>
		/// Gets property name.
		/// </summary>
		property String^ Name {
			String^ get( void ) {return _name;}
		}
		/// <summary>
		/// Gets property value.
		/// </summary>
		property ValueBox Value {
			ValueBox get( void ) {return _value;}
		}
		/// <summary>
		/// Gets action this property is performed.
		/// </summary>
		property STATE State {
			STATE get( void ) {return _state;}
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
		/// <param name="where">SQL WHERE clause.</param>
		/// <param name="order">SQL ORDER BY clause.</param>
		/// <param name="bottom">Bottom limit in the request.</param>
		/// <param name="count">Count limit in the request.</param>
		/// <param name="headers">Array of found object headers.</param>
		/// <returns>
		/// Number of objects found
		/// </returns>
		int Search( String ^type, Where ^where, OrderBy ^order,
					int bottom, int count,
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
		/// <param name="params">Represents a list of parameters are passed to
		/// SQL request.</param>
		/// <remarks>
		/// Parametrized request uses standard String.Format naming style.
		/// </remarks><returns>
		/// Disconnected an in-memory cache of data.
		/// </returns>
		DataSet^ ProcessSQL( String ^sql, array<Object^> ^params );
	};
}_RPL_END
