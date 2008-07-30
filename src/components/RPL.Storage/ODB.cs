//****************************************************************************
//*
//*	Project		:	Robust Persistence Layer
//*
//*	Module		:	ODB.cs
//*
//*	Content		:	Implements object oriented DB manipulation
//*	Author		:	Alexander Kurbatov
//*	Copyright	:	Copyright © 2006-2008 Alexander Kurbatov
//*
//*	Implement Search, Retrive, Save, Delete of PersistentObject in MS SQL DB
//*
//****************************************************************************
using System;
using System.IO;
using System.Data;
using System.Data.SqlClient;
using System.Collections.Generic;
using System.Text.RegularExpressions;

#if (DEBUG)
using System.Diagnostics;
#endif


namespace Toolkit.RPL.Storage
{
/// <summary>
/// Object Oriented DB implementation
/// </summary>
public class ODB : IPersistenceStorage
{
	// connection on which all commands are executed
	readonly SqlConnection m_con;
	private SqlTransaction m_trans = null;
	// number of currently opened transactions
	private int m_TransactionCount = 0;
	private const int BUFFER_LENGTH = 1024 * 1024;

	#region regexps for search query translation
	private const string REGEXP_BEGIN = @"(?i)(?:\((?:\s*)";
	private const string REGEXP_END = @"(?:\s*))\)";

	// pattern specifies following SQL predicate:
	// expression { = | < > | ! = | > | > = | ! > | < | < = | ! < } expression
	private const string REGEXP_PRED_COMPARE = REGEXP_BEGIN +
		@"(?<prop>\w+)(?:\s*)" +
		@"(?<cond>(?<op>(?:=|<>|!=|>|>=|!>|<|<=|!<))(?:\s*)" +
		@"(?<exp>(?<char_exp>\'.*?\')" +
				"|" +
				@"(?<digit_exp>\-?\d+((\.|\,)\d+)?)" +
				"|" +
				@"(?<sub_exp>CAST\s*\(.+?\))" + // catch CAST construction
				"|" +
				@"(?<sub_exp>CONVERT\s*\(.+?\))" + // catch CONVERT construction
		"))" +
		REGEXP_END;
	// pattern specifies following SQL predicate:
	// string_expression [ NOT ] LIKE string_expression
	private const string REGEXP_PRED_LIKE = REGEXP_BEGIN +
		@"(?<prop>\w+)(?:\s+)" +
		@"(?<cond>(?<op>(?:NOT)?(?:\s+)?(?:LIKE))(?:\s+)" +
		@"(?<exp>(?<char_exp>\'.*?\')" +
				"|" +
				@"(?<digit_exp>\-?\d+((\.|\,)\d+)?)" +
				"|" +
				@"(?<sub_exp>CAST\s*\(.+?\))" + // catch CAST construction
				"|" +
				@"(?<sub_exp>CONVERT\s*\(.+?\))" + // catch CONVERT construction
		"))" +
		REGEXP_END;
	// pattern specifies following SQL predicate: 
	// expression IS [ NOT ] NULL 
	private const string REGEXP_PRED_ISNULL = REGEXP_BEGIN +
		@"(?<prop>\w+)(?:\s+)(?<cond>(?<op>(?:(?:IS)(?:\s+)(?:NOT(?:\s+))?))(?<exp>(?:NULL)))" +
		REGEXP_END;
	// pattern specifies combination of all previous pattern
	private const string REGEXP_PREDS = REGEXP_PRED_COMPARE + "|" +
										REGEXP_PRED_LIKE + "|" +
										REGEXP_PRED_ISNULL;

	private const string REGEXP_CHECK =
		@"^(((?:\s*)(?<neg>NOT)?(?:\s*)(" +
		REGEXP_PREDS + @")(?:\s*)(?<modif>AND|OR)))*(?:\s*)(?<neg>NOT)?(?:\s*)(?:\s*)(" +
		REGEXP_PREDS + @")(?:\s)*$";
	#endregion
	
	#region error messages
	private static string ERROR_CHANGED_OBJECT = "Newer object exist. Please retrive object first!";
	private static string ERROR_IMAGE_IS_ABSENT = "Specified value is absent!";
	#endregion

	///////////////////////////////////////////////////////////////////////
	//						Private Section
	///////////////////////////////////////////////////////////////////////
	/// <summary>
	/// gets saved object properties
	/// </summary>
	/// <param name="id">ID of object</param>
	/// <returns>HEADER of requested object</returns>
	private HEADER getHeader( int id )
	{
		SqlCommand cmdSql =
			new SqlCommand(
				"SELECT @Name = [ObjectName], @Type = [ObjectType], @Stamp = [TimeStamp] " +
				"FROM [dbo].[_objects] " +
				"WHERE [ID] = " + id, m_con, m_trans);
		cmdSql.Parameters.Add( "@Name", SqlDbType.NVarChar, 4000 )
			.Direction = ParameterDirection.Output;
		cmdSql.Parameters.Add( "@Type", SqlDbType.NVarChar, 4000 )
			.Direction = ParameterDirection.Output;
		cmdSql.Parameters.Add( "@Stamp", SqlDbType.DateTime )
			.Direction = ParameterDirection.Output;
		try {
			cmdSql.ExecuteNonQuery();

			// return new header
			return new HEADER(
						(string) cmdSql.Parameters["@Type"].Value,
						id,
						(DateTime) cmdSql.Parameters["@Stamp"].Value,
						(string) cmdSql.Parameters["@Name"].Value);
		} catch {
			throw new ArgumentException("Object with id = " + id + 
				" doesn't exist in DB!");
		}
	}

