using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.IO;

[Flags]
enum NumericEnum
{
	One = 1,
	Two = 2,
	Three = 4,
	Four = 8
}

namespace Toolkit.Workflow.Schema.Test
{
	class Program
	{
		static void Main( string[] args )
		{
			Stream xml = new FileStream(
				@"Toolkit.Workflow.Schema.Test.TestSchema.xml",
				FileMode.Open );
			Stream xsd = new FileStream(
				@"..\etc\Toolkit.Workflow.Schema.xsd",
				FileMode.Open );
			MetaData.InitSchema("uk-UA", xml, xsd );
			xml.Close();
			xsd.Close();
			Console.WriteLine( MetaData.Instance.Name );
			TestObject to = new TestObject();
			Console.WriteLine( "Class caption: {0}", MetaData.Instance[ to ].Caption );
			Console.WriteLine( "Current state: {0}", MetaData.Instance[ to ].StateName );
			Console.WriteLine( "Properties" );
			foreach( string s in MetaData.Instance[ to ].PropertiesNames ) {
				Console.WriteLine( "Property: {0}", s );
				SProperty prop = MetaData.Instance[ to ].GetProperty( s );
				Console.WriteLine( "\tCaption: {0}", prop.Caption );
				Console.WriteLine( "\tDescription: {0}", prop.Description );
				Console.WriteLine( "\tType: {0}", prop.Type);
				Console.WriteLine( "\tDefault value: {0}", prop.DefaultValue );
				Console.WriteLine( "\tDisplay order: {0}", prop.DisplayOrder );
				Console.WriteLine( "\tIs read only: {0}", prop.IsReadOnly );
				Console.WriteLine( "\tIs required: {0}", prop.IsRequired );
			}
			Console.WriteLine( "Methods" );
			foreach( string s in MetaData.Instance[ to ].MethodsNames ) {
				Console.WriteLine( "Method: {0}", s );
				SMethod method = MetaData.Instance[ to ].GetMethod( s );
				Console.WriteLine( "\tCaption: {0}", method.Caption );
				Console.WriteLine( "\tDescription: {0}", method.Description );
				Console.WriteLine( "\tDisplay order: {0}", method.DisplayOrder );
				Console.WriteLine( "\tIs available: {0}", method.IsAvailable );
			}


			EnumInfo enumInfo = MetaData.Instance.GetEnumInfo( "NumericEnum" );
			Console.WriteLine( "{0}, Flags={1}:", enumInfo.EnumType.FullName, enumInfo.Flags );
			foreach( int constValue in enumInfo.Constants )
				Console.WriteLine( "{0}: {1}", constValue, enumInfo.GetCaption( constValue ) );

			Console.WriteLine("Press <Enter> to exit...");
			Console.ReadLine();
		}
	}
}
