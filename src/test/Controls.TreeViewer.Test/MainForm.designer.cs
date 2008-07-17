namespace NBU.Line.GUI.Controls
{
	partial class MainForm
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
			this.btAdd = new System.Windows.Forms.Button();
			this.btRemoveTab = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// btAdd
			// 
			this.btAdd.Location = new System.Drawing.Point( 214, 34 );
			this.btAdd.Name = "btAdd";
			this.btAdd.Size = new System.Drawing.Size( 75, 23 );
			this.btAdd.TabIndex = 0;
			this.btAdd.Text = "Add item";
			this.btAdd.UseVisualStyleBackColor = true;
			this.btAdd.Click += new System.EventHandler( this.cmdAdd );
			// 
			// btRemoveTab
			// 
			this.btRemoveTab.Location = new System.Drawing.Point( 214, 64 );
			this.btRemoveTab.Name = "btRemoveTab";
			this.btRemoveTab.Size = new System.Drawing.Size( 75, 23 );
			this.btRemoveTab.TabIndex = 1;
			this.btRemoveTab.Text = "Remove Tab";
			this.btRemoveTab.UseVisualStyleBackColor = true;
			this.btRemoveTab.Click += new System.EventHandler( this.cmdRemoveTab );
			// 
			// MainForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF( 6F, 13F );
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size( 310, 333 );
			this.Controls.Add( this.btRemoveTab );
			this.Controls.Add( this.btAdd );
			this.Name = "MainForm";
			this.Text = "Form1";
			this.Shown += new System.EventHandler( this.MainForm_Shown );
			this.ResumeLayout( false );

		}

		#endregion

		private System.Windows.Forms.Button btAdd;
		private System.Windows.Forms.Button btRemoveTab;

	}
}

