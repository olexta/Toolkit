
namespace Toolkit.Controls.Test
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
			this.m_Viewer = new Toolkit.Controls.ImagesViewer();
			this.numericUpDown1 = new System.Windows.Forms.NumericUpDown();
			this.button1 = new System.Windows.Forms.Button();
			( (System.ComponentModel.ISupportInitialize)( this.numericUpDown1 ) ).BeginInit();
			this.SuspendLayout();
			// 
			// m_Viewer
			// 
			this.m_Viewer.Anchor = ( (System.Windows.Forms.AnchorStyles)( ( ( ( System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom )
						| System.Windows.Forms.AnchorStyles.Left )
						| System.Windows.Forms.AnchorStyles.Right ) ) );
			this.m_Viewer.Count = 5;
			this.m_Viewer.ImageScaling = true;
			this.m_Viewer.Location = new System.Drawing.Point( 12, 49 );
			this.m_Viewer.Name = "m_Viewer";
			this.m_Viewer.Size = new System.Drawing.Size( 342, 330 );
			this.m_Viewer.TabIndex = 1;
			this.m_Viewer.ShowImage += new Toolkit.Controls.ImagesViewer.ShowImageEventHandler( this.m_Viewer_ShowImage );
			// 
			// numericUpDown1
			// 
			this.numericUpDown1.Location = new System.Drawing.Point( 293, 12 );
			this.numericUpDown1.Name = "numericUpDown1";
			this.numericUpDown1.Size = new System.Drawing.Size( 61, 20 );
			this.numericUpDown1.TabIndex = 2;
			this.numericUpDown1.ValueChanged += new System.EventHandler( this.numericUpDown1_ValueChanged );
			// 
			// button1
			// 
			this.button1.Location = new System.Drawing.Point( 12, 12 );
			this.button1.Name = "button1";
			this.button1.Size = new System.Drawing.Size( 75, 23 );
			this.button1.TabIndex = 3;
			this.button1.Text = "button1";
			this.button1.UseVisualStyleBackColor = true;
			this.button1.Click += new System.EventHandler( this.button1_Click );
			// 
			// TestForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF( 6F, 13F );
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size( 366, 391 );
			this.Controls.Add( this.button1 );
			this.Controls.Add( this.numericUpDown1 );
			this.Controls.Add( this.m_Viewer );
			this.Name = "TestForm";
			this.Text = "TestForm";
			( (System.ComponentModel.ISupportInitialize)( this.numericUpDown1 ) ).EndInit();
			this.ResumeLayout( false );

		}

		#endregion

		private Toolkit.Controls.ImagesViewer m_Viewer;
		private System.Windows.Forms.NumericUpDown numericUpDown1;
		private System.Windows.Forms.Button button1;

	}
}
