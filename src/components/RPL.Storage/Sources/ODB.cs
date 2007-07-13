//****************************************************************************
//*   Project:   RPL.Storage
//*
//*   Module:      ODB.cs
//*
//*   Content:   Implements object oriented DB manipulation
//*   Author:      Alexander Kurbatov
//*   Copyright:   Copyright © 2006-2007 Alexander Kurbatov
//*
//*   Implement Search, Retrive, Save, Delete of CPersistentObject in MS SQL DB
//*
//****************************************************************************
using System;
using System.Data.SqlClient;
using System.Data.Common;
using System.IO;
using System.Text.RegularExpressions;
using System.Reflection;
using System.Collections.Generic;
using RPL;
using System.Data;

#if (DEBUG)
	using System.Diagnostics;
#endif


namespace RPL.Storage {
///<summary>Object Oriented DB implementation</summary>
public class ODB : MarshalByRefObject, IPersistenceStorage
{
	internal delegate SqlCommand GET_COMMAND( string cmdText );
	internal delegate SqlConnection GET_CONNECTION();
	
	//instance of object throught wich all SqlStream requests are handled

	private CSqlImageService m_SqlImageService;
	private SqlConnection m_con;
	private IBrokerCache m_cache;
	private SqlTransaction m_trans = null;
	private bool isClosed = true;
	private const string MSG_CLOSED = "Object is closed!";
	private const int BUFFER_LENGTH = 1024 * 1024;

	private const string REGEXP_BEGIN = @"(?i)(?:\((?:\s*)";
	private const string REGEXP_END = @"(?:\s*))\)";

	//pattern specifies following SQL predicate:
	//expression { = | < > | ! = | > | > = | ! > | < | < = | ! < } expression
	private const string REGEXP_PRED_COMPARE = REGEXP_BEGIN +
		@"(?<prop>\w+)(?:\s*)" +
		@"(?<cond>(?<op>(?:=|<>|!=|>|>=|!>|<|<=|!<))(?:\s*)" +
		@"(?<exp>(?<char_exp>\'.+\')" +
                  "|" +
                  @"(?<digit_exp>\-?\d+((\.|\,)\d+)?)" +
                  "|" +
                  @"(?<sub_exp>CAST\s*\(.+\))" + // catch CAST construction
                  "|" +
                  @"(?<sub_exp>CONVERT\s*\(.+\))" + // catch CONVERT construction
		"))" +
		REGEXP_END;
	//pattern specifies following SQL predicate:
	//string_expression [ NOT ] LIKE string_expression
	private const string REGEXP_PRED_LIKE = REGEXP_BEGIN +
		@"(?<prop>\w+)(?:\s+)" +
		@"(?<cond>(?<op>(?:NOT)?(?:\s+)?(?:LIKE))(?:\s+)" +
		@"(?<exp>(?<char_exp>\'.+\')" +
                  "|" +
                  @"(?<digit_exp>\-?\d+((\.|\,)\d+)?)" +
                  "|" +
                  @"(?<sub_exp>CAST\s*\(.+\))" + // catch CAST construction
                  "|" +
                  @"(?<sub_exp>CONVERT\s*\(.+\))" + // catch CONVERT construction
		"))" +
		REGEXP_END;
	//pattern specifies following SQL predicate: 
	//expression IS [ NOT ] NULL 
	private const string REGEXP_PRED_ISNULL = REGEXP_BEGIN +
		@"(?<prop>\w+)(?:\s+)(?<cond>(?<op>(?:(?:IS)(?:\s+)(?:NOT)?))(?:\s+)(?<exp>(?:NULL)))" + 
		REGEXP_END;

	//pattern specifies combination of all previous pattern
	private const string REGEXP_PREDS = REGEXP_PRED_COMPARE + "|" + 
										REGEXP_PRED_LIKE + "|" + 
										REGEXP_PRED_ISNULL;

	private const string REGEXP_CHECK = @"^(((?:\s*)(?<neg>NOT)?(?:\s*)(" + 
										REGEXP_PREDS + @")(?:\s*)(?<modif>AND|OR)))*(?:\s*)(?<neg>NOT)?(?:\s*)(?:\s*)(" + 
										REGEXP_PREDS + @")(?:\s)*$";

	///<summary>Default public constructor</summary>
	public ODB() {
#region debug info
#if (DEBUG)
Debug.Print( "-> ODB..ctor()" );
#endif
#endregion

		m_SqlImageService = 
			(CSqlImageService)(new SqlImageService(new GET_COMMAND( getCommand ),
													new GET_CONNECTION( get_connection )));
#region debug info
#if (DEBUG)
Debug.Print( "<- ODB..ctor()" );
#endif
#endregion
	}

	#region IPersistenceStorage Members
	///<summary>dispatch all internal objects</summary>
	public void Close()
	{
		#region debug info
		#if (DEBUG)
			Debug.Print( "-> ODB.Close()" );
		#endif
		#endregion

		m_cache = null;
		m_con = null;
		isClosed = true;
	}

	///<summary>Deletes object, it's properties and relations with other objects</summary>
	///<param name="iid">information about object to delete</param>
	public void Delete( IID iid )
	{
#region debug info
#if (DEBUG)
	Debug.Print( "-> ODB.Delete({0})", iid.ID );
#endif
#endregion

		if (isClosed) throw new InvalidOperationException( MSG_CLOSED );

		bool was_closed = (m_con.State == ConnectionState.Closed);
		
		if ( was_closed ) m_con.Open();
		#if (DEBUG)
			Debug.Print( m_con.State.ToString() );
		#endif
		
		try {
			SqlCommand cmdDelete = getCommand( "sp_DeleteObject" );
			cmdDelete.Parameters.Add( "@objectID",SqlDbType.Int ).Value = iid.ID;
			// specify use of sp_
			cmdDelete.CommandType = CommandType.StoredProcedure;

			// proccess delete opearaton
			cmdDelete.ExecuteNonQuery();
		} finally {
			if ( was_closed ) m_con.Close();
			
			#if (DEBUG)
				Debug.Print( m_con.State.ToString() );
			#endif
		}
#region debug info
#if (DEBUG)
	Debug.Print( "<- ODB.Delete({0})", iid.ID );
#endif
#endregion
	}

