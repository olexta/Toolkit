using System;
using System.Collections.Generic;
using System.IO;
using Toolkit.Collections;
using Toolkit.RPL;

namespace Toolkit.RPL.Test
{
	/// Object for use in RPL.Test projects
	public class TestObject : PersistentObject
	{
		[Flags]
		public enum FAIL { None = 0, Retrieve = 1, Save = 2, Delete = 4 };

		private FAIL m_fail = FAIL.None;

		/// <summary>
		/// Is used to identify full name of this class type
		/// </summary>
		/// <returns>String representation of this class</returns>
		private static string type()
		{
			// Assumed that all bussines objects are named same way.
			return typeof( TestObject ).FullName.ToString() + "," +
			   System.Reflection.Assembly.GetExecutingAssembly().GetName().Name;
		}

		protected override void OnRetrieve()
		{
			if( (m_fail & FAIL.Retrieve) == FAIL.Retrieve ) throw new Exception(
				"Fail on RETRIEVE.");
		}
		protected override void OnSave()
		{
			if( (m_fail & FAIL.Save) == FAIL.Save ) throw new Exception(
				"Fail on SAVE.");
		}
		protected override void OnDelete()
		{
			if( (m_fail & FAIL.Delete) == FAIL.Delete ) throw new Exception(
				"Fail on DELETE.");
		}
		
		public TestObject() {}

		public TestObject( int id, DateTime stamp, String name )
			: base( id, stamp, name ) { }

		public PersistentObjects Parents
		{
			get { return base.Links; }
		}

		/// <summary>
		/// Return string representation ot this type.
		/// </summary>
		public override string Type
		{
			get {return type();}
		}

		public FAIL FailOn { set { m_fail = value; } }

		public DBNull _null
		{
			get {
				if( Properties.ContainsKey( "_null" ) ) {
					return (DBNull)Properties["_null"];
				} else {
					return null;
				}
			}
			set {
				if( value != null ) {
					Properties["_null"] = value;
				} else {
					Properties.Remove( "_null" );
				}
			}
		}

		public bool? _bool
		{
			get {
				if( Properties.ContainsKey( "_bool" ) ) {
					return (bool)Properties["_bool"];
				} else {
					return null;
				}
			}
			set {
				if( value.HasValue ) {
					Properties["_bool"] = (bool)value;
				} else {
					Properties.Remove( "_bool" );
				}
			}
		}
		
		public int? _int
		{
			get {
				if( Properties.ContainsKey( "_int" ) ) {
					return (int)Properties["_int"];
				} else {
					return null;
				}
			}
			set {
				if( value.HasValue ) {
					Properties["_int"] = (int)value;
				} else {
					Properties.Remove( "_int" );
				}
			}
		}

		public double? _double
		{
			get {
				if( Properties.ContainsKey( "_double" ) ) {
					return (double)Properties["_double"];
				} else {
					return null;
				}
			}
			set {
				if( value.HasValue ) {
					Properties["_double"] = (double)value;
				} else {
					Properties.Remove( "_double" );
				}
			}
		}

		public DateTime? _datetime
		{
			get {
				if( Properties.ContainsKey( "_datetime" ) ) {
					return (DateTime)Properties["_datetime"];
				} else {
					return null;
				}
			}
			set {
				if( value.HasValue ) {
					Properties["_datetime"] = (DateTime)value;
				} else {
					Properties.Remove( "_datetime" );
				}
			}
		}

		public string _string
		{
			get {
				if( Properties.ContainsKey( "_string" ) ) {
					return (string)Properties["_string"];
				} else {
					return null;
				}
			}
			set {
				if( value != null ) {
					Properties["_string"] = value;
				} else {
					Properties.Remove( "_string" );
				}
			}
		}

		public PersistentStream _stream
		{
			get {
				if( Properties.ContainsKey( "_stream" ) ) {
					return (PersistentStream)Properties["_stream"];
				} else {
					return null;
				}
			}
			set {
				if( value != null ) {
					Properties["_stream"] = value;
				} else {
					Properties.Remove( "_stream" );
				}
			}
		}
	}
}
