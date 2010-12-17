//----------------------------------------------------------------------
//
// SVN:			$Id: ImagesViewer.cs 275 2008-06-10 16:56:44Z N.Marunyak $
//
//----------------------------------------------------------------------

using System;
using System.Windows.Forms;


namespace Toolkit.Controls
{
	/// <summary>
	/// Базовий графічний контрол для перегляду послідовності об'єктів.
	/// </summary><remarks>
	/// Для коректної роботи дизайнера Visual Studio графічну реалізацію
	/// об'єкту винесено в окремий не шаблонний клас.
	/// </remarks>
	public partial class MultiObjectViewerBase : UserControl
	{
		private int m_Index = -1;
		private int m_Count = 0;

		//
		// Ф-я встановлює індекс активного об'єкту та здійснює
		// вмикання/вимикання кнопок "Попереднє"/"Наступне" в
		// залежності від поточного стану перегляду.
		//
		private void set_index( int index )
		{
			m_Index = index;
			m_ToolLblCount.Text = m_Index + 1 + "/" + m_Count;

			if( m_Count == 0 ) {
				m_ToolNext.Enabled = m_ToolPrev.Enabled = false;
				return;
			}

			m_ToolPrev.Enabled = m_Index != 0;
			m_ToolNext.Enabled = m_Index != m_Count - 1;
		}

		// ====================================================================
		//                Функції управління процесом перегляду
		// ====================================================================
		private void show_first()
		{
			if( m_Index > 0 ) DisplayObject( 0 );
		}

		private void show_last()
		{
			if( m_Index != -1 && m_Index != m_Count - 1 ) {
				DisplayObject( m_Count - 1 );
			}
		}

		private void show_prev()
		{
			if( m_Index > 0 ) DisplayObject( m_Index - 1 );
		}

		private void show_next()
		{
			if( m_Index != -1 && m_Index < m_Count - 1 ) {
				DisplayObject( m_Index + 1 );
			}
		}

		// ====================================================================
		//              Обробник подій для "насильного" надання
		//                фокусу введення елементу управління
		// ====================================================================
		private void Panel_Click( object sender, EventArgs e )
		{
			this.Focus();
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

		// ====================================================================
		//         Обробники подій управління процесом переглядання
		//                    за допомогою клавіатури
		// ====================================================================
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
			}
			base.OnKeyDown( e );
		}

		/// <summary />
		protected override void OnKeyUp( KeyEventArgs e )
		{
			switch( e.KeyCode ) {
				case Keys.Home:
					show_first();
				break;
				case Keys.End:
					show_last();
				break;
			}
			base.OnKeyUp( e );
		}

		/// <summary>
		/// Створює новий екземпляр класу.
		/// </summary><remarks>
		/// Об'явлено захищеним для обов'язкового наслідування від класу.
		/// </remarks>
		protected MultiObjectViewerBase()
		{
			InitializeComponent();

			m_Panel.HorizontalScroll.SmallChange = 50;
			m_Panel.VerticalScroll.SmallChange = 50;

			m_Count = 0;
			set_index( -1 );
		}

		/// <summary>
		/// Ф-я відображення об'єкта.
		/// </summary><remarks>
		/// Для реалізації функціоналу більшого ніж відображення
		/// індексу активного об'єкту, необхідно перекрити реалізацію
		/// у дочірному класі.
		/// </remarks>
		protected virtual void DisplayObject( int index )
		{
			set_index( index );
		}

		/// <summary>
		/// Повертає/встановлює індекс об'єкту для відображається.
		/// </summary>
		public int Index
		{
			get { return m_Index; }
			set {
				if( value < -1 ) {
					throw new ArgumentException("Value must be greater or equal to -1", "value");
				} else if( value >= m_Count ) {
					throw new ArgumentException( "Index value is out of range", "value" );
				}
				DisplayObject( value );
			}
		}

		/// <summary>
		/// Повертає/встановлює кількість об'єктів.
		/// </summary>
		/// <remarks>
		/// Кількість має бути не меньше нуля і встановлення значення
		/// більше нуля призводить до показу першого об'єкта.
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
					DisplayObject( 0 );
				} else {
					DisplayObject( -1 );
				}
			}
		}
	}

	/// <summary>
	/// Клас доповнює MultiObjectViewerBase клас шаблонним інтерфейсом.
	/// </summary>
	/// <typeparam name="T">Тип ідентифікації об'єкту відображення.</typeparam>
	public abstract class MultiObjectViewer<T> : MultiObjectViewerBase
	{
		private T m_Object = default(T);

		/// <summary>
		/// Структура для виклика запиту на отримання нового об'єкту.
		/// </summary>
		public class ShowObjectEventArgs: EventArgs
		{
			private int m_Index;
			private T m_Object;

			/// <summary>
			/// Створює новий екземпляр класу.
			/// </summary>
			/// <param name="index">Індекс необхідного об'єкту.</param>
			public ShowObjectEventArgs( int index )
			{
				m_Index = index;
			}

			/// <summary>
			/// Повертає індекс запрошеного об'єкта.
			/// </summary>
			public int Index
			{
				get { return m_Index; }
			}

			/// <summary>
			/// Повертає/встановлює запрошений об'єкт.
			/// </summary>
			public T Object
			{
				get { return m_Object; }
				set { m_Object = value; }
			}
		}

		/// <summary>
		/// Делегат події отримання нового об'єкту для відображення.
		/// </summary>
		/// <param name="sender">Кореспондент запиту.</param>
		/// <param name="e">Структура з параметрами запиту.</param>
		public delegate void SHOW_OBJECT( object sender, ShowObjectEventArgs e );

		private event SHOW_OBJECT show_object;

		/// <summary>
		/// Розширена функція відображення об'єкта.
		/// </summary><remarks>
		/// Розширює стандартний функціонал подією показу об'єкта
		/// та викликом обов'язкової функції відображення об'єкту.
		/// </remarks>
		protected sealed override void DisplayObject( int index )
		{
			base.DisplayObject( index );

			if( (index >= 0) && (show_object != null) && Visible ) {
				ShowObjectEventArgs args = new ShowObjectEventArgs( index );
				show_object( this, args );
				m_Object = args.Object;
			} else {
				// на відображення піде пустий об'єкт
				m_Object = default(T);
			}

			// викликаємо у дочірнього об'єкту функцію відображення
			DrawObject( m_Object );
		}

		/// <summary>
		/// Обробники події показу контрола.
		/// </summary>
		protected override void OnVisibleChanged( EventArgs e )
		{
			// якщо об'єкт досі не завантажено (контрол був невидимий)
			// спробуємо його отримати
			if( Visible && (Index >= 0) && (m_Object == null) ) {
				DisplayObject( Index );
			}
			base.OnVisibleChanged( e );
		}

		/// <summary>
		/// Абстрактна функція відображення об'єкту.
		/// </summary>
		/// <param name="obj">Об'єкт для відображення.</param>
		protected abstract void DrawObject( T obj );

		/// <summary>
		/// Подія отримання нового об'єкту для показу.
		/// </summary><remarks>
		/// У цієї події може бути лише один обробник.
		/// </remarks>
		public event SHOW_OBJECT ShowObject
		{
			add
			{
				if( show_object != null
					&& show_object.GetInvocationList().Length == 1 ) {
					throw new InvalidOperationException( "This event can't have more than one handler." );
				}
				show_object += value;
			}
			remove
			{
				show_object -= value;
			}
		}
	}
}
