using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.IO;
using System.Windows.Forms;
using Toolkit.Controls;

namespace Toolkit.Controls.Test
{
	public partial class TestForm : Form
	{
		List<string> m_List = new List<string>();

		public TestForm()
		{
			InitializeComponent();
		}

		private void m_Viewer_ShowObject( object sender, MultiImageViewer.ShowObjectEventArgs e )
		{
			if( m_List.Count > e.Index )
				e.Object = Image.FromFile( m_List[e.Index] );
		}

		private void button1_Click( object sender, EventArgs e )
		{
			FolderBrowserDialog dlg = new FolderBrowserDialog();
			if( dlg.ShowDialog() == DialogResult.OK ) {
				m_List.Clear();
				foreach( string path in Directory.GetFiles( dlg.SelectedPath, "*.*" ) ) {
					m_List.Add( path );
				}
				m_Viewer.Count = m_List.Count;
			}
		}

		private void numericUpDown1_ValueChanged( object sender, EventArgs e )
		{
			m_Viewer.Count = (int)numericUpDown1.Value;
		}
	}
}
