using System;
using System.IO;
using System.Text.RegularExpressions;
using System.Reflection;
using System.Runtime.Remoting.Channels;
using System.Runtime.Remoting;
using System.Runtime.Remoting.Messaging;
using System.Runtime.Remoting.Proxies;
using System.Runtime.Remoting.Channels.Tcp;
using System.Diagnostics;
using Toolkit.Remoting;
using Toolkit.RPL.Factories;


namespace Toolkit.RPL.Test
{
	class Program
	{
		static void Main( string[] args )
		{
			new ConsoleParser();
		}
	}

	class ConsoleParser
	{
		static string[] m_listFuct = 
			new string[] { "add - adds object(s), with or without file attachment",
							"delete - deletes object(s)", 
							"update - updates object(s) property(ies)",
							"search - searches saved object(s)",
							"trans - switches transaction mode", 
							"perform - performes created transaction", 
							"error - adds error command to transaction " +
								"(used for transaction rollback test)", 
							"exit - quits application" };
		private bool m_transEnabled = false;
		private RPL.PersistentTransaction m_trans = null;
		private string m_hostname;

		public ConsoleParser()
		{
			string str;
			bool needExit = false;
			Console.WriteLine( "Enter host name" );
			m_hostname = Console.ReadLine();
			if ( m_hostname == string.Empty ) m_hostname = "localhost";
			remotingActivation(); Console.WriteLine( "Remoting activated." ); 

			while ( !needExit ) {
				try {	
					str = Console.ReadLine();
					switch ( str ) {
						case "exit":
							needExit = true;
							break;
						case "add":
							cmdAdd();
							break;
						case "delete":
							cmdDelete();
							break;
						case "trans":
							cmdTrans();
							break;
						case "perform":
							cmdPerform();
							break;
						case "search":
							cmdSearch();
							break;
						case "error":
							cmdAddError();
							break;
						case "test":
							cmdTest();
							break;
						default:
							cmdList();
							break;
					}
				} catch( Exception ex ) {
					Console.WriteLine( ex );
				}
			}
			PersistenceBroker.Close();
		}

		private void cmdTest()
		{
			TestObject obj = new TestObject();
			obj._datetime = DateTime.Now;
			Console.WriteLine( obj._datetime.Value.ToString( "yyyy-MM-dd HH:mm:ss.fff" ) );
			obj.Save();
			Console.WriteLine( obj._datetime.Value.ToString("yyyy-MM-dd HH:mm:ss.fff") );	
        }

        private void cmdAddError()
		{
            RetrieveCriteria rc = new RetrieveCriteria( (new TestObject()).Type, " (fail :)");
            if( m_trans == null ) {
                rc.Perform();
            } else {
                TestObject obj = new TestObject();
                obj.FailOn = TestObject.FAIL.Save;
                m_trans.Add( obj, PersistentTransaction.ACTION.Save );
            }
		}

		private void cmdSearch()
		{
			string sWhere = string.Empty;
			string sOrderBy = string.Empty;
			int sCount = 100;
			int sBottom = 0;
			bool sProxy = true;
			RetrieveCriteria crit;

			Console.WriteLine( "Enter SQL Where condition ex.:(Name = 'test')" );
			sWhere = Console.ReadLine();
			if ( string.IsNullOrEmpty( sWhere ) ) {
				sWhere = "(1 = 1)";
				Console.WriteLine( "Search Condition assumed as:" + sWhere );
			}
			// get Order request from user input
			Console.WriteLine( "Enter Order By clause:" );
			sOrderBy = Console.ReadLine();
			if ( string.IsNullOrEmpty( sOrderBy ) )
				Console.WriteLine( "No order condition" );
			// get search count from user input
			Console.WriteLine( "Search count:" );
			try {
				sCount = Convert.ToInt32( Console.ReadLine() );
			} catch { } finally {
				Console.WriteLine( "Count limit is: " + sCount );
			}
			// get bottom count from user input
			Console.WriteLine( "Bottom count:" );
			try {
				sBottom = Convert.ToInt32( Console.ReadLine() );
			} catch { } finally {
				Console.WriteLine( "Bottom is: {0}", sBottom );
			}
			// do retrive proxy or not objects
			Console.WriteLine( "Retrive proxy objects? (Y/N)" );
			if ( string.Compare( Console.ReadLine(), "Y", true ) != 0 ) {
				sProxy = false;
				Console.WriteLine( "\tProxy switch is: {0}", sProxy );
			}

			crit = 
				new RetrieveCriteria( (new TestObject()).Type,
									  sWhere,
									  sOrderBy );
			crit.CountLimit = sCount;
			crit.BottomLimit = sBottom;
			crit.AsProxies = sProxy;
			
			Console.WriteLine( DateTime.Now.ToLongTimeString() +
								": Search request begin" );
			
			crit.Perform();
			
			Console.WriteLine( DateTime.Now.ToLongTimeString() + 
								": Search request completed" );
			Console.WriteLine( "Was found: " + crit.Count );
			
			if ( crit.Count > 0 ) {
				Console.WriteLine( "Count to list:" );

				try {
					int count = Convert.ToInt32( Console.ReadLine() );
					for ( int i = 0; i < count; i++ ) {
						Console.WriteLine( crit[i].ID + "\t" + crit[i].Name + "\t" + crit[i].Stamp.ToString() );
					}
				} catch {
					Console.WriteLine( crit[0].ID + "\t" + crit[0].Name + "\t" + crit[0].Stamp.ToString() );
				}
			}
		}

