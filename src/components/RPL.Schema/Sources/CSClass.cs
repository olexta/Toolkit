//****************************************************************************
//*   Project:   Schema Layer
//*
//*   Module:      CSClass.cs
//*
//*   Content:   Implementation of CSClass class
//*   Author:      Alexander Kurbatov
//*   Copyright:   Copyright © 2006 Alexander Kurbatov
//*
//*   This class implements collection of CSState based on CollectionBase
//*
//*   Implements ISerializable,IDeserializationCallback for serialize/deserialize purpose
//*
//****************************************************************************
using System;
using System.Collections.Generic;
using System.Collections;
using System.Runtime.Serialization;
using System.Text.RegularExpressions;


namespace RPL.Schema {
///<summary>Implements collection of CSState based on CollectionBase</summary>
[Serializable]
public class SClass : CollectionBase, ISerializable, IDeserializationCallback {
	///<summary>Default error when someone tries to change read-only List collection</summary>
	private const string ERROR_STRING = "Collection is read-only.";
	
	///<summary>Private ArrayList for deserialize purpose</summary>
	private ArrayList m_list;

	///<summary>Stores class name</summary>
	private string m_Name;

	///<summary>Override Object method to return needed value</summary>
	public override string ToString() {
		return Name;
	}

	#region Accessor methods for internal fields
	///<summary>Property accessor to nameField
	/// Set accessor has only internal access modifier</summary>
	public virtual string Name {
		get { return this.m_Name; }
		// can't change property from out side in security reason
		internal set { this.m_Name = value; }
	}
	#endregion

	///<summary>Default constructor of class</summary>
	internal SClass() {}

	/// <summary>Constructor that is used to be created for specific object</summary>
	/// <param name="stateComponents">Array of strings that contains component parts of state Name</param>
	/// <param name="className">Bussines name of object for which instance is created</param>
	internal SClass( string[] stateComponents, string className  ) {
		// Input string for regular expression
		string list = "";
		// Pattern string for regular expression.
		string regexp;

		// get singleton object that contains all available states for current object
		SClass common = MetaData.Singleton[className];
		// assigning same name to current instance
		m_Name = common.m_Name;

		// Create string that contains all available states situated
		// in separate lines
		foreach ( SState item in common ) {
			list +=  item.Name.ToString() + "\n";
		}

		#region build pattern string
		// 1) Specifies multiline mode
		// 2) Specifies that the match must occur at the beginning of the line
		regexp = @"(?m)^";

		// repeatably adds stateComponents elements to pattern string
		for ( int i = 0; i < stateComponents.Length; i++ ) {
			// 1) First character must be '|'
			// 2) Next must be specified string or '*'
			regexp += @"\|(?:" + stateComponents[i].ToString() + @"|\*)";
		}
		// 1) Last character must be '|'
		// 2) Continues match only if the subexpression (zero or one carriage return (\r) and end of line($))
		//	  matches at this position on the right
		regexp += @"\|(?=\r?$)";
		#endregion

		// search all state names in schema
		// that can be applyed to input stateComponents by matching pattern string.
		foreach ( Match item in Regex.Matches( list, regexp) ) {
			// Adding to current instance
			add( common[item.Value] );
		}

		// Sorting states by their priority
		InnerList.Sort();
		// Inverting List. So state with biggest priority will be at top of list.
		InnerList.Reverse();
	}

	#region Implementation of interaction with object
	///<summary>Integer index accessor to List collection
	///Can't reassign CSState objects, so there is no setter</summary>
	public SState this[int index] {
		get { return (SState)List[index];	}
	}

	///<summary>String index accessor to List collection
	///Can't reassign CSState objects, so there is no setter</summary>
	public SState this[String index] {
		get {
			// Check for empty input
			if ( string.IsNullOrEmpty( index ) ) throw new ArgumentNullException( "index" );
			// Searching index in collection
			int _index = this.indexOf( index );
			// Index=-1 meanth that we found no item that meets requirements.
			// Raising error
			if ( _index == -1 ) throw new KeyNotFoundException();
			// Return index was found
			return (SState)this.List[_index];
		}
	}

