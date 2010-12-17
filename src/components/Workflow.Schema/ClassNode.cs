//****************************************************************************
//*	Project		:	Workflow.Schema
//*
//*	Module		:	ClassNode.cs
//*
//*	Content		:	Internal class for caching purposes.
//*	Author		:	Nikita Marunyak
//*	Copyright	:	Copyright © 2007 - 2008 Nikita Marunyak
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
	/// <summary>
	/// Class that stores information about class node in XML file.
	/// It obtain information about all properties, method and states.
	/// </summary>
	class ClassNode : IKeyedObject<string>
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

			m_Type = classNode.Attributes["ws:type"].Value;

			m_Caption = Tools.GetLocalizedString( classNode, "caption" );

			// searching all properties
			foreach( XmlNode propertyNode in classNode.SelectNodes(
				"ws:properties/ws:property",
				MetaData.Instance.XMLNsMgr ) ) {

				// create instance of SProperty with 0 values
				SProperty property = new SProperty(
					null, null, null, -1, null, false, false, null );

				// gets property name from attribute
				property.Name = propertyNode.Attributes.GetNamedItem( "ws:name" ).Value;

				// finding all property values using XPath
				property.Caption = Tools.GetLocalizedString( propertyNode, "caption" );

				property.Description = Tools.GetLocalizedString( propertyNode, "description" );

				string typeName = propertyNode.SelectSingleNode(
					"ws:type", MetaData.Instance.XMLNsMgr ).InnerText;
				switch( typeName ) {
					case "Byte":
					case "SByte":
					case "Int16":
					case "Int32":
					case "Int64":
					case "UInt16":
					case "UInt32":
					case "UInt64":
					case "Decimal":
					case "String":
					case "Char":
					case "Boolean":
					case "DateTime":
					case "Single":
					case "Double":
						property.Type = System.Type.GetType( "System." + typeName );
						break;
					default:
						if( System.Type.GetType( typeName ) != null ) {
							property.Type = System.Type.GetType( typeName );
							break;
						} else
							throw new ApplicationException( "Type not found " + typeName );
				}

				XmlNode node = propertyNode.SelectSingleNode(
					"ws:defaultValue", MetaData.Instance.XMLNsMgr );

				if( node != null )
					property.SetDefaultValue( node.InnerText );
				else
					property.SetDefaultValue( null );

				property.DisplayOrder = int.Parse( propertyNode.SelectSingleNode(
					"ws:displayOrder", MetaData.Instance.XMLNsMgr ).InnerText );

				property.IsReadOnly = bool.Parse( propertyNode.SelectSingleNode(
					"ws:isReadOnly", MetaData.Instance.XMLNsMgr ).InnerText );

				property.IsRequired = bool.Parse( propertyNode.SelectSingleNode(
					"ws:isRequired", MetaData.Instance.XMLNsMgr ).InnerText );

				// adds SProperty to cache
				m_Properties.Add( property );
			}

			// getting all methods
			foreach( XmlNode methodNode in classNode.SelectNodes(
				"ws:methods/ws:method", MetaData.Instance.XMLNsMgr ) ) {

				SMethod method = new SMethod( null, null, null, -1, false );

				// gets property name from attribute
				method.Name = methodNode.Attributes.GetNamedItem( "ws:name" ).Value;

				// finding all property values using XPath
				method.Caption = Tools.GetLocalizedString( methodNode, "caption" );

				method.Description = Tools.GetLocalizedString( methodNode, "description" );

				method.DisplayOrder = int.Parse( methodNode.SelectSingleNode(
					"ws:displayOrder", MetaData.Instance.XMLNsMgr ).InnerText );

				method.IsAvailable = bool.Parse( methodNode.SelectSingleNode(
					"ws:isAvailable", MetaData.Instance.XMLNsMgr ).InnerText );

				m_Methods.Add( method );
			}

			// getting all states
			foreach( XmlNode stateNode in classNode.SelectNodes(
				"ws:states/ws:state", MetaData.Instance.XMLNsMgr ) )
				m_StatesMap.Add(
					stateNode.Attributes.GetNamedItem( "ws:mask" ).Value,
					stateNode );
		}

		/// <summary>
		/// Returns class type.
		/// </summary>
		public string Type
		{
			get { return m_Type; }
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
