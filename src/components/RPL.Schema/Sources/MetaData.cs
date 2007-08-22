//****************************************************************************
//*	Project		:	Robust Persistence Layer
//*
//*	Module		:	CSchema.cs
//*
//*	Content		:	Implementation of CSchema class
//*	Author		:	Alexander Kurbatov
//*	Copyright	:	Copyright © 2006 Alexander Kurbatov
//*
//*	This class implements collection of CSClass based on CollectionBase
//*
//*	Implements ISerializable,IDeserializationCallback for serialize/deserialize purpose
//*
//****************************************************************************
using System;
using System.Collections.Generic;
using System.Runtime.Serialization;
using System.Collections;
using System.IO;
using System.Resources;
using System.Globalization;


namespace ABBYY.Toolkit.RPL.Schema
{
	/// <summary
	/// This class implements collection of CSClass based on CollectionBase
	/// </summary>
	[Serializable]
	public class MetaData : CollectionBase, ISerializable, IDeserializationCallback, IEnumerable<SClass>
	{

		/// <summary
		/// Stores path to schema. TEMP realization!
		/// </summary>
		private const string SCHEMA_PATH = "schema_output.xml";

		/// <summary
		/// Ending that refer to Caption resource for method or property
		/// </summary>
		private const string RES_CAPTION_NAME = ".Caption";

		/// <summary
		/// Ending that refer to Description resource for method or property
		/// </summary>
		private const string RES_DESCRIPTION_NAME = ".Description";


		/// <summary
		/// First part of file name for resourse files that contain
		/// regional settings based information
		/// </summary>
		private const string m_mui_name = "mui";

		/// <summary
		/// Default locale for regional setting.
		/// </summary>
		private static CultureInfo m_ci_default = new CultureInfo( "uk-UA" );

		/// <summary
		/// Default error when someone tries to change read-only List collection
		/// </summary>
		private const string m_error_string = "Collection is read-only.";

		/// <summary
		/// Private ArrayList for deserialize purpose
		/// </summary>
		private ArrayList m_List;

		/// <summary
		/// Used to get required information from resources
		/// based on regional settings.
		/// </summary>
		private static ResourceManager m_rm;

		/// <summary
		/// Stores schema name
		/// </summary>
		private string m_Name;

		/// <summary
		/// This is the one instance of this type.
		/// </summary>
		private static readonly MetaData m_SchemaSingleton = new MetaData();

		private bool isInited;

		/// <summary
		/// A method returning a reference to the Singleton.
		/// </summary>
		public static MetaData Singleton
		{
			get { return m_SchemaSingleton; }
		}

		#region Accessor methods for internal fields
		/// <summary>Property accessor to nameField
		/// Set accessor has only internal access modifier
		/// </summary>
		public string Name
		{
			get { return this.m_Name; }
			internal set { this.m_Name = value; }
		}
		#endregion

		/// <summary
		/// Override Object method to return needed value
		/// </summary>
		public override string ToString()
		{
			return Name;
		}

		/// <summary
		/// Default constructor of class
		/// </summary>
		/// <param name="stream">Stream that contains xml schema to be build on</param>
		[ObsoleteAttribute( "Some actions in CSchema() are obsolute in final version" )]
		//internal CSchema(Stream stream) {
		internal MetaData() { }

		#region Implementation of interaction with object
		/// <summary
		/// Integer index accessor to List collection
		/// Can't reassign CSState objects, so there is no set
		/// </summary>
		public SClass this[int index]
		{
			get { return (SClass) List[index]; }
		}

		/// <summary
		/// String index accessor to List collection
		/// Can't reassign CSState objects, so there is no set
		/// </summary>
		/// <exception cref="ArgumentNullException">When input paramets contain null reference</exception>
		/// <exception cref="KeyNotFoundException">When requested item was not found in collection</exception>
		public SClass this[String index]
		{
			get
			{
				// Check for empty input
				if( string.IsNullOrEmpty( index ) )
					throw new ArgumentNullException( "index" );
				// Searching index in collection
				int _index = this.indexOf( index );
				// Index=-1 mean that we found no item that meets requirements.
				// Raising error
				if( _index == -1 )
					throw new KeyNotFoundException( "Schema of objects may be not up to date!" );
				// Return index was found
				return (SClass) this.List[_index];
			}
		}