	/// <param name="Predicate">EX. (Prop1 != value ) OR (Prop1 [NOT] IS NULL) ...</param>
	/// <returns>SQL predicate that tuned to DB structure</returns>
	private string tunePridicate( string Predicate )
	{
		string prop = "";
		string query = "";


		// decompile input string using RegExp.
		Match match = 
			Regex.Match( Predicate,
				Regex.IsMatch( Predicate, REGEXP_PRED_COMPARE ) 
					?
					REGEXP_PRED_COMPARE 
					: 
					(Regex.IsMatch( Predicate, REGEXP_PRED_ISNULL ) 
						? 
						REGEXP_PRED_ISNULL 
						:
						REGEXP_PRED_LIKE) 
					);

		// check for proxy object properties
		if( match.Groups["prop"].Value.Equals( "ID", 
			StringComparison.CurrentCultureIgnoreCase ) ) {
			// doesn't need changing
			return Predicate;
		} else if( match.Groups["prop"].Value.Equals( "NAME",
			StringComparison.CurrentCultureIgnoreCase ) ) {
			// change column name
			return "( ObjectName " + match.Groups["cond"].Value + ")";
		} else {
			prop = (string.IsNullOrEmpty( 
					match.Groups["char_exp"].Value ) 
					? 
					"Value" 
					: 
					"CAST(Value as nvarchar(4000))");

			// modify query to match _properies table and it's structure
			query += "(EXISTS " +
						"(SELECT ID " +
						" FROM [dbo].[_properties] AS p " + // Image comparison is unsupported
						" WHERE " +
							"(ObjectID = Source.ID) AND " + // declare object id property belongs
				// avoid implicit type conversion by MS SQL
				// if non digit or string value is compareted
								(string.IsNullOrEmpty( match.Groups["sub_exp"].Value ) ?
									""
									:
									"(SQL_VARIANT_PROPERTY( " + 
									match.Groups["sub_exp"].Value + 
									", 'BaseType') = " +
									"SQL_VARIANT_PROPERTY( Value, 'BaseType')) AND " ) +
							"(Name = '" + match.Groups["prop"] + "') AND (" +// name of Property
				// workaround for comparison of string type property values
							prop +
							" " + match.Groups["cond"] + ")))"; // Value and comparison of property value
		}
		return query;
	}

	/// <summary>
	/// Converts DateType type according to MS SQL Server precision.
	/// </summary>
	/// <param name="dt">DateTime to be converted</param>
	/// <returns>DateTime rounded to .**0, .**3, .**7 milliseconds</returns>
	private DateTime dateTime2Sql( DateTime dt )
	{
		int remainder = dt.Millisecond % 10; // need to know last digit
		int add; // stores delta

		// get correction delta value for milliseconds
		if( remainder == 9 ) {
			add = 1;
		} else if( remainder >= 5 && remainder <= 8 ) {
			add = 7 - remainder;
		} else if( remainder >= 2 && remainder <= 4 ) {
			add = 3 - remainder;
		} else {
			add = -remainder;
		}

		// create new corrected DateTime
		return new DateTime(dt.Year, dt.Month, dt.Day,
							dt.Hour, dt.Minute, dt.Second, dt.Millisecond + add);
	}
	
