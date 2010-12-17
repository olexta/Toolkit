***************************************************************************
//*	Project		:	Workflow.Schema
//*
//*	Module		:	MetaData.cs
//*
//*	Content		:	Main library class.
//*	Author		:	Alexander Kurbatov, Nikita Marunyak
//*	Copyright	:	Copyright © 2006 - 2009 Alexander Kurbatov, Nikita Marunyak
//*
//* SVN			:	$Id$
//*
//****************************************************************************

using System;
using System.Collections.Generic;
using System.Xml;
using System.Xml.Schema;
using System.Globalization;

using Toolkit.Collections;
using System.IO;

namespace Toolkit.Workflow.Schema
{
	/// <summary>
	/// Main library class.
	/// </summary>
	public class MetaData
	{
		/// <summary>
		/// This is the one instance of this type.
		/// </summary>
		private static MetaData m_Instance;

		/// <summary>
		/// Indicates that schema was already loaded.
		/// </summary>
		private static bool m_IsInitialized = false;

		/// <summary>
		/// Stores locale for l10n.
		/// </summary>
		private string m_CurrentUICulture;

		/// <summary>
		/// Schema name.
		/// </summary>
		private string m_Name;

		/// <summary>
		/// Namespace manager for internal XPath purposes.
		/// </summary>
		private XmlNamespaceManager m_NsMgr;

		/// <summary>
		/// Saves a correspondence map: Class type -> Class description.
		/// </summary>
		private KeyedMap<string, ClassNode> m_Classes;

		/// <summary>
		/// Saves a correspondence map: Enum type name -> Enum description.
		/// </summary>
		private KeyedMap<string, EnumInfo> m_Enums;

		/// <summary>
		/// Saves available PropSets;
		/// </summary>
		private KeyedMap<string, PropSetInfo> m_PropSets;

		/// <summary>
		/// Saves available PropertyVariantsInfo
		/// </summary>
		private KeyedMap<string, StringCollectionInfo> m_StringCollections;

		/// <summary>
		/// Memory copy of schema file.
		/// </summary>
		private XmlDocument m_SchemaXML;
		
		/// <summary>
		/// A method returning a reference to the Singleton.
		/// </summary>
		public static MetaData Instance
		{
			get {
				if( m_Instance == null ) {
					throw new ApplicationException( "Schema isn't yet initialized. Call InitSchema() first." );
				}
				return m_Instance;
			}
		}

		/// <summary>
		/// Returns string than represents object type and its assembly.
		/// You can use this function to identify objects types in schema.
		/// </summary>
		public static string GetObjectType( object obj )
		{
			return obj.GetType().FullName.ToString() + "," +
				System.Reflection.Assembly.GetAssembly( obj.GetType() ).GetName().Name;
		}

		/// <summary>
		/// Indicates that schema was already loaded.
		/// </summary>
		public static bool IsInitialized
		{
			get { return m_Instance != null; }
		}

		/// <summary>
		/// This method creates singleton instance of MetaData class.
		/// You must call this method before using MetaData instance.
		/// <param name="UICulture">Culture identifier.</param>
		/// <param name="xml">Schema file (.xml).</param>
		/// <param name="xsd">Schema description file (.xsd).</param>
		/// </summary>
		public static void InitSchema( string UICulture, Stream xml, Stream xsd )
		{
			if( m_Instance != null ) {
				throw new ApplicationException( "Schema is already initialized." );
			}
			// creating singleton instance of MetaData class
			m_Instance = new MetaData();
			// trying to initialize singleton instance
			try {
				m_Instance.initialize( UICulture, xml, xsd );
			} catch( Exception e ) {
				m_Instance = null;
				throw e;
			}
		}

		/// <summary>
		/// Deinitialize schema.
		/// </summary><remarks>
		/// You must use this function to reinitialize schema.
		/// </remarks>
		public static void DeinitSchema()
		{
			if( m_Instance == null ) {
				throw new ApplicationException( "Schema isn't yet initialized." );
			}
			m_Instance = null;
		}

		/// <summary>
		/// Gets object description.
		/// </summary>
		public SClass this[ IClassInfo instance ]
		{
			get
			{
				return this[ instance, null ];
			}
		}

		/// <summary>
		/// Gets description for object in specified state
		/// </summary>
		/// <remarks>
		/// Putting null by stateComponents means work with current object state
		/// </remarks>
		public SClass this[ IClassInfo instance, string[] stateComponents ]
		{
			get {
				if( instance == null )
					throw new ArgumentNullException( "instance" );

				if( m_Classes.Contains( instance.Type ) )
					return new SClass( instance, m_Classes[ instance.Type ], stateComponents );
				else
					throw new NoClassInformationException( instance );
			}
		}

