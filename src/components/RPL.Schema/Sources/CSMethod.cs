//****************************************************************************
//*   Project:   Schema Layer
//*
//*   Module:      CSMethod.cs
//*
//*   Content:   Implementation of CSMethod class
//*   Author:      Alexander Kurbatov
//*   Copyright:   Copyright © 2006 Alexander Kurbatov
//*
//*   This class is used to store following some class's method information:
//*     1) Name - Method name
//*     2) isAvailable - Defines whether or not this method may be involved
//*
//*   Class implements ISerializable, so it can serialize/deserialize itself
//*
//****************************************************************************
using System;
using System.Runtime.Serialization;

namespace RPL.Schema {
/// <remarks/>
[Serializable]
public class SMethod : ISerializable {
	///<summary>
	/// Stores method name
	///</summary>
	private string m_Name;

	///<summary>
	///Stores method involve possibility
	///</summary>
	private bool m_IsAvailable;

	///<summary>
	///Default constructor of class
	///Initialises default value of m_IsAvailable
	///</summary>
	internal SMethod() {}

	#region Accessor methods for internal fields
	/// <summary>
	/// Property accessor to m_name.
	/// Set accessor has only internal access modifier
	/// </summary>
	public string Name {
		get {
			return m_Name;
		}
		// can't change property from out side in security reason
		internal set {
			m_Name = value;
		}
	}

	/// <summary>
	/// Property accessor to m_isAvailable.
	/// Set accessor has only internal access modifier
	/// </summary>
	public bool IsAvailable {
		get {
			return m_IsAvailable;
		}
		// can't change property from out side in security reason
		internal set {
			m_IsAvailable = value;
		}
	}
	#endregion

	///<summary>
	///Override Object method to return needed value
	///</summary>
	public override string ToString() {
		return Name;
	}

	#region ISerializable Members
	///<summary>
	///Implementation of ISerializable
	///</summary>
	public void GetObjectData( SerializationInfo info, StreamingContext context ) {
		// check input parameters
		if ( info == null ) throw new ArgumentNullException( "info" );
		// Declares internal fields to be serialized
		info.AddValue( "name", m_Name );
		info.AddValue( "isAvailable", m_IsAvailable );
	}

	///<summary>
	/// Constructor needed to deserialize class
	///</summary>
	protected SMethod( SerializationInfo info, StreamingContext context ) {
		// check input parameters
		if ( info == null )	throw new ArgumentNullException( "info" );
		// Declares internal fields to be deserialized from
		m_Name = (string)info.GetValue( "name", typeof( string ) );
		m_IsAvailable = (bool)info.GetValue( "isAvailable", typeof( bool ) );
	}
	#endregion
}
}