	///<summary>Opens CPersistentDataStorage</summary>
	///<remarks>Instance cann't be used before open invoke</remarks>
	///<param name="db">Opened connection all command will be executed on</param>
	///<param name="cache">Reference to BrokerCache of current session</param>
	public void Open( IDbConnection db, IBrokerCache cache )
	{
#region debug info
#if (DEBUG)
	Debug.Print( "-> ODB.Open(\"{0}\",\n\t{1} )", db.ConnectionString, cache.ToString() );
#endif
#endregion

		// check input parameters
		if ( db == null ) throw new ArgumentNullException( "db" );
		if ( cache == null ) throw new ArgumentNullException( "cache" );

		db.Open();// check connection availability
		db.Close(); // return connection state
		
		// save to class variables
		m_con = (SqlConnection)db;
		m_cache = cache;
		isClosed = false;

#region debug info
#if (DEBUG)
	Debug.Print( "<- ODB.Open()" );
#endif
#endregion
	}

	///<summary>Retrive properties and links for passed proxy object</summary>
	///<remarks>Retrived information contains full object instance</remarks>
	///<param name="iid">RPL.IID used to find certain object</param>
	///<param name="links">Return collection of referenced objects</param>
	///<param name="props">Returns collection of object's saved properties</param>
	public void Retrieve( IID iid, 
		                  ref System.Collections.Generic.IEnumerable<CPersistentObject> links, 
		                  ref System.Collections.Generic.IEnumerable<CPersistentProperty> props )
	{
		#region debug info
#if (DEBUG)
			Debug.Print( "-> ODB.Retrieve( {0}, {1}, {2} )", iid.ID, "", "" );
#endif
		#endregion

		// check object state
		if ( isClosed ) throw new InvalidOperationException( MSG_CLOSED );

		List<CPersistentObject> _links = new List<CPersistentObject>();
		List<CPersistentProperty> _props = new List<CPersistentProperty>();
		SqlDataReader sqlReader = null;
		SqlCommand cmdSql = null;

		//check input params
		if ( iid == null )
			throw new ArgumentNullException( "iid" );

		bool was_closed = (m_con.State == ConnectionState.Closed);

#if (DEBUG)
			Debug.Print( m_con.ToString() + " is "+ m_con.State.ToString() );
#endif

		if ( was_closed ) m_con.Open();
		
#if (DEBUG)
			Debug.Print( m_con.State.ToString() );
#endif

		try {
			#region retrive props from _properties
			cmdSql = getCommand("SELECT [Name], [Value] " +
								"FROM _properties " +
								"WHERE [ObjectID] = " + iid.ID );
			try {
				sqlReader = cmdSql.ExecuteReader( CommandBehavior.SequentialAccess );
				// read all simple properties of object
				while ( sqlReader.Read() ) {
					// build CPersistentProperty upon recieved name and value
					CPersistentProperty prop = 
						new CPersistentProperty( (string)sqlReader["Name"], 
													 sqlReader["Value"] );
					// save property in collection
					_props.Add( prop );
				}
			} finally {
				// Dispose SqlDataReader
				if ( sqlReader != null ) sqlReader.Close();
			}
			#endregion

			#region retrive props from _images
			cmdSql = getCommand( "SELECT [ID], [Name] " +
								 "FROM _images " +
								 "WHERE [ObjectID] = " + iid.ID );
			try {
				sqlReader = cmdSql.ExecuteReader( CommandBehavior.SequentialAccess );
				while ( sqlReader.Read() ) {
					// save data from SqlDataReader because we need non SequentialAccess in datarow
					int ID = (int)sqlReader["ID"];
					string Name = (string)sqlReader["Name"];
					// build CPersistentProperty upon recieved name and value
					CPersistentProperty Prop = 
						//new CPersistentProperty( Name, new SqlStream( ID , ) );
						new CPersistentProperty( Name, new SqlStream( ID, m_SqlImageService ) );

					// save property in collection
					_props.Add( Prop );
				}
			} 
			finally {
				// Dispose SqlDataReader
				if ( sqlReader != null ) sqlReader.Close();
			}
			#endregion

			#region retrive links
			// execute function in DB. Result must consist of ID separated with '|'
			cmdSql = getCommand( "SELECT @Links = [dbo].[fn_GetLinks](" + iid.ID + ")" );
			// add output parameter
			cmdSql.Parameters.Add( "@Links", SqlDbType.NVarChar, 4000 ).Direction = ParameterDirection.Output;
			// execute function
			cmdSql.ExecuteNonQuery();
			// save result to lose touch with SqlCommand
			string LinksID = (string)cmdSql.Parameters["@Links"].Value;
			// add object proxy to Link collection 
			foreach (string link in LinksID.Split(new char[1]{'|'}, StringSplitOptions.RemoveEmptyEntries)) {
				int ID = Convert.ToInt32( link );
				string Name = string.Empty;
				string ObjType = iid.Type;
				DateTime Stamp = new DateTime();
				//createPersistentObject
				CPersistentObject persObj = 
					m_cache.Search( ID, ObjType );
				if ( persObj == null ) {
					getProxyProps( ID, ref Name, ref ObjType, ref Stamp );
					persObj = createObject( ID, ObjType, Name, Stamp );
				}
				_links.Add( persObj );
			}
			#endregion
			// assign to output parameters
			links = (IEnumerable<CPersistentObject>)_links;
			props = (IEnumerable<CPersistentProperty>)_props;
		} finally {
			if ( was_closed ) m_con.Close();

#if (DEBUG)
				Debug.Print( m_con.State.ToString() );
#endif
		}
		#region debug info
		#if (DEBUG)
			Debug.Print( "<- ODB.Retrieve()" );
		#endif
		#endregion
	}
	
