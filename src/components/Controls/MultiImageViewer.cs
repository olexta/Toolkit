using System;
using System.Drawing;
using System.Windows.Forms;


namespace Toolkit.Controls
{
	/// <summary>
	/// ������� ��������� ��� ��������� ����������� ���������.
	/// </summary>
	public partial class MultiImageViewer : MultiImageViewerBase
	{
		private Point m_DragStart = Point.Empty;

		// ====================================================================
		//            �-� ������������ �� ������������ ����������
		// ====================================================================

		private void rescale()
		{
			if( (Index == -1) || (m_PicBox.Image == null) ) return;

			if( m_ToolScale.Checked ) {
				float hpercent = (float)m_Panel.Width / m_PicBox.Image.Width;
				float vpercent = (float)m_Panel.Height / m_PicBox.Image.Height;
				float percent = ( hpercent < vpercent ) ? hpercent : vpercent;

				if( percent < 1.0F ) {
					m_PicBox.SizeMode = PictureBoxSizeMode.StretchImage;
					m_PicBox.Width = (int)( m_PicBox.Image.Width * percent );
					m_PicBox.Height = (int)( m_PicBox.Image.Height * percent );
				} else {
					m_PicBox.Size = m_PicBox.Image.Size;
				}
			} else {
				m_PicBox.SizeMode = PictureBoxSizeMode.Normal;
				m_PicBox.Location = Point.Empty;
				m_PicBox.Size = m_PicBox.Image.Size;
			}
			m_Panel.AutoScrollPosition = new Point( 0, 0 );
			m_Panel.AutoScrollMinSize = m_PicBox.Size;
			int x = 0, y = 0;
			if( m_Panel.Width > m_PicBox.Width )
				x = m_Panel.Width / 2 - m_PicBox.Width / 2;
			if( m_Panel.Height > m_PicBox.Height )
				y = m_Panel.Height / 2 - m_PicBox.Height / 2;
			m_PicBox.Location = new Point( x, y );
		}

		private void scroll_v( int lines )
		{
			if( (m_PicBox.Image != null) && !m_ToolScale.Checked ) {
				m_Panel.AutoScrollPosition = new Point(
					-m_Panel.AutoScrollPosition.X,
					-m_Panel.AutoScrollPosition.Y +
						lines * m_Panel.VerticalScroll.SmallChange );
			}
		}

		private void scroll_h( int lines )
		{
			if( (m_PicBox.Image != null) && !m_ToolScale.Checked ) {
				m_Panel.AutoScrollPosition = new Point(
					-m_Panel.AutoScrollPosition.X +
						lines * m_Panel.HorizontalScroll.SmallChange,
					-m_Panel.AutoScrollPosition.Y );
			}
		}

		// ====================================================================
		//          ��������� ���� ��� ��������� ���������� ����� ����
		// ====================================================================

		private void PicBox_MouseDown( object sender, MouseEventArgs e )
		{
			if( (Index != -1) && !m_ToolScale.Checked &&
				(e.Button == MouseButtons.Left) ) {
				// ���������� �������� ��� ������ ����
				m_PicBox.Cursor = Cursors.Hand;
				m_DragStart = e.Location;
			}
		}

		private void PicBox_MouseUp( object sender, MouseEventArgs e )
		{
			if( (m_DragStart != Point.Empty) &&
				(e.Button == MouseButtons.Left) ) {
				// ���������� �������� ��� ������ ����
				m_PicBox.Cursor = Cursors.Arrow;
				m_DragStart = Point.Empty;
			}
		}

		private void PicBox_MouseMove( object sender, MouseEventArgs e )
		{
			if( m_DragStart != Point.Empty ) {
				m_Panel.AutoScrollPosition = new Point(
					 -m_Panel.AutoScrollPosition.X - ( e.X - m_DragStart.X ),
					 -m_Panel.AutoScrollPosition.Y - ( e.Y - m_DragStart.Y ) );
			}
		}

