//----------------------------------------------------------------------
//
// SVN:			$Id$
//
//----------------------------------------------------------------------

namespace Toolkit.Controls.NotificationForm
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
			this.btnQuit = new System.Windows.Forms.Button();
			this.pbNotification = new System.Windows.Forms.PictureBox();
			this.lblCaption = new System.Windows.Forms.Label();
			this.tbMessage = new System.Windows.Forms.TextBox();
			( (System.ComponentModel.ISupportInitialize)( this.pbNotification ) ).BeginInit();
			this.SuspendLayout();
			// 
			// tbDetails
			// 
			resources.ApplyResources( this.tbDetails, "tbDetails" );
			this.tbDetails.Name = "tbDetails";
			this.tbDetails.ReadOnly = true;
			// 
			// btnDetails
			// 
			resources.ApplyResources( this.btnDetails, "btnDetails" );
			this.btnDetails.Name = "btnDetails";
			this.btnDetails.UseVisualStyleBackColor = true;
			this.btnDetails.Click += new System.EventHandler( this.btnDetails_Click );
			// 
			// btnQuit
			// 
			this.btnQuit.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			resources.ApplyResources( this.btnQuit, "btnQuit" );
			this.btnQuit.Name = "btnQuit";
			this.btnQuit.UseVisualStyleBackColor = true;
			this.btnQuit.Click += new System.EventHandler( this.btnQuit_Click );
			// 
			// pbNotification
			// 
			resources.ApplyResources( this.pbNotification, "pbNotification" );
			this.pbNotification.Name = "pbNotification";
			this.pbNotification.TabStop = false;
			// 
			// lblCaption
			// 
			resources.ApplyResources( this.lblCaption, "lblCaption" );
			this.lblCaption.Name = "lblCaption";
			// 
			// tbMessage
			// 
			resources.ApplyResources( this.tbMessage, "tbMessage" );
			this.tbMessage.Name = "tbMessage";
			this.tbMessage.ReadOnly = true;
			// 
			// NotificationForm
			// 
			this.AcceptButton = this.btnQuit;
			resources.ApplyResources( this, "$this" );
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
			this.CancelButton = this.btnQuit;
			this.Controls.Add( this.tbMessage );
			this.Controls.Add( this.lblCaption );
			this.Controls.Add( this.pbNotification );
			this.Controls.Add( this.btnQuit );
			this.Controls.Add( this.btnDetails );
			this.Controls.Add( this.tbDetails );
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "NotificationForm";
			this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
			( (System.ComponentModel.ISupportInitialize)( this.pbNotification ) ).EndInit();
			this.ResumeLayout( false );
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.TextBox tbDetails;
		private System.Windows.Forms.Button btnDetails;
		private System.Windows.Forms.Button btnQuit;
		private System.Windows.Forms.PictureBox pbNotification;
		private System.Windows.Forms.Label lblCaption;
		private System.Windows.Forms.TextBox tbMessage;
	}
}