	///////////////////////////////////////////////////////////////////////
	//						SQL BLOB Section
	///////////////////////////////////////////////////////////////////////
	#region SQL BLOB interactions
	/// <summary>
	/// Saves stream property to SQL BLOB field
	/// </summary>
	/// <param name="objID">Stream owner object ID</param>
	/// <param name="propName">Name of stream property</param>
	/// <param name="stream">Stream property</param>
	private void imageSave( int objID, string propName, PersistentStream stream )
	{
		#region debug info
#if (DEBUG)
		Debug.Print( "-> ODB.imageSave( {0}, '{1}' )", objID, propName );
#endif
		#endregion
		// open connection and start new transaction if required
		TransactionBegin();

		// command that creates new record or cleares existing
		SqlCommand CmdSql = new SqlCommand(
			@"IF (EXISTS(SELECT * FROM [dbo].[_images] " +
						"WHERE [ObjectID] = " + objID + " " +
						"AND " +
						"Name ='" + propName + "' )) " +
			"BEGIN " +
				"UPDATE [dbo].[_images] SET Value = " + ((stream.Length > 0) 
				? 
				"0x0" 
				: "NULL") + 
				" " +
				"WHERE [ObjectID] = " + objID + " " +
						"AND " +
						"Name ='" + propName + "'; " +
				"SELECT @Pointer = TEXTPTR(Value) " +
				"FROM [dbo].[_images] " +
				"WHERE [ObjectID] = " + objID + " " +
						"AND " +
						"Name ='" + propName + "'; " +
			"END " +
			"ELSE " +
			"BEGIN " +
				"INSERT INTO [dbo].[_images] " +
				"([ObjectID], [Name], [Value]) " +
				"VALUES ( " + objID + ", '" +
						  propName + "', " +
						  ((stream.Length > 0) ? "0x0" : "NULL") + " ); " +
				"SELECT @Pointer = TEXTPTR(Value) " +
				"FROM [dbo].[_images] " +
				"WHERE [ID] = SCOPE_IDENTITY(); " +
			"END", m_con, m_trans);

		SqlParameter pointerParam =
			CmdSql.Parameters.Add( "@Pointer", SqlDbType.VarBinary, 16 );
		pointerParam.Direction = ParameterDirection.Output;

		try {
			// get pointer to image data
			CmdSql.ExecuteNonQuery();
			// check that pointer exists
			if( pointerParam.Value == null )
				throw new KeyNotFoundException( "Can't save image!");
			// set up UPDATETEXT command, parameters, and open BinaryReader.	
			CmdSql = new SqlCommand(
				"UPDATETEXT [dbo].[_images].Value @Pointer @Offset @Delete " +
				"WITH LOG @Bytes", m_con, m_trans);
			// assign value of pointer previously recieved
			CmdSql.Parameters.Add( "@Pointer", SqlDbType.Binary, 16 ).Value =
				pointerParam.Value;
			SqlParameter OffsetParam =
				CmdSql.Parameters.Add( "@Offset", SqlDbType.Int );
			SqlParameter DeleteParam =
				CmdSql.Parameters.Add( "@Delete", SqlDbType.Int );
			DeleteParam.Value = 1; //delete 0x0 character
			SqlParameter BytesParam =
				CmdSql.Parameters.Add( "@Bytes", SqlDbType.Binary );
			stream.Seek( 0, SeekOrigin.Begin );

			OffsetParam.Value = 0; // start insertion from begin
			// read buffer full of data and execute UPDATETEXT statement.
			Byte[] Buffer = new Byte[BUFFER_LENGTH];
			// make first read from stream
			int ret = stream.Read( Buffer, 0, BUFFER_LENGTH );

			// while something is read from stream, write to apend to BLOB field
			while( ret > 0 ) {
				// initing parameters for write
				BytesParam.Value = Buffer;
				BytesParam.Size = ret;
				// write to BLOB field
				CmdSql.ExecuteNonQuery(); // execute iteration
				DeleteParam.Value = 0; // don't delete any other data
				// prepare to next iteration
				OffsetParam.Value =
					Convert.ToInt32( OffsetParam.Value ) + ret;
				// read from stream for next iteration
				ret = stream.Read( Buffer, 0, BUFFER_LENGTH );
			}
		} catch( Exception ex ) {
			#region debug info
#if (DEBUG)
			Debug.Print( "[ERROR] @ ODB.imageSave: " + ex.ToString() );
#endif
			#endregion
			// rollback failed transaction
			TransactionRollback();
			throw;
		}
		// close connection and commit transaction if required
		TransactionCommit();
		#region debug info
#if (DEBUG)
		Debug.Print( "<- ODB.imageSave(  {0}, '{1}' )", objID, propName );
#endif
		#endregion
	}

