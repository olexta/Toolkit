namespace Toolkit.Controls.TreeViewer
{
	partial class TreeViewer
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

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager( typeof( TreeViewer ) );
			this.tsMain = new System.Windows.Forms.ToolStrip();
			this.tsMain.SuspendLayout();
			this.SuspendLayout();
			// 
			// tsMain
			// 
			this.tsMain.Location = new System.Drawing.Point( 0, 0 );
			this.tsMain.Name = "tsMain";
			this.tsMain.Size = new System.Drawing.Size( 203, 25 );
			this.tsMain.TabIndex = 0;
			// 
			// BrowserControl
			// 
			this.Controls.Add( this.tsMain );
			this.Name = "BrowserControl";
			this.Size = new System.Drawing.Size( 203, 315 );
			this.tsMain.ResumeLayout( false );
			this.tsMain.PerformLayout();
			this.ResumeLayout( false );
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.ToolStrip tsMain;
	}
}
