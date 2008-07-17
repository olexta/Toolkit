using System;
using System.Drawing;
using System.Windows.Forms;
using Toolkit.Controls.TreeViewer;

namespace NBU.Line.GUI.Controls
{
	public partial class MainForm : Form
	{
		ImageList imageList;
		TreeViewer m_tw;

		public MainForm()
		{
			InitializeComponent();
			m_tw = new TreeViewer();
		}

		private void MainForm_Shown( object sender, EventArgs e )
		{

			TreeViewer.Tab tab = new  TreeViewer.Tab( "Test1" );
			tab.TreeView.Nodes.Add( "Test1" );
			tab.TreeView.Nodes[0].Nodes.Add( "Child" );
			m_tw.Tabs.Add( tab );

			tab = new TreeViewer.Tab( "Test2" );
			tab.TreeView.Nodes.Add( "Parent" );
			tab.TreeView.Nodes[0].Nodes.Add( "Child" );
			m_tw.Tabs.Add( tab );
			
			this.Controls.Add( m_tw );
		}

		private void cmdRemoveTab( object sender, EventArgs e )
		{
			if( m_tw.Tabs.Count > 0) {
				m_tw.Tabs.RemoveAt(0);
			}
		}

		private void cmdAdd( object sender, EventArgs e )
		{
			if( m_tw.SelectedTab != null ) {
				TreeNode tn = m_tw.SelectedTab.TreeView.Nodes[0].Nodes.Add( "Child" );
				tn.EnsureVisible();
			}
		}
	}
}