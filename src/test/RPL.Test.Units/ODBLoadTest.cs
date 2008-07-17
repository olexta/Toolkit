using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using Toolkit.RPL.Factories;
using System.Runtime.Remoting;

namespace Toolkit.RPL.Test
{
	[ TestClass() ]
	public class ODBLoadTest
	{
		private static string m_obj_name = "VS_Load_Test";
		private TestContext testContextInstance;

		/// <summary>
		/// Gets or sets the test context which provides
		/// information about and functionality for the current test run.
		/// </summary>
		public TestContext TestContext
		{
			get
			{
				return testContextInstance;
			}
			set
			{
				testContextInstance = value;
			}
		}
		#region Additional test attributes
		// 
		// You can use the following additional attributes as you write your tests:
		//

		// Use ClassInitialize to run code before running the first test in the class

		[ClassInitialize()]
		public static void MyClassInitialize( TestContext testContext )
		{
			RemotingConfiguration rc = RemotingConfiguration.Instance;
		}

		// Use ClassCleanup to run code after all tests in a class have run

		[ClassCleanup()]
		public static void MyClassCleanup()
		{
			PersistenceBroker.Close();
		}
		#endregion

		[Priority( 5 ), TestMethod()]
		public void Add_1LoadTest()
		{
			for ( int i = 0; i < 20; i++ ) {
				TestObject t_obj = new TestObject();
				t_obj.Name = m_obj_name + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff");
				t_obj._string = m_obj_name;
				t_obj._int = 111;
				t_obj._stream = new PersistentStream( @"c:\ntldr" );
				t_obj.Save();
			}
		}

		/// <summary>
		/// Same as Add_1LoadTest, but doesn't save image properties
		/// </summary>
		[Priority( 6 ), TestMethod()]
		public void Add_2LoadTest()
		{
			for ( int i = 0; i < 20; i++ ) {
				TestObject t_obj = new TestObject();
				t_obj.Name = m_obj_name + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff");
				t_obj._string = m_obj_name + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff");
				t_obj._int = 111;
				t_obj.Save();
			}
		}
		
		[Priority( 4 ), TestMethod()]
		public void AddTransLoadTest()
		{
			PersistentTransaction trans = new PersistentTransaction();
			for ( int i = 0; i < 20; i++ ) {
				TestObject t_obj = new TestObject();
				t_obj.Name = m_obj_name + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff");
				t_obj._int = 111;
				t_obj._stream = new PersistentStream( @"c:\ntldr" );
				trans.Add( t_obj, PersistentTransaction.ACTION.Save );
			}
			trans.Process();
		}

		[Priority( 8 ), TestMethod()]
		public void Search_1LoadTest() {
			RetrieveCriteria ret_crit = new RetrieveCriteria((new TestObject()).Type, "( Name = '" + m_obj_name + "')");
			ret_crit.AsProxies = true;
			ret_crit.Perform();
		}

		/// <summary>
		/// Same as Search_1LoadTest but loads all props
		/// </summary>
		[Priority( 7 ), TestMethod()]
		public void Search_2LoadTest() {
			RetrieveCriteria ret_crit = new RetrieveCriteria((new TestObject()).Type, "( Name = '" + m_obj_name + "')");
			ret_crit.AsProxies = false;
			ret_crit.Perform();
		}

		[Priority( 9 ), TestMethod()]
		public void DeleteLoadTest() {
			DeleteCriteria del_crit = new DeleteCriteria((new TestObject()).Type, "( Name = '" + m_obj_name + "')");
			del_crit.Perform();
		}
	}
}