	///<summary>Add new unique object to collection</summary>
	internal void add( SState item ) {
		// Check for initialized reference
		if ( item == null ) throw new ArgumentException( "item" );
		// Check existence of same item in collection
		if ( this.Contains( item.ToString() ) ) {
			throw new ArgumentException( "There is same object already in collection" );
		}
		// Adds item to collection
		// InnerList is used to not invoke the On* methods
		InnerList.Add( item );
	}

	///<summary>Determines whether collection contains specified object already</summary>
	public bool Contains( SState item ) {
		// Invoke CollectionBase method
		return List.Contains( item );
	}

	///<summary>Determines whether collection contains object with such name</summary>
	public bool Contains( String item ) {
		// Check for empty input
		if ( string.IsNullOrEmpty( item ) ) throw new ArgumentNullException( "item" );
		// Searching index in collection
		int _index = this.indexOf( item );
		// Index=-1 meanth that we found no item that meets requirements.
		if ( _index == -1 ) { return false;	}
		// Return index was found
		else { return true; }
	}

	///<summary>Return index of specified item</summary>
	private int indexOf( SState item ) {
		// Invoke CollectionBase method
		return List.IndexOf( item );
	}

	///<summary>Finds index of element in List that has specified name</summary>
	private int indexOf( String index ) {
		// Searching item in list with needed name
		for ( int j = 0; j < List.Count; j++ ) {
			if ( List[j].ToString() == index ) return j;
		}
		// Nothing was found
		return -1;
	}
	#endregion

	#region Additional custom processes when working with List
	///<summary>This is override function from CollectionBase class. This routine
	/// call before all operations to performe additional custom processes
	/// when validating a value.
	/// Input parameter must typeof CSState, if not the error is raized
	/// When null reference is passed the error is raized</summary>
	protected override void OnValidate( object value ) {
		if ( value == null ) throw new ArgumentNullException( "value" );
		if ( !(value is SState) ) {
			throw new ArgumentException( "Collection only supports CSClass objects." );
		}
	}
	///<summary>Performs additional custom processes when clearing the contents of
	/// the collection: raise error to notify about read-
	/// only collection.</summary>
	protected override void OnClear() {
		throw new NotSupportedException( ERROR_STRING );
	}
	/// <summary>Performs additional custom processes before inserting a new object
	/// into the collection of instance: raise error to notify about
	/// read-only collection.</summary>
	protected override void OnInsert( int index, object value ) {
		throw new NotSupportedException( ERROR_STRING );
	}
	///<summary>Performs additional custom processes before removing an element
	/// from the collection of instance: raise error to notify about
	/// read-only collection.</summary>
	protected override void OnRemove( int index, object value ) {
		throw new NotSupportedException( ERROR_STRING );
	}
	///<summary>Performs additional custom processes before setting a object in
	/// the collection of instance: raise error to notify about read-
	/// only collection.</summary>
	protected override void OnSet( int index, object oldValue, object newValue ) {
		throw new NotSupportedException( ERROR_STRING );
	}
	#endregion

	#region ISerializable Members
	///<summary>Implementation of ISerializable</summary>
	public void GetObjectData( SerializationInfo info, StreamingContext context ) {
		// check input parameters
		if ( info == null ) throw new ArgumentNullException( "info" );
		// Declares internal fields to be serialized
		info.AddValue( "name", m_Name );
		info.AddValue( "states", InnerList );
	}

	///<summary>Constructor needed to deserialize class</summary>
	protected SClass( SerializationInfo info, StreamingContext context ) {
		// check input parameters
		if ( info == null ) throw new ArgumentNullException( "info" );
		// Declares internal fields to be deserialized from
		m_Name = (string)info.GetValue( "name", typeof( string ) );
		m_list = (ArrayList)info.GetValue( "states", typeof( ArrayList ) );
	}
	#endregion

	#region IDeserializationCallback Members
	///<summary>Implementation of IDeserializationCallback</summary>
	public void OnDeserialization( object sender ) {
		// After Deserialization of childs is finished copy items
		// from m_list to InnerList
		foreach ( SState item in m_list ) {
			this.add( item );
		}
		//"remove" temporary ArrayList
		m_list = null;
	}
	#endregion
}
}
