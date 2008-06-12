//----------------------------------------------------------------------
//
// SVN:			$Id: ImagesViewer.Designer.cs 270 2008-06-08 13:00:02Z N.Marunyak $
//
//----------------------------------------------------------------------

namespace Toolkit.Controls
{
	partial class ImagesViewer
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager( typeof( ImagesViewer ) );
			this.m_Tools = new System.Windows.Forms.ToolStrip();
			this.m_ToolPrev = new System.Windows.Forms.ToolStripButton();
			this.m_ToolNext = new System.Windows.Forms.ToolStripButton();
			this.m_ToolLblCount = new System.Windows.Forms.ToolStripLabel();
			this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
			this.m_ToolScale = new System.Windows.Forms.ToolStripButton();
			this.m_ToolLblInfo = new System.Windows.Forms.ToolStripLabel();
			this.m_Panel = new System.Windows.Forms.Panel();
			this.m_PicBox = new System.Windows.Forms.PictureBox();
			this.m_Tools.SuspendLayout();
			this.m_Panel.SuspendLayout();
			( (System.ComponentModel.ISupportInitialize)( this.m_PicBox ) ).BeginInit();
			this.SuspendLayout();
			// 
			// m_Tools
			// 
			this.m_Tools.Items.AddRange( new System.Windows.Forms.ToolStripItem[] {
            this.m_ToolPrev,
            this.m_ToolNext,
            this.m_ToolLblCount,
            this.toolStripSeparator1,
            this.m_ToolScale,
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
			// toolStripSeparator1
			// 
			this.toolStripSeparator1.Name = "toolStripSeparator1";
			resources.ApplyResources( this.toolStripSeparator1, "toolStripSeparator1" );
			// 
			// m_ToolScale
			// 
			this.m_ToolScale.CheckOnClick = true;
			this.m_ToolScale.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			resources.ApplyResources( this.m_ToolScale, "m_ToolScale" );
			this.m_ToolScale.Name = "m_ToolScale";
			this.m_ToolScale.Click += new System.EventHandler( this.ToolScale_Click );
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
			this.m_Panel.Controls.Add( this.m_PicBox );
			this.m_Panel.Name = "m_Panel";
			this.m_Panel.Click += new System.EventHandler( this.Panel_Click );			
			// 
			// m_PicBox
			// 
			resources.ApplyResources( this.m_PicBox, "m_PicBox" );
			this.m_PicBox.Name = "m_PicBox";
			this.m_PicBox.TabStop = false;
			this.m_PicBox.Click += new System.EventHandler( this.PicBox_Click );
			this.m_PicBox.MouseDown += new System.Windows.Forms.MouseEventHandler( this.PicBox_MouseDown );
			this.m_PicBox.MouseUp += new System.Windows.Forms.MouseEventHandler( this.PicBox_MouseUp );
			// 
			// ImagesViewer
			// 
			resources.ApplyResources( this, "$this" );
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add( this.m_Panel );
			this.Controls.Add( this.m_Tools );
			this.Name = "ImagesViewer";
			this.m_Tools.ResumeLayout( false );
			this.m_Tools.PerformLayout();
			this.m_Panel.ResumeLayout( false );
			( (System.ComponentModel.ISupportInitialize)( this.m_PicBox ) ).EndInit();
			this.ResumeLayout( false );
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.ToolStrip m_Tools;
		private System.Windows.Forms.ToolStripButton m_ToolPrev;
		private System.Windows.Forms.ToolStripButton m_ToolNext;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
		private System.Windows.Forms.ToolStripButton m_ToolScale;
		private System.Windows.Forms.ToolStripLabel m_ToolLblCount;
		private System.Windows.Forms.ToolStripLabel m_ToolLblInfo;
		private System.Windows.Forms.Panel m_Panel;
		private System.Windows.Forms.PictureBox m_PicBox;
	}
}