		/// <summary>
		/// Returns description for specified enum type.
		/// </summary>
		/// <param name="enumTypeName">Full type name of enum type.</param>
		/// <returns>Enum description class or null.</returns>
		public EnumInfo GetEnumInfo( string enumTypeName )
		{
			if( m_Enums.Contains( enumTypeName ) )
				return m_Enums[enumTypeName];
			else
				return null;
		}

		/// <summary>
		/// Gets enumeration of all available PropSets.
		/// </summary>
		public IEnumerable<string> PropSetsNames
		{
			get	{
				foreach( PropSetInfo psi in m_PropSets ) {
					yield return psi.Name;
				}
			}
		}

		/// <summary>
		/// Returns PropSet description by his name.
		/// </summary>
		/// <returns>
		/// PropSetInfo instance - if schema contains description for specified name,
		/// null - otherwise.
		/// </returns>
		public PropSetInfo GetPropSetInfo( string propSetName )
		{
			if( m_PropSets.Contains( propSetName ) ) {
				return m_PropSets[ propSetName ];
			} else {
				return null;
			}
		}

		/// <summary>
		/// Gets enumeration of all available PropertyVariants.
		/// </summary>
		public IEnumerable<string> StringCollectionNames
		{
			get {
				foreach( StringCollectionInfo sci in m_StringCollections ) {
					yield return sci.Name;
				}
			}
		}

		/// <summary>
		/// Returns PropertyVariants description by his name.
		/// </summary>
		/// <returns>
		/// PropertyVariantsInfo instance - if schema contains description for specified name,
		/// null - otherwise.
		/// </returns>
		public StringCollectionInfo GetStringCollectionInfo( string stringCollectionName )
		{
			if( m_StringCollections.Contains( stringCollectionName ) ) {
				return m_StringCollections[stringCollectionName];
			} else {
				return null;
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
		/// Overridden Object.ToString() method.
		/// </summary>
		public override string ToString()
		{
			return Name;
		}		

		/// <summary>
		/// Current culture for UI l10n.
		/// </summary>
		public string CurrentUICulture
		{
			get { return m_CurrentUICulture; }
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

		/// <summary>
		/// This method perform schema loading and building program structures.
		/// </summary>
		private void initialize( string UICulture, Stream xml, Stream xsd )
		{
			// initializing values
			m_CurrentUICulture = UICulture;

			// loading .xml
			m_SchemaXML = new XmlDocument();

			m_SchemaXML.Load( xml );

			// loading .xsd to XmlDocument
			XmlReader schemareader = XmlReader.Create( xsd );
			m_SchemaXML.Schemas.Add( null, schemareader );
			schemareader.Close();

			// Validate schema throwing exception on warnings and errors.
			m_SchemaXML.Validate( delegate( object sender, ValidationEventArgs e )
			{
				throw new ApplicationException(
					String.Format( "Schema validation failed. {0}: {1}", e.Severity, e.Message ),
					e.Exception );
			} );

			m_Name = m_SchemaXML.DocumentElement.Attributes.GetNamedItem( "ws:name" ).Value;

			m_NsMgr = new XmlNamespaceManager( m_SchemaXML.NameTable );
			m_NsMgr.AddNamespace( "ws", m_SchemaXML.DocumentElement.NamespaceURI );

			m_Classes = new KeyedMap<string, ClassNode>();

			foreach( XmlNode node in m_SchemaXML.DocumentElement.SelectNodes( "ws:class", m_NsMgr ) )
				m_Classes.Add( new ClassNode( node ) );

			m_Enums = new KeyedMap<string, EnumInfo>();
			foreach( XmlNode node in m_SchemaXML.DocumentElement.SelectNodes( "ws:enum", m_NsMgr ) )
				m_Enums.Add( new EnumInfo( node ) );

			m_PropSets = new KeyedMap<string,PropSetInfo>();
			foreach( XmlNode node in m_SchemaXML.DocumentElement.SelectNodes( "ws:propSet", m_NsMgr ) ) {
				PropSetInfo psi = new PropSetInfo( node );
				m_PropSets.Add( psi );
			}
			
			m_StringCollections = new KeyedMap<string, StringCollectionInfo>();
			foreach( XmlNode node in m_SchemaXML.DocumentElement.SelectNodes( "ws:stringCollection", m_NsMgr ) ) {
				StringCollectionInfo sci = new StringCollectionInfo( node );
				m_StringCollections.Add( sci );
			}
		}
	}
}
