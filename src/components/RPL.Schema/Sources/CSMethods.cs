//****************************************************************************
//*   Project:   Schema Layer
//*
//*   Module:      CSMethods.cs
//*
//*   Content:   Implementation of CSMethods class
//*   Author:      Alexander Kurbatov
//*   Copyright:   Copyright © 2006 Alexander Kurbatov
//*
//*   This class implements collection of CSMethod based on CollectionBase
//*
//*   Implements ISerializable,IDeserializationCallback for serialize/deserialize purpose
//*
//****************************************************************************
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.Serialization;

namespace RPL.Schema {
/// <summary>
///	 Implements collection of CSMethod based on CollectionBase
/// </summary>
[Serializable]
public class SMethods : CollectionBase, ISerializable, IDeserializationCallback
{
	///<summary>
	///Default error when someone tries to change read-only List collection
	///</summary>
	private const string ERROR_STRING = "Collection is read-only.";
	
	///<summary>        
	///Private ArrayList for deserialize purpose 
	///</summary>
	private ArrayList m_list;

	///<summary>
	///Default constructor of class
	///</summary>
	internal SMethods() {
	}

    #region Implementation of interaction with object
	///<summary>
	///Integer index accessor to List collection
	///Can't reassign CSMethod objects, so there is no setter
	///</summary>
	public SMethod this[int index] {
		get { return (SMethod)List[index]; }
	}

	///<summary>
	///String index accessor to List collection
	///Can't reassign CSMethod objects, so there is no setter
	///</summary>
	public SMethod this[String index] {
		get	{
			// Check for empty input
			if ( string.IsNullOrEmpty( index ) ) throw new ArgumentNullException("value");
			// Searching index in collection
			int _index = this.indexOf(index);
			// Index=-1 meanth that we found no item that meets requirements.
			// Raising error
			if (_index == -1) throw new KeyNotFoundException();
			// Return index was found
			return (SMethod)this.List[_index];
		}
	}

	///<summary>
	///Add new unique object to collection
	///</summary>
	internal void add( SMethod item ) {
		// Check for initialized reference
		if ( item == null ) throw new ArgumentException( "item" );
		// Check existence of same item in collection
		if ( this.Contains( item.ToString() ) ) {
			throw new ArgumentException("There is same object already in collection");
		}
		// Adds item to collection
		// InnerList is used to not invoke the On* methods
		InnerList.Add( item );
	}

	///<summary>
	///Determines whether collection contains specified object already
	///</summary>
	public bool Contains( SMethod item ) {
		// Invoke CollectionBase method
		return List.Contains( item );
	}

	///<summary>
	///Determines whether collection contains object with such name
	///</summary>
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

	///<summary>
	///	Return index of specified item
	///</summary>
	private int indexOf( SMethod item ) {
		// Invoke CollectionBase method
		return List.IndexOf( item );
	}

	///<summary>
	///Finds index of element in List that has specified name
	///</summary>
	private int indexOf( String index ) {
		// Searching item in list with needed name
		for ( int j = 0; j < List.Count; j++ ) {
			if (List[j].ToString() == index) return j;
		}
		// Nothing was found
		return -1;
	}
	#endregion

	#region Additional custom processes when working with List
	/// <summary>
	/// This is override function from CollectionBase class. This routine
	/// call before all operations to performe additional custom processes
	/// when validating a value.
	/// Input parameter must typeof CSMethod, if not the error is raized
	/// When null reference is passed the error is raized
	/// </summary>	
	protected override void OnValidate(object value) {
		if ( value == null ) throw new ArgumentNullException( "value" );
		if (!(value is SMethod)) {
			throw new ArgumentException("Collection only supports CSClass objects.");
		}
    }
	/// <summary>
	/// Performs additional custom processes when clearing the contents of
	/// the collection: raise error to notify about read-
	/// only collection.
	/// </summary>
	protected override void OnClear() {
		throw new NotSupportedException( ERROR_STRING );
	}
	/// <summary>
	/// Performs additional custom processes before inserting a new object
	/// into the collection of instance: raise error to notify about
	/// read-only collection.
	/// </summary>
	protected override void OnInsert( int index, object value ) {
		throw new NotSupportedException( ERROR_STRING );
	}
	/// <summary>
	/// Performs additional custom processes before removing an element
	/// from the collection of instance: raise error to notify about
	/// read-only collection.
	/// </summary>
	protected override void OnRemove( int index, object value ) {
		throw new NotSupportedException( ERROR_STRING );
	}
	/// <summary>
	/// Performs additional custom processes before setting a object in
	/// the collection of instance: raise error to notify about read-
	/// only collection.
	/// </summary>
	protected override void OnSet( int index, object oldValue, object newValue ) {
		throw new NotSupportedException( ERROR_STRING );
	}
	#endregion

    #region ISerializable Members
	///<summary>
	///Implementation of ISerializable
	///</summary>
	public void GetObjectData( SerializationInfo info, StreamingContext context ) {
		// check input parameters
		if ( info == null ) throw new ArgumentNullException( "info" );
		// Declares internal fields to be serialized
		info.AddValue( "methods", InnerList );
	}

	///<summary>
	///Constructor needed to deserialize class
	///</summary>
	protected SMethods( SerializationInfo info, StreamingContext context ) {
		// check input parameters
		if ( info == null ) throw new ArgumentNullException( "info" );
		// Declares internal fields to be deserialized from
		m_list = (ArrayList)info.GetValue( "methods", typeof( ArrayList ) );
    }
    #endregion

    #region IDeserializationCallback Members
	///<summary>
	///Implementation of IDeserializationCallback
	///</summary>
	public void OnDeserialization( object sender ) {
		// After Deserialization of childs is finished copy items
		// from m_list to InnerList
		foreach ( SMethod item in m_list ) {
            add(item);
        }
		//"remove" temporary ArrayList
		m_list = null;
    }
    #endregion
}
}