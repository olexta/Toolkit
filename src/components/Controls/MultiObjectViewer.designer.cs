---------------------------------------------------------------------
//
// SVN:			$Id: ImagesViewer.Designer.cs 270 2008-06-08 13:00:02Z N.Marunyak $
//
//----------------------------------------------------------------------

namespace Toolkit.Controls
{
	public partial class MultiObjectViewerBase
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose( bool disposing )
		{
			if( disposing && ( components != null ) ) {
				components.Dispose();
			}
			base.Dispose( disposing );
		}

		#region Component Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager( typeof( MultiObjectViewerBase ) );
			this.m_Tools = new System.Windows.Forms.ToolStrip();
			this.m_ToolPrev = new System.Windows.Forms.ToolStripButton();
			this.m_ToolNext = new System.Windows.Forms.ToolStripButton();
			this.m_ToolLblCount = new System.Windows.Forms.ToolStripLabel();
			this.m_ToolSeparator = new System.Windows.Forms.ToolStripSeparator();
			this.m_ToolLblInfo = new System.Windows.Forms.ToolStripLabel();
			this.m_Panel = new System.Windows.Forms.Panel();
			this.m_Tools.SuspendLayout();
			this.SuspendLayout();
			// 
			// m_Tools
			// 
			this.m_Tools.GripStyle = System.Windows.Forms.ToolStripGripStyle.Hidden;
			this.m_Tools.Items.AddRange( new System.Windows.Forms.ToolStripItem[] {
            this.m_ToolPrev,
            this.m_ToolNext,
            this.m_ToolLblCount,
            this.m_ToolSeparator,
            this.m_ToolLblInfo} );
			resources.ApplyResources( this.m_Tools, "m_Tools" );
			this.m_Tools.Name = "m_Tools";
			// 
			// m_ToolPrev
			// 
			this.m_ToolPrev.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			resources.ApplyResources( this.m_ToolPrev, "m_ToolPrev" );
			this.m_ToolPrev.Name = "m_ToolPrev";
			this.m_ToolPrev.Click += new System.EventHandler( this.ToolPrev_Click );
			// 
			// m_ToolNext
			// 
			this.m_ToolNext.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			resources.ApplyResources( this.m_ToolNext, "m_ToolNext" );
			this.m_ToolNext.Name = "m_ToolNext";
			this.m_ToolNext.Click += new System.EventHandler( this.ToolNext_Click );
			// 
			// m_ToolLblCount
			// 
			this.m_ToolLblCount.Name = "m_ToolLblCount";
			resources.ApplyResources( this.m_ToolLblCount, "m_ToolLblCount" );
			// 
			// m_ToolSeparator
			// 
			this.m_ToolSeparator.Name = "m_ToolSeparator";
			resources.ApplyResources( this.m_ToolSeparator, "m_ToolSeparator" );
			// 
			// m_ToolLblInfo
			// 
			this.m_ToolLblInfo.Alignment = System.Windows.Forms.ToolStripItemAlignment.Right;
			this.m_ToolLblInfo.Name = "m_ToolLblInfo";
			resources.ApplyResources( this.m_ToolLblInfo, "m_ToolLblInfo" );
			// 
			// m_Panel
			// 
			resources.ApplyResources( this.m_Panel, "m_Panel" );
			this.m_Panel.Name = "m_Panel";
			this.m_Panel.Click += new System.EventHandler( this.Panel_Click );
			// 
			// MultiObjectViewer
			// 
			resources.ApplyResources( this, "$this" );
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add( this.m_Panel );
			this.Controls.Add( this.m_Tools );
			this.Name = "MultiObjectViewer";
			this.m_Tools.ResumeLayout( false );
			this.m_Tools.PerformLayout();
			this.ResumeLayout( false );
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.ToolStripButton m_ToolPrev;
		private System.Windows.Forms.ToolStripButton m_ToolNext;
		private System.Windows.Forms.ToolStripSeparator m_ToolSeparator;
		private System.Windows.Forms.ToolStripLabel m_ToolLblCount;
		/// <summary>
		/// Панель інструментів.
		/// </summary>
		protected System.Windows.Forms.ToolStrip m_Tools;
		/// <summary>
		/// Мітка панелі інструментів.
		/// </summary><remarks>
		/// Мітка розташована справа на панелі інструментів.
		/// </remarks>
		protected System.Windows.Forms.ToolStripLabel m_ToolLblInfo;
		/// <summary>
		/// Панель для відображення об'єктів.
		/// </summary>
		protected System.Windows.Forms.Panel m_Panel;
	}
}