	///<summary>Retrive information about object</summary>
	///<remarks>Retrived information is used for building proxy objects</remarks>
	///<param name="iid">IID used to find certain object</param>
	///<param name="stamp">Return object save DataTime</param>
	///<param name="name">Return Name of saved object</param>
	public void Retrieve( IID iid, ref DateTime stamp, ref string name )
	{
		// check object state
		if ( isClosed ) throw new InvalidOperationException( MSG_CLOSED );
		if ( iid == null )
			throw new ArgumentNullException( "iid" );

		#region debug info
#if (DEBUG)
		Debug.Print( "-> ODB.Retrieve( {0}, , )", iid.ID );
#endif
		#endregion

		string ObjType = string.Empty;
		bool was_closed = (m_con.State == ConnectionState.Closed);

		#region debug info
#if (DEBUG)
		Debug.Print( m_con.ToString() + " is "+ m_con.State.ToString() );
#endif
		#endregion

		if ( was_closed ) m_con.Open();

		#region debug info
#if (DEBUG)
		Debug.Print( m_con.State.ToString() );
#endif
		#endregion

		try {
			getProxyProps( iid.ID, ref name, ref ObjType, ref stamp );
		} finally {
			if ( was_closed ) m_con.Close();
		#region debug info
#if (DEBUG)
		Debug.Print( m_con.State.ToString() );
#endif
		#endregion
		}
		
		#region debug info
		#if (DEBUG)
			Debug.Print( "<- ODB.Retrieve()");
		#endif
		#endregion
	}