		/// <summary>Object index accessor to collection of states
		/// </summary>
		/// <param name="value">Object that implements IClassInfo</param>
		/// <returns>Returns new CSClass object that contains
		/// states that can be applyed to input object. States are sorted by their
		/// priority </returns>
		public SClass this[object value]
		{
			get
			{
				//Cast input object to IClassInfo
				IClassInfo obj = (IClassInfo) value;
				if( obj == null )
					throw new ArgumentNullException( "value" );
				//return CSClass that is build specialy for input object.
				return new SClass( obj.StateComponents, obj.Type );
			}
		}

		/// <summary
		/// Add new unique object to collection
		/// </summary>
		/// <exception cref="ArgumentNullException">When input paramets contain null reference</exception>
		/// <exception cref="ArgumentException">When trying to add dublicate item to collection</exception>
		internal void add( SClass item )
		{
			// Check for initialized reference
			if( item == null )
				throw new ArgumentNullException( "item" );
			// Check existence of same item in collection
			if( List.Contains( item.ToString() ) ) {
				throw new ArgumentException( "There is same object already in collection" );
			}
			// Adds item to collection
			// InnerList is used to not invoke the On* methods
			InnerList.Add( item );
		}

		/// <summary
		/// Determines whether collection contains specified object already
		/// </summary>
		public bool Contains( SClass item )
		{
			// Check for empty input
			if( item == null )
				throw new ArgumentNullException( "item" );

			// check schema init state
			if( !isInited )
				throw new InvalidOperationException( "Schema isn't initialized" );

			// Invoke CollectionBase method
			return List.Contains( item );
		}

		/// <summary
		/// Determines whether collection contains object with such name
		/// </summary>
		public bool Contains( String name )
		{
			// Check for empty input
			if( string.IsNullOrEmpty( name ) )
				throw new ArgumentNullException( "name" );

			// check schema init state
			if( !isInited )
				throw new InvalidOperationException( "Schema isn't initialized" );

			int _index = this.indexOf( name );
			// Searching index in collection
			if( _index == -1 ) { return false; }
				// Return index was found
			else { return true; }
		}

		/// <summary
		/// Return index of specified item
		/// </summary>
		private int indexOf( SClass item )
		{
			//Invoke CollectionBase method
			return List.IndexOf( item );
		}

		/// <summary
		/// Finds index of element in List that has specified name
		/// </summary>
		private int indexOf( String index )
		{
			// Searching item in list with needed name
			for( int j = 0; j < List.Count; j++ ) {
				if( List[j].ToString() == index )
					return j;
			}
			// Nothing was found
			return -1;
		}
		#endregion

		#region Additional custom processes when working with List
		/// <summary>This is override function from CollectionBase class. This routine
		/// call before all operations to performe additional custom processes
		/// when validating a value.
		/// Input parameter must typeof CSClass, if not the error is raized
		/// When null reference is passed the error is raized
		/// </summary>
		/// <exception cref="ArgumentNullException">When input paramets contain null reference</exception>
		/// <exception cref="ArgumentException">When object of unsupported type tried to be added</exception>
		protected override void OnValidate( object value )
		{
			if( value == null )
				throw new ArgumentNullException( "value" );
			if( !(value is SClass) ) {
				throw new ArgumentException( "Collection only supports CSClass objects." );
			}
		}
		/// <summary
		/// Performs additional custom processes when clearing the contents of
		/// the collection: raise error to notify about read-
		/// only collection.
		/// </summary>
		protected override void OnClear()
		{
			throw new NotSupportedException( m_error_string );
		}
		/// <summary
		/// Performs additional custom processes before inserting a new object
		/// into the collection of instance: raise error to notify about
		/// read-only collection.
		/// </summary>
		protected override void OnInsert( int index, object value )
		{
			throw new NotSupportedException( m_error_string );
		}
		/// <summary
		/// Performs additional custom processes before removing an element
		/// from the collection of instance: raise error to notify about
		/// read-only collection.
		/// </summary>
		protected override void OnRemove( int index, object value )
		{
			throw new NotSupportedException( m_error_string );
		}
		/// <summary>Performs additional custom processes before setting a object in
		/// the collection of instance: raise error to notify about read-
		/// only collection.
		/// </summary>
		protected override void OnSet( int index, object oldValue, object newValue )
		{
			throw new NotSupportedException( m_error_string );
		}
		#endregion

		#region ISerializable Members
		/// <summary
		/// Implementation of ISerializable
		/// </summary>
		public void GetObjectData( SerializationInfo info, StreamingContext context )
		{
			// check input parameters
			if( info == null )
				throw new ArgumentNullException( "info" );
			// Declares internal fields to be serialized
			info.AddValue( "name", m_Name );
			info.AddValue( "classes", InnerList );
		}

