//****************************************************************************
//*   Project:   RPL.Storage
//*
//*   Module:      SqlStream.cs
//*
//*   Content:   Exposes a System.IO.Stream around a SQL image data type
//*   Author:      Alexander Kurbatov
//*   Copyright:   Copyright © 2006-2007 Alexander Kurbatov
//*
//*   This class allows to use SQL image data type as System.IO.Stream. 
//*   Modification of stream is implemented via temporary file which is automaticly disposed upon 
//*   stream dispose.
//*
//****************************************************************************
using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Data.Common;
using System.Data.SqlClient;
using System.Data;
using System.Diagnostics;

namespace RPL.Storage {

///<summary>Implements Stream on SQL Image value</summary>
public class SqlStream : Stream
{
	private long m_position; //store position of stream
	private int m_ID;	     //Store id of row
	private bool disposed = false;       //Track whether Dispose has been called.
	private TempFileStream m_tf = null;  //backup FileStream
	private int BUFFER_LENGTH = 1024 * 1024; //chunk size
	//instance of object throught which all SqlStream requests are handled
	private CSqlImageService m_SqlImageService;


	#region Implementation Of Abstract Stream
	///<summary>Gets a value indicating whether the current stream supports reading.</summary>
	public override bool CanRead
	{
		get { //check whether object is closed
			if ( disposed )
				throw new ObjectDisposedException( this.GetType().FullName );
			return true;
		}
	}

	///<summary>Gets a value indicating whether the current stream supports seeking.</summary>
	public override bool CanSeek
	{
		get { //check whether object is closed
			if ( disposed )
				throw new ObjectDisposedException( this.GetType().FullName );
			return true;
		}
	}

	///<summary>Gets a value indicating whether the current stream supports writing</summary>
	public override bool CanWrite
	{
		get {//check whether object is closed
			if ( disposed )
				throw new ObjectDisposedException( this.GetType().FullName );
			return true;
		}
	}

	///<summary>Not supported</summary>
	public override void Flush()
	{
		throw new NotSupportedException( "Flush not supported" );
	}

	///<summary>Gets the length in bytes of the stream</summary>
	public override long Length
	{
		get {
			//check whether object is closed
			if ( disposed )
				throw new ObjectDisposedException( this.GetType().FullName );
			
			// check that image isn't backuped
			if ( m_tf == null ) {
				return m_SqlImageService.Length( m_ID );
			} else {
				return m_tf.Length;
			}
		}
	}

	///<summary>Gets or sets the position within the current stream</summary>
	public override long Position
	{
		get {
			//check whether object is closed
			if ( disposed )
				throw new ObjectDisposedException( this.GetType().FullName );
			return m_position;
		}
		set {
			//check whether object is closed
			if ( disposed )
				throw new ObjectDisposedException( this.GetType().FullName );
			m_position = value;
		}
	}

	///<summary>Reads a sequence of bytes from the current stream
	/// and advances the position within the stream by the number of bytes read</summary>
	///<param name="buffer">An array of bytes. When this method returns, the buffer contains the specified byte array with the values between offset and (offset + count - 1) replaced by the bytes read from the current source</param>
	///<param name="offset">The zero-based byte offset in buffer at which to begin storing the data read from the current stream.</param>
	///<param name="count">The maximum number of bytes to be read from the current stream. </param>
	///<returns>The total number of bytes read into the buffer. This can be less than the number of bytes requested if that many bytes are not currently available, or zero (0) if the end of the stream has been reached.</returns>
	public override int Read( byte[] buffer, int offset, int count )
	{
		#region in param check
		if ( buffer == null )
			throw new ArgumentNullException( "buffer" );
		if ( offset < 0 )
			throw new ArgumentOutOfRangeException( "offset", "< 0" );
		if ( count < 0 )
			throw new ArgumentOutOfRangeException( "count", "< 0" );
		if ( offset > buffer.Length )
			throw new ArgumentException( "destination offset is beyond array size" );
		#endregion

		//check whether object is closed
		if ( disposed )
			throw new ObjectDisposedException( this.GetType().FullName );

		// check that image isn't backuped
		if ( m_tf == null) {
			//read from db and save current position depending on read count returned
			int ret_count = m_SqlImageService.Read( m_ID, buffer, offset, count, m_position );
			//save stream position
			m_position += ret_count;
			//return read count
			return ret_count;
		} else {
			//check that stream can 
			if (m_tf.CanRead) {
				if ( m_position > m_tf.Length )
					throw new ArgumentOutOfRangeException( "Position", " > image length" );
				//change current position pointer in FileStream
				m_tf.Position = m_position;
				//read from it
				return m_tf.Read( buffer, offset, count );
			} else { throw new ArgumentException( "Can't read on stream" ); }
		}
	}

