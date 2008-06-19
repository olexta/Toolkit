using System;
using System.Xml;
using System.Xml.Schema;

namespace Toolkit.ConsoleXMLValidator
{
	public delegate void ValidationMessageEventHandler( string msg );

	public class Validator
	{
		private string m_FileName = String.Empty;
		private int m_WarnCount = 0;
		private int m_ErrCount = 0;
		private string m_Schema = String.Empty;
		private ValidationMessageEventHandler MESSAGE;

		public Validator( string fileName, ValidationMessageEventHandler MSG )
		{
			m_FileName = fileName;
			MESSAGE = MSG;
		}

		public bool Validate()
		{
			XmlReaderSettings settings = new XmlReaderSettings();
			
			settings.ValidationType = ValidationType.Schema;			

			settings.ValidationFlags =
				XmlSchemaValidationFlags.AllowXmlAttributes |
				XmlSchemaValidationFlags.ProcessIdentityConstraints |
				XmlSchemaValidationFlags.ProcessInlineSchema |
				XmlSchemaValidationFlags.ProcessSchemaLocation;
		
			settings.ValidationEventHandler +=
				new System.Xml.Schema.ValidationEventHandler( ValidationCallBack );

			XmlReader reader = null;
			XmlReader schemaReader = null;
		
			try {
				if( !String.IsNullOrEmpty( m_Schema ) ) {
					schemaReader = XmlReader.Create( m_Schema );
					settings.Schemas.Add( null, schemaReader );
					schemaReader.Close();
				}
				reader = XmlReader.Create( m_FileName, settings );
				m_WarnCount = 0;
				m_ErrCount = 0;
				while( reader.Read() )
					;
			} catch( Exception e ) {
				m_ErrCount++;
				MESSAGE( String.Format( "Error: {0}", e.Message ) );
			} finally {
				MESSAGE( String.Format( "Validation complete. {0} errors and {1} warning was occured.",
					m_ErrCount, m_WarnCount ) );
				if( reader != null )
					reader.Close();
			}

			if( m_ErrCount == 0 && m_WarnCount == 0 )
				return true;
			else
				return false;
		}

		public string Schema
		{
			get { return m_Schema; }
			set { m_Schema = value; }
		}

		public int ErrorsCount
		{
			get { return m_ErrCount; }
		}

		public int WarningsCount
		{
			get { return m_WarnCount; }
		}

		private void ValidationCallBack( object sender, ValidationEventArgs e )
		{
			if( e.Severity == XmlSeverityType.Error )
				m_ErrCount++;
			else
				m_WarnCount++;

			MESSAGE( String.Format( "{0}: {1}", e.Severity, e.Message ) );
		}
	}
}