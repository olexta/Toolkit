
namespace Toolkit.Controls.ImagesViewer.Test
{
	partial class TestForm
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

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager( typeof( TestForm ) );
			this.m_btnDir = new System.Windows.Forms.Button();
			this.m_Viewer = new Toolkit.Controls.ImagesViewer.ImagesViewer();
			this.SuspendLayout();
			// 
			// m_btnDir
			// 
			this.m_btnDir.Location = new System.Drawing.Point( 12, 12 );
			this.m_btnDir.Name = "m_btnDir";
			this.m_btnDir.Size = new System.Drawing.Size( 75, 23 );
			this.m_btnDir.TabIndex = 0;
			this.m_btnDir.Text = "Directory";
			this.m_btnDir.UseVisualStyleBackColor = true;
			this.m_btnDir.Click += new System.EventHandler( this.BtnDir_Click );
			// 
			// m_Viewer
			// 
			this.m_Viewer.Anchor = ( (System.Windows.Forms.AnchorStyles)( ( ( ( System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom )
						| System.Windows.Forms.AnchorStyles.Left )
						| System.Windows.Forms.AnchorStyles.Right ) ) );
			this.m_Viewer.DirectoryPath = null;
			this.m_Viewer.FileList = null;
			this.m_Viewer.ImageScaling = true;
			this.m_Viewer.Location = new System.Drawing.Point( 12, 41 );
			this.m_Viewer.Name = "m_Viewer";
			this.m_Viewer.Size = new System.Drawing.Size( 342, 338 );
			this.m_Viewer.StreamsList = ( (System.Collections.Generic.IEnumerable<System.IO.Stream>)( resources.GetObject( "m_Viewer.StreamsList" ) ) );
			this.m_Viewer.TabIndex = 1;
			// 
			// TestForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF( 6F, 13F );
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size( 366, 391 );
			this.Controls.Add( this.m_Viewer );
			this.Controls.Add( this.m_btnDir );
			this.Name = "TestForm";
			this.Text = "TestForm";
			this.ResumeLayout( false );

		}

		#endregion

		private System.Windows.Forms.Button m_btnDir;
		private ImagesViewer m_Viewer;

	}
}