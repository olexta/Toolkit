using System;
using System.Xml;

namespace Workflow.Schema
{
	/// <summary>
	/// This class is created only for internal purposes.
	/// </summary>
	internal class Tools
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
				string.Format( "ws:{0}[@uiculture='{1}']", "localized" + suffix.ToUpper()[0] + suffix.Substring(1, suffix.Length - 1),
				MetaData.CurrentUICulture ), MetaData.Singleton.XMLNsMgr)) == null )
				locNode = node.SelectSingleNode(
					string.Format( "ws:{0}", suffix ),					
					MetaData.Singleton.XMLNsMgr );

			//string xPathQuery = string.Format(
			//    "if(exists(ws:{0}[@uiculture='{1}']))then ws{0}[@uiculture='{1}'] else ws:{2}",
			//    "localized" + suffix.ToUpper()[0] + suffix.Substring(1, suffix.Length - 1),
			//    MetaData.CurrentUICulture, suffix);

			//XmlNode locNode = node.SelectSingleNode(
			//    xPathQuery, MetaData.Singleton.XMLNsMgr );

			if( locNode != null )
				return locNode.InnerText;
			else
				return null;
		}
	}
}
