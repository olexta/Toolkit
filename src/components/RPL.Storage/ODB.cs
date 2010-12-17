***************************************************************************
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
using System.Data.Common;
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
	readonly DbConnection m_con;
	private DbTransaction m_trans = null;
	// number of currently opened transactions
	private int m_TransactionCount = 0;
	private const int BUFFER_LENGTH = 1024 * 1024;
	
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
	private HEADER get_header( int id )
	{
		DbCommand cmd = new SqlCommand( string.Format(
			"SELECT [ObjectName], [ObjectType], [TimeStamp] " +
			"FROM [_objects] WHERE [ID] = {0}",
			id) );
		cmd.Connection = m_con;
		cmd.Transaction = m_trans;
		DbDataReader dr = cmd.ExecuteReader();
		try {
			dr.Read();

			// return new header
			return new HEADER(
						(string)dr["ObjectType"],
						id,
						(DateTime) dr["TimeStamp"],
						(string) dr["ObjectName"]);
		} catch {
			throw new ArgumentException( string.Format("Object with id = {0} doesn't exist in DB!", id) );
		} finally {
			dr.Dispose();
		}
	}

	// create SqlCommand text from Where.Clause
	private string clause_to_cmd(Where.Clause clause, IDictionary<string, object> parms)
	{
		string op;
		string column = "Value";

		if( clause.Value.ToObject() == DBNull.Value ) {
			bool isProxyProperty = ((clause.OPD == "ID") || (clause.OPD == "Name") || (clause.OPD == "Stamp"));
			
			switch( clause.Operator ) {
				case Where.Clause.OP.NE:
					if( isProxyProperty ) {
						// return query that always return true
						return "(0=0)";
					} else {
						return string.Format(
								"( EXISTS( SELECT [ID] FROM [_properties] WHERE ([ObjectID] = [Source].[ID]) " +
								"AND [Name] = '{0}') OR " +
								"EXISTS( SELECT [ID] FROM [_images] WHERE ([ObjectID] = [Source].[ID]) " +
								"AND [Name] = '{0}') )",
								clause.OPD );
					}
				case Where.Clause.OP.EQ:
					if( isProxyProperty ) {
						// return query that always return false
						return "(0=1)";
					} else {
						return string.Format(
								"( NOT EXISTS(SELECT [ID] FROM [_properties] WHERE ([ObjectID] = [Source].[ID]) " +
								"AND [Name] = '{0}') AND "+
								"NOT EXISTS( SELECT [ID] FROM [_images] WHERE ([ObjectID] = [Source].[ID]) " +
								"AND [Name] = '{0}') )",
								clause.OPD );
					}
				default:
					throw new ArgumentException("Invalid operator for requested propery value.\nDBNull may coexist only with:\n\t '!=' or '='");
			}
		}
		// operator definition
		switch( clause.Operator ) {
			case Where.Clause.OP.GE:
				op = ">=";
				break;
			case Where.Clause.OP.GT:
				op = ">";
				break;
			case Where.Clause.OP.LE:
				op = "<=";
				break;
			case Where.Clause.OP.LT:
				op = "<";
				break;
			case Where.Clause.OP.NE:
				op = "<>";
				break;
			case Where.Clause.OP.EQ:
			default:
				if( clause.Value.ToObject().GetType() == typeof(string)) {
					op = "LIKE";
					column = "CAST(Value as nvarchar)";
				} else {
					op = "=";
				}
				break;
		}
		string opd = "@" + clause.OPD;
		int counter = 0;
		
		while( parms.ContainsKey( opd ) ) {
			opd = "@" + clause.OPD + "_" + counter;
			counter++;
		}
		
		// property value definition
		parms.Add( opd, clause.Value.ToObject());
		string result = "";

		if( (clause.OPD == "ID") || (clause.OPD == "Name") || (clause.OPD == "Stamp") ) {
			// skiping base object properties: ID, Name, Stamp
			string propName = "";
			// changing property name
			switch( clause.OPD ) {
				case "Stamp":
					propName = "TimeStamp";
					break;
				case "Name":
					propName = "ObjectName";
					break;
				default:
					propName = "ID";
					break;
			}
			result = string.Format( "([Source].[{0}] {1} {2}) ",
									propName, op, opd );
		} else {
			// query for non base properties
			result = string.Format(
					"(EXISTS(SELECT [ID] FROM [_properties] AS p " +
							"WHERE ([ObjectID] = [Source].[ID]) " +
								"AND " +
								"([Name] = '{0}') " + // define property name
								"AND " +
								"({1} {2} {3}))) ",
					clause.OPD, column, op, opd ); // define property value
		}
		return result;
	}

	// return query OrderBy part from OrderBy.Clause
	private void orderby_to_cmd(OrderBy.Clause orderClause, out string join, out string orderby)
	{
		// part that is responsible for joining Source table
		// with specified property values from _properties table
		join = string.Format( "LEFT JOIN [_properties] AS {0} on [Source].[ID] = [{0}].[ObjectID] AND [{0}].[Name] = '{0}' ",
							  orderClause.OPD );
		// part resposible for OrderBy definition
		orderby = string.Format( "[{0}].[Value] {1}",
								 orderClause.OPD,
								 (orderClause.Sort == OrderBy.Clause.SORT.ASC ? "ASC" : "DESC") );
	}


	// create SqlCommand text from Where
	private string where_to_cmd(Where where, IDictionary<string, object> parms)
	{
		if( where is Where.Clause ) {
			return clause_to_cmd((Where.Clause)where, parms);
		} else {
			string leftCmd = "";
			string rightCmd = "";
			string resultCmd = "";
			if( (where is Where.Operation.And) || (where is Where.Operation.Or) ) {
				if( where is Where.Operation.And ) {
					leftCmd = where_to_cmd(((Where.Operation.And)where).LeftWhere, parms);
					rightCmd = where_to_cmd(((Where.Operation.And)where).RightWhere, parms);
				} else {
					leftCmd = where_to_cmd(((Where.Operation.Or)where).LeftWhere, parms);
					rightCmd = where_to_cmd(((Where.Operation.Or)where).RightWhere, parms);
				}

				resultCmd = string.Format(
								"({0} {1} {2})",
								leftCmd,
								(where is Where.Operation.And)? "AND" : "OR",
								rightCmd );
			} else if( where is Where.Operation.Not ) {
				Where.Operation.Not opNot = (Where.Operation.Not)where;
				string cmd = where_to_cmd(opNot.SubWhere, parms);
				resultCmd = string.Format( " NOT({0})", cmd );
			}
			return resultCmd;
		}
	}

	/// <summary>
	/// Converts DateType type according to MS SQL Server precision.
	/// </summary>
	/// <param name="dt">DateTime to be converted</param>
	/// <returns>DateTime rounded to .**0, .**3, .**7 milliseconds</returns>
	private DateTime datetime_to_sql( DateTime dt )
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
	private void save_image( int objID, string propName, PersistentStream stream )
	{
		#region debug info
#if (DEBUG)
		Debug.Print( "-> ODB.imageSave( {0}, '{1}' )", objID, propName );
#endif
		#endregion
		// open connection and start new transaction if required
		TransactionBegin();

		// command that creates new record or cleares existing
		DbCommand cmd = new SqlCommand( string.Format(
			"DECLARE @_id AS int; " +
			"IF (EXISTS(SELECT [ID] FROM [_images] " +
				"WHERE [ObjectID] = {0} AND [Name] ='{1}' )) BEGIN " +
				"UPDATE [_images] SET [Value] = {2} " +
				"WHERE [ObjectID] = {0} AND [Name] ='{1}'; " +
				"SELECT @_id = [ID] FROM [_images] " +
				"WHERE [ObjectID] = {0} AND [Name] ='{1}'; " +
			"END ELSE BEGIN " +
				"INSERT INTO [_images] " +
				"([ObjectID], [Name], [Value]) " +
				"VALUES ( {0}, '{1}', {2} ); " +
				"SET @_id = SCOPE_IDENTITY() " +
			"END;" +
			"SELECT @Pointer = TEXTPTR([Value]) FROM [_images] WHERE [ID] = @_id",
			objID, propName, (stream.Length > 0) ? "0x0" : "NULL") );
		cmd.Connection = m_con;
		cmd.Transaction = m_trans;

		DbParameter pointerParam  = new SqlParameter( "@Pointer", SqlDbType.Binary, 16 );
		pointerParam.Direction = ParameterDirection.Output;
		cmd.Parameters.Add( pointerParam );
		
		try {
			// get pointer to image data
			cmd.ExecuteNonQuery();
			// set up UPDATETEXT command, parameters, and open BinaryReader.	
			cmd = new SqlCommand(
				"UPDATETEXT [_images].Value @Pointer @Offset @Delete " +
				"WITH LOG @Bytes");
			cmd.Connection = m_con;
			cmd.Transaction = m_trans;
			// assign value of pointer previously recieved
			cmd.Parameters.Add( new SqlParameter("@Pointer", SqlDbType.Binary, 16) );
			cmd.Parameters["@Pointer"].Value = pointerParam.Value;
			// start insertion from begin
			DbParameter offsetParam = new SqlParameter( "@Offset", SqlDbType.Int );
			offsetParam.Value = 0;
			cmd.Parameters.Add( offsetParam );
			//delete 0x0 character
			DbParameter deleteParam = new SqlParameter("@Delete", SqlDbType.Int);
			deleteParam.Value  = 1;
			cmd.Parameters.Add( deleteParam );
			DbParameter bytesParam = new SqlParameter( "@Bytes", SqlDbType.Binary );
			cmd.Parameters.Add( bytesParam );

			stream.Seek( 0, SeekOrigin.Begin );

			// read buffer full of data and execute UPDATETEXT statement.
			Byte[] Buffer = new Byte[BUFFER_LENGTH];
			// make first read from stream
			int ret = stream.Read( Buffer, 0, BUFFER_LENGTH );

			// while something is read from stream, write to apend to BLOB field
			while( ret > 0 ) {
				// initing parameters for write
				bytesParam.Value = Buffer;
				bytesParam.Size = ret;
				// write to BLOB field
				cmd.ExecuteNonQuery(); // execute iteration
				deleteParam.Value = 0; // don't delete any other data
				// prepare to next iteration
				offsetParam.Value =
					Convert.ToInt32( offsetParam.Value ) + ret;
				// read from stream for next iteration
				ret = stream.Read( Buffer, 0, BUFFER_LENGTH );
			}
		} catch( Exception ex ) {
			#region debug info
#if (DEBUG)
			Debug.Print( "[ERROR] @ ODB.imageSave: {0}", ex.ToString() );
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
		Debug.Print( "<- ODB.imageSave( {0}, '{1}' )", objID, propName );
#endif
		#endregion
	}

	/// <summary>
	/// Read BLOB field to stream property
	/// </summary>
	/// <param name="objID">Stream owner object ID</param>
	/// <param name="propName">Name of stream property</param>
	/// <param name="stream">Stream property</param>
	private void read_image( int objID, string propName, ref PersistentStream stream )
	{
		#region debug info
#if (DEBUG)
		Debug.Print( "<- ODB.imageRead( {0}, '{1}' )", objID, propName );
#endif
		#endregion

		// get pointer to BLOB field using TEXTPTR.
		DbCommand cmd = new SqlCommand( string.Format(
			"SELECT @Pointer = TEXTPTR([Value]), " +
			"@Length = DataLength([Value]) " +
			"FROM [_images] " +
			"WHERE [ObjectID] = {0} AND [Name] ='{1}'",
			objID, propName) );
		cmd.Connection = m_con;
		cmd.Transaction = m_trans;
		// setup parameters
		DbParameter pointerParam = new SqlParameter("@Pointer", SqlDbType.VarBinary, 16);
		pointerParam.Direction = ParameterDirection.Output;
		cmd.Parameters.Add( pointerParam );
		DbParameter lengthParam = new SqlParameter("@Length", SqlDbType.Int);
		lengthParam.Direction = ParameterDirection.Output;
		cmd.Parameters.Add( lengthParam );
		// open connection and start new transaction if required
		TransactionBegin();
		try {
			// get pointer and length of BLOB field
			cmd.ExecuteNonQuery();

			//check that BLOB field exists
			if( pointerParam.Value == null ) {
				throw new KeyNotFoundException( ERROR_IMAGE_IS_ABSENT );
			}

			// run the query.
			// set up the READTEXT command to read the BLOB by passing the following
			// parameters: @Pointer – pointer to blob, @Offset – number of bytes to
			// skip before starting the read, @Size – number of bytes to read.
			cmd = new SqlCommand(
				"READTEXT [_images].Value " +
				"@Pointer @Offset @Size HOLDLOCK");
			cmd.Connection = m_con;
			cmd.Transaction = m_trans;
			// temp buffer for read/write purposes
			Byte[] buffer = new Byte[BUFFER_LENGTH];

			// set up the parameters for the command.
			cmd.Parameters.Add( new SqlParameter("@Pointer", pointerParam.Value) );
			// current offset position
			DbParameter offset = new SqlParameter("@Offset", SqlDbType.Int);
			offset.Value = 0;
			cmd.Parameters.Add( offset );
			DbParameter size =  new SqlParameter("@Size", SqlDbType.Int);
			size.Value = 0;
			cmd.Parameters.Add( size );

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
				DbDataReader dr =
					cmd.ExecuteReader( CommandBehavior.SingleRow );

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
			Debug.Print( "[ERROR] @ ODB.imageRead: {0}", ex.Message );
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
		Debug.Print( "-> ODB.ctor()" );
#endif
		#endregion

		m_con = new SqlConnection(CnnStr);

		try {
			m_con.Open();
			m_con.Close();
		} catch( Exception ex ) {

#if (DEBUG)
			Debug.Print( "EXCEPTION @ ODB..ctor(): {0}", ex.ToString() );
#endif
			throw;
		}
		#region debug info
#if (DEBUG)
		Debug.Print( "<- ODB.ctor()" );
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
			DbCommand cmd = new SqlCommand( string.Format(
				"DECLARE @_id AS int; SET @_id = @ID;" +
				"IF ((SELECT [TimeStamp] " +
				"FROM [_objects] WHERE [ID] = @_id) > @Stamp) RAISERROR( '{0}', 11, 1 );",
				ERROR_CHANGED_OBJECT) );
			cmd.Connection = m_con;
			cmd.Transaction = m_trans;
			// add proxy stamp parameter
			cmd.Parameters.Add(new SqlParameter("@ID", header.ID));
			cmd.Parameters.Add(new SqlParameter("@Stamp", header.Stamp));

			cmd.CommandText += "DELETE FROM [_objects] WHERE [ID] = @_id";

			// proccess delete opearaton
			cmd.ExecuteNonQuery();
		} catch( Exception ex ) {
			#region dubug info
#if (DEBUG)
			Debug.Print( "[ERROR] @ ODB.Delete: {0}", ex.ToString() );
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
	/// <param name="params"></param>
	/// <returns>Disconnected table of in-memory data.</returns>
	public DataSet ProcessSQL(string sql, object[] @params)
	{
		#region debug info
#if (DEBUG)
		Debug.Print("-> ODB.ProcessSQL( '{0}' )", sql);
#endif
		#endregion
		// creating command
		DbCommand cmd = new SqlCommand();
		cmd.Connection = m_con;
		cmd.Transaction = m_trans;

		string[] names = new string[@params.Length];
		for( int i = 0; i < @params.Length; i++ )
		{
			names[i] = "@P" + i;
			cmd.Parameters.Add( new SqlParameter(names[i], @params[i]) );
		}

		sql = string.Format( sql, names );
		cmd.CommandText = sql;
		DataAdapter da = new SqlDataAdapter((SqlCommand)cmd);
		// table for result
		DataSet ds = new DataSet();
		da.Fill(ds); // fill table

		#region debug info
#if (DEBUG)
		Debug.Print("-> ODB.ProcessSQL( '{0}' ) = {1}", sql, ds.Tables.Count);
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
			header = get_header( header.ID );
		} catch( Exception ex ) {
			#region debug info
#if (DEBUG)
			Debug.Print( "[ERROR] @ ODB.Retrieve: {0}", ex.ToString() );
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

		List<PROPERTY> _props = new List<PROPERTY>();	// list to store properties of object
		List<LINK> _links = new List<LINK>();	// list to store child proxy objects
		DbDataReader dr = null;
		DbCommand cmd = null;
		// init out parameters
		links = null;
		props = null;

		// open connection and start new transaction if required
		TransactionBegin();
		try {
			// get object header
			HEADER newHeader = get_header( header.ID );

			if( header.Stamp == newHeader.Stamp ) {
				header = newHeader;
				// close connection and commit transaction if required
				TransactionCommit();
				return;
			}

			#region retrive props from _properties
			cmd = new SqlCommand( string.Format(
					"SELECT [Name], [Value] FROM [_properties] " +
					"WHERE [ObjectID] = {0}",
					header.ID) );
			cmd.Connection = m_con;
			cmd.Transaction = m_trans;

			dr = cmd.ExecuteReader( CommandBehavior.SingleResult );
			try {
				// read all simple properties of object
				while( dr.Read() ) {
					// read properties from row
					string name = (string) dr["Name"];
					object val	= dr["Value"];

					// convert byte array to memory stream
					if( val.GetType() == typeof(Byte[] ) ) {
						val = new PersistentStream((Byte[])val );
					}
					// build PersistentProperty upon recieved name and value and
					// save property in collection
					_props.Add( new PROPERTY( name, new ValueBox(val), PROPERTY.STATE.New ));
				}
			} finally {
				// Dispose SqlDataReader
				dr.Dispose();
			}
			#endregion

			#region retrive props from _images
			cmd = new SqlCommand( string.Format(
				"SELECT [Name] FROM [_images] " +
				"WHERE [ObjectID] = {0}",
				header.ID) );
			cmd.Connection = m_con;
			cmd.Transaction = m_trans;

			SqlDataAdapter da = new SqlDataAdapter( (SqlCommand)cmd );
			DataTable dt = new DataTable(); // table for object proxy properties

			da.Fill( dt ); // fill table
			DataTableReader dtr = new DataTableReader(dt);
			try {
				while( dtr.Read() ) {
					// save data from SqlDataReader because we need non SequentialAccess in datarow
					string name = (string) dtr["Name"];
					PersistentStream stream = new PersistentStream();
					read_image( header.ID, name, ref stream );
					// save property in collection
					_props.Add( 
						new PROPERTY(
							name,
							new ValueBox((Object) stream ), PROPERTY.STATE.New ));
				}
			} finally {
				dtr.Dispose();
			}
			#endregion

			#region retrive links
			cmd = new SqlCommand( string.Format(
				"SELECT [ID], [ObjectName], [ObjectType], [TimeStamp] FROM [_objects] " +
				"WHERE [ID] IN ( " + 
				"SELECT Child FROM [_links] " + 
				"WHERE Parent = {0})",
				header.ID) );
			cmd.Connection = m_con;
			cmd.Transaction = m_trans;

			dr = cmd.ExecuteReader( CommandBehavior.SingleResult );
			try {
				while( dr.Read() ) {
					// save child header
					_links.Add( new LINK(
						new HEADER((string) dr["ObjectType"],
							Convert.ToInt32( dr["ID"] ),
							Convert.ToDateTime( dr["TimeStamp"] ),
							(string) dr["ObjectName"] ),
							LINK.STATE.New));
				}
			} finally { dr.Dispose(); }
			#endregion

			props = _props.ToArray();
			links = _links.ToArray();
			header = newHeader;
		} catch( Exception ex ) {
			#region debug info
#if (DEBUG)
			Debug.Print( "[ERROR] @ ODB.Retrive: {0}", ex.ToString() );
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

		DbCommand cmd = null;	// other puroses
		List<PROPERTY> _props = null;	// list for collecting current object properties
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
				cmd =
					new SqlCommand( 
						"INSERT INTO [_objects] ( [ObjectName], [ObjectType] ) " +
						"VALUES ( @Name, @Type );" +
						/*save inserted object ID*/
						"SET @ID = SCOPE_IDENTITY();");
				cmd.Connection = m_con;
				cmd.Transaction = m_trans;
				// add proxy name parameter
				cmd.Parameters.Add(new SqlParameter("@Name", header.Name));
				// add proxy name parameter
				cmd.Parameters.Add(new SqlParameter("@Type", header.Type));
				// add proxy ID parameter
				cmd.Parameters.Add(new SqlParameter("@ID", SqlDbType.Int));
				cmd.Parameters["@ID"].Direction = ParameterDirection.Output;
			} else {
				// check object stamp. If it is newer then current -> raise error
				cmd = new SqlCommand( string.Format(
					"DECLARE @_id AS Int; SET @_id = @ID;\n" +
					"IF ((SELECT [TimeStamp] FROM [_objects] WHERE [ID] = @_id) > @Stamp) " +
					"\n\tRAISERROR( '{0}', 11, 1 );",
					ERROR_CHANGED_OBJECT ) );
				cmd.Connection = m_con;
				cmd.Transaction = m_trans;
				// add proxy ID parameter
				cmd.Parameters.Add(new SqlParameter("@ID", header.ID));
				// add proxy stamp parameter
				cmd.Parameters.Add( new SqlParameter("@Stamp", header.Stamp));
				// proxy name is always updated
				cmd.CommandText += "UPDATE [_objects] SET [ObjectName] = @Name WHERE [ID] = @_id";

				// add proxy name parameter
				cmd.Parameters.Add( new SqlParameter( "@Name", header.Name) );
			}

			// execute batch
			cmd.ExecuteNonQuery();
			// get proxy ID returned by batch execution
			objID = Convert.ToInt32( cmd.Parameters["@ID"].Value );
			#endregion

			// create new command
			cmd = new SqlCommand( "" );
			cmd.Connection = m_con;
			cmd.Transaction = m_trans;
			_props = new List<PROPERTY>();

			// iterate through recieved properties
			for( int i = 0; i < props.Length; i++ ) {
				// strore propery value suitable for saving as sql_variant
				object value;
				// converting value to sql_variant capable type
				if( props[i].Value.ToObject() is PersistentStream ) {
					// reading stream value to byte array
					PersistentStream ps = ((PersistentStream)props[i].Value.ToObject());
					byte[] buffer = new byte[ps.Length];
					ps.Seek(0, SeekOrigin.Begin);
					ps.Read(buffer, 0, (int)ps.Length);
					value = buffer;
				} else if( (props[i].Value.ToObject().GetType() == typeof(DateTime)) &&
						   (props[i].State == PROPERTY.STATE.Changed ||
							props[i].State == PROPERTY.STATE.New) ) {

					// DateTime Property must be converted to precision of sql server before sav
					value = datetime_to_sql( (DateTime)props[i].Value );
					// add to changed properies List
					_props.Add( new PROPERTY(props[i].Name, new ValueBox(value), PROPERTY.STATE.Changed) );
				} else {
					// no convertion is needed
					value = props[i].Value.ToObject();
				}
				// check property action
				switch( props[i].State ) {
					case PROPERTY.STATE.New:
						// saving large stream property. 7900 is maximum length of sql_variant field in _properties table because
						// SQL Server 2000 limits maximum row size to 8060 bytes.
						if( (props[i].Value.ToObject() is PersistentStream)
							&&
							(((PersistentStream)props[i].Value.ToObject()).Length > 7900) ) {
							// saving stream property to _images table
							save_image( objID, props[i].Name, (PersistentStream)props[i].Value );
						} else {
							// adding new row to DB
							cmd.CommandText += string.Format(
								"INSERT INTO [_properties] ([ObjectID], [Name], [Value]) " +
								"VALUES ( {0}, '{1}', @P{2} );",
								objID, props[i].Name, i );
							cmd.Parameters.Add( new SqlParameter( "@P" + i, value ) );
						}
						break;
					case PROPERTY.STATE.Changed:
						// saving large stream property. 7900 is maximum length of sql_variant field in _properties table because
						// SQL Server 2000 limits maximum row size to 8060 bytes.
						if( (props[i].Value.ToObject() is PersistentStream)
							&&
							(((PersistentStream)props[i].Value.ToObject()).Length > 7900) ) {
							// add query for delete property from _properties table (property may be there before)
							cmd.CommandText += string.Format(
								"DELETE FROM [_properties] " +
								"WHERE [ObjectID]={0} AND [Name]='{1}';",
								objID, props[i].Name );
							// saving stream property to _images table
							save_image( objID, props[i].Name, (PersistentStream)props[i].Value );
						} else {
							// update row in DB
							cmd.CommandText += string.Format(
								"UPDATE [_properties] " +
								"SET [Value] = @P{0} " +
								"WHERE [ObjectID]={1} AND [Name]='{2}';",
								i, objID, props[i].Name );
							cmd.Parameters.Add( new SqlParameter( "@P" + i, value ) );

							// try to delete property from _images table if it is PersistentStream
							if( props[i].Value.ToObject() is PersistentStream ) {
								cmd.CommandText += string.Format(
									"DELETE FROM [_images] " +
									"WHERE [ObjectID]={0} AND [Name]='{1}';",
									objID, props[i].Name );
							}
						}
						break;
					case PROPERTY.STATE.Deleted:
						// delete property from _properties table
						cmd.CommandText += string.Format(
							"DELETE FROM [_properties] " +
							"WHERE [ObjectID]={0} AND [Name]='{1}';" +
							"DELETE FROM [_images] " +
							"WHERE [ObjectID]={0} AND [Name]='{1}';",
							objID, props[i].Name );
						break;
				}
			}

			// iteration throught links
			foreach( LINK link in links ) {
				// check link action
				if( link.State == LINK.STATE.New ) {
					// add new link to DB
					cmd.CommandText += string.Format(
						"INSERT INTO [_links] ([Parent], [Child]) " +
						"VALUES ({0}, {1});",
						objID, link.Header.ID );
				} else if( link.State == LINK.STATE.Deleted ) {
					// delete link from DB
					cmd.CommandText += string.Format(
						"DELETE FROM [_links] " +
						"WHERE [Parent]={0} AND [Child]={1};",
						objID, link.Header.ID );
				}
			}

			// executing batch
			if( cmd.CommandText != "" ) {
				cmd.ExecuteNonQuery();
			}

			// return proxy properties
			header = get_header( objID );
			// and changed properies if exists
			if( _props.Count > 0 )
				mprops = _props.ToArray();
		} catch( Exception ex ) {
			#region debug info
#if (DEBUG)
			Debug.Print( "[ERROR] @ ODB.Save: {0}", ex.ToString() );
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
	/// <param name="where">Where object</param>
	/// <param name="order">>OrderBy object</param>
	/// <param name="bottom">Bottom limit in the request.</param>
	/// <param name="count">Count limit in the request.</param>
	/// <param name="headers">Array of found object headers.</param>
	/// <returns>Count of found objects</returns>
	public int Search(string type, Where where, OrderBy order, int bottom, int count, out HEADER[] headers)
	{
		#region debug info
#if (DEBUG)
		Debug.Print("-> ODB.Search( '{0}')", type );
#endif
		#endregion
		// init search command
		DbCommand cmd = new SqlCommand();
		// list for HEADERs return purpose
		List<HEADER> objects = null;

		// for SqlParameter names and values
		Dictionary<string, object> parms = new Dictionary<string, object>();
		// create sql command text
		string whereQuery = "";
		if( where != null ) {
			whereQuery = where_to_cmd(where, parms);
		}

		#region prepare OrderBy part
		string orderJoin = "";
		string orderBy = "";

		if( order != null ) {
			foreach( OrderBy.Clause clause in order ) {
				string orderJoinOut;
				string orderByOut;
				orderby_to_cmd(clause, out orderJoinOut, out orderByOut);
				orderJoin += orderJoinOut;
				orderBy += orderByOut + ", ";
			}
			orderBy = orderBy.Trim().TrimEnd(',');
		}
		#endregion

		// template for search query:
		// - gets ids of object that meets $QUERY$,
		// - skips $BOOTOM$ rows
		// - return only $COUNT$ object HEADERs
		cmd.CommandText="DECLARE @_counter as int \n" + 
						"DECLARE @_id as int \n" +
						"DECLARE @_resultID TABLE ([ids] int) \n" +
						"--Check for user rights and return requested count of items\n" +
						"DECLARE cursor_Found CURSOR SCROLL FOR \n" + 
						"$QUERY$\n" +
						"OPEN cursor_Found \n\t" +
							"SET @_counter = 0 \n\t" +
							"FETCH ABSOLUTE $BOTTOM$ FROM cursor_Found INTO @_id \n\t" +
							"WHILE( (@@FETCH_STATUS = 0) AND (@_counter < $COUNT$) ) BEGIN \n\t\t" +
								"INSERT INTO @_resultID ([ids]) VALUES (@_id) \n\t\t" +
								"FETCH NEXT FROM cursor_Found INTO @_id \n\t\t" +
								"SET @_counter = @_counter + 1 \n\t" +
							"END \n" +
						"CLOSE cursor_Found \n" +
						"DEALLOCATE cursor_Found \n" +
						"--Make SQL request \n" +
						"SELECT [ID], [ObjectName], [ObjectType], [TimeStamp] FROM _objects INNER JOIN @_resultID as resultID ON _objects.ID = resultID.ids";

		// search query part with ordering
		string query = string.Format(
						"SELECT Source.[ID] " +
						"FROM (Select * FROM _objects WHERE ObjectType = '{0}') AS Source " + 
						"{1}" + //orderJoin
						"{2}" + // WHERE
						"{3}", // ORDER BY
						type, orderJoin,
						string.IsNullOrEmpty(whereQuery) ? "" : "WHERE " + whereQuery,
						orderBy == "" ? "" : "ORDER BY " + orderBy );

		// creating SqlParameters for passed values
		foreach( string parm in parms.Keys ) {
			cmd.Parameters.Add( new SqlParameter(parm, parms[parm]));
		}
		// replacing parts in search query template
		cmd.CommandText = cmd.CommandText.Replace("$QUERY$", query);
		// if bottom is not set then set it to return from first row
		if( bottom == 0 ) { bottom = 1; }
		// replacing limit parts in search query template
		cmd.CommandText = cmd.CommandText.Replace("$BOTTOM$", bottom.ToString());
		cmd.CommandText = cmd.CommandText.Replace("$COUNT$", count.ToString());

#if (DEBUG)
		Debug.Print("ODB.Search: sql search query = '{0}'", cmd.CommandText );
#endif

		// open connection and start new transaction if required
		TransactionBegin();
		try {
			cmd.Connection = m_con;
			cmd.Transaction = m_trans;
			// search query will return table with the following columns:
			// ID, ObjectName, ObjectType, TimeStamp
			#region retrive data and create proxies
			DbDataReader dr = cmd.ExecuteReader();
			
			// create List for storing found objects
			objects = new List<HEADER>();
			try {
				while( dr.Read() ) {
					// save found proxy object
					objects.Add(new HEADER((string)dr["ObjectType"],
											Convert.ToInt32(dr["ID"]),
											Convert.ToDateTime(dr["TimeStamp"]),
											(string)dr["ObjectName"]));
				}
			} finally { dr.Dispose(); }
			#endregion
		} catch( Exception ex ) {
			#region debug info
#if (DEBUG)
			Debug.Print("[ERROR] @ ODB.Search: {0}", ex.ToString());
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
		Debug.Print("<- ODB.Search( '{0}', '{1}') = {2}", type, where, objects.Count);
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