		/// <summary
		/// Constructor needed to deserialize class
		/// </summary>
		protected MetaData( SerializationInfo info, StreamingContext context )
		{
			// check input parameters
			if( info == null )
				throw new ArgumentNullException( "info" );
			// Declares internal fields to be deserialized from
			m_Name = (string) info.GetValue( "name", typeof( string ) );
			m_List = (ArrayList) info.GetValue( "classes", typeof( ArrayList ) );
		}
		#endregion

		#region IDeserializationCallback Members
		/// <summary
		/// Implementation of IDeserializationCallback
		/// </summary>
		public void OnDeserialization( object sender )
		{
			// After Deserialization of childs is finished copy items
			// from m_list to InnerList
			foreach( SClass item in m_List ) {
				this.add( item );
			}
			//"remove" temporary ArrayList
			m_List = null;
		}
		#endregion

		#region Load/Unload Schema object
		/// <summary
		/// Loads curent object properties from input stream that contain
		/// deserialized object typeof(CSchema)
		/// </summary>
		///<exception cref="ArgumentNullException">When input paramets contain null reference</exception>
		///<exception cref="System.Runtime.Serialization.SerializationException">When deserialization of object 
		/// from stream has failed</exception>
		///<exception cref="System.Xml.XmlException">When input stream has invalid schema format</exception>
		public void InitSchema( Stream stream )
		{
			// Check for empty input
			if( stream == null )
				throw new ArgumentNullException( "stream" );

			// check schema init state
			if( isInited )
				throw new InvalidOperationException( "Schema is already inited" );

			// assign default resource file
			m_rm = ResourceManager.CreateFileBasedResourceManager(
								m_mui_name,
								AppDomain.CurrentDomain.BaseDirectory,
								null );

			IFormatter formatter = new System.Runtime.Serialization.Formatters.Soap.SoapFormatter();
			MetaData schema;
			try {
				schema = (MetaData) formatter.Deserialize( stream );
			} catch( SerializationException ex ) {
				throw new SerializationException( "Loading schema failed!", ex );
			}

			#region Assign properties of desierialized object to current instance
			// Clears Collection
			InnerList.Clear();
			// Copy objects and property from deserialized object to current instance
			foreach( SClass item in schema ) {
				InnerList.Add( item );
			}
			this.m_Name = schema.m_Name;
			#endregion

			isInited = true;
		}

		/// <summary
		/// Returns to input parameter stream that contain
		/// deserialized instance of current object
		/// </summary>
		///<exception cref="System.Runtime.Serialization.SerializationException">When serialization of 
		/// object to stream has failed</exception>"
		public void SaveSchema( Stream stream )
		{
			// Check for empty input
			if( stream == null )
				throw new ArgumentNullException( "stream" );

			// private Stream stream;
			IFormatter formatter =
				new System.Runtime.Serialization.Formatters.Soap.SoapFormatter();
			try {
				formatter.Serialize( stream, this );
				//return stream;
			} catch( SerializationException ex ) {
				throw new SerializationException( "Saving schema failed!", ex );
			}
		}
		#endregion

		#region Items to describe method or property
		/// <summary
		/// Retrives Method or Property Caption from resource using
		/// specified CultureInfo(regional settings)
		/// </summary>
		///<param name="value">
		/// Reference to object that implements IClassInfo</param>
		/// <param name="name">
		/// Indicates name of Parameter or Method information is needed about</param>
		/// <param name="ci">
		/// Indicates regional setting</param>
		/// <returns>
		/// Caption of specified Property or Method.</returns>
		/// <exception cref="System.Collections.Generic.KeyNotFoundException">
		/// When requested information was not found in resources</exception>
		/// <exception cref="System.Resources.MissingManifestResourceException"> 
		/// No usable set of resources has been found, and there are no neutral culture
		/// resources.</exception>
		public string GetCaption( object value, string name, CultureInfo ci )
		{
			// check schema init state
			if( !isInited )
				throw new InvalidOperationException( "Schema isn't initialized" );
			// cast input object to IClassInfo
			IClassInfo obj = (IClassInfo) value;
			// check cast result
			if( obj == null )
				throw new ArgumentNullException( "value" );
			// check input parameter
			if( ci == null )
				throw new ArgumentNullException( "ci" );
			if( string.IsNullOrEmpty( name ) )
				throw new ArgumentNullException( "name" );
			// identification of needed resource
			// EX: "Assembly1.Class1.Method1." + RES_CAPTION_NAME
			string resName = obj.Type + "." + name + RES_CAPTION_NAME;
			try {
				// gets string from resource using specific CultureInfo
				return m_rm.GetString( resName, ci );
			} catch( MissingManifestResourceException ex ) {
				// transform exception to dafault 
				throw new KeyNotFoundException( "Item Caption wasn't found in resource file", ex );
			}
		}

