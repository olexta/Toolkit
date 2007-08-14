//****************************************************************************
//*   Project:   Schema Layer
//*
//*   Module:      CSState.cs
//*
//*   Content:   Implementation of CSState class
//*   Author:      Alexander Kurbatov
//*   Copyright:   Copyright © 2006 Alexander Kurbatov
//*
//*   This class contain two collections: CSMethods and CSAttributes
//*
//*   Implements ISerializable for serialize/deserialize purpose
//*
//****************************************************************************
using System;
using System.Runtime.Serialization;
using System.Collections;
using System.Text.RegularExpressions;

namespace RPL.Schema {
///<summary>Defines some properties of a class that it have some according bussines workflow
/// Class contain two collections: CSMethods,CSAttributes and name of a state</summary>
[Serializable]
public class SState : ISerializable, IComparable
{

	///<summary>Stores method name</summary>
	private SStateName m_Name;

	///<summary>Declare CSAttributes object</summary>
	private SProperties m_SProperties = new SProperties();

	///<summary>Declare CSMethods object</summary>
	private SMethods m_Smethods = new SMethods();

	#region Accessor methods for internal fields

	///<summary>Property accessor to nameField
	/// Set accessor has only internal access modifier</summary>
	public SStateName Name
	{
		get { return this.m_Name; }
	}

	///<summary>Property accessor to sattributes collection.
	/// Set accessor has only internal access modifier</summary>
	public SProperties Properties
	{
		get { return this.m_SProperties; }
		// can't change property from out side in security reason
		internal set { this.m_SProperties = value; }
	}

	///<summary>Property accessor to smethods collection
	/// Set accessor has only internal access modifier</summary>
	public SMethods Methods
	{
		get { return this.m_Smethods; }
		// can't change property from out side in security reason
		internal set { this.m_Smethods = value; }
	}
	#endregion

	///<summary>Constructor of class by State name.</summary>
	internal SState( string StateName )
	{
		m_Name = new SStateName( StateName );
	}

	///<summary>Override Object method to return needed value</summary>
	public override string ToString()
	{
		return Name.ToString();
	}

	#region ISerializable Members
	///<summary>Implementation of ISerializable</summary>
	public void GetObjectData( SerializationInfo info, StreamingContext context )
	{
		// check input parameters
		if ( info == null ) throw new ArgumentNullException( "info" );
		// Declares internal fields to be serialized
		info.AddValue( "name", m_Name.ToString() );
		info.AddValue( "attributes", m_SProperties );
		info.AddValue( "methods", m_Smethods );
	}

	///<summary>Constructor needed to deserialize class</summary>
	protected SState( SerializationInfo info, StreamingContext context )
	{
		// check input parameters
		if ( info == null ) throw new ArgumentNullException( "info" );
		// Declares internal fields to be deserialized from
		m_Name = new SStateName( (string)info.GetValue( "name", typeof( string ) ) );
		m_SProperties = (SProperties)info.GetValue( "attributes", typeof( SProperties ) );
		m_Smethods = (SMethods)info.GetValue( "methods", typeof( SMethods ) );
	}
	#endregion

	#region IComparable Members

	public int CompareTo( object obj )
	{
		SState st = (SState)obj;

		if ( st == null ) throw new ArgumentNullException( "value" );

		return ((IComparable)this.m_Name).CompareTo( st.m_Name );
	}
	#endregion

	/// <summary> This class encapsulates the behavior needed to work with State Name.
	/// Supports comparability.</summary>
	public class SStateName : CollectionBase, IComparable
	{
		///<summary>Default character to indentify any State Part</summary>
		public const char cAny = '*';
		/// <summary>Default character to delimit State Parts</summary>
		public const char cDelimiter = '|';
		/// <summary>Patern to split name on parts</summary>
		private string m_StateNamePattern = @"(?<=\" + cDelimiter +	//Block must begins from c_Delimiter
                                            @")(?:\w+|\" + cAny +		// contains word or c_any
                                            @")(?=\" + cDelimiter + ")";// end is c_delimiter
		///<summary>Indicate priority of Class according to structure of parts</summary>
		private int m_priority;

