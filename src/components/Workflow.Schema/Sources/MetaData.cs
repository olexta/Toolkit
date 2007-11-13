using System;
using System.Collections.Generic;
using System.Xml;
using System.Xml.Schema;
using System.Globalization;

using Toolkit.Collections;

namespace Workflow.Schema
{
	public class MetaData
	{
		/// <summary>
		/// Error msg.
		/// </summary>
		private static readonly string c_AlreadyInitedMsg = "Schema is already inited.";
		
		/// <summary>
		/// Error msg.
		/// </summary>
		private static readonly string c_CantChangeMsg = "Can't change value because schema is already inited.";
		
		/// <summary>
		/// Error msg.
		/// </summary>
		private static readonly string c_NoInfMsg = "There's no information for specified object.";
		
		/// <summary>
		/// Path to schema instance (.xml file).
		/// </summary>
		private static string m_SchemaPath = "schema.xml";

		/// <summary>
		/// Path to schema definition (.xsd file).
		/// </summary>
		private static string m_SchemaDescrPath = "Workflow.Schema.xsd";

		/// <summary>
		/// Stores locale for l10n.
		/// </summary>
		private static string m_CurrentUICulture = "uk-UA";

		/// <summary>
		/// This is the one instance of this type.
		/// </summary>
		private static readonly MetaData m_Singleton = new MetaData();

		/// <summary>
		/// Schema name.
		/// </summary>
		private string m_Name = string.Empty;

		/// <summary>
		/// Indicates that schema was already loaded.
		/// </summary>
		private static bool m_IsInited = false;

		/// <summary>
		/// Namespace manager for internal XPath purposes.
		/// </summary>
		private XmlNamespaceManager m_NsMgr;

		/// <summary>
		/// Saves a correspondence map: Class type -> Class description.
		/// </summary>
		private KeyedMap<string, ClassNode> m_Classes = null;

		/// <summary>
		/// Memory copy of schema file.
		/// </summary>
		private XmlDocument m_SchemaXML = null;
		
		/// <summary>
		/// A method returning a reference to the Singleton.
		/// </summary>
		public static MetaData Singleton
		{
			get { return m_Singleton; }
		}

		/// <summary>
		/// Current culture for UI l10n.
		/// </summary>
		public static string CurrentUICulture
		{
			get { return m_CurrentUICulture; }
			set {
				if( m_IsInited )
					throw new ApplicationException( c_CantChangeMsg );
				m_CurrentUICulture = value;
			}
		}

		/// <summary>
		/// Path to schema instance (.xml file).
		/// </summary>
		public static string SchemaPath
		{
			get { return m_SchemaPath; }
			set {
				if( m_IsInited )
					throw new ApplicationException( c_CantChangeMsg );
				m_SchemaPath = value;
			}
		}

		/// <summary>
		/// Gets, sets path to schema description (.xsd file).
		/// </summary>
		public static string SchemaDescriptionPath
		{
			get { return m_SchemaDescrPath; }
			set {
				if( m_IsInited )
					throw new ApplicationException( c_CantChangeMsg );
				m_SchemaDescrPath = value;
			}
		}

		/// <summary>
		/// Overridden Object.ToString() method.
		/// </summary>
		public override string ToString()
		{
			return Name;
		}

		/// <summary>
		/// This method perform loading schema and building program structures.
		/// You must call this method before using MetaData instance.
		/// </summary>
		public void InitSchema()
		{
			if( m_IsInited )
				throw new InvalidOperationException( c_AlreadyInitedMsg );			

            // loading .xml
            m_SchemaXML = new XmlDocument();

            m_SchemaXML.Load( m_SchemaPath );
			
            // loading .xsd to XmlDocument
           XmlReader schemareader = XmlReader.Create( m_SchemaDescrPath );
           m_SchemaXML.Schemas.Add( null, schemareader );
           schemareader.Close();			

            // Validate schema throwing exception on warnings and errors.
            m_SchemaXML.Validate( delegate( object sender, ValidationEventArgs e ) {
                throw new ApplicationException(
                    String.Format( "Schema validation failed. {0}: {1}", e.Severity, e.Message ),
                    e.Exception ); } );				

            m_Name = m_SchemaXML.DocumentElement.Attributes.GetNamedItem( "name" ).Value;

			m_NsMgr = new XmlNamespaceManager( m_SchemaXML.NameTable );
			m_NsMgr.AddNamespace( "ws", m_SchemaXML.DocumentElement.NamespaceURI );
		
			foreach( XmlNode node in m_SchemaXML.DocumentElement.SelectNodes( "ws:class", m_NsMgr ) )
				Classes.Add( new ClassNode( node ) );			

			m_IsInited = true;
		}

		/// <summary>
		/// Gets class description.
		/// </summary>
		public SClass this[ IClassInfo instance ]
		{
			get
			{
				if( instance == null )
					throw new ArgumentNullException( "instance" );

				if( m_Classes.Contains( instance.Type ) )
					return new SClass( instance );
				else
					throw new ArgumentException( c_NoInfMsg );
			}
		}

		/// <summary>
		/// Gets current loaded schema name.
		/// </summary>
		public string Name
		{
			get { return m_Name; }
		}

		/// <summary>
		/// Indicates that schema was already loaded.
		/// </summary>
		public bool IsInited
		{
			get { return m_IsInited; }
		}

		/// <summary>
		/// Gets class name - class information corresponds map.
		/// </summary>
		internal KeyedMap<string, ClassNode> Classes
		{
			get
			{		
				if( m_Classes == null )
					m_Classes = new KeyedMap<string, ClassNode>();

				return m_Classes;				
			}
		}		
		
		/// <summary>
		/// Gets instance of Namespace manager for internal XPath purposes.
		/// </summary>
		internal XmlNamespaceManager XMLNsMgr
		{
			get { return m_NsMgr; }
		}

		/// <summary>
		/// Default ctor.
		/// </summary>
		private MetaData()
		{
		}
	}
}
