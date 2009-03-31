/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentStream.cpp										*/
/*																			*/
/*	Content:	Implementation of PersistentStream class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008 Alexey Tkachuk								*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentStream.h"

using namespace _RPL;


//
// Define macro to silent file deletion.
//
#define DELETE_FILE(path)													\
try {																		\
	File::Delete( path );													\
} catch( IOException ^e ) {													\
	System::Diagnostics::Trace::WriteLine( String::Format(					\
	ERR_DELETE_FILE, path, e->Message ) );									\
}


//-----------------------------------------------------------------------------
//				Toolkit::RPL::PersistentStream::StreamWrapper
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Initializes a new instance of the StreamWrapper class for the
// specified owner.
//
//-------------------------------------------------------------------
PersistentStream::										  \
StreamWrapper::StreamWrapper( PersistentStream ^owner ) : \
	_owner(owner)
{
	if( _owner == nullptr ) throw gcnew ArgumentNullException("owner");
}

//-------------------------------------------------------------------
//
// Class disposer.
//
//-------------------------------------------------------------------
PersistentStream::StreamWrapper::~StreamWrapper( void )
{
	delete _owner;
}


//-------------------------------------------------------------------
//
// Gets a value indicating whether the current stream supports
// reading.
//
//-------------------------------------------------------------------
bool PersistentStream::StreamWrapper::CanRead::get( void )
{
	return _owner->CanRead;
}


//-------------------------------------------------------------------
//
// Gets a value indicating whether the current stream supports
// seeking.
//
//-------------------------------------------------------------------
bool PersistentStream::StreamWrapper::CanSeek::get( void )
{
	return _owner->CanSeek;
}


//-------------------------------------------------------------------
//
// Gets a value indicating whether the current stream supports
// writing.
//
//-------------------------------------------------------------------
bool PersistentStream::StreamWrapper::CanWrite::get( void )
{
	return _owner->CanWrite;
}


//-------------------------------------------------------------------
//
// Gets the length in bytes of the stream.
//
//-------------------------------------------------------------------
__int64 PersistentStream::StreamWrapper::Length::get( void )
{
	return _owner->Length;
}


//-------------------------------------------------------------------
//
// Gets or sets the position within the current stream.
//
//-------------------------------------------------------------------
__int64 PersistentStream::StreamWrapper::Position::get( void )
{
	return _owner->Position;
}

void PersistentStream::StreamWrapper::Position::set( __int64 value )
{
	_owner->Position = value;
}
	

//-------------------------------------------------------------------
//
// Clears all buffers for this stream and causes any buffered data to
// be written to the underlying device.
//
//-------------------------------------------------------------------
void PersistentStream::StreamWrapper::Flush( void )
{
	_owner->Flush();
}


//-------------------------------------------------------------------
//
// Reads a sequence of bytes from the current stream and advances the
// position within the stream by the number of bytes read.
//
//-------------------------------------------------------------------
int PersistentStream::
StreamWrapper::Read( [In] [Out] array<unsigned char> ^buffer,
					 int offset, int count )
{
	return _owner->Read( buffer, offset, count );
}


//-------------------------------------------------------------------
//
// Reads a byte from the stream and advances the position within the
// stream by one byte, or returns -1 if at the end of the stream.
//
//-------------------------------------------------------------------
int PersistentStream::StreamWrapper::ReadByte( void )
{
	return _owner->ReadByte();
}


//-------------------------------------------------------------------
//
// Sets the position within the current stream.
//
//-------------------------------------------------------------------
__int64 PersistentStream::
StreamWrapper::Seek( __int64 offset, SeekOrigin origin )
{
	return _owner->Seek( offset, origin );
}


//-------------------------------------------------------------------
//
// Sets the length of the current stream.
//
//-------------------------------------------------------------------
void PersistentStream::StreamWrapper::SetLength( __int64 value )
{
	_owner->SetLength( value );
}


//-------------------------------------------------------------------
//
// Writes a sequence of bytes to the current stream and advances the
// current position within this stream by the number of bytes
// written.
//
//-------------------------------------------------------------------
void PersistentStream::
StreamWrapper::Write( array<unsigned char> ^buffer, int offset, int count )
{
	_owner->Write( buffer, offset, count );
}


//-------------------------------------------------------------------
//
// Writes a byte to the current position in the stream and advances
// the position within the stream by one byte.
//
//-------------------------------------------------------------------
void PersistentStream::StreamWrapper::WriteByte( unsigned char value )
{
	return _owner->WriteByte( value );
}


//-------------------------------------------------------------------
//
// Returns a String that represents the current Object.
//
//-------------------------------------------------------------------
String^ PersistentStream::StreamWrapper::ToString( void )
{
	return _owner->ToString();
}


//-----------------------------------------------------------------------------
//						Toolkit::RPL::PersistentStream
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Check for stream being in the correct state.
//
//-------------------------------------------------------------------
void PersistentStream::check_state( void )
{
	// check for disposed stream
	if( m_disposed ) throw gcnew ObjectDisposedException(
		this->GetType()->ToString(), ERR_STREAM_CLOSED);
}


//-------------------------------------------------------------------
//
// ITransaction::Begin implementation.
//
// Stores all stream's data until trans_commit will be called to have
// ability to restore data by trans_rollback.
//
//-------------------------------------------------------------------
void PersistentStream::trans_begin( void )
{
	// create new temporary file
	String	^path = Path::GetTempFileName();
	// and save stream content to it
	ExportToFile( path );

	// create backup record
	RESTORE_POINT	point;
	// fill it by current data
	point._path = path;
	point._position = m_fs->Position;
	// push record to stack
	backup.Push( point );
}


//-------------------------------------------------------------------
//
// ITransaction::Commit implementation.
//
// Transaction was completed successfuly, so free backup record.
//
//-------------------------------------------------------------------
void PersistentStream::trans_commit( void )
{
	// remove top record from stack
	RESTORE_POINT	point = backup.Pop();
	// delete temporary file
	DELETE_FILE( point._path );
}


//-------------------------------------------------------------------
//
// ITransaction::Rollback implementation.
//
// Transaction fails, so rollback stream to previous state.
//
//-------------------------------------------------------------------
void PersistentStream::trans_rollback( void )
{
	// close internal stream
	m_fs->Close();

	// get top record from stack
	RESTORE_POINT	point = backup.Pop();
	// restore previous state
	m_path = point._path;
	m_fs = gcnew FileStream(m_path, FileMode::OpenOrCreate,
							FileAccess::ReadWrite, FileShare::None, 8,
							FileOptions::DeleteOnClose);
	m_fs->Position = point._position;
}


//-------------------------------------------------------------------
//
// Creates a new instance of the PersistentStream class using
// serialization info.
//
//-------------------------------------------------------------------
PersistentStream::PersistentStream( SerializationInfo ^info,
								    StreamingContext context )
{
	// check for null reference
	if( info == nullptr ) throw gcnew ArgumentNullException("info");

	// get path to temporary directory
	String				^tp = Path::GetTempPath();
	// and assert write permission to it
	FileIOPermission	^io = gcnew FileIOPermission(
										FileIOPermissionAccess::Read |
										FileIOPermissionAccess::Write, tp);
	io->Assert();
	try {
		// create empty temporary file
		m_path = Path::GetTempFileName();
		// open this file with FileStream
		m_fs = gcnew FileStream(m_path, FileMode::OpenOrCreate,
								FileAccess::ReadWrite, FileShare::None, 8,
								FileOptions::DeleteOnClose);

		// restore stream content
		array<unsigned char>	^buf = static_cast<array<unsigned char>^>(
											info->GetValue( "content",
											array<unsigned char>::typeid ) );
		m_fs->Write( buf, 0, buf->Length );
		// and stream position
		m_fs->Position = info->GetInt64( "position" );
	} finally {
		// revert write assert
		FileIOPermission::RevertAssert();
	}
}


//-------------------------------------------------------------------
//
// ISerializable::GetObjectData implementation.
//
// Populates a System.Runtime.Serialization.SerializationInfo with
// the data needed to serialize the target object.
//
//-------------------------------------------------------------------
void PersistentStream::get_object_data( SerializationInfo ^info,
										StreamingContext context )
{
	// check for null reference
	if( info == nullptr ) throw gcnew ArgumentNullException("info");
	// check stream state
	check_state();
	// check for the stream length
	if( m_fs->Length > int::MaxValue ) throw gcnew SerializationException(
		ERR_STREAM_LENGTH);

	// allocate memory for stream content
	array<unsigned char>	^buf = gcnew array<unsigned char>(
										(int) m_fs->Length);
	// save current position
	__int64		pos = m_fs->Position;
	// copy stream content to the buffer
	m_fs->Seek( 0, SeekOrigin::Begin );
	m_fs->Read( buf, 0, buf->Length );
	// restore position
	m_fs->Position = pos;
	
	// save serialization data
	info->AddValue( "content",  buf );
	info->AddValue( "position", pos );
}


//-------------------------------------------------------------------
//
// on_change event implementation.
//
// Overrides the default event implementation to allow only one
// subscription (it means that stream is assigned to only one
// property).
//
//-------------------------------------------------------------------
void PersistentStream::on_change::add( ON_CHANGE ^d )
{
	// check for empty invocation list
	if( m_on_change != nullptr ) {
		// throw exception
		throw gcnew InvalidOperationException(ERR_STREAM_ASSIGN);
	}
	m_on_change += d;
}

void PersistentStream::on_change::remove( ON_CHANGE ^d )
{
	m_on_change -= d;
}

void PersistentStream::on_change::raise( PersistentStream ^sender )
{
	if( m_on_change != nullptr ) {
		// perform all delegates in event
		m_on_change->Invoke( sender );
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Initializes a new instance of the PersistentStream class with
/// empy content.
/// </summary>
//-------------------------------------------------------------------
PersistentStream::PersistentStream( void ) : \
	m_path(nullptr), m_disposed(false)
{
	// create empty temporary file
	m_path = Path::GetTempFileName();
	// open this file with FileStream
	m_fs = gcnew FileStream(m_path, FileMode::OpenOrCreate,
							FileAccess::ReadWrite, FileShare::None, 8,
							FileOptions::DeleteOnClose);
}


//-------------------------------------------------------------------
/// <summary>
/// Initializes a new instance of the PersistentStream class with the
/// specified file content.
/// </summary>
//-------------------------------------------------------------------
PersistentStream::PersistentStream( String ^path ) : \
	m_path(nullptr), m_disposed(false)
{
	// create empty temporary file
	m_path = Path::GetTempFileName();
	// open this file with FileStream
	m_fs = gcnew FileStream(m_path, FileMode::OpenOrCreate,
							FileAccess::ReadWrite, FileShare::None, 8,
							FileOptions::DeleteOnClose);
	
	try {
		// open specified file
		FileStream	^fs = gcnew FileStream(path,
										   FileMode::Open, FileAccess::Read);
		// and copy it's content to the internal stream
		array<unsigned char>	^buf = gcnew array<unsigned char>(1024*1024);
		for( int size = 0;
			 size = fs->Read( buf, 0, buf->Length );
			 m_fs->Write( buf, 0, size ) );
		// close source stream
		fs->Close();
		// set position to the begin of the stream
		m_fs->Seek( 0, SeekOrigin::Begin );
	} catch( Exception^ ) {
		// close internal stream
		m_fs->Close();
		// and restore exception
		throw;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Initializes a new instance of the PersistentStream class based on
/// the specified byte array.
/// </summary>
//-------------------------------------------------------------------
PersistentStream::PersistentStream( array<unsigned char> ^buffer ) : \
	m_path(nullptr), m_disposed(false)
{
	// create empty temporary file
	m_path = Path::GetTempFileName();
	// open this file with FileStream
	m_fs = gcnew FileStream(m_path, FileMode::OpenOrCreate,
							FileAccess::ReadWrite, FileShare::None, 8,
							FileOptions::DeleteOnClose);

	// copy buffer's content to internal stream
	m_fs->Write( buffer, 0, buffer->Length );
	// set position to the begin of the stream
	m_fs->Seek( 0, SeekOrigin::Begin );
}


//-------------------------------------------------------------------
/// <summary>
/// Class disposer.
/// </summary><remarks>
/// Disposes internal stream and call finalizer.
/// </remarks>
//-------------------------------------------------------------------
PersistentStream::~PersistentStream( void )
{
	if( !m_disposed ) {
		// dispose internal stream
		delete m_fs;
		// call finalizer to clean up unmanaged resources
		this->!PersistentStream();
		// prevent from future cals
		m_disposed = true;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Class finalizer.
/// </summary><remarks>
/// Removes all temporary files (ignore IO exceptions).
/// </remarks>
//-------------------------------------------------------------------
PersistentStream::!PersistentStream( void )
{
	if( !m_disposed ) {
		// check for the null reference (finalizer 
		// calls even object creation failed)
		if( %backup != nullptr ){
			// if contains unfinished transactions
			while( backup.Count > 0 ) {
				// delete transaction files
				DELETE_FILE( backup.Pop()._path );
			}
		}
		// prevent from future cals
		m_disposed = true;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Implicit cast operator from PersistentStream to Stream.
/// </summary><remarks>
/// This operator creates inherited from Stream proxy object that
/// pass all calls to the specified PersistentStream instance.
/// </remarks>
//-------------------------------------------------------------------
PersistentStream::operator Stream^( PersistentStream ^ps )
{
	if( ps == nullptr ) return nullptr;

	return gcnew StreamWrapper(ps);
}


//-------------------------------------------------------------------
/// <summary>
/// Gets a value indicating whether the current stream supports
/// reading.
/// </summary>
//-------------------------------------------------------------------
bool PersistentStream::CanRead::get( void )
{
	return m_fs->CanRead;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets a value indicating whether the current stream supports
/// seeking.
/// </summary>
//-------------------------------------------------------------------
bool PersistentStream::CanSeek::get( void )
{
	return m_fs->CanSeek;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets a value indicating whether the current stream supports
/// writing.
/// </summary>
//-------------------------------------------------------------------
bool PersistentStream::CanWrite::get( void )
{
	return m_fs->CanWrite;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets the length in bytes of the stream.
/// </summary>
//-------------------------------------------------------------------
__int64 PersistentStream::Length::get( void )
{
	// check stream state
	check_state();

	return m_fs->Length;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets the position within the current stream.
/// </summary>
//-------------------------------------------------------------------
__int64 PersistentStream::Position::get( void )
{
	// check stream state
	check_state();

	return m_fs->Position;
}

void PersistentStream::Position::set( __int64 value )
{
	// check stream state
	check_state();

	m_fs->Position = value;
}


//-------------------------------------------------------------------
/// <summary>
/// Stores stream content to the specified file.
/// </summary><remarks>
/// If the file already exists, it will be overwritten.
/// </remarks>
//-------------------------------------------------------------------
void PersistentStream::ExportToFile( String ^path )
{
	// open specified file for writing
	FileStream	^fs = gcnew FileStream(path,
									   FileMode::Create, FileAccess::Write);
	// store current position
	__int64		pos = m_fs->Position;
	try {
		// seek to begin of the internal stream
		m_fs->Seek( 0, SeekOrigin::Begin );
		// and copy it's content to the new stream
		array<unsigned char>	^buf = gcnew array<unsigned char>(1024*1024);
		for( int size = 0;
			 size = m_fs->Read( buf, 0, buf->Length );
			 fs->Write( buf, 0, size ) );
	} finally {
		// close output stream
		fs->Close();
		// restore internal stream position
		m_fs->Position = pos;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Clears all buffers for this stream and causes any buffered data to
/// be written to the underlying device.
/// </summary>
//-------------------------------------------------------------------
void PersistentStream::Flush( void )
{
	// check stream state
	check_state();

	m_fs->Flush();
}


//-------------------------------------------------------------------
/// <summary>
/// Reads a sequence of bytes from the current stream and advances
/// the position within the stream by the number of bytes read.
/// </summary>
//-------------------------------------------------------------------
int PersistentStream::Read( [In] [Out] array<unsigned char> ^buffer,
							int offset, int count )
{
	// check stream state
	check_state();

	return m_fs->Read( buffer, offset, count );
}


//-------------------------------------------------------------------
/// <summary>
/// Reads a byte from the stream and advances the position within the
/// stream by one byte, or returns -1 if at the end of the stream.
/// </summary>
//-------------------------------------------------------------------
int PersistentStream::ReadByte( void )
{
	// check stream state
	check_state();
	
	return m_fs->ReadByte();
}


//-------------------------------------------------------------------
/// <summary>
/// Sets the position within the current stream.
/// </summary>
//-------------------------------------------------------------------
__int64 PersistentStream::Seek( __int64 offset, SeekOrigin origin )
{
	// check stream state
	check_state();
	
	return m_fs->Seek( offset, origin );
}


//-------------------------------------------------------------------
/// <summary>
/// Sets the length of the current stream.
/// </summary>
//-------------------------------------------------------------------
void PersistentStream::SetLength( __int64 value )
{
	// check stream state
	check_state();
	// save length before action
	__int64	length = m_fs->Length;

	m_fs->SetLength( value );

	// notify about content change
	if( length != m_fs->Length ) on_change( this );
}


//-------------------------------------------------------------------
/// <summary>
/// Writes a sequence of bytes to the current stream and advances the
/// current position within this stream by the number of bytes
/// written.
/// </summary>
//-------------------------------------------------------------------
void PersistentStream::Write( array<unsigned char> ^buffer, \
							  int offset, int count )
{
	// check stream state
	check_state();
	// save position before action
	__int64	pos = m_fs->Position;

	m_fs->Write( buffer, offset, count );

	// notify about content change
	if( pos != m_fs->Position ) on_change( this );
}


//-------------------------------------------------------------------
/// <summary>
/// Writes a byte to the current position in the stream and advances
/// the position within the stream by one byte.
/// </summary>
//-------------------------------------------------------------------
void PersistentStream::WriteByte( unsigned char value )
{
	// check stream state
	check_state();
	// save position before action
	__int64	pos = m_fs->Position;

	m_fs->WriteByte( value );

	// notify about content change
	if( pos != m_fs->Position ) on_change( this );
}


//-------------------------------------------------------------------
/// <summary>
/// Returns a String that represents the current Object.
/// </summary><remarks>
/// Overrides standart object method ToString() to provide special
/// value for stream data.
/// </remarks>
//-------------------------------------------------------------------
String^ PersistentStream::ToString( void )
{
	return "<binary data>";
}
