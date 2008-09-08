/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		Program.cs													*/
/*																			*/
/*	Content:	This is the main executable file.							*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008 Alexey Tkachuk								*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

using System;
using System.Runtime.Remoting.Channels;
using System.Runtime.Remoting.Channels.Tcp;
using Toolkit.Remoting;
using Toolkit.RPL.Storage;
using Toolkit.RPL.Factories;


namespace Toolkit.RPL.Test
{
	class Program
	{
		// define client disconnect timeout in minutes
		static int CLIENT_TIMEOUT = 30;
		// define server port for comunication
		static ushort SERVER_PORT = 8888;
		// connection string to SQL database
		static string CNN_STR = "Data Source=.;" +
								"Initial Catalog=Toolkit.RPL.Storage.ODB;" +
								"Persist Security Info=False;" +
								"Integrated Security=SSPI";

		/// <summary>
		/// This class is used to get reference to the PersistenceBroker class.
		/// </summary><remarks>
		/// We have to return MarshalByRef object from service factory, but
		/// PersistenceBroker has only static methods.
		///</remarks>
		private class Broker : PersistenceBroker
		{
			//
			// Default constructor. 
			// 
			private Broker()
			{
				// initialize PersistenceBroker (parent class was
				// created already, so it is posible access it)
				PersistenceBroker.Connect( new ODB( CNN_STR ) );
			}

			/// <summary>
			/// Represents the factory method that is used by CrossDomainMarshaler
			/// to create per client service.
			/// </summary>
			public static MarshalByRefObject CreateService( string clientID )
			{
				return new Broker();
			}
		};

		static void Main( string[] args )
		{
			try {
				// configure .NET Remoting subsystem.
				ChannelServices.RegisterChannel( new TcpServerChannel( SERVER_PORT ),
												 false );

				// create cross domain marshaler to manage client calls
				using( new CrossDomainMarshaler(
								"Toolkit.RPL.Test.Server.rem",
								new CrossDomainMarshaler.SERVICE_FACTORY(
										Broker.CreateService ),
								TimeSpan.FromMinutes( CLIENT_TIMEOUT ) ) ) {
					// and wait for user reques to unload
					Console.WriteLine( "Press ENTER to exit..." );
					Console.ReadLine();
				}
			} catch( Exception e ) {
				// write error message
				Console.WriteLine( e );
				Console.ReadLine();
			}
		}
	}
}