	/// <summary>
	/// Read BLOB field to stream property
	/// </summary>
	/// <param name="objID">Stream owner object ID</param>
	/// <param name="propName">Name of stream property</param>
	/// <param name="stream">Stream property</param>
	private void imageRead( int objID, string propName, ref PersistentStream stream )
	{
		#region debug info
#if (DEBUG)
		Debug.Print( "<- ODB.imageRead( {0}, '{1}' )", objID, propName );
#endif
		#endregion

		// get pointer to BLOB field using TEXTPTR.
		SqlCommand CmdSql = new SqlCommand(
			"SELECT @Pointer = TEXTPTR(Value), " +
			"@Length = DataLength(Value) " +
			"FROM [dbo].[_images] " +
			"WHERE [ObjectID] = " + objID + " " +
						"AND " +
						"Name ='" + propName + "'", m_con, m_trans);
		// setup parameters
		SqlParameter pointerParam = new SqlParameter("@Pointer", SqlDbType.VarBinary, 16);
		CmdSql.Parameters.Add( pointerParam ).Direction =
			ParameterDirection.Output;
		SqlParameter lengthParam = new SqlParameter("@Length", SqlDbType.Int);
		CmdSql.Parameters.Add( lengthParam ).Direction =
			ParameterDirection.Output;
		// open connection and start new transaction if required
		TransactionBegin();
		try {
			// get pointer and length of BLOB field
			CmdSql.ExecuteNonQuery();

			//check that BLOB field exists
			if( CmdSql.Parameters["@Pointer"].Value == null ) {
				throw new KeyNotFoundException( ERROR_IMAGE_IS_ABSENT );
			}

			// run the query.
			// set up the READTEXT command to read the BLOB by passing the following
			// parameters: @Pointer – pointer to blob, @Offset – number of bytes to
			// skip before starting the read, @Size – number of bytes to read.
			CmdSql = new SqlCommand(
				"READTEXT [dbo].[_images].Value " +
				"@Pointer @Offset @Size HOLDLOCK", m_con, m_trans);
			// set up the parameters for the command.
			CmdSql.Parameters.Add( "Pointer", SqlDbType.VarBinary, 16 ).Value =
				pointerParam.Value;

			
			// temp buffer for read/write purposes
			Byte[] buffer = new Byte[BUFFER_LENGTH];
			
			// current offset position
			SqlParameter offset = CmdSql.Parameters.Add( "@Offset", SqlDbType.Int );
			offset.Value = 0;
			SqlParameter size = CmdSql.Parameters.Add( "@Size", SqlDbType.Int );
			size.Value = 0;

			while( Convert.ToInt32(offset.Value) < Convert.ToInt32( lengthParam.Value ) ) {
				// calculate buffer size - may be less than BUFFER_LENGTH for last block.				
				if( (Convert.ToInt32( offset.Value ) + buffer.GetUpperBound( 0 )) 
					>= 
					Convert.ToInt32( lengthParam.Value ) )
					// setting size parameter
					size.Value =
						Convert.ToInt32( lengthParam.Value ) -
						Convert.ToInt32( offset.Value );
				else
					size.Value = buffer.GetUpperBound( 0 );

				// execute reader
				SqlDataReader dr =
					CmdSql.ExecuteReader( CommandBehavior.SingleRow );

				try {
					// read data from SqlDataReader
					dr.Read();
					// put data to buffer
					// and return size of read data
					int count = Convert.ToInt32(
						dr.GetBytes( 0, 0, buffer, 0, Convert.ToInt32( size.Value ) ) );
					// append buffer data to stream
					stream.Write( buffer, 0, count );
					// increment offset
					offset.Value = Convert.ToInt32( offset.Value ) + count;
				} finally { dr.Dispose(); /*dispose DataReader*/}
			}
		} catch( Exception ex ) {
			#region dubug info
#if (DEBUG)
			Debug.Print( "[ERROR] @ ODB.imageRead: " + ex.Message );
#endif
			#endregion
			// rollback failed transaction
			TransactionRollback();
			throw;
		}
		// close connection and commit transaction if required
		TransactionCommit();

		#region debug info
#if (DEBUG)
		Debug.Print( "<- ODB.imageRead( {0}, '{1}' )", objID, propName );
#endif
		#endregion
	}
	#endregion

	///////////////////////////////////////////////////////////////////////
	//						Public Section
	///////////////////////////////////////////////////////////////////////
	/// <summary>
	/// Default public constructor
	/// </summary>
	public ODB( string CnnStr )
	{
		#region debug info
#if (DEBUG)
		Debug.Print( "-> ODB..ctor()" );
#endif
		#endregion

		m_con = new SqlConnection(CnnStr);

		try {
			m_con.Open();
			m_con.Close();
		} catch( Exception ex ) {

#if (DEBUG)
			Debug.Print( "EXCEPTION @ ODB..ctor(): " + ex.ToString() );
#endif
			throw;
		}
		#region debug info
#if (DEBUG)
		Debug.Print( "<- ODB..ctor()" );
#endif
		#endregion
	}
	
	#region IPersistenceStorage Members
	/// <summary>
	/// Delete object with specified header from storage.
	/// </summary>
	/// <param name="header">Header value.</param>
	public void Delete( HEADER header )
	{
		#region debug info
#if (DEBUG)
		Debug.Print( "-> ODB.Delete({0})", header.ID );
#endif
		#endregion

		// open connection and start new transaction if required
		TransactionBegin();
		try {
			// check object stamp. If it is newer then current -> raise error
			SqlCommand sqlCmd = new SqlCommand( "IF ((SELECT [TimeStamp] " +
					"FROM [dbo].[_objects] WHERE [ID] = @objectID) > @Stamp) " +
					"RAISERROR( '" + ERROR_CHANGED_OBJECT + "', 11, 1 );",
					m_con,
					m_trans );
			// add proxy stamp parameter
			sqlCmd.Parameters.Add( "@Stamp", SqlDbType.DateTime ).Value =
				header.Stamp;
			sqlCmd.Parameters.Add( "@objectID", SqlDbType.Int ).Value = header.ID;

			sqlCmd.CommandText += "EXEC [dbo].[sp_DeleteObject] @objectID";

			// proccess delete opearaton
			sqlCmd.ExecuteNonQuery();
		} catch( Exception ex ) {
			#region dubug info
#if (DEBUG)
			Debug.Print( "[ERROR] @ ODB.Delete: " + ex.ToString() );
#endif
			#endregion
			// rollback failed transaction
			TransactionRollback();
			throw;
		}
		// close connection and commit transaction if required
		TransactionCommit();
		#region debug info
#if (DEBUG)
		Debug.Print( "<- ODB.Delete({0})", header.ID );
#endif
		#endregion
	}

	/// <summary>
	/// Execute specified SQL request on the storage.
	/// </summary>
	/// <param name="sql">SQL request to be executed.</param>
	/// <returns>Disconnected table of in-memory data.</returns>
	public DataSet ProcessSQL( string sql )
	{
		#region debug info
#if (DEBUG)
		Debug.Print( "-> ODB.ProcessSQL( '{0}' )", sql );
#endif
		#endregion

		// creating command
		SqlCommand CmdSql = new SqlCommand(sql, m_con, m_trans);
		SqlDataAdapter da = new SqlDataAdapter(CmdSql);
		// table for result
		DataSet ds = new DataSet();
		da.Fill( ds ); // fill table

		#region debug info
#if (DEBUG)
		Debug.Print( "-> ODB.ProcessSQL( '{0}' ) = {1}", sql, ds.Tables.Count );
#endif
		#endregion

		return ds;
	}