		/// <summary
		/// Retrives Method or Property Caption from resource using
		/// default CultureInfo(regional settings)
		/// </summary>
		/// <param name="value">
		///		Reference to object that implements IClassInfo</param>
		/// <param name="name">
		///		Indicates name of Parameter or Method information is needed about</param>
		/// <returns>
		///		Caption of specified Property or Method.</returns>
		/// <exception cref="System.Collections.Generic.KeyNotFoundException">
		///		When requested information was not found in resources</exception>
		/// <exception cref="System.Resources.MissingManifestResourceException"> 
		///	  No usable set of resources has been found, and there are no neutral culture
		///	  resources.</exception>
		public string GetCaption( object value, string name )
		{
			// check schema init state
			if( !isInited )
				throw new InvalidOperationException( "Schema isn't initialized" );
			//cast input object to IClassInfo
			IClassInfo obj = (IClassInfo) value;
			//if cast failed raise error
			if( obj == null )
				throw new ArgumentNullException( "value" );
			// check input parameters
			if( string.IsNullOrEmpty( name ) )
				throw new ArgumentNullException( "name" );
			// identification of needed resource
			// EX: "Assembly1.Class1.Method1." + RES_CAPTION_NAME
			string resName = obj.Type + "." + name + RES_CAPTION_NAME;
			try {
				// gets string from resource using default CultureInfo
				return m_rm.GetString( resName, m_ci_default );
			} catch( MissingManifestResourceException ex ) {
				// transform exception to dafault 
				throw new KeyNotFoundException( "Item Caption wasn't found in resource file", ex );
			}
		}

		/// <summary
		/// Retrives Method or Property Description from resource using
		/// specified CultureInfo(regional settings)
		/// </summary>
		/// <param name="value">
		///		Reference to object that implements IClassInfo</param>
		/// <param name="name">
		///		Indicates name of Parameter or Method information is needed about</param>
		/// <param name="ci">
		///		Indicates regional setting</param>
		/// <returns>
		///		Description of specified Property or Method.</returns>
		/// <exception cref="System.Collections.Generic.KeyNotFoundException">
		///		When requested information was not found in resources</exception>
		/// <exception cref="System.Resources.MissingManifestResourceException"> 
		///	  No usable set of resources has been found, and there are no neutral culture
		///	  resources.</exception>
		public string GetDescription( object value, string name, CultureInfo ci )
		{
			// check schema init state
			if( !isInited )
				throw new InvalidOperationException( "Schema isn't initialized" );
			//cast input object to IClassInfo
			IClassInfo obj = (IClassInfo) value;
			// check cast result
			if( obj == null )
				throw new ArgumentNullException( "value" );
			// check input parameter
			if( ci == null )
				throw new ArgumentNullException( "ci" );
			if( string.IsNullOrEmpty( name ) )
				throw new ArgumentNullException( "name" );
			// identification of needed resource
			// EX: "Assembly1.Class1.Method1." + RES_DESCRIPTION_NAME
			string resName = obj.Type + "." + name + RES_DESCRIPTION_NAME;
			try {
				// gets string from resource using specific CultureInfo
				return m_rm.GetString( resName, ci );
			} catch( MissingManifestResourceException ex ) {
				// transform exception to dafault 
				throw new KeyNotFoundException( "Item Description wasn't found in resource file", ex );
			}
		}

		/// <summary
		/// Retrives Method or Property Description from resource using
		/// default CultureInfo(regional settings)
		/// </summary>
		/// <param name="value">
		///		Reference to object that implements IClassInfo</param>
		/// <param name="name">
		///		Indicates name of Parameter or Method information is needed about</param>
		/// <returns>
		///		Description of specified Property or Method.</returns>
		/// <exception cref="System.Collections.Generic.KeyNotFoundException">
		///		When requested information was not found in resources</exception>
		/// <exception cref="System.Resources.MissingManifestResourceException"> 
		///	  No usable set of resources has been found, and there are no neutral culture
		///	  resources.</exception>
		public string GetDescription( object value, string name )
		{
			// check schema init state
			if( !isInited )
				throw new InvalidOperationException( "Schema isn't initialized" );
			//cast input object to IClassInfo
			IClassInfo obj = (IClassInfo) value;
			//if cast failed raise error
			if( obj == null )
				throw new ArgumentNullException( "value" );
			if( string.IsNullOrEmpty( name ) )
				throw new ArgumentNullException( "name" );
			// identification of needed resource
			// EX: "Assembly1.Class1.Method1." + RES_DESCRIPTION_NAME
			string resName = obj.Type + "." + name + RES_DESCRIPTION_NAME;
			try {
				// gets string from resource using default CultureInfo
				return m_rm.GetString( resName, m_ci_default );
			} catch( MissingManifestResourceException ex ) {
				// transform exception to dafault 
				throw new KeyNotFoundException( "Item Description wasn't found in resource file", ex );
			}
		}

