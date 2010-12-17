using System;
using System.Xml;
using System.Xml.Schema;

namespace Toolkit.ConsoleXMLValidator
{
	class Program
	{
		static void Main( string[] args )
		{
			if( args.Length < 1 || args.Length > 2 ) {
				Console.WriteLine( "Usage: Toolkit.ConsoleXMLParser xmlfile [schemafile]" );
				return;
			}

			Validator validator = new Validator( args[ 0 ],
				delegate( string msg) { Console.WriteLine( msg + "\n" ); } );

			if( args.Length == 2 )
				validator.Schema = args[ 1 ];

			validator.Validate();

			//Console.WriteLine( "..." ); Console.ReadLine();
		}
	}
}
