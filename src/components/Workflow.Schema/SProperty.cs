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
//* SVN			:	$Id$	  
//*
//****************************************************************************
using System;
using System.Globalization;

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
		/// String, Char, Boolean, DateTime, Single, Double, Stream and enums.
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
			if( m_Type.IsEnum ) {
				DefaultValue = Enum.Parse( m_Type, strValue );
			} else {
				switch( m_Type.ToString() ) {
					case "System.Byte":
						if( strValue.Length > 0 )
							DefaultValue = Byte.Parse( strValue, CultureInfo.InvariantCulture.NumberFormat );
						break;
					case "System.SByte":
						if( strValue.Length > 0 )
							DefaultValue = SByte.Parse( strValue, CultureInfo.InvariantCulture.NumberFormat );
						break;
					case "System.Int16":
						if( strValue.Length > 0 )
							DefaultValue = Int16.Parse( strValue, CultureInfo.InvariantCulture.NumberFormat );
						break;
					case "System.Int32":
						if( strValue.Length > 0 )
							DefaultValue = Int32.Parse( strValue, CultureInfo.InvariantCulture.NumberFormat );
						break;
					case "System.Int64":
						if( strValue.Length > 0 )
							DefaultValue = Int64.Parse( strValue, CultureInfo.InvariantCulture.NumberFormat );
						break;
					case "System.UInt16":
						if( strValue.Length > 0 )
							DefaultValue = UInt16.Parse( strValue, CultureInfo.InvariantCulture.NumberFormat );
						break;
					case "System.UInt32":
						if( strValue.Length > 0 )
							DefaultValue = UInt32.Parse( strValue, CultureInfo.InvariantCulture.NumberFormat );
						break;
					case "System.UInt64":
						if( strValue.Length > 0 )
							DefaultValue = UInt64.Parse( strValue, CultureInfo.InvariantCulture.NumberFormat );
						break;
					case "System.Decimal":
						if( strValue.Length > 0 )
							DefaultValue = Decimal.Parse( strValue, CultureInfo.InvariantCulture.NumberFormat );
						break;
					case "System.Single":
						if( strValue.Length > 0 )
							DefaultValue = Single.Parse( strValue, CultureInfo.InvariantCulture.NumberFormat );
						break;
					case "System.Double":
						if( strValue.Length > 0 )
							DefaultValue = Double.Parse( strValue, CultureInfo.InvariantCulture.NumberFormat );
						break;
					case "System.Boolean":
						if( strValue.Length > 0 )
							DefaultValue = Boolean.Parse( strValue );
						break;
					case "System.String":
						DefaultValue = strValue;
						break;
					case "System.Char":
						if( strValue.Length > 0 )
							DefaultValue = strValue[0];
						break;
					case "System.DateTime":
						if( strValue.Length > 0 )
							DefaultValue = DateTime.Parse( strValue, CultureInfo.InvariantCulture.DateTimeFormat );
						break;
					default:
						throw new ApplicationException(
							"Property of " + m_Type.ToString() + " type cannot have default value." );
				}
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
