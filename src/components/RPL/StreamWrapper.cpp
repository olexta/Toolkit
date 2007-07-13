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

using namespace System::Threading;
using namespace RPL;


// Define lock macroses
#define ENTER(lock)		try { Monitor::Enter( lock );
#define EXIT(lock)		} finally { Monitor::Exit( lock ); }

// Define macros to ignore exceptions
#define TRY(expr)		try {expr;} catch ( Exception^ ) {};


//----------------------------------------------------------------------------
//								StreamWrapper
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// I must override the default access on the add, remove, and raise
// events methods to present event as delegate.
//
//-------------------------------------------------------------------
void StreamWrapper::OnChange::add( SW_CHANGE ^d )
{
	m_on_change += d;
}


void StreamWrapper::OnChange::remove( SW_CHANGE ^d )
{
	m_on_change -= d;
}


void StreamWrapper::OnChange::raise( StreamWrapper ^sender )
{
	if ( m_on_change != nullptr ) {
		// perform all delegates in event
		m_on_change->Invoke( sender );
	}
}


//-------------------------------------------------------------------
//
// Return stream, that this object is wrapped around.
//
//-------------------------------------------------------------------
Stream^ StreamWrapper::RealValue::get( void )
{
	return m_stream;
}


//-------------------------------------------------------------------
/// <summary>
/// Create new StreamWrapper instance around given stream.
/// </summary>
//-------------------------------------------------------------------
StreamWrapper::StreamWrapper( Stream ^stream ): \
	_lock_this(gcnew Object())
{
	dbgprint( String::Format( "-> {0}", stream ) );

	if ( stream == nullptr ) throw gcnew ArgumentNullException( "stream" );

	// get real stream if internal stream is StreamWrapper
	IIStreamWrapper ^sw = dynamic_cast<StreamWrapper^> (stream);
	if ( sw != nullptr ) stream = sw->RealValue;

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
{ENTER(_lock_this)

	// dispose m_stream
	delete m_stream;

	// notify about change (after disposing all
	// Streams become unaccessable)
	TRY( OnChange( this ) );

EXIT(_lock_this)}


//----------------------------------------------------------------------------
//									Stream
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Gets a value indicating whether the current stream supports
/// reading.
/// </summary>
//-------------------------------------------------------------------
bool StreamWrapper::CanRead::get( void )
{ENTER(_lock_this)

	return m_stream->CanRead;

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Gets a value indicating whether the current stream supports
/// seeking.
/// </summary>
//-------------------------------------------------------------------
bool StreamWrapper::CanSeek::get( void )
{ENTER(_lock_this)

	return m_stream->CanSeek;

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Gets a value that determines whether the current stream can time
/// out.
/// </summary>
//-------------------------------------------------------------------
bool StreamWrapper::CanTimeout::get( void )
{ENTER(_lock_this)

	return m_stream->CanTimeout;

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Gets a value indicating whether the current stream supports
/// writing.
/// </summary>
//-------------------------------------------------------------------
bool StreamWrapper::CanWrite::get( void )
{ENTER(_lock_this)

	return m_stream->CanWrite;

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Gets the length in bytes of the stream.
/// </summary>
//-------------------------------------------------------------------
__int64 StreamWrapper::Length::get( void )
{ENTER(_lock_this)

	return m_stream->Length;

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets the position within the current stream.
/// </summary>
//-------------------------------------------------------------------
__int64 StreamWrapper::Position::get( void )
{ENTER(_lock_this)

	return m_stream->Position;

EXIT(_lock_this)}


void StreamWrapper::Position::set( __int64 value )
{ENTER(_lock_this)

	m_stream->Position = value;

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets a value that determines how long the stream will
/// attempt to read before timing out.
/// </summary>
//-------------------------------------------------------------------
int StreamWrapper::ReadTimeout::get( void )
{ENTER(_lock_this)

	return m_stream->ReadTimeout;

EXIT(_lock_this)}


void StreamWrapper::ReadTimeout::set( int value )
{ENTER(_lock_this)

	m_stream->ReadTimeout = value;

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets a value that determines how long the stream will
/// attempt to write before timing out. 
/// </summary>
//-------------------------------------------------------------------
int StreamWrapper::WriteTimeout::get( void )
{ENTER(_lock_this)

	return m_stream->WriteTimeout;

EXIT(_lock_this)}


void StreamWrapper::WriteTimeout::set( int value )
{ENTER(_lock_this)

	m_stream->WriteTimeout = value;

EXIT(_lock_this)}
 

//-------------------------------------------------------------------
/// <summary>
/// Closes the current stream and releases any resources (such as
/// sockets and file handles) associated with the current stream.
/// </summary>
//-------------------------------------------------------------------
void StreamWrapper::Close( void )
{ENTER(_lock_this)

	m_stream->Close();

	// notify about change (after Close many
	// Streams become unaccessable)
	TRY( OnChange( this ) );

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Clears all buffers for this stream and causes any buffered data to
/// be written to the underlying device.
/// </summary>
//-------------------------------------------------------------------
void StreamWrapper::Flush( void )
{ENTER(_lock_this)

	m_stream->Flush();

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Reads a sequence of bytes from the current stream and advances the
/// position within the stream by the number of bytes read.
/// </summary>
//-------------------------------------------------------------------
int StreamWrapper::Read( array<unsigned char> ^buffer, int offset, int count )
{ENTER(_lock_this)

	return m_stream->Read( buffer, offset, count );

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Reads a byte from the stream and advances the position within the
/// stream by one byte, or returns -1 if at the end of the stream.
/// </summary>
//-------------------------------------------------------------------
int StreamWrapper::ReadByte( void )
{ENTER(_lock_this)

	return m_stream->ReadByte();

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Sets the position within the current stream.
/// </summary>
//-------------------------------------------------------------------
__int64 StreamWrapper::Seek( __int64 offset, SeekOrigin origin )
{ENTER(_lock_this)

	return m_stream->Seek( offset, origin );

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Sets the length of the current stream.
/// </summary>
//-------------------------------------------------------------------
void StreamWrapper::SetLength( __int64 value )
{ENTER(_lock_this)

	m_stream->SetLength( value );

	// notify about change (only succeded changes
	// will be notified)
	TRY( OnChange( this ) );

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Writes a sequence of bytes to the current stream and advances the
/// current position within this stream by the number of bytes written.
/// </summary>
//-------------------------------------------------------------------
void StreamWrapper::Write( array<unsigned char> ^buffer, int offset, int count )
{ENTER(_lock_this)

	m_stream->Write( buffer, offset, count );
	
	// notify about change (only succeded changes
	// will be notified)
	TRY( OnChange( this ) );

EXIT(_lock_this)}


//-------------------------------------------------------------------
/// <summary>
/// Writes a byte to the current position in the stream and advances
/// the position within the stream by one byte.
/// </summary>
//-------------------------------------------------------------------
void StreamWrapper::WriteByte( unsigned char value )
{ENTER(_lock_this)

	m_stream->WriteByte( value );

	// notify about change (only succeded changes
	// will be notified)
	TRY( OnChange( this ) );

EXIT(_lock_this)}