//****************************************************************************
//*	Project		:	Workflow.Schema
//*
//*	Module		:	PropSetInfo.cs
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
using Toolkit.Collections;
using System.Xml;

namespace Toolkit.Workflow.Schema
{
	public class PropSetInfo: IKeyedObject<string>
	{
		public class PropSetPropertyInfo
		{
			private string m_Name;
			private string m_Caption;
			private string m_Description;

			internal PropSetPropertyInfo(
				string name,
				string caption,
				string description )
			{
				m_Name = name;
				m_Caption = caption;
				m_Description = description;
			}

			public string Name
			{
				get {
					return m_Name;
				}
			}

			public string Caption
			{
				get {
					return m_Caption;
				}
			}

			public string Description
			{
				get {
					return m_Description;
				}
			}
		}

		private string m_Name;
		private string m_Caption;
		private Map<string, PropSetPropertyInfo> m_Properties;

		internal PropSetInfo( XmlNode node )
		{
			m_Properties = new Map<string, PropSetPropertyInfo>();
			m_Name = node.Attributes.GetNamedItem( "ws:name" ).Value;
			m_Caption = Tools.GetLocalizedString( node, "caption" );
			string keyName, keyCaption, keyDescription;
			foreach( XmlNode keyNode in node.SelectNodes(
				"ws:properties/ws:property",
				MetaData.Instance.XMLNsMgr ) ) {
				keyName = keyNode.Attributes.GetNamedItem( "ws:name" ).Value;
				keyCaption = Tools.GetLocalizedString( keyNode, "caption" );
				keyDescription = Tools.GetLocalizedString( keyNode, "description" );
				m_Properties.Add(
					keyName,
					new PropSetPropertyInfo( keyName, keyCaption, keyDescription ) );
			}
		}

		public string Name
		{
			get {
				return m_Name;
			}
		}

		public string Caption
		{
			get {
				return m_Caption;
			}
		}

		public IEnumerable<string> PropertiesNames
		{
			get {
				return m_Properties.Keys;
			}
		}

		public PropSetPropertyInfo GetPropertynfo( string propertyName )
		{
			if( m_Properties.ContainsKey( propertyName ) ) {
				return m_Properties[ propertyName ];
			} else {
				return null;
			}
		}

		string IKeyedObject<string>.Key
		{
			get {
				return Name;
			}
		}
	}
}
