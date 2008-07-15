/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentStream.h											*/
/*																			*/
/*	Content:	Definition of PersistentStream class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"
#include "ITransaction.h"

using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace System::Security::Permissions;
using namespace System::Runtime::Serialization;
using namespace System::Runtime::InteropServices;


_RPL_BEGIN
/// <sumamry>
/// Represents stream property that can be stored in persistent storage.
/// </sumamry><remarks><para>
/// This stream is designed as atomic value, so you can use one instance in one
/// property only.</para><para>
/// Attempt to assign existing instance to more than one property will throw
/// InvalidOperationException.
/// </para></remarks>
[Serializable]
public ref class PersistentStream sealed : ISerializable, ITransaction
{
private:
	//
	// Represent stream wrapper class.
	//
	// All objects inherited from MarshalByRef can't have custom
	// serialization, so PersistentStream cann't be inherited from
	// Stream.
	// To avoid this restriction implicit cast operator to Stream was
	// implemented. And this class is designed to be proxy, that pass
	// all Stream's calls to it's owner.
	//
	ref class StreamWrapper : Stream
	{
	private:
		PersistentStream^	const _owner;

	public:
		StreamWrapper( PersistentStream ^owner );
		~StreamWrapper( void );

		property bool CanRead {
			virtual bool get( void ) override;
		}
		property bool CanSeek {
			virtual bool get( void ) override;
		}
		property bool CanWrite {
			virtual bool get( void ) override;
		}
		property __int64 Length {
			virtual __int64 get( void ) override;
		}
		property __int64 Position {
			virtual __int64 get( void ) override;
			virtual void set( __int64 value ) override;
		}
		
		virtual void Flush( void ) override;
	    virtual int Read( [In] [Out] array<unsigned char> ^buffer,
						  int offset, int count ) override;
		virtual int ReadByte( void ) override;
		virtual __int64 Seek( __int64 offset, SeekOrigin origin ) override;
		virtual void SetLength( __int64 value ) override;
		virtual void Write( array<unsigned char> ^buffer,
							int offset, int count ) override;
		virtual void WriteByte( unsigned char value ) override;

		virtual String^ ToString( void ) override;
	};
internal:
	delegate void ON_CHANGE( PersistentStream ^sender );

private:
	bool			m_disposed;
	String			^m_path;
	FileStream		^m_fs;
	ON_CHANGE		^m_on_change;

	void check_state( void );

// ITransaction
private:
	value class RESTORE_POINT {
	public:
		String		^_path;
		__int64		_position;
	};
	Stack<RESTORE_POINT>	backup;

	virtual void trans_begin( void ) sealed = ITransaction::Begin;
	virtual void trans_commit( void ) sealed = ITransaction::Commit;
	virtual void trans_rollback( void ) sealed = ITransaction::Rollback;

// ISerializable
private:
	[EnvironmentPermission(SecurityAction::Assert,Unrestricted=true)]
	[SecurityPermission(SecurityAction::Demand,SerializationFormatter=true)]
	PersistentStream( SerializationInfo ^info, StreamingContext context );

	[SecurityPermission(SecurityAction::Demand,SerializationFormatter=true)]
	virtual void get_object_data( SerializationInfo ^info,
								  StreamingContext context ) sealed =
		ISerializable::GetObjectData;

internal:
	event ON_CHANGE^ on_change {
		void add( ON_CHANGE ^d );
		void remove( ON_CHANGE ^d );
	private:
		void raise( PersistentStream ^sender );
	};

public:
	PersistentStream( void );
	explicit PersistentStream( String ^path );
	explicit PersistentStream( array<unsigned char> ^buffer );
	~PersistentStream( void );
	!PersistentStream( void );

	static operator Stream^( PersistentStream ^ps );

	property bool CanRead {
		bool get( void );
	}
	property bool CanSeek {
		bool get( void );
	}
	property bool CanWrite {
		bool get( void );
	}
	property __int64 Length {
		__int64 get( void );
	}
	property __int64 Position {
		__int64 get( void ) ;
		void set( __int64 value );
	}
	
	void ExportToFile( String ^path );
	void Flush( void )  ;
    int Read( [In] [Out] array<unsigned char> ^buffer,
					  int offset, int count );
	int ReadByte( void );
	__int64 Seek( __int64 offset, SeekOrigin origin );
	void SetLength( __int64 value );
	void Write( array<unsigned char> ^buffer,
						int offset, int count );
	void WriteByte( unsigned char value );

	virtual String^ ToString( void ) override;
};
_RPL_END