		/// <summary />
		protected override void OnMouseWheel( MouseEventArgs e )
		{
			scroll_v( -e.Delta * SystemInformation.MouseWheelScrollLines
							   / SystemInformation.MouseWheelScrollDelta );

			base.OnMouseWheel( e );
		}

		// ====================================================================
		//         ��������� ���� ��������� �������� ������������
		//                    �� ��������� ���������
		// ====================================================================

		/// <summary />
		protected override bool IsInputKey( Keys keyData )
		{
			switch( keyData ) {
				case Keys.Up:
				case Keys.Down:
				case Keys.Left:
				case Keys.Right:
					return true;
				default:
					return base.IsInputKey( keyData );
			}
		}

		/// <summary />
		protected override void OnKeyDown( KeyEventArgs e )
		{
			switch( e.KeyCode ) {
				case Keys.Up:
					scroll_v( -1 );
				break;
				case Keys.Down:
					scroll_v( 1 );
				break;
				case Keys.Left:
					scroll_h( -1 );
				break;
				case Keys.Right:
					scroll_h( 1 );
				break;
			}
			base.OnKeyDown( e );
		}

		/// <summary />
		protected override void OnKeyUp( KeyEventArgs e )
		{
			if( e.KeyCode == Keys.F ) ImageScaling = !ImageScaling;

			base.OnKeyUp( e );
		}

		// ====================================================================
		//                   ������� ��������� ����
		// ====================================================================

		private void PicBox_Click( object sender, EventArgs e )
		{
			this.Focus();
		}

		private void ToolScale_Click( object sender, EventArgs e )
		{
			this.Focus();
		}

		private void ToolScale_CheckedChanged( object sender, EventArgs e )
		{
			if( Index != -1 ) rescale();
		}

		/// <summary>
		/// �������� ��䳿 ���� ������ ����.
		/// </summary>
		protected override void OnSizeChanged( EventArgs e )
		{
			base.OnSizeChanged( e );

			if( (Index != -1) && m_ToolScale.Checked ) rescale();
		}

		/// <summary>
		/// ������� ����������� ����������.
		/// </summary>
		/// <param name="image">���������� ��� �����������.</param>
		protected override void DrawObject( Image image )
		{
			if( m_PicBox.Image != null ) m_PicBox.Image.Dispose();
			m_PicBox.Image = image;

			rescale();

			if( image != null ) {
				m_ToolLblInfo.Text =
					image.Width + "x" +
					image.Height + "x" +
					Bitmap.GetPixelFormatSize( image.PixelFormat );
			} else {
				m_ToolLblInfo.Text = "";
			}
		}

		/// <summary>
		/// ������� ����� ������� ��������� ���������� �����������
		/// ���������.
		/// </summary>
		public MultiImageViewer()
		{
			InitializeComponent();
		}

		/// <summary>
		/// �������/���������� �������� ����� ������������� ����������
		/// �� ������ ����.
		/// </summary>
		public bool ImageScaling
		{
			get { return m_ToolScale.Checked; }
			set {
				m_ToolScale.Checked = value;
			}
		}
	}

	/// <summary>
	/// ������� ���� �������� ��������� ��� ��������� ����������� ���������.
	/// </summary><remarks>
	/// ����������� � ������ ���� ������ ����� �������������
	/// ��������� VisualStudio.
	/// </remarks>
	public class MultiImageViewerBase : MultiObjectViewer<Image>
	{
		/// <summary>
		/// ������� ����� ��������� �����.
		/// </summary><remarks>
		/// ����������� ������ ��������� ��� ����'�������� ������������.
		/// </remarks>
		protected MultiImageViewerBase() {}

		/// <summary>
		/// ������� ����������� ����������.
		/// </summary>
		/// <param name="image">���������� ��� �����������.</param>
		/// <remarks>
		/// ����'������ �� ���� ���������� � ��������� ����. �
		/// ������� ��� �� �������� ����������� �����!
		/// </remarks>
		protected override void DrawObject( Image image )
		{
			throw new NotImplementedException();
		}
	}
}