	///<summary>Saves object instance to DB</summary>
	///<param name="obj">CPersistentObject used to save object proxy properties</param>
	///<param name="id">Return id of saved object</param>
	///<param name="stamp">Return object save DataTime</param>
	///<param name="name">Return Name of saved object</param>
	///<param name="links">Reference to collection of linked objects to save object relations</param>
	///<param name="props">Reference to properties collection to save object properties</param>
	///<param name="newlinks">Return collection of referenced objects</param>
	///<param name="newprops">Returns collection of object's saved properties</param>
	public void Save( CPersistentObject obj, 
		              ref int id, ref DateTime stamp, ref string name,
		              CObjectLinks links, CObjectProperties props,
		              ref IEnumerable<CPersistentObject> newlinks,
		              ref IEnumerable<CPersistentProperty> newprops )
	{
		// check object state
		if ( isClosed )	throw new InvalidOperationException( MSG_CLOSED );
		//check input params
		if ( obj == null )
			throw new ArgumentNullException( "obj" );
		if (links == null)
			throw new ArgumentNullException( "links" );
		if ( props == null )
			throw new ArgumentNullException( "props" );

		#region debug info
#if (DEBUG)
		Debug.Print( "-> ODB.Save( {0}, {1}, {2}, {3}, {4}, {5},{6}, {7} )",
				obj.GetType(), "", "", "",
				links.ToString(), props.ToString(),
				"", "" );
		Debug.Print( "\t\t{0} ( ID:{1}, Name:'{2}', Stamp:{3} )", obj.GetType(), obj.ID, obj.Name, obj.Stamp );
#endif
		#endregion

		SqlCommand cmdInsert = null; //command used for insert purpose
		SqlCommand cmdUpdate = null; //command used for update purpose
		SqlCommand cmdSql = null;    //other puroses
		CPersistentProperties outprops = 
			new CPersistentProperties( props ); //list for collecting current object properties
		CPersistentObjects outlinks = 
			new CPersistentObjects( links ); //list for collecting current object properties
		List<string> oldProps = new List<string>(); //to store old props (need to delete props with the name in this list @ the end)
		int objID; //object ID


		bool was_closed = ( m_con.State == ConnectionState.Closed );
		#region debug info
#if (DEBUG)
		Debug.Print( m_con.ToString() + " is "+ m_con.State.ToString() );
#endif
		#endregion

		if ( was_closed ) {
			TransactionBegin();
			#region debug info
#if (DEBUG)
			Debug.Print( "Local transaction was started" );
#endif
			#endregion
		}
		#region debug info
#if (DEBUG)
		Debug.Print( m_con.ToString() + " is "+ m_con.State.ToString() );
#endif
		#endregion
		try {
			#region create object item if it is new or check it's stamp
			//assign command which will insert or (check, update) object record in DB
			cmdSql =  createValidateScript( obj );
			cmdSql.ExecuteNonQuery();
			// get object id returned after command execution
			objID = Convert.ToInt32( cmdSql.Parameters[ "@ID" ].Value );
			#endregion

			#region get available properties for this object
			cmdSql = getCommand( "[sp_GetObjectProperies]" );
			cmdSql.Parameters.Add( "@objectID", SqlDbType.Int ).Value = objID;
			cmdSql.CommandType = CommandType.StoredProcedure;
			SqlDataReader sdr = cmdSql.ExecuteReader( CommandBehavior.SequentialAccess );
			// read all props that are available for current object in DB
			try {while ( sdr.Read() ) oldProps.Add( sdr.GetString( 0 ));}
			finally {sdr.Close();}
			// needed for BinarySearch later
			oldProps.Sort();
			#endregion
			
			// modify props only in case of their change in object
			if ( props.IsChanged ) {
				#region update/save properties
				foreach ( CPersistentProperty prop in props ) {
					int imgID = -1; // ID of image row inserted or updated
					int oldPropIndex = oldProps.BinarySearch( prop.Name );

					//save property only if it is changed
					if ( prop.IsChanged ) {
						if ( prop.Value is Stream ) {
							//if property was already saved (retrive from DB gives SqlStream)
							//so we'll update existing record 
							if ( prop.Value is SqlStream ) {
								imgID = ((SqlStream)prop.Value).ID;
								m_SqlImageService.Save( imgID,
														(Stream)prop.Value );
							} else { //property value is a new stream
								m_SqlImageService.Save( ref imgID, 
														objID, 
														prop.Name,
														(Stream)prop.Value );
							}
						} else {				
							#region MAKE COMMAND FOR SAVE/UPDATE Variant PROPERTIES
							string PropName = prop.Name.Replace( "'", "''" );

							//adopt DateTime value to Sql precision
							if (prop.Value.GetType() ==
								typeof(DateTime)) { outprops[prop.Name].Value = dateTime2Sql((DateTime)prop.Value);}
							
							if ( oldProps.Contains( prop.Name ) ) {
								// this is old property
								if ( cmdUpdate == null ) {
									cmdUpdate = getCommand( string.Empty );
								}
								cmdUpdate.CommandText += " UPDATE _properties "+
														 " SET [Value] = @PropValue_" + PropName +
														 " WHERE ( [ObjectID] = " + objID + " AND " +
																  "[Name] = '" + PropName + "');";
								// add parametrs to command
								cmdUpdate.Parameters.Add( "@PropValue_" + PropName, SqlDbType.Variant ).Value = 
									outprops[prop.Name].Value;
							} else {
								//this is new property
								if ( cmdInsert == null ) {
									cmdInsert = getCommand( string.Empty );
								}
								cmdInsert.CommandText += "INSERT INTO _properties ([ObjectID], [Name], [Value]) " +
														 "VALUES (" + objID + ", " +
																 "'" + PropName + "', " +
																 "@PropValue_" + PropName + ");";
								// add parametrs to command
								cmdInsert.Parameters.Add( "@PropValue_" + PropName, SqlDbType.Variant ).Value = 
									outprops[prop.Name].Value;
							}
						#endregion
						}
					}
					//if stream was replaced or created than assign new stream
					if ( imgID != -1 )
						outprops[prop.Name].Value =
							new SqlStream( imgID, m_SqlImageService );
					//remove property name from list
					if ( oldPropIndex >= 0 )
						oldProps.RemoveAt( oldPropIndex );
				}
				// try to execute first update command
				if ( cmdUpdate != null ) {
					cmdUpdate.ExecuteNonQuery(); cmdUpdate = null;
				}

				// try to execute insert command
				if ( cmdInsert != null ) {
					cmdInsert.ExecuteNonQuery(); cmdInsert = null;
				}
				#endregion

				#region DELETE PROPERTIES FROM DB IN CASE THEY ARE IN oldProps
				if (oldProps.Count > 0 ) {
					string inQuery = string.Empty;
					foreach (string prop in oldProps) inQuery += "'" + prop + "',";

					inQuery = inQuery.TrimEnd( new char[1]{','} );
					cmdSql = getCommand( "DELETE FROM _images " +
										 "WHERE [ObjectID] = " + objID +
												" AND " +
												"[Name] in (" + inQuery + ");" +
										 "DELETE FROM _properties " +
										 "WHERE [ObjectID] = " + objID +
												" AND " +
												"[Name] in (" + inQuery + ");" );
					cmdSql.ExecuteNonQuery();
				}
				#endregion
			}

			//modify links only in case of their modification in object
			if ( links.IsListChanged ) {
				#region save object links
				//create list of referenced object ID's
				List<int> temp_LinksID = new List<int>();
				foreach ( CPersistentObject link_obj in outlinks ) {
					// check that object isn't new or delete
					int linkId = link_obj.ID;
					if ( linkId > 0 ) {
						temp_LinksID.Add( linkId );
					}
				}
				temp_LinksID.Sort();
				SqlDataAdapter da =
					new SqlDataAdapter( getCommand( "SELECT [ObjectID_1], [ObjectID_2] " +
													"FROM _links " +
													"WHERE (([ObjectID_1] = " + objID + ") " +
													"OR " +
													"([ObjectID_2] = " + objID + "))" ) );
				DataSet ds = new DataSet();
				SqlCommandBuilder scb = new SqlCommandBuilder( da );
				da.Fill( ds );

				//check table count
				if ( ds.Tables.Count > 0 ) {
					//TODO: check what is ds.Tables[0]
					DataTable dt = ds.Tables[0];
					if ( dt.Rows.Count > 0 ) {
						foreach ( DataRow dr in dt.Rows ) {
							// save id of object this related with
							int relID = (((int)dr["ObjectID_1"] == objID) ? (int)dr["ObjectID_2"] : (int)dr["ObjectID_1"]);
							// search and delete those row which contain canceled relations
							if ( temp_LinksID.BinarySearch( relID ) < 0 )
								dr.Delete();
							// relation was found, so we don't need to add it later
							else 
								temp_LinksID.Remove( relID );
						}
					}
					// now try to add new rows if required
					foreach ( int i in temp_LinksID ) {
						// add relation
						dt.Rows.Add( (new Object[] { objID, i }) );
					}
					// execute update of DataSet to server
					da.Update( ds );
				} else
					throw new InvalidOperationException( "Propbably DB structure is changed!" );
				#endregion
			}

			#region get object proxy atributes
			name = string.Empty; //initialize variable
			stamp = new DateTime(); //initialize variable
			string type = string.Empty; //willn't be used but must be passed.

			// retrive object proxy properties
			getProxyProps(objID, ref name, ref type , ref stamp);
			// IF TRIGERS IS USED:
			// MAY NEED FULL PROPS RETRIVE?!
			#endregion

			// assign out parameters
			id = objID;
			//create full copy of collections
			newlinks = (IEnumerable<CPersistentObject>)outlinks;// 
			newprops = (IEnumerable<CPersistentProperty>)outprops;//

			//check that local transaction was used
			if ( was_closed ) {
				//try to commit local transaction
				TransactionCommit();

				#region debug info
#if ( DEBUG )
				Debug.Print( "Local transaction was commited" );
#endif
				#endregion
			}
		} catch ( Exception ex ) {
			//role back local transaction
			if ( was_closed ) {
				//try to commit local transaction
				TransactionRollback();

				#region debug info
#if (DEBUG)
				Debug.Print( "Local transaction was rollbacked" );
#endif
				#endregion
			}
			throw ex;
		}

		#region debug info
#if (DEBUG)
		Debug.Print( m_con.ToString() + " is "+ m_con.State.ToString() );
		Debug.Print( "<- ODB.Save() : \t\t{0} ( ID:{1}, Name:'{2}', Stamp:{3} )", obj.GetType(), id, name, stamp );
#endif
		#endregion
	}

