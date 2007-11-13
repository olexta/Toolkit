//****************************************************************************
//*	Project		:	Robust Persistence Layer
//*
//*	Module		:	SProperty.cs
//*
//*	Content		:	Implementation of SProperty class
//*	Author		:	Alexander Kurbatov, Nikita Marunyak
//*	Copyright	:	Copyright © 2006, 2007 Alexander Kurbatov, Nikita Marunyak
//*
//*	This class is used to store following some class's property information:
//*	  1) IsRequired - Determines whether or not this property is requiered to be entered
//*   2) IsReadOnly - Indicates that user can or can't change value of this property
//*	  2) DefaultValue - Default value of property
//*	  3) DisplayOrder - Order of property according to it's state
//*
//*	Class implements ISerializable, so it can serialize/deserialize itself
//*
//****************************************************************************
using System;

namespace Workflow.Schema
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
		private string m_Type;

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
		/// Stores iformation about Display Order of Property.
		/// </summary>
		private int m_DisplayOrder = 0;

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
		public string Type
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
		/// Translates string representation of default value
		/// to properly type and stores it.
		/// </summary>
		/// <param name="strValue"></param>
		internal void SetDefaultValue( string strValue )
		{
			switch( Type ) {
				case "Byte":
					DefaultValue = Byte.Parse( strValue );
					break;
				case "SByte":
					DefaultValue = SByte.Parse( strValue );
					break;
				case "Int16":
					DefaultValue = Int16.Parse( strValue );
					break;
				case "Int32":
					DefaultValue = Int32.Parse( strValue );
					break;
				case "Int64":
					DefaultValue = Int64.Parse( strValue );
					break;
				case "UInt16":
					DefaultValue = UInt16.Parse( strValue );
					break;
				case "UInt32":
					DefaultValue = UInt32.Parse( strValue );
					break;
				case "UInt64":
					DefaultValue = UInt64.Parse( strValue );
					break;
				case "Decimal":
					DefaultValue = Decimal.Parse( strValue );
					break;
				case "Single":
					DefaultValue = Single.Parse( strValue );
					break;
				case "Double":
					DefaultValue = Double.Parse( strValue );
					break;
				case "Bollean":
					DefaultValue = Boolean.Parse( strValue );
					break;
				case "String":
					DefaultValue = strValue;
					break;
				case "Char":
					DefaultValue = strValue[ 0 ];
					break;
				case "DateTime":
					DefaultValue = DateTime.Parse( strValue );
					break;
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
		/// Overridden Object.ToString().
		/// </summary>
		public override string ToString()
		{
			return Name;
		}

		/// <summary>
		/// Internal class ctor.
		/// </summary>
		internal SProperty()
		{
		}

		/// <summary>
		/// Internal copy ctor.
		/// </summary>
		internal SProperty( SProperty prop )
			: base( prop )
		{
			Type = prop.Type;
			IsReadOnly = prop.IsReadOnly;
			IsReadOnly = prop.IsRequired;
			DisplayOrder = prop.DisplayOrder;
			DefaultValue = prop.DefaultValue;
		}
	}
}
