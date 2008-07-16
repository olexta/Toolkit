//****************************************************************************
//*	Project		:	Workflow.Schema
//*
//*	Module		:	EnumInfo.cs
//*
//*	Content		:	
//*	Author		:	Nikita Marunyak
//*	Copyright	:	Copyright © 2008 Nikita Marunyak
//*
//* SVN			:	$Id$	  
//*
//****************************************************************************

using System;
using System.Collections.Generic;
using System.Xml;

using Toolkit.Collections;

namespace Toolkit.Workflow.Schema
{
	public class EnumInfo : IKeyedObject<string>
	{
		private Type m_EnumType;
		private bool m_Flags = false;
		private Map<int, string> m_Consts;

		internal EnumInfo( XmlNode enumNode )
		{
			// retrieving enum type name
			string typeName;
			typeName = enumNode.Attributes["ws:type"].Value;
			// trying to find type with retrieved name
			m_EnumType = Type.GetType( typeName );
			// throw exception if enum type was not found
			if( m_EnumType == null || !m_EnumType.IsEnum )
				throw new EnumTypeIsNotFoundException( typeName );
			// checking for FlagsAttribute in finded enum type
			if( m_EnumType.GetCustomAttributes(
					typeof( FlagsAttribute ), false ).Length > 0 )
				m_Flags = true;

			m_Consts = new Map<int, string>();
			// getting all values from enum
			Array enumValues = Enum.GetValues( m_EnumType );
			foreach( int value in enumValues ) {
				string caption;
				XmlNode constNode = null;
				// finding corresponded to current value xml node
				constNode = enumNode.SelectSingleNode(
					string.Format( "ws:const[ws:value={0}]", value ),
					MetaData.Instance.XMLNsMgr );
				// getting caption for current value from xml node or reflection
				if( constNode != null )
					caption = Tools.GetLocalizedString( constNode, "caption" );
				else
					caption = Enum.GetName( m_EnumType, value );
				// adding value->caption to dictionary
				m_Consts.Add( value, caption );
			}
		}

		public Type EnumType
		{
			get
			{
				return m_EnumType;
			}
		}

		public bool Flags
		{
			get
			{
				return m_Flags;
			}
		}

		public string GetCaption( int? constValue )
		{
			if( constValue == null )
				return null;
			if( !Flags ) {
				if( m_Consts.ContainsKey( (int)constValue ) )
					return m_Consts[(int)constValue];
				else
					return constValue.ToString();
			} else {
				string caption = "";
				foreach( int v in Constants ) {
					if( ( constValue & v ) == v ) {
						caption += m_Consts[v] + ",";
					}
				}
				if( caption != "" ) {
					caption = caption.TrimEnd( ',' );
				} else {
					caption = constValue.ToString();
				}
				return caption;
			}
		}

		public IEnumerable<int> Constants
		{
			get	{				
				return m_Consts.Keys;
			}
		}		

		string IKeyedObject<string>.Key
		{
			get
			{
				return EnumType.FullName;
			}
		}
	}
}
