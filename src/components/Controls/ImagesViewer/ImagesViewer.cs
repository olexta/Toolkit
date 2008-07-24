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

namespace Toolkit.Controls.ImagesViewer
{
	/// <summary>
	/// Компонент почергового огляду сукупності зображень,
	/// що можуть бути задані трьома способами:
	/// 1: шлях до директорії з файлами зображень;
	/// 2: перелік шляхів до файлів зображень;
	/// 3: сукупність об'єктів Stream, що містять в собі зображення.
	/// </summary>
	public partial class ImagesViewer : UserControl
	{
		private bool			m_Scale			= true;
		private string			m_Path			= null;
		private IList<string>	m_Files			= null;
		private IList<Stream>	m_Streams		= null;
		private int				m_Index			= -1;

		private bool			m_Drag			= false;
		private Point			m_DragStart		= Point.Empty;

		/// <summary>
		/// Конструктор.
		/// </summary>
		public ImagesViewer()
		{
			InitializeComponent();

			m_ToolScale.Checked = m_Scale;

			m_Panel.HorizontalScroll.SmallChange = 50;
			m_Panel.VerticalScroll.SmallChange = 50;

			check_buttons();
		}

		/// <summary>
		/// Встановлює/повертає шлях до дерикторії з файлами зображень.
		/// </summary>
		[Category( "Images" )]
		[DisplayName( "DirectoryPath" )]
		[Description( "Path to directory wich you want to explore." )]
		public string DirectoryPath
		{
			get {
				return m_Path;
			}
			set {
				m_Path = value;

				if( Directory.Exists( value ) )
					FileList = Directory.GetFiles( value );
				else
					FileList = null;

				display_image( m_Index );
			}
		}

		/// <summary>
		/// Повертає/встановлює сукупніть шляхів до файлів зображень.
		/// </summary>
		[Category( "Images" )]
		[DisplayName( "FilesList" )]
		[Description( "List of files wich you want to view." )]
		public IEnumerable<string> FileList
		{
			get {
				return m_Files;
			}
			set {
				m_Files = new List<string>();
				m_Streams = null;
				m_Path = null;

				if( value != null )
					foreach( string file in value ) {
						switch ( Path.GetExtension( file ) ) {
							case ".bmp":
							case ".jpg":
							case ".tiff":
								m_Files.Add( file );
								break;
						}
					}

				if( m_Files.Count > 0 )
					m_Index = 0;
				else
					m_Index = -1;				

				display_image( m_Index );
			}
		}

		/// <summary>
		/// Повертає/встановлює сукупність об'єктів Stream,
		/// що містять в собі зображення.
		/// </summary>
		[Category( "Images" )]
		[DisplayName( "StreamsList" )]
		[Description( "List of streams wich you want to view." )]
		public IEnumerable<Stream> StreamsList
		{
			get {
				return m_Streams;
			}
			set {
				m_Streams  = new List<Stream>();
				m_Files = null;
				m_Path = null;

				if( value != null )
					foreach( Stream stream in value )
						m_Streams.Add( stream );
				
				if( m_Streams.Count > 0 )
					m_Index = 0;
				else
					m_Index = -1;

				display_image( m_Index );
			}
		}

		/// <summary>
		/// Повертає порядковий номер поточного зображення.
		/// </summary>
		[Category( "Images" )]
		[DisplayName( "ImageIndex" )]
		[Description( "Index of image that is currently displayed." )]
		public int ImageIndex
		{
			get {
				return m_Index;
			}
		}
		
		/// <summary>
		/// Повертає/встановлює значення флагу масштабування зображення
		/// під розміри вікна.
		/// </summary>
		[Category( "Images" )]
		[DisplayName( "ImageScaling" )]
		[Description( "Indicates to provide image scaling to fit window or not." )]
		public bool ImageScaling
		{
			get {
				return m_Scale;
			}
			set {
				m_Scale = value;
				m_ToolScale.Checked = m_Scale;
				if( m_Index != -1 )
					rescale();
			}
		}

		//
		// Ф-я завантаження зображення
		//
		private Image get_image( int index )
		{
			if( m_Files != null )
				return Image.FromFile( m_Files[ index ] );
			if( m_Streams != null )
				return Image.FromStream( m_Streams[ index ] );

			return null;
		}

		//
		// Повертає кількість зображень
		//
		private int images_count()
		{
			if( m_Files != null )
				return m_Files.Count;
			if( m_Streams != null )
				return m_Streams.Count;
			return 0;
		}

