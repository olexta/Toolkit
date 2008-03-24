/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		StreamWrapper.cpp											*/
/*																			*/
/*	Content:	Implementation of StreamWrapper class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "StreamWrapper.h"

using namespace _RPL;


//
// Define macros to ignore exceptions
//
#define TRY(expr)		try { expr; } catch( Exception^ ) {};


//-----------------------------------------------------------------------------
//						Toolkit::RPL::StreamWrapper
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// OnCahnge event implementation.
//
// I must override the default access on the add, remove, and raise
// events methods to present event as delegate.
//
//-------------------------------------------------------------------
void StreamWrapper::on_change::add( SW_CHANGE ^d )
{
	m_on_change += d;
}

void StreamWrapper::on_change::remove( SW_CHANGE ^d )
{
	m_on_change -= d;
}

void StreamWrapper::on_change::raise( StreamWrapper ^sender )
{
	if( m_on_change != nullptr ) {
		// perform all delegates in event
		m_on_change->Invoke( sender );
	}
}


//-------------------------------------------------------------------
//
// Returns stream, that this object is wrapped around.
//
//-------------------------------------------------------------------
Stream^ StreamWrapper::real_value( void )
{
	return m_stream;
}


//-------------------------------------------------------------------
/// <summary>
/// Create new StreamWrapper instance around given stream.
/// </summary>
//-------------------------------------------------------------------
StreamWrapper::StreamWrapper( Stream ^stream )
{
	dbgprint( String::Format( "-> {0}", stream ) );

	if( stream == nullptr ) throw gcnew ArgumentNullException("stream");

	// get real stream if internal stream is StreamWrapper
	IIStreamWrapper ^sw = dynamic_cast<StreamWrapper^>( stream );
	if( sw != nullptr ) stream = sw->RealValue;

	m_stream = stream;

	dbgprint( String::Format( "<- {0}", stream ) );
}


//-------------------------------------------------------------------
/// <summary>
/// Class disposer.
/// </summary><remarks>
/// This class is wrapper that all stream manipulations must pass
/// throught themself. So i must delete stream. .NET translate MC++
/// destructor to Dispose representation, which is supported by
/// stream. 
/// </remarks>
//-------------------------------------------------------------------
StreamWrapper::~StreamWrapper( void )
{
	// dispose m_stream
	delete m_stream;

	// notify about change (after disposing all
	// Streams become unaccessable)
	TRY( on_change( this ) );
}


//-----------------------------------------------------------------------------
//					Toolkit::RPL::StreamWrapper::Stream
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Gets a value indicating whether the current stream supports
/// reading.
/// </summary>
//-------------------------------------------------------------------
bool StreamWrapper::CanRead::get( void )
{
	return m_stream->CanRead;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets a value indicating whether the current stream supports
/// seeking.
/// </summary>
//-------------------------------------------------------------------
bool StreamWrapper::CanSeek::get( void )
{
	return m_stream->CanSeek;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets a value that determines whether the current stream can time
/// out.
/// </summary>
//-------------------------------------------------------------------
bool StreamWrapper::CanTimeout::get( void )
{
	return m_stream->CanTimeout;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets a value indicating whether the current stream supports
/// writing.
/// </summary>
//-------------------------------------------------------------------
bool StreamWrapper::CanWrite::get( void )
{
	return m_stream->CanWrite;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets the length in bytes of the stream.
/// </summary>
//-------------------------------------------------------------------
__int64 StreamWrapper::Length::get( void )
{
	return m_stream->Length;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets the position within the current stream.
/// </summary>
//-------------------------------------------------------------------
__int64 StreamWrapper::Position::get( void )
{
	return m_stream->Position;
}

void StreamWrapper::Position::set( __int64 value )
{
	m_stream->Position = value;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets a value that determines how long the stream will
/// attempt to read before timing out.
/// </summary>
//-------------------------------------------------------------------
int StreamWrapper::ReadTimeout::get( void )
{
	return m_stream->ReadTimeout;
}

void StreamWrapper::ReadTimeout::set( int value )
{
	m_stream->ReadTimeout = value;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets a value that determines how long the stream will
/// attempt to write before timing out. 
/// </summary>
//-------------------------------------------------------------------
int StreamWrapper::WriteTimeout::get( void )
{
	return m_stream->WriteTimeout;
}

void StreamWrapper::WriteTimeout::set( int value )
{
	m_stream->WriteTimeout = value;
}
 

//-------------------------------------------------------------------
/// <summary>
/// Closes the current stream and releases any resources (such as
/// sockets and file handles) associated with the current stream.
/// </summary>
//-------------------------------------------------------------------
void StreamWrapper::Close( void )
{
	m_stream->Close();

	// notify about change (after Close many
	// Streams become unaccessable)
	TRY( on_change( this ) );
}


//-------------------------------------------------------------------
/// <summary>
/// Clears all buffers for this stream and causes any buffered data to
/// be written to the underlying device.
/// </summary>
//-------------------------------------------------------------------
void StreamWrapper::Flush( void )
{
	m_stream->Flush();
}


//-------------------------------------------------------------------
/// <summary>
/// Reads a sequence of bytes from the current stream and advances the
/// position within the stream by the number of bytes read.
/// </summary>
//-------------------------------------------------------------------
int StreamWrapper::Read( array<unsigned char> ^buffer, int offset, int count )
{
	return m_stream->Read( buffer, offset, count );
}


//-------------------------------------------------------------------
/// <summary>
/// Reads a byte from the stream and advances the position within the
/// stream by one byte, or returns -1 if at the end of the stream.
/// </summary>
//-------------------------------------------------------------------
int StreamWrapper::ReadByte( void )
{
	return m_stream->ReadByte();
}


//-------------------------------------------------------------------
/// <summary>
/// Sets the position within the current stream.
/// </summary>
//-------------------------------------------------------------------
__int64 StreamWrapper::Seek( __int64 offset, SeekOrigin origin )
{
	return m_stream->Seek( offset, origin );
}


//-------------------------------------------------------------------
/// <summary>
/// Sets the length of the current stream.
/// </summary>
//-------------------------------------------------------------------
void StreamWrapper::SetLength( __int64 value )
{
	m_stream->SetLength( value );

	// notify about change (only succeded changes
	// will be notified)
	TRY( on_change( this ) );
}


//-------------------------------------------------------------------
/// <summary>
/// Writes a sequence of bytes to the current stream and advances the
/// current position within this stream by the number of bytes written.
/// </summary>
//-------------------------------------------------------------------
void StreamWrapper::Write( array<unsigned char> ^buffer, int offset, int count )
{
	m_stream->Write( buffer, offset, count );
	
	// notify about change (only succeded changes
	// will be notified)
	TRY( on_change( this ) );
}


//-------------------------------------------------------------------
/// <summary>
/// Writes a byte to the current position in the stream and advances
/// the position within the stream by one byte.
/// </summary>
//-------------------------------------------------------------------
void StreamWrapper::WriteByte( unsigned char value )
{
	m_stream->WriteByte( value );

	// notify about change (only succeded changes
	// will be notified)
	TRY( on_change( this ) );
}


//-------------------------------------------------------------------
/// <summary>
/// Returns a String that represents the current Object.
/// </summary><remarks>
/// Override standart object method ToString(): must provide special
/// processing for stream representation.
/// </remarks>
//-------------------------------------------------------------------
String^ StreamWrapper::ToString( void )
{
	return "<binary data>";
}