	///<summary>Search objects that sutisfies CPersistentCriteria parameter.</summary>
	///<param name="crit">The CPersistentCriteria that defines the conditions of the element to search for.</param>
	///<param name="objs">Collection filled with found objects</param>
	///<returns>The number of elements contained in the objs param</returns>
	///<remarks>Search criteria in CPersistent criteria must meet following requirements:
	///{ [ NOT ] &#60;predicate&#62; | ( &#60;search_condition&#62; ) } 
	///[ { AND | OR } [ NOT ] { &#60;predicate&#62; | ( &#60;search_condition&#62; ) }
	///Where:
	/// &#60;predicate&#62; ::= 
	///{ expression { = | &#60; &#62; | ! = | &#62; | &#62; = | ! &#62; | &#60; | &#60; = | ! &#60; } expression 
    ///| string_expression [ NOT ] LIKE string_expression 
	///| expression IS [ NOT ] NULL
	///}
	///Left expression:
	///For all date types except int CAST or CONVERT must be declared.
	///For example: 
	///<list type="bullet">
	/// <item><term>( Prop_1 = CAST( dt.ToString( "yyyy-MM-dd HH:mm:ss.fff" ) as datetime ) </term><description> for (DateTime dt) </description></item>
	/// <item><term>( Prop_1 = CAST( (bl ? 1 : 0) as bit )  </term><description> for (Boolean bl)</description></item>
	/// <item><term>( Prop_1 = 123123 )  </term><description> for int </description></item>
	/// <item><term>( Prop_1 = CONVERT( dbl.ToString( "R" ) + " as float ) as float )  </term><description> for (Double dbl)</description></item>
	/// </list>
	///
	///</remarks>
	public int Search( CPersistentCriteria crit,
					   ref System.Collections.Generic.IEnumerable<CPersistentObject> objs )
	{
		// check input parameter
		if ( crit == null )
			throw new ArgumentNullException( "crit" );

		#region debug info
#if (DEBUG)
		Debug.Print( "-> ODB.Search( {0}, {1})", crit.GetType(), "" );
#endif
		#endregion

		if ( !string.IsNullOrEmpty( crit.Where ) ) {
			if ( !Regex.IsMatch( crit.Where, REGEXP_CHECK ) )
				throw new ArgumentException( "Invalid search condition format!" );
		}

		List<CPersistentObject> return_objects = new List<CPersistentObject>();
		MatchCollection mc;
		int count = 0;
		string sqlWhereText = crit.Where;

		#region adopt search condition according to DB structure
		// get collection of predicates which will be replaced
		mc = Regex.Matches( sqlWhereText, REGEXP_PREDS );
		// replace each predicate in search condition with one that fits DB
		for ( int i = mc.Count - 1; i >= 0; i-- ) {
			sqlWhereText =
				sqlWhereText.Replace( mc[i].Value,
									  tunePridicate( mc[i].Value ));
		}
		#endregion
		// surround expression with brackets
		sqlWhereText = "(" + ( sqlWhereText == "" ? "1=1" : sqlWhereText ) + ")";
		
		// add Object Type definition if exist to search condition
		if ( !string.IsNullOrEmpty(crit.Type) ) {
			sqlWhereText = "(ObjectType = '" + crit.Type + "') AND " + sqlWhereText;
		}
		bool was_closed = (m_con.State == ConnectionState.Closed);
		#region debug info
#if (DEBUG)
		Debug.Print( m_con.ToString() + " is "+ m_con.State.ToString() );
#endif
		#endregion

		if ( was_closed ) m_con.Open();
		#region debug info
#if (DEBUG)
	Debug.Print( m_con.State.ToString() );
#endif
		#endregion

		try {
			//sp_GetObjectsByCriteria will return table with the following columns:
			//ID, ObjectName, ObjectType, TimeStamp
			SqlCommand cmdSelect = getCommand( "sp_GetObjectsByCriteria" );
			cmdSelect.CommandType = CommandType.StoredProcedure;
			cmdSelect.Parameters.Add( "@Request", SqlDbType.NVarChar, sqlWhereText.Length).Value = sqlWhereText;
			cmdSelect.Parameters.Add( "@View", SqlDbType.NVarChar, "_objects".Length ).Value = "_objects";
			cmdSelect.Parameters.Add( "@First", SqlDbType.Int ).Value = crit.BottomLimit;
			cmdSelect.Parameters.Add( "@Count", SqlDbType.Int ).Value = crit.CountLimit;
			cmdSelect.Parameters.Add( "@OrderBy", SqlDbType.NVarChar, crit.OrderBy.Length ).Value = crit.OrderBy;

			#region get DateTable with results
			SqlDataAdapter da = new SqlDataAdapter( cmdSelect );
			DataTable dt = new DataTable(); // table for object proxy properties
			da.Fill( dt ); //fill table
			#endregion

			#region create reader over table we got
			DataTableReader dtr = new DataTableReader( dt );
			try {
				while ( dtr.Read() ) {
					//save properties we found
					int id = Convert.ToInt32( dtr["ID"] );
					string name = (string)dtr["ObjectName"];
					string type = (string)dtr["ObjectType"];
					DateTime stamp = Convert.ToDateTime( dtr["TimeStamp"] );

					CPersistentObject obj =
							m_cache.Search( id, type, stamp, name );
					//object wasn't found in cache. manualy create proxy object.
					if ( obj == null ) {
						//retrive properties of object from DB
						getProxyProps( id, ref name, ref type, ref stamp );
						//create new object based on retrived poperties
						obj = createObject( id, type, name, stamp );
					}
					//add object to output List
					return_objects.Add( obj );
					//increment counter
					count += 1;
				}
			} finally { dtr.Close(); }
			#endregion

			//set output parameter
			objs = (IEnumerable<CPersistentObject>)return_objects;
		} finally {
			if ( was_closed ) m_con.Close();

			#if (DEBUG)
				Debug.Print( m_con.State.ToString() );
			#endif
		}
		#region debug info
#if (DEBUG)
		Debug.Print( "<- ODB.Search( {0}, {1}) = {2}", crit.GetType(), objs.ToString(), count );
#endif
		#endregion

		return count;
	}

	public void TransactionBegin()
	{
		// check object state
		if ( isClosed )
			throw new InvalidOperationException( MSG_CLOSED );

		#region debug info
#if (DEBUG)
		Debug.Print( "-> ODB.TransactionBegin()" );
#endif
		#endregion

		if ( m_trans == null ) {
			m_con.Open();
			m_trans = m_con.BeginTransaction();
		}
		else throw new InvalidOperationException( "Parallel transactions are not supported.");

	#region debug info
#if (DEBUG)
	Debug.Print( "<- ODB.TransactionBegin()" );
#endif
	#endregion
	}

