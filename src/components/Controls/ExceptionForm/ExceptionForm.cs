using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
//----------------------------------------------------------------------
//
// SVN:			$Id$
//
//----------------------------------------------------------------------

using System.Text;
using System.Windows.Forms;

namespace Toolkit.Controls.ExceptionForm
{
	public partial class ExceptionForm: Form
	{
		private System.Resources.ResourceManager m_rm = new System.Resources.ResourceManager(
					"Toolkit.Controls.ExceptionForm.ExceptionForm_Imgs",
					System.Reflection.Assembly.GetExecutingAssembly() );

		public ExceptionForm( Exception ex )
		{
			InitializeComponent();
			this.Text = System.Reflection.Assembly.GetCallingAssembly().GetName().Name;
			tbMessage.Text = ex.Message;
			tbDetails.Text = ex.ToString();

			this.Icon = (Icon)m_rm.GetObject( "error_ico" );
			pbError.Image = (Bitmap)m_rm.GetObject( "error_img" );
			update_btnDetails( );
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