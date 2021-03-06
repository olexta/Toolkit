//----------------------------------------------------------------------
//
// SVN:			$Id$
//
//----------------------------------------------------------------------

using System;
using System.Drawing;
using System.Windows.Forms;
using System.Text.RegularExpressions;

namespace Toolkit.Controls
{
	/// <summary>
	/// Dialog wich display information about occured exception or custom message with details.
	/// </summary>
	public partial class NotificationForm: Form
	{
		/// <summary>
		/// Notification type
		/// </summary>
		public enum NotificationType
		{
			/// <summary>Warning.</summary>
			Warning = 1,
			/// <summary>Error.</summary>
			Error = 2,
		}

		private System.Resources.ResourceManager m_rm =
			new System.Resources.ResourceManager(typeof( NotificationForm ));

		/// <summary>
		/// Shows notification dialog with specified strings and notification type.
		/// </summary>
		public static DialogResult Show( string msg, string details, NotificationType type )
		{
			NotificationForm form = new NotificationForm( msg, details, type );
			return form.ShowDialog();
		}

		/// <summary>
		/// Shows notification dialog with specified strings and default notification type(warning).
		/// </summary>
		public static DialogResult Show( string msg, string details )
		{
			return Show( msg, details, NotificationType.Warning );
		}

		/// <summary>
		/// Show dialog with specified type and information strings stored in exception.
		/// </summary>
		public static DialogResult Show( Exception ex, NotificationType type )
		{
			return Show( ex.Message, ex.ToString(), type );
		}

		/// <summary>
		/// Show dialog with default(warning) notification type and information strings stored in exception.
		/// </summary> 
		public static DialogResult Show( Exception ex )
		{
			return Show( ex.Message, ex.ToString() );
		}

		private NotificationForm( string msg, string details, NotificationType type )
		{
			InitializeComponent();

			if( Application.OpenForms.Count > 0 ) {
				this.Text = Application.OpenForms[ 0 ].Text;
			} else {
				this.Text = Application.ProductName;
				this.StartPosition = FormStartPosition.CenterScreen;
			}

			tbMessage.Text = Regex.Replace( msg, @"\\n", @"\\r\\n" );
			if( !string.IsNullOrEmpty( details ) ) {
				tbDetails.Text = details;
			} else {
				btnDetails.Visible = false;
			}

			btnClose.Text = m_rm.GetString( type.ToString() + "_btnClose_text" );
			this.Icon = (Icon)m_rm.GetObject( type.ToString() + "_ico" );

			update_btnDetails();
		}

		private void update_btnDetails()
		{
			btnDetails.Image = (Bitmap)m_rm.GetObject(
				tbDetails.Visible ? "up" : "down" );
		}

		private void btnQuit_Click( object sender, EventArgs e )
		{
			this.Close();
		}

		private void btnDetails_Click( object sender, EventArgs e )
		{
			tbDetails.Visible = !tbDetails.Visible;
			update_btnDetails();
		}
	}
}