	public void TransactionCommit()
	{
		#region debug info
#if (DEBUG)
		Debug.Print( "-> ODB.TransactionCommit()" );
#endif
		#endregion

		// check object state
		if ( isClosed )
			throw new InvalidOperationException( MSG_CLOSED );

		if ( m_trans == null )
			throw new InvalidOperationException( "The transaction has already been committed or rolled back." );
		else {
			try {
				m_trans.Commit();
			} finally { m_trans.Dispose(); m_trans = null; m_con.Close(); }
		}

		#region debug info
#if (DEBUG)
		Debug.Print( "<- ODB.TransactionCommit()" );
#endif
		#endregion
	}

	public void TransactionRollback()
	{
#region debug info
#if (DEBUG)
	Debug.Print( "-> ODB.TransactionRollback()" );
#endif
#endregion

		// check object state
		if ( isClosed )
			throw new InvalidOperationException( MSG_CLOSED );

		if ( m_trans == null )
			throw new InvalidOperationException( "The transaction has already been committed or rolled back." );
		else {
			try {
				m_trans.Rollback();
			} finally { m_trans.Dispose(); m_trans = null; m_con.Close(); }
		}
#region debug info
#if (DEBUG)
	Debug.Print( "<- ODB.TransactionRollback()" );
#endif
#endregion
	}
	#endregion

	private SqlCommand getCommand( string cmdText ){
		// if connection is opened than transaction may exist
		if ( m_trans != null )
			// return command attached to transaction
			return new SqlCommand( cmdText, m_con, m_trans );
		else //return new command not attached to transaction
			return new SqlCommand( cmdText, m_con );
	}

	//gets saved object properties
	private void getProxyProps( int id, 
		                        ref string name,
		                        ref string type,
		                        ref DateTime stamp )
	{
		SqlCommand cmdSql =
			getCommand( "SELECT @Name = [ObjectName], @Type = [ObjectType], @Stamp = [TimeStamp] " +
						 "FROM _objects " +
						 "WHERE [ID] = " + id );
		cmdSql.Parameters.Add( "@Name", SqlDbType.NVarChar, 4000 ).Direction = ParameterDirection.Output;
		cmdSql.Parameters.Add( "@Type", SqlDbType.NVarChar, 4000 ).Direction = ParameterDirection.Output;
		cmdSql.Parameters.Add( "@Stamp", SqlDbType.DateTime ).Direction = ParameterDirection.Output;
		cmdSql.ExecuteNonQuery();
		try {
			// assign return parameters
			name = (string)cmdSql.Parameters["@Name"].Value;
			type = (string)cmdSql.Parameters["@Type"].Value;
			stamp = (DateTime)cmdSql.Parameters["@Stamp"].Value;
		} catch (Exception ex) {
			throw new ArgumentException( "Object doesn't exist in DB!", ex );
		}
	}

	///<param name="Predicate">EX. (Prop1 != value ) OR (Prop1 [NOT] IS NULL) ...</param>
	///<returns>SQL predicate that tuned to DB structure</returns>
	private string tunePridicate(string Predicate) 
	{
		string prop = string.Empty;
		string val = string.Empty;
		string query = string.Empty;


		//decompile input string using RegExp.
		Match match = Regex.Match( Predicate,
							Regex.IsMatch( Predicate, REGEXP_PRED_COMPARE ) ? 
								REGEXP_PRED_COMPARE : (Regex.IsMatch( Predicate, REGEXP_PRED_ISNULL ) ? 
								REGEXP_PRED_ISNULL : REGEXP_PRED_LIKE ));
		
		//check for proxy object properties
		if ( match.Groups["prop"].Value.Equals( "ID", StringComparison.CurrentCultureIgnoreCase )) {
			// doesn't need changing
			return Predicate;
		} else if ( match.Groups["prop"].Value.Equals( "NAME", StringComparison.CurrentCultureIgnoreCase ) ) {
			//change column name
			return "( ObjectName " + match.Groups["cond"].Value + ")";
		} else {
			prop = (string.IsNullOrEmpty(match.Groups["char_exp"].Value) ? "Value" : "CAST(Value as nvarchar(4000))");
			
			// modify query to match _properies table and it's structure
			query += "(EXISTS " +
						"(SELECT ID " +
						" FROM _properties AS p " + // Image comparison is unsupported
						" WHERE " +
							"(ObjectID = Source.ID) AND " + // declare object id property belongs
								//avoid implicit type conversion by MS SQL if non digit or string value is compareted
								(string.IsNullOrEmpty(match.Groups["sub_exp"].Value) ? 
									string.Empty 
									:
									"(SQL_VARIANT_PROPERTY( " + match.Groups["sub_exp"].Value  + ", 'BaseType') = " +
									"SQL_VARIANT_PROPERTY( Value, 'BaseType')) AND " ) +
  							"(Name = '" + match.Groups["prop"] + "') AND ("  +// name of Property
							// workaround for comparison of string type property values
							prop + 
							" " + match.Groups["cond"] + ")))"; // Value and comparison of property value
		}
		return query;
	}

	///<summary>Create new CPersistentObject from Assembly. To do this proper objectType
	/// required (Ex: "NameSpace.ClassName,AssemblyName" )</summary>
	///<param name="id">ID of saved object</param>
	///<param name="objectType">Type of object</param>
	///<param name="objectName">Name of object</param>
	///<param name="timeStamp">Time of object save</param>
	///<returns>CPersistent proxy object build on input parameters</returns>
	private CPersistentObject createObject(int id, 
		                                   string objectType,
		                                   string objectName,
		                                   DateTime timeStamp)
	{		
		try {
			// Load assembly by its name. (Assebly name is the last substring in objectType after '|'
			// Then get type of Object from assembly.
            Type t = 
				Assembly.Load( 
					objectType.Substring( objectType.IndexOf(',') + 1 )).GetType( 
											objectType.Substring( 0 , objectType.IndexOf(',') ));

			// Create array of parametrs to path
			// int id, DateTime stamp, String ^name
			object[] args = new object[] { id, timeStamp, objectName };

			// Creates an instance of the specified type using the
			// constructor that best matches the specified parameters. 
			return (CPersistentObject)Activator.CreateInstance( t, args );
		} catch ( Exception ex ) { throw new NotSupportedException( "Type of requested object is not supported", ex ); }
	}


