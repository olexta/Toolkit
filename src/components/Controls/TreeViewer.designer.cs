espace Toolkit.Controls
{
	partial class TreeViewer
	{
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		System.ComponentModel.IContainer components = null;
		System.Windows.Forms.ToolStrip tsMain;


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
		void InitializeComponent()
		{
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager( typeof( TreeViewer ) );
			this.tsMain = new System.Windows.Forms.ToolStrip();
			this.tsMain.SuspendLayout();
			this.SuspendLayout();
			//
			// tsMain
			//
			this.tsMain.Location = new System.Drawing.Point( 0, 0 );
			this.tsMain.AutoSize = false;
			this.tsMain.Name = "tsMain";
			this.tsMain.Size = new System.Drawing.Size( 203, 0 );
			this.tsMain.TabIndex = 0;
			// 
			// BrowserControl
			// 
			this.Controls.Add( this.tsMain );
			this.Name = "BrowserControl";
			this.Size = new System.Drawing.Size( 203, DEFAULT_HEIGHT );
			this.tsMain.ResumeLayout( false );
			this.tsMain.PerformLayout();
			this.ResumeLayout( false );
			this.PerformLayout();

		}

		#endregion
	}
}