	/// <summary>
	/// Retrieve object header from storage.
	/// </summary>
	/// <param name="header">In/Out header value.</param>
	public void Retrieve( ref HEADER header )
	{
		#region debug info
#if (DEBUG)
		Debug.Print( "-> ODB.Retrieve( {0}, {1} )", header.ID, header.Type );
#endif
		#endregion

		// open connection and start new transaction if required
		TransactionBegin();

		try {
			header = getHeader( header.ID );
		} catch( Exception ex ) {
			#region debug info
#if (DEBUG)
			Debug.Print( "[ERROR] @ ODB.Retrieve: " + ex.ToString() );
#endif
			#endregion
			// rollback failed transaction
			TransactionRollback();
			throw;
		}
		// close connection and commit transaction if required
		TransactionCommit();
		#region debug info
#if (DEBUG)
		Debug.Print( "<- ODB.Retrieve( {0}, {1} )", header.ID, header.Type );
#endif
		#endregion
	}

	/// <summary>
	/// Retrieve object header, links and properties.
	/// </summary>
	/// <param name="header">In/Out header value.</param>
	/// <param name="links">Array of object links.</param>
	/// <param name="props">Array of object properties.</param>
	public void Retrieve( ref HEADER header, out LINK[] links,
						  out PROPERTY[] props )
	{
		#region debug info
#if (DEBUG)
		Debug.Print( "-> ODB.Retrieve( {0}, {1} )", header.ID, header.Type );
#endif
		#endregion

		List<PROPERTY> Props;	// list to store properties of object
		List<LINK> Links;	// list to store child proxy objects
		SqlDataReader SqlReader = null;
		SqlCommand CmdSql = null;
		// init out parameters
		links = null;
		props = null;

		// open connection and start new transaction if required
		TransactionBegin();
		try {
			// get object header
			HEADER newHeader = getHeader( header.ID );

			if( header.Stamp == newHeader.Stamp ) {
				header = newHeader;
				// close connection and commit transaction if required
				TransactionCommit();
				return;
			}

			#region retrive props from _properties
			CmdSql =
				new SqlCommand("SELECT [Name], [Value] " +
							   "FROM [dbo].[_properties] " +
							   "WHERE [ObjectID] = " + header.ID, m_con, m_trans);
			SqlReader =
					CmdSql.ExecuteReader( CommandBehavior.SingleResult );
			try {
				Props = new List<PROPERTY>();
				// read all simple properties of object
				while( SqlReader.Read() ) {
					// build PersistentProperty upon recieved name and value and
					// save property in collection
					Props.Add( new PROPERTY((string) SqlReader["Name"],
									  new ValueBox( SqlReader["Value"] ),
									  PROPERTY.STATE.New ));
				}
			} finally {
				// Dispose SqlDataReader
				SqlReader.Close();
				SqlReader = null;
			}
			#endregion

			#region retrive props from _images
			SqlDataAdapter da = 
				new SqlDataAdapter( 
					new SqlCommand(
						"SELECT [Name] " +
						"FROM [dbo].[_images] " +
						"WHERE [ObjectID] = " + header.ID, m_con, m_trans ) );
			DataTable dt = new DataTable(); // table for object proxy properties
			da.Fill( dt ); // fill table
			DataTableReader dtr = new DataTableReader(dt);
			//SqlReader = CmdSql.ExecuteReader( CommandBehavior.SingleRow );
			try {
				while( dtr.Read() ) {
					// save data from SqlDataReader because we need non SequentialAccess in datarow
					string name = (string) dtr["Name"];
					PersistentStream stream = new PersistentStream();
					imageRead( header.ID, name, ref stream );
					// save property in collection
					Props.Add( 
						new PROPERTY(
							name,
							new ValueBox((Object) stream ), PROPERTY.STATE.New ));
				}
			} finally {
				dtr.Close();
			}
			#endregion

			#region retrive links
			CmdSql = 
				new SqlCommand(
						"SELECT [ID], [ObjectName], [ObjectType], [TimeStamp] " +
						"FROM [dbo].[_objects] " +
						"WHERE [ID] IN ( " + 
							"SELECT Child FROM [dbo].[_links] " + 
							"WHERE Parent = " + header.ID + ")", 
						m_con, m_trans);

			#region get DateTable with results
			da = new SqlDataAdapter(CmdSql);
			dt = new DataTable(); // table for object proxy properties
			da.Fill( dt ); // fill table
			#endregion

			#region create reader over table we got
			dtr = new DataTableReader( dt );

			try {
				Links = new List<LINK>();
				while( dtr.Read() ) {
					// save child header
					Links.Add( new LINK(
							new HEADER((string) dtr["ObjectType"],
									   Convert.ToInt32( dtr["ID"] ),
									   Convert.ToDateTime( dtr["TimeStamp"] ),
									   (string) dtr["ObjectName"] ),
									   LINK.STATE.New));
				}
			} finally { dtr.Close(); }
			#endregion
			#endregion

			if( Props.Count > 0 ) { props = Props.ToArray(); }
			if( Links.Count > 0 ) { links = Links.ToArray(); }
			header = newHeader;
		} catch( Exception ex ) {
			#region debug info
#if (DEBUG)
			Debug.Print( "[ERROR] @ ODB.Retrive: " + ex.ToString() );
#endif
			#endregion
			// rollback failed transaction
			TransactionRollback();
			throw;
		}
		// close connection and commit transaction if required
		TransactionCommit();

		#region debug info
#if (DEBUG)
		Debug.Print( "<- ODB.Retrieve({0}, {1})", header.ID, header.Type );
#endif
		#endregion
	}