	///<summary>Create script for (addition of object) or (update and check it's stamp)</summary>
	///<param name="obj">CPersistentObject actions are generated for</param>
	///<returns>new SqlCommand that have @ID parameter to get object's ID</returns>
	private SqlCommand createValidateScript( CPersistentObject obj )
	{
		SqlCommand cmd;

		if ( obj.ID == 0 ) {// this is new ob+ject. Creating script for unsertion of object
			cmd = getCommand( "INSERT INTO _objects ( [ObjectName], [ObjectType] ) " +
			         "VALUES ( '" + obj.Name.Replace( "'", "''" ) + "', " + 
							  "'" + obj.Type.Replace( "'", "''" ) + "');" +
			         "SET @ID = SCOPE_IDENTITY();" ); // save inserted object ID

		} else {
			// check object stamp. If it is new then current -> raise error
			cmd = getCommand( "IF ((SELECT [TimeStamp] from _objects WHERE [ID] = @ID) > @Stamp) " +
							      "RAISERROR( 'Newer object exist. Please retrive object first!', 11, 1 );" );
			cmd.Parameters.Add( "@Stamp", SqlDbType.DateTime ).Value = 
				obj.Stamp;
			// if object is changed then update it's name 
			if ( obj.IsChanged ){
				cmd.CommandText += 
					"UPDATE _objects SET [ObjectName] = @Name WHERE [ID] = @ID";
				cmd.Parameters.Add( "@Name", SqlDbType.NVarChar, obj.Name.Length ).Value = 
					obj.Name;
			}
		}
		//add parameter that is always used
		cmd.Parameters.Add( "@ID", SqlDbType.Int).Value = obj.ID;
		cmd.Parameters[ "@ID" ] .Direction = ParameterDirection.InputOutput;

		return cmd;
	}


	///<summary>Converts DateType type according to MS Sql Server precision.</summary>
	///<param name="dt">DateTime to be converted</param>
	///<returns>DateTime rounded to .**0, .**3, .**7 milliseconds</returns>
	private DateTime dateTime2Sql( DateTime dt )
	{
		int remainder = dt.Millisecond % 10; //need to know last digit
		int add; //stores delta

		//get correction delta value for milliseconds
		if ( remainder == 9) {
			add = 1;
		} else if (	remainder >= 5 && remainder <= 8 ) {
			add = 7 - remainder;
		} else if ( remainder >= 2 && remainder <= 4 ) {
			add = 3 - remainder;
		} else {
			add = -remainder;
		}
		
		//create new corrected DateTime
		return new DateTime(dt.Year, dt.Month, dt.Day, 
							dt.Hour, dt.Minute, dt.Second, dt.Millisecond + add);
	}

	//used for delegate that returns connection
	private SqlConnection get_connection(){
		return m_con;
	}

	internal class SqlImageService : CSqlImageService
	{
		//table name in DB
		private	string m_tableName = "_images";
		//column name in DB
		private string m_columnName = "Value";
		private int BUFFER_LENGTH = 1024 * 1024; //chunk size
		GET_COMMAND m_getcommand;
		GET_CONNECTION m_con;


		internal SqlImageService( GET_COMMAND del_com, GET_CONNECTION del_con )
		{
			#region debug info
#if (DEBUG)
			Debug.Print( "-> SqlImageService.con( {0}, {1} ) @ \n\t{2}", "", "", AppDomain.CurrentDomain.FriendlyName );	
#endif
			#endregion

			m_getcommand = del_com;
			m_con = del_con;
		}

		#region CSqlImageService Members
		public override void Save( ref int id, int objID, string propName, Stream stream )
		{
			//TODO: check input params

			SqlCommand cmdGetPointer =
				m_getcommand(
					// prop is new! create it
					"INSERT INTO _images ([ObjectID], [Name], [Value]) " +
			        "VALUES (" + objID + ", '" + propName + "', " +
							((stream.Length > 0) ? "0x0" : "NULL") + " ); " + 
			        // Store inserted item id
			        "SET @ID = SCOPE_IDENTITY(); " +
			        // get Pointer to insert
			        "SELECT @Pointer = TEXTPTR(Value) " + 
			        "FROM _images " +
			        "WHERE [ID] = @ID;" );

			SqlParameter pointerOutParam =
				cmdGetPointer.Parameters.Add( "@Pointer", SqlDbType.VarBinary, 16 );
			pointerOutParam.Direction = ParameterDirection.Output;
			cmdGetPointer.Parameters.Add("@ID", SqlDbType.Int).Direction = 
				ParameterDirection.Output;
			//save connection state
			bool was_closed = (m_con().State == ConnectionState.Closed);
			//if connection was closed then open it
			if ( was_closed )
				m_con().Open();
			try {
				cmdGetPointer.ExecuteNonQuery();
				if ( pointerOutParam.Value == null )
					throw new KeyNotFoundException( "Specified value is absent!" );
				//save image
				save( stream, pointerOutParam );
				//assign return parameter
				id = Convert.ToInt32(cmdGetPointer.Parameters["@ID"].Value);
			} finally { if ( was_closed ) m_con().Close(); /*close connection if was closed*/}
		}

		public override void Save( int id, Stream stream ) {
			//TODO: check input params

			SqlCommand cmdGetPointer =
			    m_getcommand(" UPDATE _images SET Value = " + (( stream.Length > 0 ) ? "0x0" : "NULL") +
							 " WHERE [ID] = " + id + ";" +
							 " SELECT @Pointer=TEXTPTR(Value)" + 
							 " FROM _images"+
							 " WHERE [ID] = " + id + ";");
			SqlParameter pointerOutParam =
				cmdGetPointer.Parameters.Add( "@Pointer", SqlDbType.VarBinary, 16 );
			pointerOutParam.Direction = ParameterDirection.Output;

			//save connection state
			bool was_closed = (m_con().State == ConnectionState.Closed);
			//if connection was closed then open it
			if ( was_closed ) m_con().Open();
			try {
				cmdGetPointer.ExecuteNonQuery();
				if ( pointerOutParam.Value == null ) 
					throw new KeyNotFoundException( "Specified value is absent!" );
				save( stream, pointerOutParam );
			} finally { if ( was_closed ) m_con().Close(); /*close connection if was closed*/}
		}

