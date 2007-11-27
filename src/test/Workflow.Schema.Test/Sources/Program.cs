using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;

namespace Workflow.Schema.Test
{
	class Program
	{
		static void Main( string[] args )
		{
			MetaData.InitSchema("uk-UA", @"..\var\schema.xml", @"..\etc\Workflow.Schema.xsd");
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
	
			Console.WriteLine("Press <Enter> to exit...");
			Console.ReadLine();
		}
	}
}
