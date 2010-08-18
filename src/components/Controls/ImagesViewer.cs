//----------------------------------------------------------------------
//
// SVN:			$Id: ImagesViewer.cs 275 2008-06-10 16:56:44Z N.Marunyak $
//
//----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;
using System.IO;

namespace Toolkit.Controls
{
	/// <summary>
	/// Елемент керування для перегляду послідовності зображень.
	/// </summary>
	public partial class ImagesViewer: UserControl
	{
		/// <summary>
		/// Структура для виклику та отримання запиту на отримання нового зображення.
		/// </summary>
		public class ShowImageEventArgs: EventArgs
		{
			private int m_ImageIndex;
			private Image m_Image;

			/// <summary>
			/// Створює новий екземпляр класу.
			/// </summary>
			/// <param name="imageIndex">Індекс необхідного зображення.</param>
			public ShowImageEventArgs( int imageIndex )
			{
				m_ImageIndex = imageIndex;
			}

			/// <summary>
			/// Повертає індекс запрошеного зображення.
			/// </summary>
			public int ImageIndex
			{
				get { return m_ImageIndex; }
			}

			/// <summary>
			/// Повертає/встановлює запрошене зображення.
			/// </summary>
			public Image Image
			{
				get { return m_Image; }
				set { m_Image = value; }
			}
		}

		/// <summary>
		/// Делегат події отримання нового зображення для відображення.
		/// </summary>
		/// <param name="sender">Кореспондент запиту.</param>
		/// <param name="e">Структура з параметрами запиту.</param>
		public delegate void ShowImageEventHandler( object sender, ShowImageEventArgs e );

		private bool m_Scale = true;
		private int m_Index = -1;
		private int m_Count = 0;
		private Image m_Image = null;

		private bool m_Drag = false;
		private Point m_DragStart = Point.Empty;

		private event ShowImageEventHandler show_image;

		/// <summary>
		/// Створює новий екземпляр класу.
		/// </summary>
		public ImagesViewer()
		{
			InitializeComponent();

			m_Panel.HorizontalScroll.SmallChange = 50;
			m_Panel.VerticalScroll.SmallChange = 50;

			ImageScaling = true;

			Count = 0;
		}

		/// <summary>
		/// Подія отримання нового зображення для показу.
		/// </summary>
		/// <remarks>
		/// У цієї події може бути лише один обробник.
		/// </remarks>
		public event ShowImageEventHandler ShowImage
		{
			add
			{
				if( show_image != null
					&& show_image.GetInvocationList().Length == 1 ) {
					throw new InvalidOperationException( "This event can't have more than one handler." );
				}
				show_image += value;
			}
			remove
			{
				show_image -= value;
			}
		}

		/// <summary>
		/// Повертає/встановлює кількість зображень.
		/// </summary>
		/// <remarks>
		/// Кількість має бути не меньше нуля і встановлення значення
		/// більше нуля призводить до показу першого зображення.
		/// </remarks>
		public int Count
		{
			get { return m_Count; }
			set
			{
				if( value < 0 ) {
					throw new ArgumentException( "Value must be greater or equal to zero", "value" );
				}
				m_Count = value;

				if( value > 0 ) {
					display_image( 0 );
				} else {
					display_image( -1 );
				}
			}
		}

		/// <summary>
		/// Повертає/встановлює значення флагу масштабування зображення
		/// під розміри вікна.
		/// </summary>
		public bool ImageScaling
		{
			get { return m_Scale; }
			set
			{
				m_Scale = value;
				m_ToolScale.Checked = m_Scale;
				if( m_Index != -1 ) {
					rescale();
				}
			}
		}

		//
		// Ф-я відображення зображення.
		//
		private void display_image( int index )
		{
			if( (index >= 0) && (show_image != null) && Visible ) {
				ShowImageEventArgs args = new ShowImageEventArgs( index );
				show_image( this, args );
				m_Image = args.Image;
			} else {
				m_Image = null;
			}

			m_PicBox.Image = m_Image;
			m_Index = index;

			rescale();

			m_ToolLblCount.Text = m_Index + 1 + "/" + m_Count;
			if( m_Image != null ) {
				m_ToolLblInfo.Text =
					m_Image.Width + "x" +
					m_Image.Height + "x" +
					Bitmap.GetPixelFormatSize( m_Image.PixelFormat );
			} else {
				m_ToolLblInfo.Text = "";
			}

			check_buttons();
		}

