using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Runtime.Serialization;
using BLEngine.SchemaLayer;
using System.Diagnostics;
using RPL.Schema;


namespace BLEngine.SchemaLayer
{
	class app
	{
		static void Main()
		{

			//for ( int i = 5; i-- > 0; Debug.WriteLine( i ) ) ;
			//CSState.CSStateName name = new CSState.CSStateName( "|Test|test|" );

			#region generate schema file
			string[] methods = new string[] { "Mark", "Availability", "Assignment", "Stream", "Priority", "CardType", "PageCount" };
			MetaData schema = MetaData.Singleton;
			schema.Name = "LineNBU";

			SClass sclass = new SClass();
			SState state;
			SProperty prop;

			state = new SState("|*|*|*|");
			int i = 0;
			foreach (String item in methods){
				prop = new SProperty();
				prop.IsRequired = true;
				prop.Name = item;
				prop.DefaultValue = "";
				prop.DisplayOrder = i;
				state.Properties.add( prop );
				i += i;
			}

			sclass.Name = "Line.BL.CDocument,Line.BL";
			sclass.add( state );
			schema.add( sclass );

			//for ( int h = 0; h < 2; h++ ) {
			//    sclass = new SClass();
			//    sclass.Name =(h % 2 == 0 ? "LineNBU_BL.CUser" : "LineNBU_BL.CDocument");
			//    for ( int j = 0; j < 5; j++ ) {
			//        state = new SState( SState.SStateName.cDelimiter + 
			//                            (j % 4 == 0 ? "New" : "*") +
			//                            SState.SStateName.cDelimiter + 
			//                            (j % 3 == 0 ? "Free" : "*") + 
			//                            SState.SStateName.cDelimiter +
			//                            (j % 2 == 0 ? "NoMark" : "*") +
			//                            SState.SStateName.cDelimiter	);
			//        for ( int i = 0; i < 2; i++ ) {
			//            prop = new SProperty();
			//            prop.DefaultValue = 1.1;
			//            prop.IsRequired = (i % 2 == 0 ? true : false);
			//            prop.Name = "Prop" + i.ToString();
			//            prop.DisplayOrder = i;

			//            method = new SMethod();
			//            method.IsAvailable = (i % 2 == 0 ? true : false);
			//            method.Name = "foo" + i.ToString();

			//            state.Properties.add( prop );
			//            state.Methods.add( method );
			//        }
			//        sclass.add( state );
			//    }
			//    schema.add( sclass );
			//}

			Stream stream;

			#region	Serialize
			stream = new FileStream( "schema_output.xml",
									FileMode.Create,
									FileAccess.Write,
									FileShare.None );
			schema.SaveSchema( stream );
			stream.Close();
			#endregion
			#endregion

			//stream = new FileStream( "..\\..\\SchemaClasses\\_output\\schema_output.xml",
			//                                    FileMode.Open,
			//                                    FileAccess.Read,
			//                                    FileShare.Read );
			//schema.InitSchema( stream );

			
			//#region Deserialize
			//formatter = new System.Runtime.Serialization.Formatters.Soap.SoapFormatter();
			////formatter.SurrogateSelector = selector;
			//stream = new FileStream("b:\\SerializeTest.xml", FileMode.Open,
			//                               FileAccess.ReadWrite, FileShare.Read);
			//CSchema obj = (CSchema)formatter.Deserialize(stream);
			//#endregion

			//CSClass sclass = new CSClass();
			//CSClass sclass1 = new CSClass();
			//CSState sstate = new CSState();
			////CSAttribute sattribute = new CSAttribute();
			//CSAttribute sattribute1 = new CSAttribute();

			//sclass.name = "Assemble.CNew";
			////sclass1.name = "Assemble.COld";
			//sstate.name = "State1";
			//schema.Add(sclass);
			//schema["Assemble.CNew"].States.Add(sstate); 
			//for (int i = 0; i < 3; i++)
			//{
			//    CSAttribute sattribute = new CSAttribute();
			//    sattribute.name = "Atrrib" + i.ToString();
			//    sattribute.defaultValue = "TETSVALUEfor" + i;
			//    sattribute.isRequired = false;
			//    schema["Assemble.CNew"].States[0].Attributes.Add(sattribute);
			//}

			//for (int i = 0; i < 3; i++)
			//{
			//    CSMethod smethod = new CSMethod();
			//    smethod.name = "Method" + i;
			//    smethod.isAvailable = false;
			//    schema["Assemble.CNew"].States[0].Methods.Add(smethod);
			//}
		}
	}
}