		private void cmdPerform()
		{
			if ( m_transEnabled && m_trans != null ) {
				Console.WriteLine( DateTime.Now.ToLongTimeString() + ": Begin Transaction" );
				try {
					m_trans.Process();
				} catch ( Exception ex ) {
					Console.WriteLine( "Transaction Failed because of:\n" + ex.Message );
				}
				Console.WriteLine( DateTime.Now.ToLongTimeString() + ": End Transaction" );
			} else { Console.WriteLine( " No transaction exist or transactions is Off" ); }
		}

		private void cmdTrans()
		{
			m_transEnabled = (m_transEnabled ? false : true);
			if ( !m_transEnabled )
				m_trans = null;
			Console.WriteLine( DateTime.Now.ToLongTimeString() + ": Transaction Mode " + (m_transEnabled ? "On":"Off") );
		}

		private PersistentTransaction getTrans()
		{
			if ( m_trans == null )
				return new PersistentTransaction();
			else
				return m_trans;
		}

		private void cmdDelete()
		{
			string sWhere = "";

			Console.WriteLine( "Enter SQL Where condition ex.:(Name = 'test')" );
			sWhere = Console.ReadLine();
		
			if ( string.IsNullOrEmpty( sWhere ) ) {
				sWhere = "(Name like '%')";
			}
			
			if( m_trans == null ) {
				DeleteCriteria crit = new DeleteCriteria( (new TestObject()).Type, sWhere );
			
				crit.Perform();
				Console.WriteLine( "{0} : Delete request completed = {1}",DateTime.Now.ToLongTimeString(), crit.Count );
			} else {
				RetrieveCriteria rt = new RetrieveCriteria((new TestObject()).Type, sWhere );
				rt.Perform();
				m_trans.Add( rt, PersistentTransaction.ACTION.Delete );
				Console.WriteLine( "{0} : added to delete transaction = {1}", DateTime.Now.ToLongTimeString(), rt.Count );
			}
		}

		private void cmdAdd()
		{
			bool repeat = true;
			int count = 0;
			string path = "";
			string objName = "";
			PersistentObjects objs = new PersistentObjects();
			
			while ( repeat ) {
				Console.WriteLine( "Enter count to add:" );
				string str = Console.ReadLine();
				try {
					count = Convert.ToInt32( str );
					repeat = false;
				} catch { }
			}

			Console.WriteLine( "Enter object name:" );
			objName = Console.ReadLine();

			repeat = true;
			while ( repeat ) {
				Console.WriteLine( "Enter path to file:" );
				path = Console.ReadLine();
				if ( (string.IsNullOrEmpty( path )) || File.Exists( path ) )
					repeat = false;
			}

			Console.WriteLine( DateTime.Now.ToLongTimeString() + ": Starting to create objects " );
			TestObject prevObj = null;
			TestObject obj;
			Random rnd = new Random();
			
			for ( int i = 0; i < count; i++ ) {
				obj = new TestObject();
				obj.Name = (string.IsNullOrEmpty( objName ) ? i.ToString() : objName);
				obj._int = rnd.Next( 0, 100 );
				obj._bool = ( rnd.Next(0,1) == 1 ? true : false );
				obj._double = (Double) rnd.Next( 1, 10 ) / 0.33;
				obj._datetime = DateTime.Now;
				obj._string = "String for object" + obj._int;
				
				if ( !string.IsNullOrEmpty( path ) )
					obj._stream = new PersistentStream( path );
				if ( prevObj != null )
					obj.Parents.Add( prevObj );
				objs.Add( obj );
				prevObj = obj;
			}

			Console.WriteLine( DateTime.Now.ToLongTimeString() + ": Starting to SAVE objects " );

			if ( m_transEnabled ) {
				m_trans = getTrans();
				m_trans.Add( objs, PersistentTransaction.ACTION.Save );
				
				Console.WriteLine( DateTime.Now.ToLongTimeString() + ": Add request added to transaction" );
			} else {
				foreach ( PersistentObject cpobj in objs ) cpobj.Save();
				
				Console.WriteLine( DateTime.Now.ToLongTimeString() + ": End" );
				Console.WriteLine( "  " + objs.Count + " was added.\n" );
			}
		}

		private void cmdList()
		{
			Console.WriteLine( "Supported functions:" );
			
			foreach ( string item in m_listFuct ) {
				Console.WriteLine( "\t" + item );
			}
		}

		private PersistenceBroker broker_factory()
		{
			// create a unique identifier
			return new CrossDomainProxy<PersistenceBroker>(
							"tcp://" + m_hostname + ":8888/Toolkit.RPL.Test.Server.rem",
							"/310001/" + Guid.NewGuid().ToString());
		}
		
		private PersistentObject object_factory( string type,
									int id, DateTime stamp, string name )
		{
			Debug.Print( type + "\n" + id.ToString() + "\n" + name );
			
			// Get type of Object by it's string type.
			Type t = Type.GetType( type );

			// Create array of parametrs to path to constructor:
			// int id, DateTime stamp, String ^name
			object[] args = new object[] { id, stamp, name };

			// Creates an instance of the specified type using the
			// constructor that best matches the specified parameters. 
			return (PersistentObject) Activator.CreateInstance( t, args );
		}

		private void remotingActivation()
		{
			// RemotingConfiguration.CustomErrorsMode = CustomErrorsModes.Off;
			ChannelServices.RegisterChannel( new TcpChannel(), false );
			
			PersistenceBroker.BrokerFactory = new BROKER_FACTORY(broker_factory);
			PersistenceBroker.ObjectFactory = new OBJECT_FACTORY(object_factory);
		}
	}
}