		private void save(Stream stream, SqlParameter pointerParam )
		{
			//set up UPDATETEXT command, parameters, and open BinaryReader.	
			SqlCommand cmdUpload =
				m_getcommand( "UPDATETEXT _images.Value @Pointer @Offset @Delete " +
			                  "WITH LOG @Bytes" );
			//assign value of pointer previously recieved
			cmdUpload.Parameters.Add( "@Pointer", SqlDbType.Binary, 16 ).Value =
				pointerParam.Value;
			SqlParameter OffsetParam =
				cmdUpload.Parameters.Add( "@Offset", SqlDbType.Int );
			SqlParameter DeleteParam =
				cmdUpload.Parameters.Add( "@Delete", SqlDbType.Int );
			DeleteParam.Value = 1; //delete 0x0 character
			SqlParameter BytesParam =
				cmdUpload.Parameters.Add( "@Bytes", SqlDbType.Binary );
			stream.Seek( 0, SeekOrigin.Begin );

			OffsetParam.Value = 0; //start insertion from begin
			//read buffer full of data and execute UPDATETEXT statement.
			Byte[] Buffer = new Byte[BUFFER_LENGTH];
			int ret = stream.Read( Buffer, 0, BUFFER_LENGTH );

			while ( ret > 0 ) {
				BytesParam.Value = Buffer;
				BytesParam.Size = ret;
				cmdUpload.ExecuteNonQuery(); //execute iteration
				DeleteParam.Value = 0; //don't delete any other data
				//prepare to next iteration
				OffsetParam.Value =
					Convert.ToInt32( OffsetParam.Value ) + ret;
				ret = stream.Read( Buffer, 0, BUFFER_LENGTH );
			}
		}

		public override int Read( int id, byte[] buffer, int offset, int count, long position )
		{
			//TODO: Check input params
			
			//save connection state
			bool was_closed = (m_con().State == ConnectionState.Closed);
			//if connection was closed then open it
			if ( was_closed ) m_con().Open();
			try {
				//obtain a pointer to the BLOB using TEXTPTR.
				SqlCommand cmdGet = m_getcommand(
							" SELECT @Pointer = TEXTPTR(" + m_columnName + ")," +
								   " @Length = DataLength(" + m_columnName + ")" +
			                " FROM " + m_tableName +
			                " WHERE ID = " + id );
				//set up the parameters.
				cmdGet.Parameters.Add( "@Pointer", SqlDbType.VarBinary, 16 ).Direction =
			        ParameterDirection.Output;
				cmdGet.Parameters.Add( "@Length", SqlDbType.Int ).Direction =
			        ParameterDirection.Output;
				cmdGet.ExecuteNonQuery();
				//check that BLOB exists
				if ( cmdGet.Parameters["@Pointer"].Value == null ) {
					throw new KeyNotFoundException( "Specified value is absent!" );
				}
				//run the query.
				//set up the READTEXT command to read the BLOB by passing the following
				//parameters: @Pointer – pointer to blob, @Offset – number of bytes to
				//skip before starting the read, @Size – number of bytes to read.
				SqlCommand cmdRead =  m_getcommand(
							" READTEXT " + m_tableName + "." + m_columnName + 
			                " @Pointer @Offset @Size HOLDLOCK" );
				//set up the parameters for the command.
				cmdRead.Parameters.Add( "Pointer", SqlDbType.VarBinary, 16 ).Value  = 
			        cmdGet.Parameters[ "@Pointer" ].Value;
				cmdRead.Parameters.Add( "@Offset", SqlDbType.Int ).Value = position;
				cmdRead.Parameters.Add( "@Size", SqlDbType.Int );
				//calculate buffer size - may be less than BUFFER_LENGTH for last block.				
				if ( (position + count) >= Convert.ToInt32( cmdGet.Parameters["@Length"].Value ) )
					cmdRead.Parameters["@Size"].Value =
			            Convert.ToInt32( cmdGet.Parameters["@Length"].Value ) - position;
				else
					cmdRead.Parameters["@Size"].Value = count;
				//execute reader
				SqlDataReader dr =
			        cmdRead.ExecuteReader( CommandBehavior.SingleRow );
				try {
					//read data from SqlDataReader
					dr.Read();
					//create array needed dimention
					buffer = new byte[Convert.ToInt32(cmdRead.Parameters["@Size"].Value)];
					//read data to buffer and change read position
					//return size of read data
					return Convert.ToInt32(
							dr.GetBytes( 0,
										 0,
										 buffer,
										 0,
										 Convert.ToInt32( cmdRead.Parameters["@Size"].Value ) ));
				} finally {	dr.Dispose(); /*dispose DataReader*/}
			} finally { if ( was_closed ) m_con().Close(); /*close connection if was closed*/}
		}

		public override long Length( int id )
		{
			//save connection state
			bool was_closed = (m_con().State == ConnectionState.Closed);
			//if connection was closed then open it
			if ( was_closed )
				m_con().Open();
			try {
				//set command to get length of image property.
				SqlCommand cmdGetLength = m_getcommand(
							" SELECT @Length = DataLength(" + m_columnName + ")" +
			                " FROM " + m_tableName +
			                " WHERE ID = " + id );
				//set up the parameters.
				cmdGetLength.Parameters.Add( "@Length", SqlDbType.Int ).Direction = 
					ParameterDirection.Output;
				cmdGetLength.ExecuteNonQuery();

				if ( cmdGetLength.Parameters["@Length"] == null ) {
					throw new KeyNotFoundException( "Specified value is absent!" );
				} else { return Convert.ToInt32( cmdGetLength.Parameters["@Length"] ); }
			} finally { if ( was_closed ) m_con().Close(); /*close connection if was closed*/}
		}
		#endregion
	}
}
}