	/// <summary>
	/// Save object header, links and properties to storage.
	/// </summary>
	/// <param name="header">In/Out header value.</param>
	/// <param name="links">Array of modified object links.</param>
	/// <param name="props">Array of modified object properties.</param>
	/// <param name="mlinks">Array of new object links.</param>
	/// <param name="mprops">Array of new object properties.</param>
	public void Save( ref HEADER header, LINK[] links, PROPERTY[] props,
					  out LINK[] mlinks, out PROPERTY[] mprops )
	{
		#region debug info
#if (DEBUG)
		Debug.Print( "-> ODB.Save( {0}, {1}, {2}, {3}, {4} )",
					header.ID, header.Type, 
					header.Stamp, props.GetUpperBound( 0 ),
					links.GetUpperBound( 0 ) );
#endif
		#endregion

		SqlCommand CmdSql = null;	// other puroses
		List<PROPERTY> Props = null;	// list for collecting current object properties
		int objID; // object ID

		// initialize out parameters
		mlinks = null;
		mprops = null;

		// open connection and start new transaction if required
		TransactionBegin();
		try {
			#region create new object proxy if it is new or check it's stamp
			// assign command which will insert or (check, update) object record in DB
			if( header.ID == 0 ) {
				// this is new object. Creating script for insertion of object
				CmdSql =
					new SqlCommand( 
						"INSERT INTO [dbo].[_objects] ( [ObjectName], [ObjectType] ) " +
						"VALUES ( '" + header.Name.Replace( "'", "''" ) + "', " +
						"'" + header.Type.Replace( "'", "''" ) + "');" +
					/*save inserted object ID*/
						"SET @ID = SCOPE_IDENTITY();", m_con, m_trans);
			} else {
				// check object stamp. If it is newer then current -> raise error
				CmdSql =
					new SqlCommand( "IF ((SELECT [TimeStamp] " +
						"FROM [dbo].[_objects] WHERE [ID] = @ID) > @Stamp) " +
						"RAISERROR( '" + ERROR_CHANGED_OBJECT + "', 11, 1 );",
						m_con,
						m_trans );
				// add proxy stamp parameter
				CmdSql.Parameters.Add( "@Stamp", SqlDbType.DateTime ).Value =
					header.Stamp;
				// proxy name is always updated
				CmdSql.CommandText +=
					"UPDATE [dbo].[_objects] SET [ObjectName] = @Name WHERE [ID] = @ID";
				// add proxy name parameter
				CmdSql.Parameters
					.Add( "@Name", SqlDbType.NVarChar, header.Name.Length )
						.Value = header.Name;
			}
			// add proxy ID parameter that is always used
			CmdSql.Parameters.Add( "@ID", SqlDbType.Int ).Value = header.ID;
			CmdSql.Parameters["@ID"].Direction = ParameterDirection.InputOutput;

			// execute batch
			CmdSql.ExecuteNonQuery();
			// get proxy ID returned by batch execution
			objID = Convert.ToInt32( CmdSql.Parameters["@ID"].Value );
			#endregion

			// create new command
			CmdSql = new SqlCommand( "", m_con, m_trans );
			Props = new List<PROPERTY>();

			// iterate through recieved properties
			foreach( PROPERTY prop in props ) {
				// check propery type
				if( prop.Value.ToObject() is PersistentStream ) {
					imageSave( objID, prop.Name, (PersistentStream) prop.Value );
				} else {
					// to store property value
					object value;
					// DateTime Property must be converted before save
					if( prop.Value.ToObject().GetType() == typeof( DateTime ) &&
					   ( prop.State == PROPERTY.STATE.Changed ||
					     prop.State == PROPERTY.STATE.New )) {
						// converting DateTime Value
						value = dateTime2Sql( (DateTime) prop.Value );
						// add to changed properies List
						Props.Add(
							new PROPERTY(prop.Name,
										 new ValueBox( value ),
										 PROPERTY.STATE.Changed) );
					} else {
						// no convertion is needed
						value = prop.Value.ToObject();
					}

					// check property action
					if( prop.State == PROPERTY.STATE.New ) {
						// adding new row to DB
						CmdSql.CommandText +=
							"INSERT INTO [dbo].[_properties] ([ObjectID], [Name], [Value]) " + 
							"VALUES " +
								"(" + objID + ", '" +
								prop.Name + "', " +
								"@PropValue_" + prop.Name + ")";
						CmdSql.Parameters.Add( "@PropValue_" + prop.Name,
												SqlDbType.Variant ).Value = value;
					} else if( prop.State == PROPERTY.STATE.Changed ) {
						// update row in DB
						CmdSql.CommandText +=
							"UPDATE [dbo].[_properties] " +
							"SET [Value] = " + "@PropValue_" + prop.Name + " " +
							"WHERE [ObjectID]=" + objID + " AND " +
								  "[Name]='" + prop.Name + "'";
						CmdSql.Parameters.Add( "@PropValue_" + prop.Name,
												SqlDbType.Variant ).Value = value;
					} else if( prop.State == PROPERTY.STATE.Deleted ) {
						// delete row in DB
						CmdSql.CommandText +=
							"DELETE FROM [dbo].[_properties] " +
							"WHERE [ObjectID]=" + objID + " AND " +
								  "[Name]='" + prop.Name + "'";
					}
				}
			}

			// iteration throught links
			foreach( LINK link in links ) {
				// check link action
				if( link.State == LINK.STATE.New ) {
					// add new link to DB
					CmdSql.CommandText +=
						"INSERT INTO [dbo].[_links] ([Parent], [Child]) " +
						"VALUES (" + objID + ", " + link.Header.ID + ")";
				} else if( link.State == LINK.STATE.Deleted ) {
					// delete link from DB
					CmdSql.CommandText +=
						"DELETE FROM [dbo].[_links] " +
						"WHERE [Parent]=" + objID + " AND " + 
							  "[Child]=" + link.Header.ID;
				}
			}

			// executing batch
			if( CmdSql.CommandText != "" ) {
				CmdSql.ExecuteNonQuery();
			}

			// return proxy properties
			header = getHeader( objID );
			// and changed properies if exists
			if( Props.Count > 0 )
				mprops = Props.ToArray();
		} catch( Exception ex ) {
			#region debug info
#if (DEBUG)
			Debug.Print( "[ERROR] @ ODB.Save: " + ex.ToString() );
#endif
			#endregion
			// rollback failed transaction
			TransactionRollback();
			throw;
		}
		// close connection and commit transaction if required
		TransactionCommit();

		#region debug info
#if (DEBUG)
		Debug.Print( "<- ODB.Save(ID:{0}, Type:'{1}', Name:'{2}', Stamp:'{3}' )",
					header.ID, header.Type, header.Name, header.Stamp );
#endif
		#endregion
	}

