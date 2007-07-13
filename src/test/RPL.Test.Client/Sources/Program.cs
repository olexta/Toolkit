using System;
using System.Collections.Generic;
using System.Text;
using System.Data.Common;
using System.IO;
using System.Text.RegularExpressions;
using System.Data.SqlClient;
using System.Data;
using System.Runtime.Remoting.Channels;
using System.Runtime.Remoting;
using System.Runtime.Remoting.Messaging;
using System.Runtime.Remoting.Proxies;
using System.Net;
using System.Collections;
using System.Runtime.Remoting.Channels.Tcp;
using System.Diagnostics;
using RPL;
using RPL.Storage;

namespace RPL.Test
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
		static string[] m_listFuct = new string[] { "add - adds object(s), with or without file attachment",
													"delete - deletes object(s)", 
													"update - updates object(s) property(ies)",
													"search - searches saved object(s)",
													"trans - switches transaction mode", 
													"perform - performes created transaction", 
													"error - adds error command to transaction (used for transaction rollback test)", 
													"exit - quits application" };
		private bool m_transEnabled = false;
		private RPL.CPersistentTransaction m_trans = null;
		private string m_hostname;

		public ConsoleParser()
		{
			string str;
			bool needExit = false;
			Console.WriteLine( "\tPress 'l' key for on-server object creation, \nelse .Net Remoting will be used" );
			if ( Console.ReadLine() == "l" )
				LocalConnect();
			else {
				Console.WriteLine( "Enter host name" );
				m_hostname = Console.ReadLine();
				if ( m_hostname == string.Empty ) m_hostname = "localhost";
				remotingActivation(); Console.WriteLine( "Remoting activated." ); 
			}
			while ( !needExit ) {
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
					case "update":
						cmdUpdate();
						break;
					case "search":
						cmdSearch();
						break;
					case "error":
						cmdAddTransError();
						break;
					case "test":
						cmdTest();
						break;
					default:
						cmdList();
						break;
				}
			}
			try {
				CPersistenceBroker.Instance.Dispose();
			} catch ( Exception ex ) {
				Debug.Print( "Error occured: {0}", ex.ToString() );
				throw ex;
			}
		}

		private void cmdTest()
		{
			CTestObject obj = new CTestObject();
			obj.Name= "link_obj";
			obj.Save();

			CTestObject obj_1 = new CTestObject();
			obj_1.Name= "Obj_1";
			obj_1.AddParent( obj );
			obj_1.Save();
		}

		private void cmdAddTransError()
		{
			if ( m_transEnabled ) {
				m_trans = getTrans();
				m_trans.Add( new CRetrieveCriteria( CTestObject.type(), "FAIL :)" ) );
				Console.WriteLine( DateTime.Now.ToLongTimeString() + ": Corrupted criteria added to transaction" );
			} else {
				Console.WriteLine( "No transaction is active!" );
			}

		}

		private void cmdSearch()
		{
			string sWhere = string.Empty;
			string sOrderBy = string.Empty;
			int sCount = 100;
			int sBottom = 0;
			bool sProxy = true;
			CRetrieveCriteria crit;

			Console.WriteLine( "Enter SQL Where condition ex.:(Name = 'test')" );
			sWhere = Console.ReadLine();
			if ( string.IsNullOrEmpty( sWhere ) ) {
				sWhere = "(1 = 1)";
				Console.WriteLine( "Search Condition assumed as :" + sWhere );
			}
			//get Order request from user input
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
			//get bottom count from user input
			Console.WriteLine( "Bottom count:" );
			try {
				sBottom = Convert.ToInt32( Console.ReadLine() );
			} catch { } finally {
				Console.WriteLine( "Bottom is: {0}", sBottom );
			}
			//do retrive proxy or not objects
			Console.WriteLine( "Retrive proxy objects? (Y/N)" );
			if ( string.Compare( Console.ReadLine(), "Y", true ) != 0 ) {
				sProxy = false;
				Console.WriteLine( "\tProxy switch is: {0}", sProxy );
			}

			crit = new CRetrieveCriteria( CTestObject.type(), sWhere, sOrderBy );
			crit.CountLimit = sCount;
			crit.BottomLimit = sBottom;
			crit.AsProxies = sProxy;

			if ( m_transEnabled ) {
				m_trans = getTrans();
				m_trans.Add( crit );
				Console.WriteLine( DateTime.Now.ToLongTimeString() + ": Search request added to transaction" );
			} else {
				Console.WriteLine( DateTime.Now.ToLongTimeString() + ": Search request begin" );
				crit.Perform();
				Console.WriteLine( DateTime.Now.ToLongTimeString() + ": Search request completed" );
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
		}

		private void cmdUpdate()
		{
			bool repeat = true;
			bool invoke = true;
			string sWhere = string.Empty;
			MatchCollection mc = null;

			Console.WriteLine( "Enter SQL Where condition ex.:(Name = 'test')" );
			sWhere = Console.ReadLine();
			if ( string.IsNullOrEmpty( sWhere ) ) {
				sWhere = "1 = 1";
			}

			while ( repeat ) {
				Console.WriteLine( "Specifies columns that are updated and their new values\n(Sql: Column_Name1 = Expression1[, Column_Name2 = Expression2 )" );
				string str = Console.ReadLine();
				if ( str == "break" ) { repeat = false; invoke = false; Console.WriteLine( "Operation canceled." ); } else {
					mc = Regex.Matches( str, @"\s*(?<prop>[A-z_0-9]+)\s*\=\s*(?<value>[A-z_0-9']+)\,*" );
					if ( mc.Count > 0 ) {
						repeat = false;
					} else
						Console.WriteLine( "Wrong format!" );
				}
			}

			if ( invoke ) {
				CUpdateCriteria crit = new CUpdateCriteria( CTestObject.type(), sWhere );
				foreach ( Match m in mc ) {
					CPersistentProperty prop = 
					new CPersistentProperty( m.Groups["prop"].Value );
					prop.Value = m.Groups["value"].Value;
					crit.Properties.Add( prop );
				}

				if ( m_transEnabled ) {
					m_trans = getTrans();
					m_trans.Add( crit );
					Console.WriteLine( DateTime.Now.ToLongTimeString() + ": Update request added to transaction" );
				} else { crit.Perform(); Console.WriteLine( DateTime.Now.ToLongTimeString() + ": Update request completed" ); }
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

		private CPersistentTransaction getTrans()
		{
			if ( m_trans == null )
				return new CPersistentTransaction();
			else
				return m_trans;
		}

		private void cmdDelete()
		{
			string sWhere = string.Empty;

			Console.WriteLine( "Enter SQL Where condition ex.:(Name = 'test')" );
			sWhere = Console.ReadLine();
			if ( string.IsNullOrEmpty( sWhere ) ) {
				sWhere = "(1=1)";
			}
			CDeleteCriteria crit = new CDeleteCriteria( CTestObject.type(), sWhere );

			if ( m_transEnabled ) {
				m_trans = getTrans();
				m_trans.Add( crit );
				Console.WriteLine( DateTime.Now.ToLongTimeString() + ": Delete request added to transaction" );
			} else { crit.Perform(); Console.WriteLine( DateTime.Now.ToLongTimeString() + ": Delete request completed" ); }
		}

		private void cmdAdd()
		{
			CPersistentObjects objs = new CPersistentObjects();
			bool repeat = true;
			int count = 0;
			string path = string.Empty;
			string objName = string.Empty;

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
			CTestObject prevObj = null;
			CTestObject obj;
			Random rnd = new Random();
			for ( int i = 0; i < count; i++ ) {
				obj = new CTestObject();
				obj.Name = (string.IsNullOrEmpty( objName ) ? i.ToString() : objName);
				obj._int = rnd.Next( 0, 100 );
				obj._boolean = ( rnd.Next(0,1) == 1 ? true : false );
				obj._datetime = DateTime.Now;
				obj._string = "String for object" + obj._int;
				if ( !string.IsNullOrEmpty( path ) )
					obj._stream = new FileStream( path, FileMode.Open, FileAccess.Read, FileShare.Read );
				if ( prevObj != null )
					obj.AddParent( prevObj );
				objs.Add( obj );
				prevObj = obj;
			}

			Console.WriteLine( DateTime.Now.ToLongTimeString() + ": Starting to SAVE objects " );

			if ( m_transEnabled ) {
				m_trans = getTrans();
				foreach ( CPersistentObject cpobj in objs ) {
					m_trans.Add( cpobj, CPersistentTransaction.Actions.actSave );
				}
				Console.WriteLine( DateTime.Now.ToLongTimeString() + ": Add request added to transaction" );
			} else {
				foreach ( CPersistentObject cpobj in objs ) {
					cpobj.Save();
				}
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

		private void remotingActivation()
		{
			int m_port = freePort;
			remoting_config( m_port );
			CPersistenceBroker.Factory = new CFactory( "tcp://" + m_hostname + ":43667/RPL.Server.rem", m_port );
		}

		private void LocalConnect()
		{
			DbProviderFactory prov = DbProviderFactories.GetFactory( "System.Data.SqlClient" );
			DbConnection con = prov.CreateConnection();
			con.ConnectionString = "Data Source=CORP;Initial Catalog=LineNBU;Persist Security Info=True;User ID=sa;Password=12345";
			ODB adodb = new ODB();

			CPersistenceBroker.Instance.Connect( con, adodb );
			Console.WriteLine( "Connected to :" );
			Console.WriteLine( "  " + con.ConnectionString );
		}

		private class CFactory : IBrokerFactory
		{
			private string m_uri;
			private int m_port;


			public CFactory( String uri, int port ) { m_uri = uri; m_port = port; }

			CPersistenceBroker IBrokerFactory.CreateInstance()
			{
				return (CPersistenceBroker)(new CustomProxy(
						m_uri, typeof( CPersistenceBroker ), m_port )).GetTransparentProxy();
			}

			private class CustomProxy : RealProxy
			{
				private String m_uri;
				private String m_clientID;
				private IMessageSink m_messageSink;

				public override IMessage Invoke( IMessage msg )
				{
					msg.Properties["__Uri"] = m_uri;

					// pass the client's unique identifier as part of the call context
					LogicalCallContext callContext = (LogicalCallContext)
												(msg.Properties["__CallContext"]);
					callContext.SetData( "__ClientID", m_clientID );

					return m_messageSink.SyncProcessMessage( msg );
				}
				public CustomProxy( String uri, Type type, int port )
					: base( type )
				{
					// store uri of remote service
					m_uri = uri;

					// pass through registered chanels
					foreach ( IChannel channel in ChannelServices.RegisteredChannels ) {
						// and try to find chanel, that can be used to
						// create message sink for given uri
						IChannelSender sender = (IChannelSender)channel;
						if ( sender != null ) {
							// check for messaging protocol
							if ( sender.ChannelName == "tcp" ) {
								// try to create message sink for specified uri
								string objectUri = string.Empty;
								m_messageSink = sender.CreateMessageSink( m_uri, null, out objectUri );
								// all right, can return
								if ( m_messageSink != null )
									break;
							}
						}
					}
					if ( m_messageSink == null )
						throw new Exception( "No channel found for " + m_uri );

					// create a unique identifier
					// здесь необходимо получить IP и порт "серверной" части
					// этого экземпл€ра клиента (делаетс€ это через сторонний
					// компониент, который создает мост клиент-сервер)
					m_clientID = "tcp://" + Dns.GetHostName() + ":" + port + 
			              "/" + "310001" + Guid.NewGuid().ToString();
				}
			}
		}

		private static void remoting_config( int port )
		{
			RemotingConfiguration.RegisterActivatedServiceType( typeof( CTestObject ) );
			RemotingConfiguration.RegisterActivatedServiceType( typeof( SqlStream ) );
			RemotingConfiguration.CustomErrorsMode = CustomErrorsModes.Off;

			// activate client chanel
			BinaryServerFormatterSinkProvider srvProv = 
			new BinaryServerFormatterSinkProvider();
			srvProv.TypeFilterLevel = System.Runtime.Serialization.Formatters.TypeFilterLevel.Full;
			BinaryClientFormatterSinkProvider cliProv =
		    new BinaryClientFormatterSinkProvider();

			System.Collections.IDictionary props = 
						new System.Collections.Hashtable();
			props["port"] = port;
			// register TCP chanel
			System.Runtime.Remoting.Channels.Tcp.TcpChannel chan = 
						new System.Runtime.Remoting.Channels.Tcp.TcpChannel( props, cliProv, srvProv );
			ChannelServices.RegisterChannel( chan, false );
		}

		private static int freePort
		{
			get
			{
				for ( int i = 1024; i < 65535; i++ ) {
					try {
						IDictionary _props = 
							new Hashtable();
						_props["port"] = i;

						// register TCP chanel
						TcpChannel _chan = 
						new TcpChannel( i );
						ChannelServices.RegisterChannel( _chan, false );
						ChannelServices.UnregisterChannel( _chan );
						return i;
					} catch { }
				}
				throw new RemotingException( "No free ports?!" );
			}
		}
	}
}
