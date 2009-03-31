//----------------------------------------------------------------------
//
// SVN:			$Id$
//
//----------------------------------------------------------------------

namespace Toolkit.Controls
{
	partial class NotificationForm
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager( typeof( NotificationForm ) );
			this.tbDetails = new System.Windows.Forms.TextBox();
			this.btnDetails = new System.Windows.Forms.Button();
			this.btnClose = new System.Windows.Forms.Button();
			this.tbMessage = new System.Windows.Forms.TextBox();
			this.SuspendLayout();
			// 
			// tbDetails
			// 
			resources.ApplyResources( this.tbDetails, "tbDetails" );
			this.tbDetails.Name = "tbDetails";
			this.tbDetails.ReadOnly = true;
			this.tbDetails.TabStop = false;
			// 
			// btnDetails
			// 
			resources.ApplyResources( this.btnDetails, "btnDetails" );
			this.btnDetails.Name = "btnDetails";
			this.btnDetails.UseVisualStyleBackColor = true;
			this.btnDetails.Click += new System.EventHandler( this.btnDetails_Click );
			// 
			// btnClose
			// 
			resources.ApplyResources( this.btnClose, "btnClose" );
			this.btnClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.btnClose.Name = "btnClose";
			this.btnClose.UseVisualStyleBackColor = true;
			this.btnClose.Click += new System.EventHandler( this.btnQuit_Click );
			// 
			// tbMessage
			// 
			resources.ApplyResources( this.tbMessage, "tbMessage" );
			this.tbMessage.Name = "tbMessage";
			this.tbMessage.ReadOnly = true;
			this.tbMessage.TabStop = false;
			// 
			// NotificationForm
			// 
			this.AcceptButton = this.btnClose;
			resources.ApplyResources( this, "$this" );
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.CancelButton = this.btnClose;
			this.Controls.Add( this.tbMessage );
			this.Controls.Add( this.btnClose );
			this.Controls.Add( this.btnDetails );
			this.Controls.Add( this.tbDetails );
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "NotificationForm";
			this.ShowInTaskbar = false;
			this.ResumeLayout( false );
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.TextBox tbDetails;
		private System.Windows.Forms.Button btnDetails;
		private System.Windows.Forms.Button btnClose;
		private System.Windows.Forms.TextBox tbMessage;
	}
}