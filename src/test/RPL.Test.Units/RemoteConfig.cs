using System;
using System.Runtime.Remoting.Messaging;
using Toolkit.RPL.Factories;
using System.Runtime.Remoting.Channels;
using System.Runtime.Remoting.Proxies;
using System.Runtime.Remoting.Channels.Tcp;
using System.Diagnostics;

namespace Toolkit.RPL.Test
{
	class RemotingConfiguration
	{
		private string	m_hostname = "localhost";
		
		private RemotingConfiguration() {
			remotingActivation();
		}
		
		public static RemotingConfiguration Instance
		{
			get
			{
				return Nested.instance;
			}
		}

		class Nested
		{
			// Explicit static constructor to tell C# compiler
			// not to mark type as beforefieldinit
			static Nested()
			{
			}

			internal static readonly RemotingConfiguration instance = new RemotingConfiguration();
		}
		
		
		private class Factories : IBrokerFactory, IObjectFactory
		{
			private static string m_uri;

			private class CustomProxy : RealProxy
			{
				private static String m_uri;
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
				public CustomProxy( String uri, Type type )
					: base( type )
				{
					// store uri of remote service
					m_uri = uri;

					// pass through registered chanels
					foreach( IChannel channel in ChannelServices.RegisteredChannels ) {
						// and try to find chanel, that can be used to
						// create message sink for given uri
						IChannelSender sender = (IChannelSender) channel;
						if( sender != null ) {
							// check for messaging protocol
							if( sender.ChannelName == "tcp" ) {
								// try to create message sink for specified uri
								string objectUri = string.Empty;
								m_messageSink = sender.CreateMessageSink( m_uri, null, out objectUri );
								// all right, can return
								if( m_messageSink != null )
									break;
							}
						}
					}
					if( m_messageSink == null )
						throw new Exception( "No channel found for " + m_uri );

					// create a unique identifier
					m_clientID = "/310001/" + Guid.NewGuid().ToString();
				}
			}

			PersistenceBroker IBrokerFactory.CreateInstance()
			{
				return (PersistenceBroker) (new CustomProxy(
						m_uri, typeof( PersistenceBroker ) )).GetTransparentProxy();
			}

			PersistentObject IObjectFactory.CreateInstance(
				string type, int id, DateTime stamp, string name )
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

			public Factories( string uri ) { m_uri = uri; }
		}

		private void remotingActivation()
		{
			ChannelServices.RegisterChannel( new TcpChannel(), false );

			Factories factories = new Factories( "tcp://" + m_hostname + ":8888/Toolkit.RPL.Test.Server.rem" );

			PersistenceBroker.BrokerFactory = factories;
			PersistenceBroker.ObjectFactory = factories;
		}
	}
}
