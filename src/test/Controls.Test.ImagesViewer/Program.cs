using System;
using System.Windows.Forms;
using System.Threading;

namespace Toolkit.Controls.Test
{
	class Program
	{
		[STAThread]
		static void Main()
		{
			Thread.CurrentThread.CurrentUICulture = new System.Globalization.CultureInfo( "uk-UA" );
			Application.Run( new TestForm() );
		}
	}
}
