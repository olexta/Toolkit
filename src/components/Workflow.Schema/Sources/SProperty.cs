//****************************************************************************
//*	Project		:	Workflow.Schema
//*
//*	Module		:	SProperty.cs
//*
//*	Content		:	Implementation of SProperty class
//*	Author		:	Alexander Kurbatov, Nikita Marunyak
//*	Copyright	:	Copyright © 2006 - 2008 Alexander Kurbatov, Nikita Marunyak
//*
//*	This class is used to store following some class's property information:
//*	  1) IsRequired - Determines whether or not this property is requiered to be entered
//*   2) IsReadOnly - Indicates that user can or can't change value of this property
//*	  2) DefaultValue - Default value of property
//*	  3) DisplayOrder - Order of property according to it's state
//*
//****************************************************************************
using System;

namespace Toolkit.Workflow.Schema
{
	/// <summary>
	/// This class is used to store class's property information
	/// </summary>
	public class SProperty : SClassMember
	{
		/// <summary>
		/// Stores property type.
		/// Available types:
		/// Byte, SByte, Int16, Int32, Int64, UInt16, UInt32, UInt64, Decimal
		/// String, Char, Boolean, DateTime, Single, Double
		/// </summary>
		private Type m_Type;

		/// <summary>
		/// Stores property requirement.
		/// </summary>
		private bool m_IsRequired = true;

		/// <summary>
		/// Stores default value of property. Stored as string.
		/// Convertion to another type and check needed to be performed
		/// in class that uses it.
		/// </summary>
		private object m_DefaultValue = null;

		/// <summary>
		/// Stores readonly flag
		/// </summary>
		private bool m_IsReadOnly = true;

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
		/// Property type.
		/// </summary>
		public Type Type
		{
			get
			{
				return m_Type;
			}
			internal set
			{
				m_Type = value;
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
		/// Property accessor to m_IsReadOnly.
		/// Set accessor has only internal access modifier
		/// </summary>
		public bool IsReadOnly
		{
			get { return m_IsReadOnly; }
			internal set { m_IsReadOnly = value; }
		}

		/// <summary>
		/// Translates string representation of default value
		/// to properly type and stores it.
		/// </summary>
		internal void SetDefaultValue( string strValue )
		{
			if( strValue == null ) {
				DefaultValue = null;
				return;
			}
			switch( m_Type.ToString() ) {
				case "System.Byte":
					DefaultValue = Byte.Parse( strValue );
					break;
				case "System.SByte":
					DefaultValue = SByte.Parse( strValue );
					break;
				case "System.Int16":
					DefaultValue = Int16.Parse( strValue );
					break;
				case "System.Int32":
					DefaultValue = Int32.Parse( strValue );
					break;
				case "System.Int64":
					DefaultValue = Int64.Parse( strValue );
					break;
				case "System.UInt16":
					DefaultValue = UInt16.Parse( strValue );
					break;
				case "System.UInt32":
					DefaultValue = UInt32.Parse( strValue );
					break;
				case "System.UInt64":
					DefaultValue = UInt64.Parse( strValue );
					break;
				case "System.Decimal":
					DefaultValue = Decimal.Parse( strValue );
					break;
				case "System.Single":
					DefaultValue = Single.Parse( strValue );
					break;
				case "System.Double":
					DefaultValue = Double.Parse( strValue );
					break;
				case "System.Boolean":
					DefaultValue = Boolean.Parse( strValue );
					break;
				case "System.String":
					DefaultValue = strValue;
					break;
				case "System.Char":
					DefaultValue = strValue[ 0 ];
					break;
				case "System.DateTime":
					DefaultValue = DateTime.Parse( strValue );
					break;
				case "System.IO.Stream":
					throw new InvalidOperationException(
						"Object of System.IO.Stream type can't have default value!" );
			}
		}

		/// <summary>
		/// Overridden Object.ToString().
		/// </summary>
		public override string ToString()
		{
			return Name;
		}

		/// <summary>
		/// Internal class ctor.
		/// </summary>
		public SProperty(
			string name,
			string caption,
			string description,
			int displayOrder,
			Type type,
			bool isReadOnly,
			bool isRequired,
			object defaultValue)
			: base( name, caption, description, displayOrder )
		{
			m_Type = type;
			m_IsReadOnly = isReadOnly;
			m_IsRequired = isRequired;
			m_DefaultValue = defaultValue;
		}

		/// <summary>
		/// Internal copy ctor.
		/// </summary>
		public SProperty( SProperty prop )
			: base( prop )
		{
			Type = prop.Type;
			IsReadOnly = prop.IsReadOnly;
			IsRequired = prop.IsRequired;
			DefaultValue = prop.DefaultValue;
		}
	}
}