		///<summary> Constructs class based on input string.</summary>
		/// <param name="value">Must have next format: "|??|??|??|",
		/// where ?? is any word or "*" which means any word"</param>
		public SStateName( string value )
		{
			if ( string.IsNullOrEmpty(value) ) throw new ArgumentNullException( "value" );
			// used to check input and output string length
			int matchLength = 0;
			// initialize internal field

			// create collection of strings according to pattern
			MatchCollection mc = Regex.Matches( value, m_StateNamePattern );				

			#region Check for input and output caracter count
			foreach ( Match match in mc ) {
				matchLength += match.Length;
			}

			//	Check that chars count from parsed strings is equal to input string.
			//m_StateNamePattern removes all c_delimeters
			if ( value.Length != matchLength + mc.Count + 1 ) {
				throw new ArgumentException( "Format is invalid", "value" );
			}
			#endregion

			for ( int i = mc.Count - 1; i >= 0; i-- ) {
				// count priority of State Name using binnary approach.
				// 2^i + .. + 2^0, each sum block is added if mc[i].Value is not "*".
				m_priority += (mc[i].Value == "*" ? 0 : 1) * (int)Math.Pow( 2, mc.Count - i -1 );
				// Initializing internal list with State Parts
				List.Insert( 0, mc[i].Value );
			}
		}

		///<summary>Override Object method to return needed value</summary>
		public override string ToString()
		{
			// Must begin from c_delimeter
			string result = cDelimiter.ToString();

			// adding available parts of state name
			for ( int i = 0; i < List.Count; i++ ) {
				// parts are separated from each other by c_delimeter
				result += List[i] + cDelimiter.ToString();
			}
			return result;
		}
		///<summary>Property accessor to m_priority
		/// Set accessor has only internal access modifier</summary>
		public int Priority
		{
			get { return m_priority; }
		}

		#region IComparable Members
		///<summary>Implementation of IComparable</summary>
		/// <param name="obj"></param>
		/// <returns>difference</returns>
		public int CompareTo( object obj )
		{
			SStateName sn = (SStateName)obj;
			// cast failed, so invalid object in input
			if ( sn == null ) {
				throw new ArgumentNullException( "obj" );
			}
			// Only names with same count of parts are comparable
			if (List.Count != sn.List.Count ) {
				throw new ArgumentOutOfRangeException( "States are for different object types");
			}
			return (m_priority - sn.m_priority);
		}

		///<summary>Overloading of less than operator</summary>
		public static bool operator<( SStateName lhs, SStateName rhs )
		{
			// check input parameters
			if ( lhs == null ) throw new ArgumentNullException( "lhs" );
			if ( rhs == null ) throw new ArgumentNullException( "rhs" );
			return ((IComparable)lhs).CompareTo( rhs ) < 0;
		}

		///<summary>Overloading Less Than or Equal To operator</summary>
		public static bool operator<=( SStateName lhs, SStateName rhs )
		{
			// check input parameters
			if ( lhs == null ) throw new ArgumentNullException( "lhs" );
			if ( rhs == null ) throw new ArgumentNullException( "rhs" );
			return ((IComparable)lhs).CompareTo( rhs ) <= 0;
		}

		///<summary>Overloading of greater than operator</summary>
		public static bool operator>( SStateName lhs, SStateName rhs )
		{
			// check input parameters
			if ( lhs == null ) throw new ArgumentNullException( "lhs" );
			if ( rhs == null ) throw new ArgumentNullException( "rhs" );
			return ((IComparable)lhs).CompareTo( rhs ) > 0;
		}

		///<summary>Overloading of greater than or equal operator</summary>
		public static bool operator>=( SStateName lhs, SStateName rhs )
		{
			// check input parameters
			if ( lhs == null ) throw new ArgumentNullException( "lhs" );
			if ( rhs == null ) throw new ArgumentNullException( "rhs" );
			return ((IComparable)lhs).CompareTo( rhs ) >= 0;
		}
		#endregion
	}
}
}
