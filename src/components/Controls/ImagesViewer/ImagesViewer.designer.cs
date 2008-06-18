//----------------------------------------------------------------------
//
// SVN:			$Id: ImagesViewer.Designer.cs 270 2008-06-08 13:00:02Z N.Marunyak $
//
//----------------------------------------------------------------------

namespace Toolkit.Controls.ImagesViewer
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
			this.m_ToolSeparator = new System.Windows.Forms.ToolStripSeparator();
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
			this.m_Tools.AccessibleDescription = null;
			this.m_Tools.AccessibleName = null;
			resources.ApplyResources( this.m_Tools, "m_Tools" );
			this.m_Tools.BackgroundImage = null;
			this.m_Tools.Font = null;
			this.m_Tools.GripStyle = System.Windows.Forms.ToolStripGripStyle.Hidden;
			this.m_Tools.Items.AddRange( new System.Windows.Forms.ToolStripItem[] {
            this.m_ToolPrev,
            this.m_ToolNext,
            this.m_ToolLblCount,
            this.m_ToolSeparator,
            this.m_ToolScale,
            this.m_ToolLblInfo} );
			this.m_Tools.Name = "m_Tools";
			// 
			// m_ToolPrev
			// 
			this.m_ToolPrev.AccessibleDescription = null;
			this.m_ToolPrev.AccessibleName = null;
			resources.ApplyResources( this.m_ToolPrev, "m_ToolPrev" );
			this.m_ToolPrev.BackgroundImage = null;
			this.m_ToolPrev.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.m_ToolPrev.Name = "m_ToolPrev";
			this.m_ToolPrev.Click += new System.EventHandler( this.ToolPrev_Click );
			// 
			// m_ToolNext
			// 
			this.m_ToolNext.AccessibleDescription = null;
			this.m_ToolNext.AccessibleName = null;
			resources.ApplyResources( this.m_ToolNext, "m_ToolNext" );
			this.m_ToolNext.BackgroundImage = null;
			this.m_ToolNext.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.m_ToolNext.Name = "m_ToolNext";
			this.m_ToolNext.Click += new System.EventHandler( this.ToolNext_Click );
			// 
			// m_ToolLblCount
			// 
			this.m_ToolLblCount.AccessibleDescription = null;
			this.m_ToolLblCount.AccessibleName = null;
			resources.ApplyResources( this.m_ToolLblCount, "m_ToolLblCount" );
			this.m_ToolLblCount.BackgroundImage = null;
			this.m_ToolLblCount.Name = "m_ToolLblCount";
			// 
			// m_ToolSeparator
			// 
			this.m_ToolSeparator.AccessibleDescription = null;
			this.m_ToolSeparator.AccessibleName = null;
			resources.ApplyResources( this.m_ToolSeparator, "m_ToolSeparator" );
			this.m_ToolSeparator.Name = "m_ToolSeparator";
			// 
			// m_ToolScale
			// 
			this.m_ToolScale.AccessibleDescription = null;
			this.m_ToolScale.AccessibleName = null;
			resources.ApplyResources( this.m_ToolScale, "m_ToolScale" );
			this.m_ToolScale.BackgroundImage = null;
			this.m_ToolScale.CheckOnClick = true;
			this.m_ToolScale.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.m_ToolScale.Name = "m_ToolScale";
			this.m_ToolScale.Click += new System.EventHandler( this.ToolScale_Click );
			// 
			// m_ToolLblInfo
			// 
			this.m_ToolLblInfo.AccessibleDescription = null;
			this.m_ToolLblInfo.AccessibleName = null;
			this.m_ToolLblInfo.Alignment = System.Windows.Forms.ToolStripItemAlignment.Right;
			resources.ApplyResources( this.m_ToolLblInfo, "m_ToolLblInfo" );
			this.m_ToolLblInfo.BackgroundImage = null;
			this.m_ToolLblInfo.Name = "m_ToolLblInfo";
			// 
			// m_Panel
			// 
			this.m_Panel.AccessibleDescription = null;
			this.m_Panel.AccessibleName = null;
			resources.ApplyResources( this.m_Panel, "m_Panel" );
			this.m_Panel.BackgroundImage = null;
			this.m_Panel.Controls.Add( this.m_PicBox );
			this.m_Panel.Font = null;
			this.m_Panel.Name = "m_Panel";
			this.m_Panel.Click += new System.EventHandler( this.Panel_Click );
			// 
			// m_PicBox
			// 
			this.m_PicBox.AccessibleDescription = null;
			this.m_PicBox.AccessibleName = null;
			resources.ApplyResources( this.m_PicBox, "m_PicBox" );
			this.m_PicBox.BackgroundImage = null;
			this.m_PicBox.Font = null;
			this.m_PicBox.ImageLocation = null;
			this.m_PicBox.Name = "m_PicBox";
			this.m_PicBox.TabStop = false;
			this.m_PicBox.Click += new System.EventHandler( this.PicBox_Click );
			this.m_PicBox.MouseDown += new System.Windows.Forms.MouseEventHandler( this.PicBox_MouseDown );
			this.m_PicBox.MouseUp += new System.Windows.Forms.MouseEventHandler( this.PicBox_MouseUp );
			// 
			// ImagesViewer
			// 
			this.AccessibleDescription = null;
			this.AccessibleName = null;
			resources.ApplyResources( this, "$this" );
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackgroundImage = null;
			this.Controls.Add( this.m_Panel );
			this.Controls.Add( this.m_Tools );
			this.Font = null;
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
		private System.Windows.Forms.ToolStripSeparator m_ToolSeparator;
		private System.Windows.Forms.ToolStripButton m_ToolScale;
		private System.Windows.Forms.ToolStripLabel m_ToolLblCount;
		private System.Windows.Forms.ToolStripLabel m_ToolLblInfo;
		private System.Windows.Forms.Panel m_Panel;
		private System.Windows.Forms.PictureBox m_PicBox;
	}
}