	/// <summary>
	/// Search objects that sutisfies search criteria.
	/// </summary>
	/// <param name="type">Objects type.</param>
	/// <param name="query">Additional SQL request.</param>
	/// <param name="where">SQL WHERE clause.</param>
	/// <param name="order">>SQL ORDER BY clause.</param>
	/// <param name="bottom">Bottom limit in the request.</param>
	/// <param name="count">Count limit in the request.</param>
	/// <param name="headers">Array of found object headers.</param>
	/// <returns>Count of found objects</returns>
	/// <remarks>Search criteria in where criteria must meet following requirements:
	/// { [ NOT ] &#60;predicate&#62; | ( &#60;search_condition&#62; ) } 
	/// [ { AND | OR } [ NOT ] { &#60;predicate&#62; | ( &#60;search_condition&#62; ) }
	/// Where:
	///  &#60;predicate&#62; ::= 
	/// { expression { = | &#60; &#62; | ! = | &#62; | &#62; = | ! &#62; | &#60; | &#60; = | ! &#60; } expression 
	/// | string_expression [ NOT ] LIKE string_expression 
	/// | expression IS [ NOT ] NULL
	/// }
	/// Left expression:
	/// For all date types except int CAST or CONVERT must be declared.
	/// For example: 
	/// <list type="bullet">
	///  <item><term>( Prop_1 = CAST( dt.ToString( "yyyy-MM-dd HH:mm:ss.fff" ) as datetime )</term>
	///	   <description>for (DateTime dt)</description></item>
	///  <item><term>( Prop_1 = CAST( (bl ? 1 : 0) as bit )</term>
	///   <description>for (Boolean bl)</description></item>
	///  <item><term>( Prop_1 = 123123 )</term>
	///   <description>for int </description></item>
	///  <item><term>( Prop_1 = CONVERT( dbl.ToString( "R" ) + " as float ) as float )</term>
	///   <description>for (Double dbl)</description></item>
	/// </list>
	/// </remarks>
	public int Search( string type, string query, string where,
					   string order, int bottom, int count,
					   out HEADER[] headers )
	{

		#region debug info
#if (DEBUG)
		Debug.Print( "-> ODB.Search( '{0}', '{1}')", type, where );
#endif
		#endregion

		if( !string.IsNullOrEmpty( where ) ) {
			if( !Regex.IsMatch( where, REGEXP_CHECK ) ) {
#if (DEBUG)
				Debug.Print( 
					"[ERROR] @ ODB.Search: 'Invalid search condition format!'" + 
						"\n\t" + where +
						"\n\t Regexp = '" + REGEXP_CHECK );
#endif
				throw new ArgumentException( "Invalid search condition format!" );
			}
		}

		MatchCollection mc;
		List<HEADER> objects = null;

		#region adopt search condition according to DB structure
		// get collection of predicates which will be replaced
		mc = Regex.Matches( where, REGEXP_PREDS );
		#region debug info
#if (DEBUG)
		Debug.Print( "ODB.Search: \n\tpattern = '" + REGEXP_PREDS + "'\n\t" +
					"Parse string = '" + where + "'" );
#endif
		#endregion

		// replace each predicate in search condition with one that fits DB
		for( int i = mc.Count - 1; i >= 0; i-- ) {
			where =
				where.Replace( mc[i].Value,
									tunePridicate( mc[i].Value ) );
		}

		#region debug info
#if (DEBUG)
		Debug.Print( "ODB.Search: search script = '" + where + "'" );
#endif
		#endregion
		#endregion

		// surround expression with brackets
		where = "(" + (where == "" ? "1=1" : where) + ")";

		// add Object Type definition if exist to search condition
		if( !string.IsNullOrEmpty( type ) ) {
			where = "(ObjectType = '" + type + "') AND " + where;
		}
		
		// modify query
		if( string.IsNullOrEmpty(query) ) {
			// default query table
			query = "_objects";
		} else {
			// surround expression with brackets
			query = "(" + query + ")";
		}
		
		// open connection and start new transaction if required
		TransactionBegin();
		try {
			// sp_GetObjectsByCriteria will return table with the following columns:
			// ID, ObjectName, ObjectType, TimeStamp
			SqlCommand cmdSelect = 
				new SqlCommand( "[dbo].[sp_GetObjectsByCriteria]", 
								m_con, m_trans );
			cmdSelect.CommandType = CommandType.StoredProcedure;
			cmdSelect.Parameters.Add( 
				"@Request", SqlDbType.NVarChar, where.Length )
					.Value = where;
			// TODO: Подставлять query, если не пустой.
			cmdSelect.Parameters.Add( "@View", SqlDbType.NVarChar, query.Length )
					.Value = query;
			cmdSelect.Parameters.Add( "@First", SqlDbType.Int )
				.Value = bottom;
			cmdSelect.Parameters.Add( "@Count", SqlDbType.Int )
				.Value = count;
			cmdSelect.Parameters.Add( "@OrderBy", SqlDbType.NVarChar, order.Length )
				.Value = order;

			#region get DateTable with results
			SqlDataAdapter da = new SqlDataAdapter( cmdSelect );
			DataTable dt = new DataTable(); // table for object proxy properties
			da.Fill( dt ); // fill table
			#endregion

			#region retrive proxies that meets criteria
			DataTableReader dtr = new DataTableReader( dt );
			// create List for storing found objects
			objects = new List<HEADER>();

			// read all proxy props found
			try {
				while( dtr.Read() ) {
					// save found proxy object
					objects.Add( new HEADER((string) dtr["ObjectType"],
											Convert.ToInt32( dtr["ID"] ),
											Convert.ToDateTime( dtr["TimeStamp"] ),
											(string) dtr["ObjectName"]) );
				}
			} finally { dtr.Close(); }
			#endregion
		} catch( Exception ex ) {
			#region debug info
#if (DEBUG)
			Debug.Print( "[ERROR] @ ODB.Search: " + ex.ToString() );
#endif
			#endregion
			// rollback failed transaction
			TransactionRollback();
			throw;
		}
		// close connection and commit transaction if required
		TransactionCommit();

		// return objects found
		headers = objects.ToArray();
		#region debug info
#if (DEBUG)
		Debug.Print( "<- ODB.Search( '{0}', '{1}') = {2}", type, where, objects.Count );
#endif
		#endregion
		// return count objects found
		return objects.Count;
	}

