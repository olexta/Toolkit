/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		StreamWrapper.h												*/
/*																			*/
/*	Content:	Definition of StreamWrapper class							*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"

using namespace System;
using namespace System::IO;


_RPL_BEGIN
ref class StreamWrapper;
delegate void SW_CHANGE( StreamWrapper ^sender );

/// <sumamry>
/// Provide internal access to the StreamWrapper class.
/// </sumamry><remarks>
/// This is more flexible realisation of a "internal" access modifier. This
/// interface can be used in .NET Remoting.
/// </remarks>
private interface class IIStreamWrapper
{
	event SW_CHANGE ^OnChange {
		void add( SW_CHANGE ^d );
		void remove( SW_CHANGE ^d );
	}
	property Stream^ RealValue {
		Stream^ get( void );
	}
};

/// <sumamry>
/// Internal wrapper around of any stream object.
/// </sumamry><remarks>
/// It provide common stream interface to manipulate with stored object and
/// raise event in case of data change. I use this wrapper to control property
/// state.
/// </remarks>
private ref class StreamWrapper sealed : Stream, IIStreamWrapper
{
private:
	Stream			^m_stream;
	SW_CHANGE		^m_on_change;

// IIStreamWrapper implementation
private:
	event SW_CHANGE ^on_change {
		virtual void add( SW_CHANGE ^d ) sealed =
			IIStreamWrapper::OnChange::add;
		virtual void remove( SW_CHANGE ^d ) sealed =
			IIStreamWrapper::OnChange::remove;
		virtual void raise( StreamWrapper ^sender ) sealed;
	}

	virtual Stream^ real_value( void ) sealed =
		IIStreamWrapper::RealValue::get;

public:
	StreamWrapper( Stream ^stream );
	~StreamWrapper( void );

	property bool CanRead {
		virtual bool get( void ) override;
	}
	property bool CanSeek {
		virtual bool get( void ) override;
	}
	property bool CanTimeout {
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
	property int ReadTimeout {
		virtual int get( void ) override;
		virtual void set( int value ) override;
	}
	property int WriteTimeout {
		virtual int get( void ) override;
		virtual void set( int value ) override;
	}
	
	virtual void Close( void )  override;
	virtual void Flush( void )  override;
    virtual int Read( array<unsigned char> ^buffer, int offset, int count )  override;
	virtual int ReadByte( void )  override;
	virtual __int64 Seek( __int64 offset, SeekOrigin origin )  override;
	virtual void SetLength( __int64 value )  override;
	virtual void Write( array<unsigned char> ^buffer, int offset, int count )  override;
	virtual void WriteByte( unsigned char value )  override;

	virtual String^ ToString( void ) override;
};
_RPL_END