	///<summary>Sets the position within the current stream.</summary>
	///<param name="offset">A byte offset relative to the origin parameter.</param>
	///<param name="origin">A value of type SeekOrigin indicating the reference point used to obtain the new position.</param>
	///<returns>The new position within the current stream.</returns>
	public override long Seek( long offset, SeekOrigin origin )
	{
		//check whether object is closed
		if ( disposed )
			throw new ObjectDisposedException( this.GetType().FullName );

		long tempPos = m_position;
		// case needed opearation
		switch ( origin ) {
			case SeekOrigin.Begin :
				m_position = offset;
				break;
			default:
				m_position += offset;
				break;
		}
		// check that value meet stream boundaries
		if ((m_position < 0) || (m_position > Length)){
			// restore previous value
			m_position = tempPos;
			throw new System.IO.IOException( "offset" );
		}
		return m_position;
	}

	///<summary>Sets the length of the current stream.</summary>
	///<param name="value">The desired length of the current stream in bytes.</param>
	public override void SetLength( long value )
	{
		// check that image isn't backuped
		if ( (m_tf == null) )
			//need to create backup
			backupImage();

		// readressing request to FileStream
		m_tf.SetLength( value );
	}

	///<summary>Writes a sequence of bytes to the current stream and
	///advances the current position within this stream by the number of bytes written.</summary>
	///<param name="buffer">An array of bytes. This method copies count bytes from buffer to the current stream.</param>
	///<param name="offset">The zero-based byte offset in buffer at which to begin copying bytes to the current stream.</param>
	///<param name="count">The number of bytes to be written to the current stream.</param>
	public override void Write( byte[] buffer, int offset, int count )
	{
		//check whether object is closed
		if ( disposed )
			throw new ObjectDisposedException( this.GetType().FullName );
		if ( (offset < 0) || (count < 0) )
			throw new ArgumentOutOfRangeException();
		if ( offset + count > buffer.Length )
			throw new ArgumentException();
		// check that image isn't backuped
		if ( (m_tf == null) ) 
			//need to create backup
			backupImage();

		// readressing request to FileStream
		m_tf.Position = m_position;
		m_tf.Write( buffer, offset, count );
		m_position = m_tf.Position;
	}
	#endregion

	///<param name="ID">ID of value</param>
	///<param name="dbcon">SqlConnection to DB</param>
	public SqlStream( int ID, CSqlImageService SqlImageService )
	{
		// check input parametrs
		if ( ID <= 0 ) throw new ArgumentOutOfRangeException( "ID", "<= 0");
		if ( SqlImageService == null )
			throw new ArgumentNullException( "SqlImageService", "Invalid object passed." );

		#region debug info
#if (DEBUG)
		Debug.Print( "-> SqlStream.con( {0}, {1} ) @ \n\t{2}", ID, SqlImageService.ToString(), AppDomain.CurrentDomain.FriendlyName );
#endif
		#endregion

		m_ID = ID;
		m_SqlImageService = SqlImageService;

		#region debug info
#if (DEBUG)
		Debug.Print( "<- SqlStream.con" );
#endif
		#endregion

	}

	///<summary>Returns ID of stream</summary>
	///<returns>id of row where stream property is saved</returns>
	public int ID {
		get {
			return m_ID;
		}
	}

	private void backupImage() {
		//need to create backup
		m_tf = new TempFileStream();
		Byte[] buffer = new byte[BUFFER_LENGTH];

		int count;
		long prev_position = m_position;
		this.m_position = 0;
		try {
			while ( (count = this.Read( buffer, 0, BUFFER_LENGTH )) > 0 ) {
				m_tf.Write( buffer, 0, count );
				m_tf.Flush();
			}
		} finally { m_position = prev_position; }
	}

	~SqlStream()
	{
		Dispose( false );
	}

	///<param name="disposing">true to release both managed and unmanaged resources; false to release only unmanaged resources.</param>
	protected override void Dispose( bool disposing )
	{
		lock ( this ) {
			try {
				if ( disposing ) {
					if ( m_tf != null )
						m_tf.Dispose();
				}
				disposed = true;
			} finally {
				// Call Dispose on your base class.
				base.Dispose( disposing );
			}
		}
	}

	///<summary>Class to create temporary file that will be deleted upon finalization of object
	///or explicit Dispose() invoke</summary>
	private class TempFileStream : FileStream
	{
		String filename;
		private bool disposed = true; //this value to not suppress Dispose(bool)

		///<summary>Creates temp file using system GetTempFile</summary>
		public TempFileStream(): base(System.IO.Path.GetTempFileName(), FileMode.Create)
		{
			disposed = false; // indicate that constructor doesn't fail
			filename = base.Name; //store file name
		}

		///<param name="disposing">true to release both managed and unmanaged resources; false to release only unmanaged resources.</param>
		protected override void Dispose( bool disposing )
		{
			lock ( this ) {
				base.Dispose( disposing ); //it will close all handle to file
				if ( !disposed ) {
					File.Delete( filename ); //delete temp file
					disposed = true; //to allow multiple Dispose(bool) invoke
				}
			}
		}
	}
}
}