	/// <summary>
	/// Starts a storage transaction.
	/// </summary>
	public void TransactionBegin()
	{
		// if no opened transactions then 
		// open connection and start new transaction
		if( m_TransactionCount == 0 ) {
			m_con.Open();
			m_trans = m_con.BeginTransaction();
		}
		// increment number of requested new transaction
		m_TransactionCount++;
		
		#region debug info
#if (DEBUG)
		Debug.Print( "ODB.TransactionBegin( '{0}' )", m_TransactionCount );
#endif
		#endregion
	}

	/// <summary>
	/// Commits the storage transaction.
	/// </summary>
	public void TransactionCommit()
	{
		m_TransactionCount--;
		// if last virtual transaction commited then commit
		// transaction and close opened connection
		if( m_TransactionCount == 0 ) {
			m_trans.Commit();
			m_trans = null;
			m_con.Close();
		}
		
		#region debug info
#if (DEBUG)
		Debug.Print( "ODB.TransactionCommit( '{0}' )", m_TransactionCount );
#endif
		#endregion
	}

	/// <summary>
	/// Rolls back a transaction from a pending state.
	/// </summary>
	public void TransactionRollback()
	{
		m_TransactionCount--;
		// check opened transactions count
		if( m_TransactionCount == 0 ) {
			// rollback transaction and close connection
			m_trans.Rollback();
			m_trans = null;
			m_con.Close();
		}
		
		#region debug info
#if (DEBUG)
		Debug.Print( "ODB.TransactionRollback( '{0}' )", m_TransactionCount );
#endif
		#endregion
	}
	#endregion
}
}