		//
		// Ф-я відображення зображення
		//
		private void display_image( int index )
		{
			if( m_PicBox.Image != null )
				m_PicBox.Image.Dispose();

			if( index == -1 ) {
				m_PicBox.Image = null;
				m_ToolLblCount.Text = "0/0";
				m_ToolLblInfo.Text = "";
				return;
			}

			try {
				m_PicBox.Image = get_image( index );
				m_Index = index;
				rescale();
				m_ToolLblCount.Text = index + 1 + "/" + images_count();
				m_ToolLblInfo.Text =
					m_PicBox.Image.Width + "x" +
					m_PicBox.Image.Height + "x" +
					Bitmap.GetPixelFormatSize( m_PicBox.Image.PixelFormat );
				check_buttons();
			} catch ( Exception e ) {
				System.Resources.ResourceManager rm = new System.Resources.ResourceManager(
					"Toolkit.Controls.ImagesViewer.ImagesViewer_Msgs",
					System.Reflection.Assembly.GetExecutingAssembly() );
				// TODO: Еще необходимо показать InnerException и CallStack
				MessageBox.Show(
					rm.GetString( "FailToLoadImage_msg" ),
					rm.GetString( "FailToLoadImage_title" ),
					MessageBoxButtons.OK, MessageBoxIcon.Warning );
			}
		}
		//
		// Ф-я масшабування та позиціювання зображення
		//
		private void rescale()
		{
			if( m_Index == -1 )
				return;

			if( m_Scale ) {		
				float hpercent = (float)m_Panel.Width / m_PicBox.Image.Width;
				float vpercent = (float)m_Panel.Height / m_PicBox.Image.Height;
				float percent = ( hpercent < vpercent ) ? hpercent : vpercent;
				
				if( percent < 1.0F ) {
					m_PicBox.SizeMode = PictureBoxSizeMode.StretchImage;
					m_PicBox.Width = (int)( m_PicBox.Image.Width * percent );
					m_PicBox.Height = (int)( m_PicBox.Image.Height * percent );
				}
				else {
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
				y = m_Panel.Height / 2 - m_PicBox.Height / 2 ;
			m_PicBox.Location = new Point( x, y );
		}
		// 
		// Здійснює вмикання/вимикання кнопок "Попереднє"/"Наступне"
		// в залежності від поточного стану перегляду
		//
		private void check_buttons()
		{
			if( images_count() == 0 ) {
				m_ToolNext.Enabled = m_ToolPrev.Enabled = false;
				return;
			}
			
			m_ToolPrev.Enabled = m_Index != 0;
			m_ToolNext.Enabled = m_Index != images_count() - 1;
		}
		// ***
		// Обробник події зміни розміру вікна
		// ***
		protected override void OnSizeChanged( EventArgs e )
		{
			base.OnSizeChanged( e );

			if( m_Index != -1 )
				rescale();
		}
		// ***
		// Обробники подій елементу ToolBar
		// ***
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
		// ***
		// Функції управління процесом перегляду
		// ***
		private void show_first()
		{
			if( m_Index > 0 )
				display_image( 0 );
		}

		private void show_last()
		{
			if( m_Index != -1 && m_Index != images_count() - 1 )
				display_image( images_count() - 1 );
		}

		private void show_prev()
		{
			if( m_Index > 0 )
				display_image( --m_Index );
		}

		private void show_next()
		{
			if( m_Index != -1 && m_Index < images_count() - 1 )
				display_image( ++m_Index );
		}

		private void scroll_up()
		{
			if( m_PicBox.Image != null && !m_Scale )
				m_Panel.AutoScrollPosition = new Point(
					- m_Panel.AutoScrollPosition.X,
					- m_Panel.AutoScrollPosition.Y - 
						m_Panel.VerticalScroll.SmallChange );
		}

		private void scroll_down()
		{
			if( m_PicBox.Image != null && !m_Scale )
				m_Panel.AutoScrollPosition = new Point(
					- m_Panel.AutoScrollPosition.X,
					 - m_Panel.AutoScrollPosition.Y + 
						m_Panel.VerticalScroll.SmallChange );
		}

		private void scroll_left()
		{
			if( m_PicBox.Image != null && !m_Scale )
				m_Panel.AutoScrollPosition = new Point(
					- m_Panel.AutoScrollPosition.X  - 
						m_Panel.HorizontalScroll.SmallChange,
					- m_Panel.AutoScrollPosition.Y );
		}

		private void scroll_right()
		{
			if( m_PicBox.Image != null && !m_Scale )
				m_Panel.AutoScrollPosition = new Point(
					- m_Panel.AutoScrollPosition.X  + 
						m_Panel.HorizontalScroll.SmallChange,
					- m_Panel.AutoScrollPosition.Y );
		}
		// ***
		// Обробники подій управління процесом переглядання за допомогою клавіатури
		// ***
		protected override bool IsInputKey( Keys keyData )
		{
			switch( keyData )
			{
				case Keys.Left:
				case Keys.Right:
				case Keys.Up:
				case Keys.Down:
					return true;
				default:
					return base.IsInputKey( keyData );
			}
		}

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
		// ***
		// Обробники подій для "насильного" надання фокусу введення елементу управління
		// ***
		private void Panel_Click( object sender, EventArgs e )
		{
			this.Focus();
		}

		private void PicBox_Click( object sender, EventArgs e )
		{
			this.Focus();
		}
		// ***
		// Обробники подій для прокрутки зображення рухом миші
		// ***
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
					 - m_Panel.AutoScrollPosition.X - (e.X - m_DragStart.X ),
					 - m_Panel.AutoScrollPosition.Y - (e.Y - m_DragStart.Y ) );
			}
		}
	}
}