using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using Toolkit.Controls;

namespace Toolkit.Controls.ImagesViewer.Test
{
	public partial class TestForm : Form
	{
		public TestForm()
		{
			InitializeComponent();
		}

		private void BtnDir_Click( object sender, EventArgs e )
		{
			FolderBrowserDialog dlg = new FolderBrowserDialog();
			if( dlg.ShowDialog() == DialogResult.OK )
				m_Viewer.DirectoryPath = dlg.SelectedPath;
		}
	}
}