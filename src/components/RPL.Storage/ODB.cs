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
		DbCommand cmd = new SqlCommand(
			"SELECT [ObjectName], [ObjectType], [TimeStamp] " +
				"FROM [_objects] WHERE [ID] = " + id);
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
			throw new ArgumentException("Object with id = " + id + " doesn't exist in DB!");
		} finally {
			dr.Dispose();
		}
	}

	// create SqlCommand text from Where.Clause
	private string clause_to_cmd(Where.Clause clause, IDictionary<string, object> parms)
	{
		string op;
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
				op = "=";
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
			result = "( Source.[" + propName + "] " + op + " " + opd + ") ";
		} else {

			// query for non base properties
			result = "(EXISTS(SELECT ID FROM [_properties] AS p WHERE (ObjectID = Source.ID) " +
								"AND " +
								"(Name = '" + clause.OPD + "') " + // define property name
								"AND " +
								"(Value " + op + " " + opd + "))) "; // define property value
		}
		return result;
	}

	// return query OrderBy part from OrderBy.Clause
	private void orderby_to_cmd(OrderBy.Clause orderClause, out string join, out string orderby)
	{
		// part that is responsible for joining Source table
		// with specified property values from _properties table
		join = "LEFT JOIN _properties AS " + orderClause.OPD + " on Source.[ID] = " + orderClause.OPD + ".ObjectID AND " + orderClause.OPD + ".Name = '" + orderClause.OPD + "' ";
		// part resposible for OrderBy definition
		orderby = orderClause.OPD + ".Value " + (orderClause.Sort == OrderBy.Clause.SORT.ASC ? "ASC" : "DESC");
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

				resultCmd = "(" + leftCmd + 
					" " + ((where is Where.Operation.And)? "AND" : "OR") + " " +
					rightCmd + ")";
			} else if( where is Where.Operation.Not ) {
				Where.Operation.Not opNot = (Where.Operation.Not)where;
				string cmd = where_to_cmd(opNot.SubWhere, parms);
				resultCmd = " NOT(" + cmd + ")";
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
		DbCommand cmd = new SqlCommand(
			"DECLARE @_id AS int; " +
			"IF (EXISTS(SELECT ID FROM [_images] " +
						"WHERE [ObjectID] = " + objID + " " +
						"AND " +
						"[Name] ='" + propName + "' )) BEGIN " +
				"UPDATE [_images] SET Value = " + ((stream.Length > 0) 
				? 
				"0x0" 
				: "NULL") + 
				" " +
				"WHERE [ObjectID] = " + objID + " " +
						"AND " +
						"[Name] ='" + propName + "'; " +
				"SELECT @_id = [ID] FROM [_images] " +
				"WHERE [ObjectID] = " + objID + " " +
						"AND " +
						"[Name] ='" + propName + "'; " +
			"END ELSE BEGIN " +
				"INSERT INTO [_images] " +
				"([ObjectID], [Name], [Value]) " +
				"VALUES ( " + objID + ", '" +
						  propName + "', " +
						  ((stream.Length > 0) ? "0x0" : "NULL") + " ); " +
				"SET @_id = SCOPE_IDENTITY() " +
			"END;" +
			"SELECT @Pointer = TEXTPTR(Value) FROM [_images] WHERE [ID] = @_id");
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
	private void read_image( int objID, string propName, ref PersistentStream stream )
	{
		#region debug info
#if (DEBUG)
		Debug.Print( "<- ODB.imageRead( {0}, '{1}' )", objID, propName );
#endif
		#endregion

		// get pointer to BLOB field using TEXTPTR.
		DbCommand cmd = new SqlCommand(
			"SELECT @Pointer = TEXTPTR(Value), " +
			"@Length = DataLength(Value) " +
			"FROM [_images] " +
			"WHERE [ObjectID] = " + objID + " " +
				"AND " +
				"[Name] ='" + propName + "'" );
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
			DbCommand cmd = new SqlCommand( "DECLARE @_id AS int; SET @_id = @ID;" +
				"IF ((SELECT [TimeStamp] " +
				"FROM [_objects] WHERE [ID] = @_id) > @Stamp) " +
					"RAISERROR( '" + ERROR_CHANGED_OBJECT + "', 11, 1 );" );
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

		List<PROPERTY> _props;	// list to store properties of object
		List<LINK> _links;	// list to store child proxy objects
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
			cmd = new SqlCommand(
					"SELECT [Name], [Value] FROM [_properties] " +
					"WHERE [ObjectID] = " + header.ID);
			cmd.Connection = m_con;
			cmd.Transaction = m_trans;

			dr = cmd.ExecuteReader( CommandBehavior.SingleResult );
			try {
				_props = new List<PROPERTY>();
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
			cmd = new SqlCommand(
				"SELECT [Name] FROM [_images] " +
				"WHERE [ObjectID] = " + header.ID);
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
			cmd = new SqlCommand(
				"SELECT [ID], [ObjectName], [ObjectType], [TimeStamp] FROM [_objects] " +
				"WHERE [ID] IN ( " + 
				"SELECT Child FROM [_links] " + 
				"WHERE Parent = " + header.ID + ")");
			cmd.Connection = m_con;
			cmd.Transaction = m_trans;

			dr = cmd.ExecuteReader( CommandBehavior.SingleResult );
			try {
				_links = new List<LINK>();
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

			if( _props.Count > 0 ) { props = _props.ToArray(); }
			if( _links.Count > 0 ) { links = _links.ToArray(); }
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
				cmd = new SqlCommand( 
					"DECLARE @_id AS Int; SET @_id = @ID;\n" +
					"IF ((SELECT [TimeStamp] FROM [_objects] WHERE [ID] = @_id) > @Stamp) " +
					"\n\tRAISERROR( '" + ERROR_CHANGED_OBJECT + "', 11, 1 );" );
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
			foreach( PROPERTY prop in props ) {
				// saving large stream property. 8000 is maximum length of sql_variant
				if( (prop.Value.ToObject() is PersistentStream ) 
					&& 
					(((PersistentStream)prop.Value.ToObject()).Length > 8000) ) {
					
					// different actions based on property state
					switch( prop.State ) {
						case PROPERTY.STATE.Changed:
							// add query for delete property from _properties table (property may be there before)
							cmd.CommandText += 
								"DELETE FROM [_properties] " +
								"WHERE [ObjectID]=" + objID + " AND " +
								"[Name]='" + prop.Name + "';";
							goto case PROPERTY.STATE.New;
						case PROPERTY.STATE.New:
							// saving stream property to _images table
							save_image(objID, prop.Name, (PersistentStream)prop.Value);
							break;
						case PROPERTY.STATE.Deleted:
							// add query to delete property from both tables
							cmd.CommandText += 
								"DELETE FROM [_properties] WHERE [ObjectID]=" + objID + " AND " +
								"[Name]='" + prop.Name + "';" +
								"DELETE FROM [_images] WHERE [ObjectID]=" + objID + " AND " +
								"[Name]='" + prop.Name + "';";
							break;
					}
				} else { // saving simple property
					// strore propery value suitable for saving as sql_variant
					object value;
					// converting value to sql_variant capable type
					if( prop.Value.ToObject() is PersistentStream ) {
						// reading stream value to byte array
						PersistentStream ps = ((PersistentStream)prop.Value.ToObject());
						byte[] buffer = new byte[ps.Length];
						ps.Read(buffer, 0, (int)ps.Length);
						value = buffer;
					} else if( (prop.Value.ToObject().GetType() == typeof(DateTime)) &&
					   (prop.State == PROPERTY.STATE.Changed || prop.State == PROPERTY.STATE.New) ) {

						// DateTime Property must be converted to precision of sql server before sav
						value = datetime_to_sql( (DateTime)prop.Value );
						// add to changed properies List
						_props.Add( new PROPERTY(prop.Name, new ValueBox(value), PROPERTY.STATE.Changed) );
					} else {
						// no convertion is needed
						value = prop.Value.ToObject();
					}

					// check property action
					switch( prop.State ) {
						case PROPERTY.STATE.New:
							// adding new row to DB
							cmd.CommandText +=
								"INSERT INTO [_properties] ([ObjectID], [Name], [Value]) " + 
								"VALUES " +
									"(" + objID + ", '" +
									prop.Name + "', " +
									"@PropValue_" + prop.Name + " );";
							cmd.Parameters.Add( new SqlParameter("@PropValue_" + prop.Name, value) );
							break;
						case PROPERTY.STATE.Changed:
							// update row in DB
							cmd.CommandText +=
								"UPDATE [_properties] " +
								"SET [Value] = @PropValue_" + prop.Name + " " +
								"WHERE [ObjectID]=" + objID + " AND " +
									  "[Name]='" + prop.Name + "';";
							cmd.Parameters.Add( new SqlParameter("@PropValue_" + prop.Name, value) );

							// try to delete property from _images table if it is PersistentStream
							if( prop.Value.ToObject() is PersistentStream ) {
								cmd.CommandText += 
									"DELETE FROM [_images] " +
									"WHERE [ObjectID]=" + objID + " AND " +
									"[Name]='" + prop.Name + "';";
							}
							break;
						case PROPERTY.STATE.Deleted:
							// delete property from _properties table
							cmd.CommandText +=
								"DELETE FROM [_properties] " +
								"WHERE [ObjectID]=" + objID + " AND " +
									  "[Name]='" + prop.Name + "';";

							// if it is PersistentStream property then delete property from _images table
							if( prop.Value.ToObject() is PersistentStream ) {
								cmd.CommandText +=
									"DELETE FROM [_images] " +
									"WHERE [ObjectID]=" + objID + " AND " +
									"[Name]='" + prop.Name + "';";
							}
							break;
					}
				}
			}

			// iteration throught links
			foreach( LINK link in links ) {
				// check link action
				if( link.State == LINK.STATE.New ) {
					// add new link to DB
					cmd.CommandText +=
						"INSERT INTO [_links] ([Parent], [Child]) " +
						"VALUES (" + objID + ", " + link.Header.ID + ");";
				} else if( link.State == LINK.STATE.Deleted ) {
					// delete link from DB
					cmd.CommandText +=
						"DELETE FROM [_links] " +
						"WHERE [Parent]=" + objID + " AND " + 
							  "[Child]=" + link.Header.ID + ";";
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
		string query = "SELECT Source.[ID] FROM (Select ID FROM _objects WHERE ObjectType = '" + type +"') AS Source " + 
						orderJoin +
						(string.IsNullOrEmpty(whereQuery) ? "" : "WHERE " + whereQuery) +
						(orderBy == "" ? "" : "ORDER BY " + orderBy);

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
			Debug.Print("[ERROR] @ ODB.Search: " + ex.ToString());
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
