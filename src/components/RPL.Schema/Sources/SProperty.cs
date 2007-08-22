//****************************************************************************
//*	Project		:	Robust Persistence Layer
//*
//*	Module		:	CSProperty.cs
//*
//*	Content		:	Implementation of CSProperty class
//*	Author		:	Alexander Kurbatov
//*	Copyright	:	Copyright © 2006 Alexander Kurbatov
//*
//*	This class is used to store following some class's property information:
//*	  1) Name - Property name
//*	  2) IsRequired - Determines whether or not this property is requiered to be entered
//*	  3) DefaultValue - Default value of property
//*	  4) DisplayOrder - Order of property according to it's state
//*
//*	Class implements ISerializable, so it can serialize/deserialize itself
//*
//****************************************************************************
using System;
using System.Collections.Generic;
using System.Runtime.Serialization;

namespace ABBYY.Toolkit.RPL.Schema
{
	/// <summary>
	/// This class is used to store class's property information
	/// </summary>
	[Serializable]
	public class SProperty : ISerializable
	{
		/// <summary>
		/// Stores property name.
		/// </summary>
		private string m_Name;

		/// <summary>
		/// Stores property requirement.
		/// </summary>
		private bool m_IsRequired;

		/// <summary>
		/// Stores default value of property. Stored as string.
		/// Convertion to another type and check needed to be performed
		/// in class that uses it.
		/// </summary>
		private object m_DefaultValue;

		/// <summary>
		/// Stores iformation about Display Order of Property.
		/// </summary>
		private int m_DisplayOrder;

		/// <summary>
		/// Default constructor of class.
		/// Initialises default value of isRequiredField
		/// </summary>
		internal SProperty() { }

		#region Accessor methods for internal fields
		/// <summary>
		/// Property accessor to m_name.
		/// Set accessor has only internal access modifier
		/// </summary>
		public string Name
		{
			get
			{
				return m_Name;
			}
			// can't change property from out side in security reason
			internal set
			{
				m_Name = value;
			}
		}

		/// <summary>
		/// Property accessor to m_isRequired.
		/// Set accessor has only internal access modifier
		/// </summary>
		public bool IsRequired
		{
			get
			{
				return m_IsRequired;
			}
			// can't change property from out side in security reason
			internal set
			{
				m_IsRequired = value;
			}
		}

		/// <summary>
		/// Property accessor to m_defaultValue.
		/// Set accessor has only internal access modifier
		/// </summary>
		public object DefaultValue
		{
			get
			{
				return m_DefaultValue;
			}
			// can't change property from out side in security reason
			internal set
			{
				m_DefaultValue = value;
			}
		}

		/// <summary>
		/// Property accessor to m_defaultValue.
		/// Set accessor has only internal access modifier
		/// </summary>
		public int DisplayOrder
		{
			get
			{
				return m_DisplayOrder;
			}
			// can't change property from out side in security reason
			internal set
			{
				m_DisplayOrder = value;
			}
		}
		#endregion

		/// <summary>
		/// Override Object method to return needed value
		/// </summary>
		public override string ToString()
		{
			return Name;
		}

		#region ISerializable Members
		/// <summary>
		/// Implementation of ISerializable
		/// </summary>
		public void GetObjectData( SerializationInfo info, StreamingContext context )
		{
			// check input parameters
			if( info == null )
				throw new ArgumentNullException( "info" );
			// Declares internal fields to be serialized
			info.AddValue( "Name", m_Name );
			info.AddValue( "IsRequired", m_IsRequired );
			info.AddValue( "DefaultValue", m_DefaultValue );
			info.AddValue( "DisplayOrder", m_DisplayOrder );
		}

		/// <summary>
		/// Constructor needed to deserialize class
		/// </summary>
		protected SProperty( SerializationInfo info, StreamingContext context )
		{
			// check input parameters
			if( info == null )
				throw new ArgumentNullException( "info" );
			// Declares internal fields to be serialized
			m_Name = (string) info.GetValue( "Name", typeof( string ) );
			m_IsRequired = (bool) info.GetValue( "IsRequired", typeof( bool ) );
			m_DefaultValue = info.GetValue( "DefaultValue", typeof( object ) );
			m_DisplayOrder = (int) info.GetValue( "DisplayOrder", typeof( int ) );
		}
		#endregion
	}
}