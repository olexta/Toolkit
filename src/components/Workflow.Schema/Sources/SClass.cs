//****************************************************************************
//*	Project		:	Workflow.Schema
//*
//*	Module		:	SClass.cs
//*
//*	Content		:	Public class witch give information about specific object.
//*	Author		:	Nikita Marunyak
//*	Copyright	:	Copyright © 2007 Nikita Marunyak
//*
//****************************************************************************

using System;
using System.Text.RegularExpressions;
using System.Collections.Generic;
using System.Xml;

namespace Workflow.Schema
{
	/// <summary>
	/// This class gives full information about current state of IStateInfo object.
	/// </summary>
	public class SClass
	{
		/// <summary>
		/// // Reference to IClassInfo object.
		/// </summary>
		private IClassInfo m_object = null;

		/// <summary>
		/// Current state name.
		/// </summary>
 		private string m_StateName = String.Empty;

		/// <summary>
		///  Current
		/// </summary>
		private string m_StateMask = String.Empty;

		/// <summary>
		/// Reference to basic class information.
		/// </summary>
		private ClassNode m_Class;

		/// <summary>
		/// Internal ctor. 
		/// </summary>
		internal SClass( IClassInfo objectInstance )
		{
			m_object = objectInstance;
			m_Class = MetaData.Singleton.Classes[ m_object.Type ];

			defineStateMask();
			defineStateName();
		}

		/// <summary>
		/// Gets current state name.
		/// </summary>
		public string StateName
		{
			get { return m_StateName; }
		}

		/// <summary>
		/// Gets properties names.
		/// </summary>
		public string[] Properties
		{
			get
			{
				string[] res = new string[ m_Class.Properties.Count ];
				int i = 0;
				foreach( SProperty prop in m_Class.Properties )
					res[ i++ ] = prop.Name;

				return res;
			}
		}

		/// <summary>
		/// Gets methods names.
		/// </summary>
		public string[] Methods
		{
			get
			{
				string[] res = new string[ m_Class.Methods.Count ];
				int i = 0;
				foreach( SMethod method in m_Class.Methods )
					res[ i++ ] = method.Name;

				return res;
			}
		}

		/// <summary>
		/// Gets information about specified property.
		/// </summary>
		public SProperty GetProperty( string propName )
		{
			if( !m_Class.Properties.Contains( propName ) )
				throw new NoMemberInformationException( propName );

			SProperty prop = new SProperty( m_Class.Properties[ propName ] );

			if( !String.IsNullOrEmpty( m_StateName ) ) {
				XmlNode propertyNode = m_Class.StatesMap[ m_StateMask ].SelectSingleNode(
					@"ws:properties/ws:property[@ws:name='" + propName + "']", MetaData.Singleton.XMLNsMgr );

				if( propertyNode != null ) {
					XmlNode node;	
			
					// caption
					string cap = Tools.GetLocalizedString( propertyNode, "caption" );
					if( cap != null )
						prop.Caption = cap;

					// description
					string descr = Tools.GetLocalizedString( propertyNode, "description" );
					if( descr != null )
						prop.Description = descr;

					// default value
					node = propertyNode.SelectSingleNode( "ws:defaultValue",
						MetaData.Singleton.XMLNsMgr );
					if( node != null )
						prop.SetDefaultValue( node.InnerText );

					// display order
					node = propertyNode.SelectSingleNode( "ws:displayOrder",
						MetaData.Singleton.XMLNsMgr );
					if( node != null )
						prop.DisplayOrder = int.Parse( node.InnerText );

					// required flag
					node = propertyNode.SelectSingleNode( "ws:isRequired",
						MetaData.Singleton.XMLNsMgr );
					if( node != null )
						prop.IsRequired = bool.Parse( node.InnerText );

					// read only flag
					node = propertyNode.SelectSingleNode( "ws:isReadOnly",
						MetaData.Singleton.XMLNsMgr );
					if( node != null )
						prop.IsReadOnly = bool.Parse( node.InnerText );
				}
			}

			return prop;
		}

		/// <summary>
		/// Gets information about specified method.
		/// </summary>
		public SMethod GetMethod( string methName )
		{
			if( !m_Class.Methods.Contains( methName ) )
				throw new NoMemberInformationException( methName );

			SMethod method = new SMethod( m_Class.Methods[ methName ] );

			if( !String.IsNullOrEmpty( m_StateName ) ) {
				XmlNode methodNode = m_Class.StatesMap[ m_StateMask ].SelectSingleNode(
					@"ws:methods/ws:method[@ws:name='" + methName + "']", MetaData.Singleton.XMLNsMgr );

				if( methodNode != null ) {
					XmlNode node;

					// caption
					string cap = Tools.GetLocalizedString( methodNode, "caption" );
					if( cap != null )
						method.Caption = cap;

					// description
					string descr = Tools.GetLocalizedString( methodNode, "description" );
					if( descr != null )
						method.Description = descr;

					// availability flag
					node = methodNode.SelectSingleNode( "ws:isAvailable",
						MetaData.Singleton.XMLNsMgr );
					if( node != null )
						method.IsAvailable = bool.Parse( node.InnerText );
				}
			}

			return method;
		}

		/// <summary>
		/// Define mask for current state.
		/// </summary>
		private void defineStateMask()
		{
			// Input string for regular expression
			string list = "";
			// Pattern string for regular expression.
			string regexp;
			// Needed for finding max priority.
			int priority, maxPriority = 0;
			// Temporary array.
			string[] temp;

			// Create string that contains all available states situated
			// in separate lines
			foreach( string stateMask in m_Class.StatesMap.Keys ) {
				list += stateMask + "\n";
			}

			// 1) Specifies multiline mode
			// 2) Specifies that the match must occur at the beginning of the line
			regexp = @"(?m)^";

			// repeatably adds stateComponents elements to pattern string
			for( int i = 0; i < m_object.StateComponents.Length; i++ ) {
				// 1) First character must be '|'
				// 2) Next must be specified string or '*'
				regexp += @"\|(?:" + m_object.StateComponents[ i ] + @"|\*)";
			}
			// 1) Last character must be '|'
			// 2) Continues match only if the subexpression (zero or one carriage return (\r) and end of line($))
			//	  matches at this position on the right
			regexp += @"\|(?=\r?$)";

			// search all state names in schema
			// that can be applyed to input stateComponents by matching pattern string
			// and find state with max priority
			foreach( Match item in Regex.Matches( list, regexp ) ) {
				temp = item.Value.Split( '|' );			
				priority = 0;
				// Computing priority for current state mask.
				for( int i = temp.Length - 2; i >= 1; i-- )
					if( temp[ i ] != "*" )
						priority += (int)Math.Pow( 2, temp.Length - i - 1 );

				if( priority > maxPriority ) {
					maxPriority = priority;
					m_StateMask = item.Value;
				}
			}
		}

		/// <summary>
		/// Retrieves current state name and stores it in private field.
		/// </summary>
		private void defineStateName()
		{
			if( String.IsNullOrEmpty( m_StateMask ) )
				return;

			XmlNode stateNode = m_Class.StatesMap[ m_StateMask ];

			m_StateName = Tools.GetLocalizedString( stateNode, "name" );
		}
	}
}