		/// <summary
		/// Retrives DisplayOrder Property from schema
		/// </summary>
		/// <param name="value">
		///		Reference to object that implements IClassInfo</param>
		/// <param name="name">
		///		Indicates name of Property information is needed about</param>
		/// <returns>
		///		0..n: Display Order of specified Property
		///		-1 : property is unavailable</returns>
		public int GetDisplayOrder( object value, string name )
		{
			// check schema init state
			if( !isInited )
				throw new InvalidOperationException( "Schema isn't initialized" );
			// check input parameters
			if( value == null )
				throw new ArgumentNullException( "value" );
			if( string.IsNullOrEmpty( name ) )
				throw new ArgumentNullException( "name" );
			// get collection of states that comply with input object
			SClass sclass = this[value];
			// Search state that have specific property
			foreach( SState item in sclass ) {
				if( item.Properties.Contains( name ) )
					// state was found.
					return item.Properties[name].DisplayOrder;
			}
			// default value when nothing found.
			return -1;
		}

		/// <summary>Indicates wether requested Property is requiered.
		/// </summary>
		/// <param name="value">
		///		Reference to object that implements IClassInfo</param>
		/// <param name="name">
		///		Indicates name of Parameter information is needed about</param>
		/// <returns>
		///		If information was not found in schema return false</returns>
		public bool IsRequiredProperty( object value, string name )
		{
			// check schema init state
			if( !isInited )
				throw new InvalidOperationException( "Schema isn't initialized" );
			// check input parameters
			if( value == null )
				throw new ArgumentNullException( "value" );
			if( string.IsNullOrEmpty( name ) )
				throw new ArgumentNullException( "name" );
			// get collection of states that comply with input object
			SClass sclass = this[value];
			// Search state that have specific property
			foreach( SState item in sclass ) {
				if( item.Properties.Contains( name ) )
					// state was found.
					return item.Properties[name].IsRequired;
			}
			// default value when nothing found.
			return false;
		}

		/// <summary>Indicates wether requested Method is available.
		/// </summary>
		/// <param name="value">
		///		Reference to object that implements IClassInfo</param>
		/// <param name="name">
		///		Indicates name of Method information is needed about</param>
		public bool IsMethodAvailable( object value, string name )
		{
			// check schema init state
			if( !isInited )
				throw new InvalidOperationException( "Schema isn't initialized" );
			// check input parameters
			if( value == null )
				throw new ArgumentNullException( "value" );
			if( string.IsNullOrEmpty( name ) )
				throw new ArgumentNullException( "name" );
			// get collection of states that comply with input object
			SClass sclass = this[value];
			// Search state that have specific property
			foreach( SState item in sclass ) {
				if( item.Methods.Contains( name ) )
					// state was found.
					return item.Methods[name].IsAvailable;
			}
			// default value when nothing found.
			return false;
		}

		/// <summary>Gets default value of Property.
		/// </summary>
		/// <param name="value">
		///		Reference to object that implements IClassInfo</param>
		/// <param name="name">
		///		Indicates name of Property information is needed about</param>
		///	<returns>Return null if information is unavalible, else object</returns>
		public object GetDefaultValue( object value, string name )
		{
			// check schema init state
			if( !isInited )
				throw new InvalidOperationException( "Schema isn't initialized" );
			// check input parameters
			if( value == null )
				throw new ArgumentNullException( "value" );
			if( string.IsNullOrEmpty( name ) )
				throw new ArgumentNullException( "name" );
			// get collection of states that comply with input object
			SClass sclass = this[value];
			// Search state that have specific property
			foreach( SState item in sclass ) {
				if( item.Properties.Contains( name ) )
					// state was found.
					return item.Properties[name].DefaultValue;
			}
			// default value when nothing found.
			return null;
		}
		#endregion

		#region IEnumerable<CSClass> Members
		/// <summary
		/// Gets the element in the collection at the current position of the enumerator.
		/// </summary>
		///<returns>The element in the collection at the current position of the enumerator.</returns>
		public new IEnumerator<SClass> GetEnumerator()
		{
			foreach( SClass data in InnerList ) {
				yield return data;
			}
		}
		#endregion
	}
}