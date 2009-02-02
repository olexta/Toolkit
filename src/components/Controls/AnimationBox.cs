//----------------------------------------------------------------------
//
// SVN:			$Id$
//
//----------------------------------------------------------------------

using System;
using System.Windows.Forms;
using System.Drawing;
using System.ComponentModel;

namespace Toolkit.Controls
{
	/// <summary>
	/// Represents a animated Windows picture box control.
	/// </summary><remarks>
	/// Please don't change Image and Size properties while animation
	/// is running because it can cause unpredictable behaviour.
	/// </remarks>
	public class AnimationBox : PictureBox
	{
		private Timer m_Timer;
		// animation parameters
		private int m_TilesInRow;
		private int m_TilesInColumn;
		private int m_TilesCount;
		private int m_CurrentTile;
		// for saving original image
		private Bitmap m_BigImage;

		/// <summary>
		/// Initializes a new instance of the class. 
		/// </summary>
		public AnimationBox()
		{
			this.Visible = false;
			m_Timer = new Timer();
			m_Timer.Enabled = false;
			m_Timer.Tick += new EventHandler( m_Timer_Tick );
		}

		/// <summary>
		/// Gets or sets animation speed in ms.
		/// </summary>
		[Browsable( true )]
		[Category( "Animation" )]
		[Description( "Animation speed in ms." )]
		public int Speed
		{
			get { return m_Timer.Interval; }
			set { m_Timer.Interval = value; }
		}

		/// <summary>
		/// Gets a value indicating whether animation is running.
		/// </summary>
		[Browsable( false )]
		public bool IsStarted
		{
			get { return m_Timer.Enabled; }
		}

		/// <summary>
		/// Starts animation.
		/// </summary>
		public void Start()
		{
			if( this.Image != null ) {
				m_BigImage = new Bitmap( this.Image );
				m_TilesInRow = this.Image.Width / this.Size.Width;
				m_TilesInColumn = this.Image.Height / this.Size.Height;
				m_TilesCount = m_TilesInRow * m_TilesInColumn;
				m_CurrentTile = 0;
				show_curr_tile();
				this.Visible = true;
				m_Timer.Enabled = true;
			}
		}

		/// <summary>
		/// Stops animation.
		/// </summary>
		public void Stop()
		{
			if( m_Timer.Enabled ) {
				m_Timer.Enabled = false;
				this.Visible = false;
				this.Image = m_BigImage;
			}
		}

		void m_Timer_Tick( object sender, EventArgs e )
		{
			if( m_CurrentTile < m_TilesCount - 1 ) {
				m_CurrentTile++;
			} else {
				m_CurrentTile = 0;
			}
			show_curr_tile();
		}

		void show_curr_tile()
		{
			this.Image = m_BigImage.Clone(
				new Rectangle(
					( m_CurrentTile / m_TilesInRow ) * this.Size.Width,
					( m_CurrentTile % m_TilesInColumn ) * this.Size.Height,
					this.Size.Width,
					this.Size.Height ),
				m_BigImage.PixelFormat);
		}
	}
}
