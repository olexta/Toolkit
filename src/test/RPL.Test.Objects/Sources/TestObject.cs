using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Diagnostics;

namespace RPL.Test
{
	///Object for use in RPL.Testing project
	public class CTestObject : CPersistentObject
	{
		public CTestObject()
		{
		}

		public CTestObject( int id, DateTime stamp, String name )
			: base( id, stamp, name ) { }

		///<summary>Name of Object</summary>
		public override string Name
		{
			get { return base.Name; }
			set { base.Name = value; }
		}

		public void AddParent( RPL.CPersistentObject obj )
		{
			base.Links.Add( obj );
		}

		public RPL.CPersistentObjects GetParents
		{
			get { return base.Links; }
		}

		///<summary>Implemetation of PersistentObject and IClassInfo</summary>
		public override string Type
		{
			get {
				// name of bussines object.
				return type();
			}
		}

		///<summary>Used to identify full name of this class type</summary>
		///<returns>String representation of this class</returns>
		public static string type()
		{
			// gets name of class.
			// Assumed that all bussines objects are named same way.
			return typeof( CTestObject ).FullName.ToString() + "," +
			   System.Reflection.Assembly.GetExecutingAssembly().GetName().Name;
		}

		///<summary>Static method that is used to get RetriveCriteria for searching
		/// objects of that class</summary>
		/// <returns>CRetrieveCriteria build for this class</returns>
		public static CRetrieveCriteria GetRetriveCriteria
		{
			get { //create Criteria based on internal name of class
				return new CRetrieveCriteria( CTestObject.type() );
			}
		}

		///<summary>Verification of instance integrity</summary>
		/// <returns>true if varificaton succeded</returns>
		/// <remarks>Before Validation all properties with DBNull must be removed</remarks>
		/// <exception cref="ArgumentOutOfRangeException">When number of required properties is insufficient</exception>
		private void checkState()
		{
			#region debug info
#if (DEBUG)
			Debug.Print( ">> CTestObject.checkState()" );
#endif
			#endregion

			int i = 0;
			foreach ( CPersistentObject obj in Links ) {
				Debug.Print( "Object link #{0} is {1}", i, obj.Name );
				i++;
			}

			#region debug info
#if (DEBUG)
			Debug.Print( "<< CTestObject.checkState()" );
#endif
			#endregion
		}

		public Stream _stream
		{
			get {
				if ( ID < 0 ) { throw new InvalidOperationException( "Object is deleted" ); };
				if ( IsProxy )
					Retrieve();
				if ( !Properties.Contains( "_stream" ) ) {
					return Stream.Null;
				} else {
					return (Stream)Properties["_stream"].Value;
				}
			}
			set {
				if ( IsProxy )
					Retrieve();
				Properties["_stream"].Value = value;
			}
		}

		public bool _boolean
		{
			get {
				if ( ID < 0 ) { throw new InvalidOperationException( "Object is deleted" ); };
				if ( IsProxy )
					Retrieve();
				if ( !Properties.Contains( "_boolean" ) ) {
					return false;
				} else {
					return (bool)Properties["_boolean"].Value;
				}
			}
			set {
				if ( IsProxy )
					Retrieve();
				Properties["_boolean"].Value = value;
			}
		}

		public string _string
		{
			get {
				if ( ID < 0 ) { throw new InvalidOperationException( "Object is deleted" ); };
				if ( IsProxy )
					Retrieve();
				if ( !Properties.Contains( "_string" ) ) {
					return string.Empty;
				} else {
					return (string)Properties["_string"].Value;
				}
			}
			set {
				if ( IsProxy )
					Retrieve();
				Properties["_string"].Value = value;
			}
		}

		public int _int
		{
			get {
				if ( ID < 0 ) { throw new InvalidOperationException( "Object is deleted" ); };
				if ( IsProxy )
					Retrieve();
				if ( !Properties.Contains( "_int" ) ) {
					return -1;
				} else {
					return (int)Properties["_int"].Value;
				}
			}
			set {
				if ( IsProxy )
					Retrieve();
				Properties["_int"].Value = value;
			}
		}

		public DateTime _datetime
		{
			get {
				if ( ID < 0 ) { throw new InvalidOperationException( "Object is deleted" ); };
				if ( IsProxy )
					Retrieve();
				if ( !Properties.Contains( "_datetime" ) ) {
					return new DateTime();
				} else {
					return (DateTime)Properties["_datetime"].Value;
				}
			}
			set {
				if ( IsProxy )
					Retrieve();
				Properties["_datetime"].Value = value;
			}
		}

		public double _double
		{
			get {
				if ( ID < 0 ) { throw new InvalidOperationException( "Object is deleted" ); };
				if ( IsProxy )
					Retrieve();
				if ( !Properties.Contains( "_double" ) ) {
					return -1;
				} else {
					return (double)Properties["_double"].Value;
				}
			}
			set {
				if ( IsProxy )
					Retrieve();
				Properties["_double"].Value = value;
			}
		}

		#region On***
		protected override void OnSave()
		{
			checkState();
			base.OnSave();
		}
		#endregion
	}
}
