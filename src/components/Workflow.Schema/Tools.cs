//****************************************************************************
//*	Project		:	Workflow.Schema
//*
//*	Module		:	Tools.cs
//*
//*	Content		:	Usefull code for internal purposes
//*	Author		:	Nikita Marunyak
//*	Copyright	:	Copyright © 2007 - 2008 Nikita Marunyak
//*
//* SVN			:	$Id$	  
//*
//****************************************************************************

using System;
using System.Xml;

namespace Toolkit.Workflow.Schema
{
	/// <summary>
	/// This class is created only for internal purposes.
	/// </summary>
	class Tools
	{
		/// <summary>
		/// Gets localized string for specified node.
		/// First it search for "localizedSuffix" node
		/// and if it not present search for "suffix" node.
		/// </summary>
		/// <returns>Null if string with specified suffix is not founded
		/// otherwise returns desired value.</returns>
		public static string GetLocalizedString( XmlNode node, string suffix )
		{
			XmlNode locNode;

			if( (locNode = node.SelectSingleNode(
				string.Format( "ws:{0}[@ws:uiculture='{1}']", "localized" + suffix.ToUpper()[0] + suffix.Substring(1, suffix.Length - 1),
				MetaData.Instance.CurrentUICulture ), MetaData.Instance.XMLNsMgr) ) == null )
				locNode = node.SelectSingleNode(
					string.Format( "ws:{0}", suffix ),					
					MetaData.Instance.XMLNsMgr );

			if( locNode != null )
				return locNode.InnerText;
			else
				return null;
		}
	}
}