		//
		// Ф-я масшабування та позиціювання зображення.
		//
		private void rescale()
		{
			if( m_Index == -1 || m_Image == null )
				return;

			if( m_Scale ) {
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

		//
		// Здійснює вмикання/вимикання кнопок "Попереднє"/"Наступне"
		// в залежності від поточного стану перегляду.
		//
		private void check_buttons()
		{
			if( m_Count == 0 ) {
				m_ToolNext.Enabled = m_ToolPrev.Enabled = false;
				return;
			}

			m_ToolPrev.Enabled = m_Index != 0;
			m_ToolNext.Enabled = m_Index != m_Count - 1;
		}

		//
		// Обробник події показу контрола.
		//
		protected override void OnVisibleChanged( EventArgs e )
		{
			// якщо зображення досі не завантажено (контрол був невидимий)
			// спробуємо його отримати
			if( Visible && (m_Image == null) ) display_image( m_Index );

			base.OnVisibleChanged( e );
		}

		// 
		// Обробник події зміни розміру вікна.
		// 
		/// <summary />
		protected override void OnSizeChanged( EventArgs e )
		{
			base.OnSizeChanged( e );

			if( m_Index != -1 ) rescale();
		}

		// ====================================================================
		//                  Обробники подій елементу ToolBar
		// ====================================================================
		private void ToolPrev_Click( object sender, EventArgs e )
		{
			show_prev();
			this.Focus();
		}

		private void ToolNext_Click( object sender, EventArgs e )
		{
			show_next();
			this.Focus();
		}

		private void ToolScale_Click( object sender, EventArgs e )
		{
			ImageScaling = m_ToolScale.Checked;
			this.Focus();
		}

		// ====================================================================
		//                Функції управління процесом перегляду
		// ====================================================================
		private void show_first()
		{
			if( m_Index > 0 ) display_image( 0 );
		}

		private void show_last()
		{
			if( m_Index != -1 && m_Index != m_Count - 1 ) {
				display_image( m_Count - 1 );
			}
		}

		private void show_prev()
		{
			if( m_Index > 0 ) display_image( m_Index - 1 );
		}

		private void show_next()
		{
			if( m_Index != -1 && m_Index < m_Count - 1 ) {
				display_image( m_Index + 1 );
			}
		}

		private void scroll_up()
		{
			if( m_PicBox.Image != null && !m_Scale ) {
				m_Panel.AutoScrollPosition = new Point(
					-m_Panel.AutoScrollPosition.X,
					-m_Panel.AutoScrollPosition.Y -
						m_Panel.VerticalScroll.SmallChange );
			}
		}

		private void scroll_down()
		{
			if( m_PicBox.Image != null && !m_Scale ) {
				m_Panel.AutoScrollPosition = new Point(
					-m_Panel.AutoScrollPosition.X,
					-m_Panel.AutoScrollPosition.Y +
						m_Panel.VerticalScroll.SmallChange );
			}
		}

		private void scroll_left()
		{
			if( m_PicBox.Image != null && !m_Scale ) {
				m_Panel.AutoScrollPosition = new Point(
					-m_Panel.AutoScrollPosition.X -
						m_Panel.HorizontalScroll.SmallChange,
					-m_Panel.AutoScrollPosition.Y );
			}
		}

		private void scroll_right()
		{
			if( m_PicBox.Image != null && !m_Scale ) {
				m_Panel.AutoScrollPosition = new Point(
					-m_Panel.AutoScrollPosition.X +
						m_Panel.HorizontalScroll.SmallChange,
					-m_Panel.AutoScrollPosition.Y );
			}
		}

		// ====================================================================
		//         Обробники подій управління процесом переглядання
		//                    за допомогою клавіатури
		// ====================================================================
		/// <summary />
		protected override bool IsInputKey( Keys keyData )
		{
			switch( keyData ) {
				case Keys.Left:
				case Keys.Right:
				case Keys.Up:
				case Keys.Down:
					return true;
				default:
					return base.IsInputKey( keyData );
			}
		}

		/// <summary />
		protected override void OnKeyDown( KeyEventArgs e )
		{
			switch( e.KeyCode ) {
				case Keys.PageDown:
					show_next();
					break;
				case Keys.PageUp:
					show_prev();
					break;
				case Keys.Up:
					scroll_up();
					break;
				case Keys.Down:
					scroll_down();
					break;
				case Keys.Left:
					scroll_left();
					break;
				case Keys.Right:
					scroll_right();
					break;
			}
			base.OnKeyDown( e );
		}

		/// <summary />
		protected override void OnKeyUp( KeyEventArgs e )
		{
			switch( e.KeyCode ) {
				case Keys.Back:
					show_prev();
					break;
				case Keys.Space:
					show_next();
					break;
				case Keys.F:
					ImageScaling = !ImageScaling;
					break;
				case Keys.Home:
					show_first();
					break;
				case Keys.End:
					show_last();
					break;
			}
			base.OnKeyUp( e );
		}

		// ====================================================================
		//              Обробники подій для "насильного" надання
		//                фокусу введення елементу управління
		// ====================================================================
		private void Panel_Click( object sender, EventArgs e )
		{
			this.Focus();
		}

		private void PicBox_Click( object sender, EventArgs e )
		{
			this.Focus();
		}

		// ====================================================================
		//          Обробники подій для прокрутки зображення рухом миші
		// ====================================================================
		private void PicBox_MouseDown( object sender, MouseEventArgs e )
		{
			if( m_Index != -1 && !m_Scale ) {
				m_Drag = true;
				m_PicBox.Cursor = Cursors.Hand;
				m_DragStart = e.Location;
			}
		}

		private void PicBox_MouseUp( object sender, MouseEventArgs e )
		{
			if( m_Drag ) {
				m_Drag = false;
				m_PicBox.Cursor = Cursors.Arrow;
				m_Panel.AutoScrollPosition = new Point(
					 -m_Panel.AutoScrollPosition.X - ( e.X - m_DragStart.X ),
					 -m_Panel.AutoScrollPosition.Y - ( e.Y - m_DragStart.Y ) );
			}
		}
	}
}
