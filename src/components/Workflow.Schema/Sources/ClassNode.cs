//****************************************************************************
//*	Project		:	Workflow.Schema
//*
//*	Module		:	ClassNode.cs
//*
//*	Content		:	Internal class for caching purposes.
//*	Author		:	Nikita Marunyak
//*	Copyright	:	Copyright © 2007 Nikita Marunyak
//*
//****************************************************************************

using System;
using System.Collections.Generic;
using System.Xml;

using Toolkit.Collections;

namespace Workflow.Schema
{
	/// <summary>
	/// Class that stores information about class node in XML file.
	/// It obtain information about all properties, method and states.
	/// </summary>
	internal class ClassNode: IKeyedObject<string>
	{
		/// <summary>
		/// Stores class type for current instance.
		/// </summary>
		private string m_Type;

		/// <summary>
		/// Stores caption for current instance.
		/// </summary>
		private string m_Caption;

		/// <summary>
		/// Using this dictionary you can retrieve XML node for specified state.
		/// </summary>
		private Map<string, XmlNode> m_StatesMap;

		/// <summary>
		/// Stores info about all properties.
		/// </summary>
		private KeyedMap<string, SProperty> m_Properties;

		/// <summary>
		/// Stores info about all methods.
		/// </summary>
		private KeyedMap<string, SMethod> m_Methods;

		/// <summary>
		/// Ctor.
		/// </summary>
		public ClassNode( XmlNode classNode )
		{
			m_StatesMap = new Map<string, XmlNode>();
			m_Properties = new KeyedMap<string, SProperty>();
			m_Methods = new KeyedMap<string, SMethod>();

			m_Type = classNode.Attributes[ "ws:type" ].Value;

			// searching all properties
			foreach( XmlNode propertyNode in classNode.SelectNodes(
				"ws:properties/ws:property",
				MetaData.Singleton.XMLNsMgr ) ) {

				// create instance of SProperty
				SProperty property = new SProperty();
				
				// gets property name from attribute
				property.Name = propertyNode.Attributes.GetNamedItem( "ws:name" ).Value;

				// finding all property values using XPath
				property.Caption = Tools.GetLocalizedString( propertyNode, "caption" );

				property.Description = Tools.GetLocalizedString( propertyNode, "description" );

				property.Type = propertyNode.SelectSingleNode(
					"ws:type", MetaData.Singleton.XMLNsMgr ).InnerText;

				property.SetDefaultValue( propertyNode.SelectSingleNode(
					"ws:defaultValue", MetaData.Singleton.XMLNsMgr ).InnerText );

				property.DisplayOrder = int.Parse( propertyNode.SelectSingleNode(
					"ws:displayOrder", MetaData.Singleton.XMLNsMgr ).InnerText );

				property.IsReadOnly = bool.Parse( propertyNode.SelectSingleNode(
					"ws:isReadOnly", MetaData.Singleton.XMLNsMgr ).InnerText );

				property.IsRequired = bool.Parse( propertyNode.SelectSingleNode(
					"ws:isRequired", MetaData.Singleton.XMLNsMgr ).InnerText );

				// adds SProperty to cache
				m_Properties.Add( property );
			}
			
			// getting all methods
			foreach( XmlNode methodNode in classNode.SelectNodes(
				"ws:methods/ws:method", MetaData.Singleton.XMLNsMgr ) ) {

				SMethod method = new SMethod();

				// gets property name from attribute
				method.Name = methodNode.Attributes.GetNamedItem( "ws:name" ).Value;

				// finding all property values using XPath
				method.Caption = Tools.GetLocalizedString( methodNode, "caption" );

				method.Description = Tools.GetLocalizedString( methodNode, "description" );

				method.IsAvailable = bool.Parse( methodNode.SelectSingleNode(
					"ws:isAvailable", MetaData.Singleton.XMLNsMgr ).InnerText );

				m_Methods.Add( method );
			}

			// getting all states
			foreach( XmlNode stateNode in classNode.SelectNodes(
				"ws:states/ws:state", MetaData.Singleton.XMLNsMgr ) )
				m_StatesMap.Add(
					stateNode.Attributes.GetNamedItem( "ws:mask" ).Value,
					stateNode );			
		}

		/// <summary>
		/// Returns class type.
		/// </summary>
		public string Type
		{
			get	{ return m_Type; }
		}

		/// <summary>
		/// Returns class type.
		/// This property is needed only for storing instance in KeyedMap collection.
		/// </summary>
		public string Key
		{
			get { return Type; }
		}

		/// <summary>
		/// Returns caption.
		/// </summary>
		public string Caption
		{
			get { return m_Caption; }
		}

		/// <summary>
		/// Returns map: State name -> XML node.
		/// </summary>
		public Map<string, XmlNode> StatesMap
		{
			get { return m_StatesMap; }
		}

		/// <summary>
		/// Returns properties.
		/// </summary>
		public KeyedMap<string, SProperty> Properties
		{
			get { return m_Properties; }
		}

		/// <summary>
		/// Returns methods.
		/// </summary>
		public KeyedMap<string, SMethod> Methods
		{
			get { return m_Methods; }
		}
	}
}
