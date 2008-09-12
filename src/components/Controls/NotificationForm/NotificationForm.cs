//----------------------------------------------------------------------
//
// SVN:			$Id$
//
//----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;

using System.Text;
using System.Windows.Forms;

namespace Toolkit.Controls.ExceptionForm
{
	/// <summary>
	/// Dialog wich display information about occured exception.
	/// </summary>
	public partial class NotificationForm: Form
	{
		/// <summary>Notification type</summary>
		public enum NotificationType
		{
			/// <summary>Warning</summary>
			Warning = 1,
			/// <summary>Error</summary>
			Error = 2,
		}

		/// <summary>Shows the dialog.</summary>
		public static DialogResult Show( Exception ex, NotificationType type )
		{
			NotificationForm form = new NotificationForm( ex, type );
			return form.ShowDialog();
		}

		/// <summary>Shows the dialog with default warning notification type</summary> 
		public static DialogResult Show( Exception ex )
		{
			return Show( ex, NotificationType.Warning );
		}

		private System.Resources.ResourceManager m_rm = new System.Resources.ResourceManager(
					"Toolkit.Controls.NotificationForm.NotificationForm_Ext",
					System.Reflection.Assembly.GetExecutingAssembly() );

		private NotificationForm( Exception ex, NotificationType type )
		{
			InitializeComponent();
			this.Text = System.Reflection.Assembly.GetCallingAssembly().GetName().Name;
			tbMessage.Text = ex.Message;
			tbDetails.Text = ex.ToString();

			this.Icon = (Icon)m_rm.GetObject( type.ToString() + "_ico" );
			pbError.Image = (Bitmap)m_rm.GetObject( type.ToString() + "_img" );
			lblCaption.Text = (String)m_rm.GetString( type.ToString() + "_msg" );

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