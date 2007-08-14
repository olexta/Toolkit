using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Data.Common;
using RPL.Storage;
using System.IO;

namespace RPL.Test
{
	[ TestClass() ]
	public class ODBLoadTest
	{
		private static string m_obj_name = "VS_Load_Test";
		private TestContext testContextInstance;

		/// <summary>
		///Gets or sets the test context which provides
		///information about and functionality for the current test run.
		///</summary>
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
		//You can use the following additional attributes as you write your tests:
		//

		//Use ClassInitialize to run code before running the first test in the class

		[ClassInitialize()]
		public static void MyClassInitialize( TestContext testContext )
		{
			DbProviderFactory prov = DbProviderFactories.GetFactory( "System.Data.SqlClient" );
			DbConnection con = prov.CreateConnection();
			con.ConnectionString = "Data Source=CORP;Initial Catalog=LineNBU;Persist Security Info=True;User ID=sa;Password=12345";
			ODB adodb = new ODB();
			RPL.CPersistenceBroker.Instance.Connect( con, adodb );
		}

		//Use ClassCleanup to run code after all tests in a class have run

		[ClassCleanup()]
		public static void MyClassCleanup()
		{
			RPL.CPersistenceBroker.Instance.Dispose();
		}

		//Use TestInitialize to run code before running each test
		//
		//[TestInitialize()]
		//public void MyTestInitialize()
		//{
		//}
		//
		//Use TestCleanup to run code after each test has run
		//
		//[TestCleanup()]
		//public void MyTestCleanup()
		//{
		//}
		//
		#endregion


		///<summary></summary>
		[Priority( 5 ), TestMethod()]
		public void Add_1LoadTest()
		{
			for ( int i = 0; i < 20; i++ ) {
				CTestObject t_obj = new CTestObject();
				t_obj._string = m_obj_name;
				t_obj._int = 111;
				t_obj._stream = new FileStream( @"c:\ntldr", FileMode.Open, FileAccess.Read, FileShare.Read );
				t_obj.Save();
			}
		}

		///<summary>Same as Add_1LoadTest, but doesn't save imege properties</summary>
		[Priority( 6 ), TestMethod()]
		public void Add_2LoadTest()
		{
			for ( int i = 0; i < 20; i++ ) {
				CTestObject t_obj = new CTestObject();
				t_obj._string = m_obj_name;
				t_obj._int = 111;
				t_obj.Save();
			}
		}
		
		///<summary></summary>
		[Priority( 4 ), TestMethod()]
		public void AddTransLoadTest()
		{
			CPersistentTransaction trans = new CPersistentTransaction();
			for ( int i = 0; i < 20; i++ ) {
				CTestObject t_obj = new CTestObject();
				t_obj._int = 111;
				t_obj._stream = new FileStream( @"c:\ntldr", FileMode.Open, FileAccess.Read , FileShare.Read);
				trans.Add( t_obj, CPersistentTransaction.Actions.actSave );
			}
			trans.Process();
		}

		///<summary></summary>
		[Priority( 8 ), TestMethod()]
		public void Search_1LoadTest() {
			CRetrieveCriteria ret_crit = new CRetrieveCriteria(CTestObject.type(), "( Name = '" + m_obj_name + "')");
			ret_crit.AsProxies = true;
			ret_crit.Perform();
		}

		///<summary>Same as Search_1LoadTest but loads all props</summary>
		[Priority( 7 ), TestMethod()]
		public void Search_2LoadTest() {
			CRetrieveCriteria ret_crit = new CRetrieveCriteria(CTestObject.type(), "( Name = '" + m_obj_name + "')");
			ret_crit.AsProxies = false;
			ret_crit.Perform();
		}

		///<summary></summary>
		[Priority( 9 ), TestMethod()]
		public void DeleteLoadTest() {
			CDeleteCriteria del_crit = new CDeleteCriteria(CTestObject.type(), "( Name = '" + m_obj_name + "')");
			del_crit.Perform();
		}
	}
}