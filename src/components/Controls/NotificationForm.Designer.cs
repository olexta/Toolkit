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
			this.pbNotification = new System.Windows.Forms.PictureBox();
			this.lblCaption = new System.Windows.Forms.Label();
			this.tbMessage = new System.Windows.Forms.TextBox();
			( (System.ComponentModel.ISupportInitialize)( this.pbNotification ) ).BeginInit();
			this.SuspendLayout();
			// 
			// tbDetails
			// 
			this.tbDetails.AccessibleDescription = null;
			this.tbDetails.AccessibleName = null;
			resources.ApplyResources( this.tbDetails, "tbDetails" );
			this.tbDetails.BackgroundImage = null;
			this.tbDetails.Font = null;
			this.tbDetails.Name = "tbDetails";
			this.tbDetails.ReadOnly = true;
			// 
			// btnDetails
			// 
			this.btnDetails.AccessibleDescription = null;
			this.btnDetails.AccessibleName = null;
			resources.ApplyResources( this.btnDetails, "btnDetails" );
			this.btnDetails.BackgroundImage = null;
			this.btnDetails.Font = null;
			this.btnDetails.Name = "btnDetails";
			this.btnDetails.UseVisualStyleBackColor = true;
			this.btnDetails.Click += new System.EventHandler( this.btnDetails_Click );
			// 
			// btnClose
			// 
			this.btnClose.AccessibleDescription = null;
			this.btnClose.AccessibleName = null;
			resources.ApplyResources( this.btnClose, "btnClose" );
			this.btnClose.BackgroundImage = null;
			this.btnClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.btnClose.Font = null;
			this.btnClose.Name = "btnClose";
			this.btnClose.UseVisualStyleBackColor = true;
			this.btnClose.Click += new System.EventHandler( this.btnQuit_Click );
			// 
			// pbNotification
			// 
			this.pbNotification.AccessibleDescription = null;
			this.pbNotification.AccessibleName = null;
			resources.ApplyResources( this.pbNotification, "pbNotification" );
			this.pbNotification.BackgroundImage = null;
			this.pbNotification.Font = null;
			this.pbNotification.ImageLocation = null;
			this.pbNotification.Name = "pbNotification";
			this.pbNotification.TabStop = false;
			// 
			// lblCaption
			// 
			this.lblCaption.AccessibleDescription = null;
			this.lblCaption.AccessibleName = null;
			resources.ApplyResources( this.lblCaption, "lblCaption" );
			this.lblCaption.Name = "lblCaption";
			// 
			// tbMessage
			// 
			this.tbMessage.AccessibleDescription = null;
			this.tbMessage.AccessibleName = null;
			resources.ApplyResources( this.tbMessage, "tbMessage" );
			this.tbMessage.BackgroundImage = null;
			this.tbMessage.Font = null;
			this.tbMessage.Name = "tbMessage";
			this.tbMessage.ReadOnly = true;
			// 
			// NotificationForm
			// 
			this.AcceptButton = this.btnClose;
			this.AccessibleDescription = null;
			this.AccessibleName = null;
			resources.ApplyResources( this, "$this" );
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackgroundImage = null;
			this.CancelButton = this.btnClose;
			this.Controls.Add( this.tbMessage );
			this.Controls.Add( this.lblCaption );
			this.Controls.Add( this.pbNotification );
			this.Controls.Add( this.btnClose );
			this.Controls.Add( this.btnDetails );
			this.Controls.Add( this.tbDetails );
			this.Font = null;
			this.Icon = null;
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
		private System.Windows.Forms.Button btnClose;
		private System.Windows.Forms.PictureBox pbNotification;
		private System.Windows.Forms.Label lblCaption;
		private System.Windows.Forms.TextBox tbMessage;
	}
}