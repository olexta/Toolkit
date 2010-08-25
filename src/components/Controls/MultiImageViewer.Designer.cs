namespace Toolkit.Controls
{
	partial class MultiImageViewer
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
			if( disposing && (components != null) ) {
				components.Dispose();
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager( typeof( MultiImageViewer ) );
			this.m_ToolScale = new System.Windows.Forms.ToolStripButton();
			this.m_PicBox = new System.Windows.Forms.PictureBox();
			this.m_Panel.SuspendLayout();
			((System.ComponentModel.ISupportInitialize) (this.m_PicBox)).BeginInit();
			this.SuspendLayout();
			// 
			// m_Tools
			// 
			this.m_Tools.Items.Insert( this.m_Tools.Items.Count, m_ToolScale );
			// 
			// m_Panel
			// 
			this.m_Panel.Controls.Add( this.m_PicBox );
			// 
			// m_ToolScale
			// 
			this.m_ToolScale.Checked = true;
			this.m_ToolScale.CheckOnClick = true;
			this.m_ToolScale.CheckState = System.Windows.Forms.CheckState.Checked;
			this.m_ToolScale.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			resources.ApplyResources( this.m_ToolScale, "m_ToolScale" );
			this.m_ToolScale.Name = "m_ToolScale";
			this.m_ToolScale.Click += new System.EventHandler( this.ToolScale_Click );
			this.m_ToolScale.CheckedChanged += new System.EventHandler( this.ToolScale_CheckedChanged );
			// 
			// m_PicBox
			// 
			resources.ApplyResources( this.m_PicBox, "m_PicBox" );
			this.m_PicBox.Name = "m_PicBox";
			this.m_PicBox.TabStop = false;
			this.m_PicBox.Click += new System.EventHandler( this.PicBox_Click );
			this.m_PicBox.MouseDown += new System.Windows.Forms.MouseEventHandler( this.PicBox_MouseDown );
			this.m_PicBox.MouseUp += new System.Windows.Forms.MouseEventHandler( this.PicBox_MouseUp );
			this.m_PicBox.MouseMove +=new System.Windows.Forms.MouseEventHandler(this.PicBox_MouseMove);
			// 
			// MultiImageViewer
			// 
			resources.ApplyResources( this, "$this" );
			this.Name = "MultiImageViewer";
			this.m_Panel.ResumeLayout( false );
			((System.ComponentModel.ISupportInitialize) (this.m_PicBox)).EndInit();
			this.ResumeLayout( false );
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.ToolStripButton m_ToolScale;
		private System.Windows.Forms.PictureBox m_PicBox;
	}
}
