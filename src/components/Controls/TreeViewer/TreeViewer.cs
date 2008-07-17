using System;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;
using System.Collections.Generic;


namespace Toolkit.Controls.TreeViewer
{
	/// <summary>
	/// Управляет набором Tab'ов
	/// </summary>
	[ CLSCompliantAttribute( true ) ]
	public partial class TreeViewer : Control
	{
		#region private declarations
		// коллекция хранящихся в контроле Tab'ов
		private TabCollection m_tc;
		// индекс текущего Tab'а
		private int m_selectedIndex = 0;
		// константа, которая описывает высоту Tab'ов
		private readonly int BUTTON_HEIGHT = System.Windows.Forms.SystemInformation.MenuButtonSize.Height + 6;
		#endregion

		#region public declaration
		/// <summary>
		/// Represents the method that will handle the 
		/// BrowserControl.Selecting or BrowserControl.Deselecting event 
		/// of a BrowserControl control.
		/// </summary>
		public delegate void BrowserControlCancelEventHandler( object sender,
						BrowserControlCancelEventArgs e );
		/// <summary>
		/// Represents the method that will handle the 
		/// BrowserControl.Selected or BrowserControl.Deselected event 
		/// of a BrowserControl control.
		/// </summary>
		public delegate void BrowserControlEventHandler( object sender,
						BrowserControlEventArgs e );

		/// <summary>
		/// Occurs when the SelectedIndex property is changed.
		/// </summary>
		public event EventHandler SelectedIndexChanged;

		/// <summary>
		/// Occurs when a tab is selected.
		/// </summary>
		public event BrowserControlEventHandler Selected;

		/// <summary>
		/// Occurs before a tab is selected, enabling a handler to cancel the tab change.
		/// </summary>
		public event BrowserControlCancelEventHandler Selecting;

		/// <summary>
		/// Occurs when a tab is deselected.
		/// </summary>
		public event BrowserControlEventHandler Deselected;

		/// <summary>
		/// Occurs before a tab is deselected, enabling a handler to cancel the tab change.
		/// </summary>
		public event BrowserControlCancelEventHandler Deselecting;
		#endregion

		#region public methods
		/// <summary>
		/// Базовый конструктор.
		/// </summary>
		public TreeViewer() : base()
		{
			// инициализация коллекции
			m_tc = new TabCollection( this );
			// подписка на уведомление о изменении коллекции
			m_tc.CollectionChanged += new EventHandler( this.on_tabs_change );
			
			// инициализация визуальных компонент
			InitializeComponent();
			
			// обновление расположения компонент на контроле
			update_layout( false );
		}
		
		#endregion
		
		#region public properties
		/// <summary>
		/// Получени коллекции с Tab'ами, которые принадлежат данному контролу
		/// </summary>
		public TabCollection Tabs
		{
			get { return m_tc; }
		}

		/// <summary>
		/// Получение/Изменение индекса текущего активного Tab'а
		/// </summary>
		public int SelectedIndex
		{
			get { return m_selectedIndex; }
			set
			{
				// проверка входного параметра
				if( (value < 0) || (value > m_tc.Count) ) {
					throw new ArgumentOutOfRangeException( "SelectedIndex" );
				}

				// изменение активного Tab'а
				change_tab( value, m_selectedIndex );
				// обновление расположения компонент на контроле,
				// с удалением всех имеющихся
				update_layout( true );
			}
		}

		/// <summary>
		/// Возвращает количество Tab'ов в BrowserControl'е
		/// </summary>
		public int TabCount
		{
			get { return m_tc.Count; }
		}

		/// <summary>
		/// Устанавливает/Возвращает текущий Tab.
		/// При установке происходит проверка на приналежность передаваемого Tab'a
		/// данному экземрляру BrowserControl'а
		/// При возвращении активного Tab'a возвращается null
		/// в случае отсутствия онного.
		/// </summary>
		public Tab SelectedTab
		{
			get
			{
				// проверка на наличие Tab'ов в текущем BrowserControl'е
				if( m_tc.Count > 0 ) { return m_tc[m_selectedIndex]; } else { return null; }
			}
			set
			{
				// проверка входного параметра
				if( (value == null) ) {
					throw new ArgumentNullException( "SelectedTab" );
				}
				if( !m_tc.Contains( value ) ) {
					throw new ArgumentException( "Tab doesn't belong to control" );
				}
				// изменение активного Tab'a
				change_tab( m_tc.IndexOf( value ), m_selectedIndex );
				// обновление расположения дочерних контролов на текущем BrowserControl'е,
				// с удалением всех имеющихся
				update_layout( true );
			}
		}

		/// <summary>
		/// Возвращает объект типа ToolStrip компоненты
		/// </summary>
		public ToolStrip ToolStrip {
			get {
				return tsMain;
			}
		}
		#endregion

		#region private methods
		/// <summary>
		/// Изменение активного Tab'а
		/// </summary>
		/// <param name="index">предполагаемый новый индекс</param>
		/// <param name="prev">индекс предыдущего</param>
		/// <returns>true в случае успешности изменения активного Tab'a, 
		/// иначе false</returns>
		private bool change_tab( int index, int prev )
		{
			// проверка существования подписчиков на уведомление
			// о попытке деактивации Tab'a
			if( Deselecting != null ) {
				// создание параметров события
				BrowserControlCancelEventArgs e =
					new BrowserControlCancelEventArgs(
						m_tc[prev], prev, true, BrowserControlAction.Deselecting );
				// генерация события
				Deselecting( this, e );
				// проверка необходимости прекращения выполнения 
				// операции изменения активного Tab'a
				if( e.Cancel ) { return false; }
			}
			
			// проверка существования подписчиков на уведомление
			// о попытке активации Tab'a
			if( Selecting != null ) {
				// создание параметров события
				BrowserControlCancelEventArgs e =
					new BrowserControlCancelEventArgs(
						m_tc[index], index, true, BrowserControlAction.Selecting );
				// генерация события
				Selecting( this, e );
				// проверка необходимости прекращения выполнения 
				// операции изменения активного Tab'a
				if( e.Cancel ) { return false; }
			}
			
			// изменение активного Tab'a
			m_selectedIndex = index;

			// проверка существования подписчиков на уведомление
			// о деактивации Tab'a
			if( Deselected != null ) {
				// создание параметров события
				BrowserControlEventArgs e =
						new BrowserControlEventArgs(
							m_tc[prev], prev, BrowserControlAction.Deselected );
				// генерация события
				Deselected( this, e );
			}
			
			// изменяем отображение деактивированной кнопки 
			m_tc[prev].m_button.Font =
				new Font( m_tc[prev].m_button.Font, new FontStyle());
			
			// проверка существования подписчиков на уведомление
			// о активации Tab'a
			if( Selected != null ) {
				// создание параметров события
				BrowserControlEventArgs e =
						new BrowserControlEventArgs(
							m_tc[m_selectedIndex], m_selectedIndex, BrowserControlAction.Selected );
				// генерация события
				Selected( this, e );
			}

			// изменяем отображение активной кнопки
			m_tc[m_selectedIndex].m_button.Font =
				new Font( m_tc[m_selectedIndex].m_button.Font, FontStyle.Bold );

			// проверка существования подписчиков на событие
			if( SelectedIndexChanged != null ) {
				// генерация события
				SelectedIndexChanged( this, new EventArgs() );
			}

			// возврат успешности выполнения процедуры
			return true;
		}
		
		/// <summary>
		/// Изменение отображения дочерних визуальных контролов
		/// </summary>
		/// <param name="fullUpdate">если true, то перед выполнением операции
		/// удаляются все дочерние визуальные контролы, иначе изменяется 
		/// расположение текущих</param>
		private void update_layout( bool fullUpdate )
		{
			// проверка на необходимость удаления дочерних контролов
			if( fullUpdate ) {
				// в цикле перебираются дочерние контролы
				for( int i = Controls.Count -1; i >= 0; i-- ) {
					// Удалять необходимо только контролы 2-х типов
					if( ( Controls[i] is Tab ) || ( Controls[i] is TreeView ) ) {
						// удаляем контрол
						this.Controls.Remove( Controls[i] );
					}
				}
			}

			// проверка на то, что есть Tab'ы, которые нужно отображать
			if( m_tc.Count > 0 ) {
				// верхняя граница расположения дерева
				int treeTop = tsMain.Bottom;
				// нижняя граница -||-
				int treeBottom = Bottom;

				int index = 0;

				// перебор кнопок "закладок" начиная с первой и до текущей активной
				for( int i = 0; i < m_tc.Count && i <= m_selectedIndex; i++ ) {
					// для сохранения верхней координаты текущего расположения дерева
					int top;

					// если первая сверху кнопка, то верхняя координата -
					// низ Toolbar'a, иначе - низ предыдущей кнопки
					if( i > 0 ) {
						top = m_tc[i - 1].Bottom;
					} else {
						top = tsMain.Bottom;
					}

					// задание размера, расположения и индекса для Tab'a
					m_tc[i].Location = new Point( 0, top );
					m_tc[i].Size = new Size( Width, BUTTON_HEIGHT );
					m_tc[i].TabIndex = index;
					
					// проверка необходимости добавления дочернего контрола
					if( fullUpdate ) {
						// добавляем дочерний контрол
						this.Controls.Add( m_tc[i] );
					}

					// сохранение верхней границы для "дерева"
					treeTop = m_tc[i].Bottom;
				}
				
				// инициализация порядкового индекса дочерних контролов
				index = m_tc.Count;
				// перебор Tab'ов в обратном порядке до текущей выделеной
				for( int i = m_tc.Count - 1; i > m_selectedIndex; i-- ) {
					int top;
					// если последний Tab, то верх текущего Tab'a выше на 
					// BUTTON_HEIGHT оносительно низа контрола, иначе относительно верха 
					// следующего Tab'a
					if( i + 1 < m_tc.Count ) {
						top = Math.Max( m_tc[i + 1].Top - BUTTON_HEIGHT,
								   m_tc[m_selectedIndex].Bottom + BUTTON_HEIGHT );
					} else {
						top = Math.Max( Bottom - BUTTON_HEIGHT,
									m_tc[m_selectedIndex].Bottom + BUTTON_HEIGHT );
					}

					// задание размера, расположения и индекса для Tab'a
					m_tc[i].Location = new Point( 0, top );
					m_tc[i].Size = new Size( Width, BUTTON_HEIGHT );
					m_tc[i].TabIndex = index;
					
					// проверка необходимости добавления дочернего контрола
					if( fullUpdate ) {
						// добавляем дочерний контрол
						this.Controls.Add( m_tc[i] );
					}
					// уменьшение индекса
					index--;
					// запоминание нижней границы для "дерева"
					treeBottom = top;
				}

				// проверка необходимости добавления дочернего контрола
				if( fullUpdate ) {
					// добавляем дочерний контрол
					this.Controls.Add( m_tc[m_selectedIndex].TreeView );
				}
				// задание размера, расположения и индекса для "дерева"
				m_tc[m_selectedIndex].TreeView.Location =
					new System.Drawing.Point( 0, treeTop );
				m_tc[m_selectedIndex].TreeView.Size =
					new System.Drawing.Size( Width, treeBottom - treeTop );
				m_tc[m_selectedIndex].TreeView.TabIndex = m_selectedIndex + 1;
			}
		}

		// обработка события изменения активного Tab'a.
		private void on_tab_select( Object sender, EventArgs e )
		{
			// проверка входных параметров
			if( sender == null ) {
				throw new ArgumentNullException( "sender" );
			}
			// проверка источника генерации события
			if( !(sender is Tab) ) {
				throw new ArgumentException( "Sender is not Tab" );
			}
			// Tab, которій сгенерировал событие должен принадлежать текущему BrowserControl'у
			if( !m_tc.Contains( (Tab) sender ) ) {
				throw new ArgumentException( "Sender doesn't belong to control" );
			}
			
			// изменение активного Tab'a
			change_tab( m_tc.IndexOf( (Tab) sender ), m_selectedIndex );
			// обновление расположения дочерних контролов на текущем BrowserControl'е,
			// с удалением всех имеющихся
			update_layout( true );
		}
		
		//
		// Обработка события о изменении коллекции Tab'ов
		//
		private void on_tabs_change( Object sender, EventArgs e )
		{
			// источником события должна быть коллекция, что принадлежит текущему
			// BrowserControl'у
			if( sender.Equals( m_tc ) ) {
				// обновление расположения дочерних контролов на текущем BrowserControl'е,
				// с удалением всех имеющихся
				update_layout( true ); 
			}
		}
		#endregion

		#region parent override 
		//
		// Перекрытие метода, генерирующего событие о изменении размера контрола
		//
		protected override void OnResize( EventArgs e )
		{
			// вызов родительского метода
			base.OnResize( e );

			// обновление расположения компонент на контроле
			update_layout( false );
		}
		#endregion

		/// <summary>
		/// Определение значений, которые представляю события BrowserControl'a
		/// </summary>
		public enum BrowserControlAction
		{
			/// <summary>
			/// представление события BrowserControl.Deselected.
			/// </summary>
			Deselected,
			/// <summary>
			/// представление события BrowserControl.Deselecting.
			/// </summary>
			Deselecting,
			/// <summary>
			/// представление события BrowserControl.Selected.
			/// </summary>
			Selected,
			/// <summary>
			/// представление события BrowserControl.Selecting.
			/// </summary>
			Selecting
		}

		/// <summary>
		/// Представление информациия для BrowserControl.Selecting и BrowserControl.Deselecting события TabControl'a
		/// </summary>
		public class BrowserControlCancelEventArgs : CancelEventArgs
		{

			private BrowserControlAction m_action;
			private Tab m_tab;
			private int m_tabIndex;

			/// <summary>
			/// Конструктор по-умолчанию
			/// </summary>
			/// <param name="tab">ссылка на объект типа Tab, к которому 
			/// относится событие</param>
			/// <param name="tabIndex">Начинающийся с 0, индекс Tab'a,
			/// который генерирует событие</param>
			/// <param name="cancel">true, чтобы отменить операцию,
			/// иначе false</param>
			/// <param name="action">одно из значений BrowserControlAction</param>
			public BrowserControlCancelEventArgs( Tab tab,
										int tabIndex,
										bool cancel,
										BrowserControlAction action )
			{
				if( tab == null )
					throw new ArgumentNullException( "tab" );

				m_tab = tab;
				m_tabIndex = tabIndex;
				m_action = action;
			}

			public BrowserControlAction Action
			{
				get { return m_action; }
			}

			public Tab TabPage
			{
				get { return m_tab; }
			}

			public int TabIndex
			{
				get { return m_tabIndex; }
			}
		}


		/// <summary>
		/// Представление информациия для BrowserControl.Selected и BrowserControl.Deselected события TabControl'a
		/// </summary>
		public class BrowserControlEventArgs : EventArgs
		{

			private BrowserControlAction m_action;
			private Tab m_tab;
			private int m_tabIndex;

			/// <summary>
			/// Конструктор по-умолчанию
			/// </summary>
			/// <param name="tab">ссылка на объект типа Tab, к которому 
			/// относится событие</param>
			/// <param name="tabIndex">Начинающийся с 0, индекс Tab'a,
			/// который генерирует событие</param>
			/// <param name="action">одно из значений BrowserControlAction</param>
			public BrowserControlEventArgs( Tab tab,
										int tabIndex,
										BrowserControlAction action )
			{
				if( tab == null )
					throw new ArgumentNullException( "tab" );

				m_tab = tab;
				m_tabIndex = tabIndex;
				m_action = action;
			}

			public BrowserControlAction Action
			{
				get { return m_action; }
			}

			public Tab TabPage
			{
				get { return m_tab; }
			}

			public int TabIndex
			{
				get { return m_tabIndex; }
			}
		}
		
		/// <summary>
		/// Содержит коллекцию объектов типа Tab.
		/// </summary>
		public class TabCollection : IList<Tab>
		{
			// список с Tab'ами
			List<Tab> m_Tabs = new List<Tab>();
			// ссылка на родительский контрол
			TreeViewer m_bc;
			
			/// <summary>
			/// событие генерируемое при внесении изменений в коллекцию
			/// </summary>
			internal event EventHandler CollectionChanged;

			/// <summary>
			/// Конструктор по-умолчанию.
			/// </summary>
			/// <param name="bc">ссылка на родительский объект типа BrowserConrtrol</param>
			public TabCollection( TreeViewer bc )
			{
				// проверка входных параметров
				if( bc == null )
					throw new ArgumentNullException( "bc" );
				// сохранение ссылки на родительский объект
				m_bc = bc;
			}

			#region IList<Tab> Members
			/// <summary>
			/// Возвращает индекс указанного Tab'a в коллекции
			/// </summary>
			/// <param name="item">объект типа Tab который 
			/// необходимо найти в коллекции</param>
			/// <returns>Начинающийся с 0 индекс Tab'a в коллекции; -1, если отсутствует</returns>
			public int IndexOf( Tab item )
			{
				return m_Tabs.IndexOf( item );
			}

			/// <summary>
			/// Вставляет существующий объект Tab в коллекцию по указаному индексу
			/// </summary>
			/// <param name="index">Tab, который необходимо добавить в коллекцию</param>
			/// <param name="item">Начинающийся с 0 индекс, по которому необходимо 
			/// произвести вставку в коллекцию</param>
			public void Insert( int index, Tab item )
			{
				// вставка Tab'а в коллекцию
				m_Tabs.Insert( index, item );
				// подписка на уведомление о смене активной закладки
				item.OnTabSelect += new Tab.ON_TAB_SELECT( m_bc.on_tab_select );
				
				// проверка существования подписчика на событие о изменении коллекции
				if( CollectionChanged != null ) { 
					// генерация события
					CollectionChanged( this, new EventArgs() ); 
				}
			}

			/// <summary>
			/// Удаляет Tab с указанным индексом из коллекции
			/// </summary>
			/// <param name="index">Начинающийся с 0 индекс Tab'a в коллекции,
			/// который необходимо удалить</param>
			public void RemoveAt( int index )
			{
				// проверка диапазона переданного индекса
				if( (index < 0) || (index > m_Tabs.Count) )
					throw new ArgumentOutOfRangeException( "index" );
				// отписывание от сбытий удаляемого Tab'a
				m_Tabs[index].OnTabSelect -= new Tab.ON_TAB_SELECT( m_bc.on_tab_select );
				// удаляем из коллекции
				m_Tabs.RemoveAt( index );
				
				// проверка существования подписчика на событие о изменении коллекции
				if( CollectionChanged != null ) {
					// генерация события 
					CollectionChanged( this, new EventArgs() ); 
				}
			}

			/// <summary>
			/// Возвращает/Устанавливает Tab в коллекции
			/// </summary>
			/// <param name="index">Начинающийся с 0 индекс Tab'a, 
			/// который возвращается/устанавливается</param>
			/// <returns>Tab по указанному индексу</returns>
			public Tab this[int index]
			{
				get {
					// проверка диапазона переданного индекса
					if( (index < 0) || (index > m_Tabs.Count) )
						throw new ArgumentOutOfRangeException( "index" );
					// возвращение Tab'a из коллекции
					return m_Tabs[index];
				} set {
					// проверка входного параметра
					if( value == null ) {
						throw new ArgumentNullException( "value" );
					}
					// проверка диапазона переданного индекса
					if( (index < 0) || (index > m_Tabs.Count) ) {
						throw new ArgumentOutOfRangeException( "index" );
					}
					// отписка от событий "старого" Tab'a
					m_Tabs[index].OnTabSelect -= new Tab.ON_TAB_SELECT( m_bc.on_tab_select );
					// присваивание нового Tab'a
					m_Tabs[index] = value;
					// подписка на события нового Tab'a
					m_Tabs[index].OnTabSelect += new Tab.ON_TAB_SELECT( m_bc.on_tab_select );
					
					// проверка существования подписчика на событие о изменении коллекции
					if( CollectionChanged != null ) { 
						// генерация события
						CollectionChanged( this, new EventArgs() ); 
					}
				}
			}

			#endregion

			#region ICollection<Tab> Members
			/// <summary>
			/// Добавление Tab'a в коллекцию
			/// </summary>
			/// <param name="item">Tab, который необходимо добавить</param>
			public void Add( Tab item )
			{
				// проверка входного параметра
				if( item == null )
					throw new ArgumentNullException( "item" );
				
				// добавляем новый Tab
				m_Tabs.Add( item );

				// подписка на уведомление о смене активной закладки
				item.OnTabSelect += new Tab.ON_TAB_SELECT( m_bc.on_tab_select );
				
				// выбирает таб, если он единственный
				if( m_Tabs.Count == 1 ) {
					item.select();
				}
				
				// проверка существования подписчика на событие о изменении коллекции
				if( CollectionChanged != null ) {
					// генерация события 
					CollectionChanged( this, new EventArgs() ); 
				}
			}

			/// <summary>
			/// Удаление всех Tab'ов из коллекции
			/// </summary>
			public void Clear()
			{
				// итерация по всей коллекции
				foreach( Tab tab in m_Tabs ) {
					// отписка от событий Tab'ов
					tab.OnTabSelect -= new Tab.ON_TAB_SELECT( m_bc.on_tab_select );
				}
				// очистка коллекции
				m_Tabs.Clear();
				
				// проверка существования подписчика на событие о изменении коллекции
				if( CollectionChanged != null ) {
					// генерация события 
					CollectionChanged( this, new EventArgs() ); 
				}
			}

			/// <summary>
			/// Опредение приналежности Tab'aколлекции
			/// </summary>
			/// <param name="item">Tab, которій необходимо найти в коллекции</param>
			/// <returns>true, если коллекция содержит Tab, иначе false</returns>
			public bool Contains( Tab item )
			{
				return m_Tabs.Contains( item );
			}


			/// <summary>
			/// Копирует коллекцию в одномерный масив
			/// </summary>
			/// <param name="array">массив в который необходимо скопировать коллекцию</param>
			/// <param name="arrayIndex">Начинающийся с 0 индекс начала в массиве,
			/// с которого производить вставку</param>
			public void CopyTo( Tab[] array, int arrayIndex )
			{
				m_Tabs.CopyTo( array, arrayIndex );
			}
			
			/// <summary>
			/// Возвращает количество Tab'ов в коллекции
			/// </summary>
			public int Count
			{
				get { return m_Tabs.Count; }
			}

			/// <summary>
			/// Возвращает значение, которое указывает пожно ли изменять коллекцию
			/// </summary>
			public bool IsReadOnly
			{
				get { return false; }
			}

			/// <summary>
			/// Удаляет Tab из коллекции
			/// </summary>
			/// <param name="item">Tab, который необходимо удалить</param>
			/// <returns>успешность выполнения операции</returns>
			public bool Remove( Tab item )
			{
				// проверка входного параметра
				if( item == null )
					throw new ArgumentNullException( "item" );

				// отписка от уведомления о смене активной закладки
				item.OnTabSelect -= new Tab.ON_TAB_SELECT( m_bc.on_tab_select );
				
				// удаляем из коллекции и запоминаем результат операции
				bool result = m_Tabs.Remove( item );

				// проверка существования подписчика на событие о изменении коллекции
				if( CollectionChanged != null ) { 
					// генерация события
					CollectionChanged( this, new EventArgs() ); 
				}

				return result;
			}

			#endregion

			#region IEnumerable<Tab> Members
			/// <summary>
			/// Returns an enumeration of all the tab pages in the collection.
			/// </summary>
			/// <returns>An System.Collections.IEnumerator for the 
			/// TabCollection.</returns>
			public IEnumerator<Tab> GetEnumerator()
			{
				return m_Tabs.GetEnumerator();
			}

			#endregion

			#region IEnumerable Members
			/// <summary>
			/// Returns an enumerator that iterates through a collection.
			/// </summary>
			/// <returns>An System.Collections.IEnumerator object that 
			/// can be used to iterate through the collection.</returns>
			System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
			{
				return ((IEnumerable<Tab>) m_Tabs).GetEnumerator();
			}

			#endregion
		}

		/// <summary>
		/// Реализация "закладки" для BrowserControl'а
		/// </summary>
		public class Tab : UserControl
		{
			private	string		m_tooltip = "";
			private	string		m_caption = "";
			private	TreeView	m_TreeView = new TreeView();
			// чтобы контрол смог изменить стиль отображения
			internal Button		m_button = new Button();
			
			internal delegate void ON_TAB_SELECT( Object sender, EventArgs e );

			// события генерируемое при нажатии на ассоциированую кнопку
			internal event ON_TAB_SELECT OnTabSelect;

			/// <summary>
			/// Возвращает/устанавливает TreeView компоненты
			/// </summary>
			public TreeView TreeView
			{
				get { return m_TreeView; }
			}

			/// <summary>
			/// Возвращает/устанавливает значение подсказки компоненты
			/// </summary>
			public string ToolTip
			{
				get { return m_tooltip; }
				set { m_tooltip = value; }
			}

			/// <summary>
			/// Конструктор по умолчанию
			/// </summary>
			/// <param name="caption">заголовок компоненты</param>
			public Tab( string caption )
				: base()
			{
				// сохранение загаловка
				m_caption = caption;
				
				// инициализация компоненты
				InitializeComponent();
				// подписка на событие нажатия кнопки
				m_button.Click += new System.EventHandler( this.button_click );
			}
			
			//
			// для выбора данной кнопки
			//
			internal void select()
			{
				button_click( this, new EventArgs());	
			}
			

			//
			// Генерация события при нажатии на ассоциированую кнопку
			//
			private void button_click( Object sender, EventArgs e )
			{
				// если существуют подписчики - событие генерируется
				if( OnTabSelect != null ) { OnTabSelect( this, new EventArgs() ); }
			}

			//
			// Задание заголовка кнопки ("закладки")
			//
			// TODO: Сделать динамическое обновление заголовка по таймеру
			private void set_caption()
			{
				m_button.Text =
					string.Format( "{0} ({1})",
								   m_caption,
								   m_TreeView.Nodes.Count );
			}

			/// <summary>
			///  Свойство для полученя/присваивания заголовка
			/// </summary>
			public string Caption
			{
				get { return m_caption; }
				set {
					// проверка входного параметра
					if( string.IsNullOrEmpty( value ) )
						throw new ArgumentNullException( "Caption" );
					// запоминание входного параметра
					m_caption = value;
					// форматирование заголовка
					set_caption();
				}
			}

			protected override void OnResize( EventArgs e )
			{
				m_button.Size = base.Size;
				base.OnResize( e );
			}

			#region Designer
			/// <summary> 
			/// Required designer variable.
			/// </summary>
			private System.ComponentModel.IContainer components = null;

			/// <summary> 
			/// Required method for Designer support - do not modify 
			/// the contents of this method with the code editor.
			/// </summary>
			private void InitializeComponent()
			{
				System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager( typeof( TreeViewer ) );
				this.SuspendLayout();

				// инициализация свойств кнопки
				m_button.Text = m_caption;
				m_button.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
				m_button.Dock = DockStyle.Fill;
				m_button.FlatStyle = FlatStyle.Flat;

				this.Controls.Add( m_button );
				this.Name = "TabControl";
				this.Size = new System.Drawing.Size(0, 0);
				this.ResumeLayout( false );
				this.PerformLayout();
			}

			/// <summary> 
			/// Clean up any resources being used.
			/// </summary>
			/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
			protected override void Dispose( bool disposing )
			{
				if( disposing && (components != null) ) {
					components.Dispose();
				}
				base.Dispose( disposing );
			}
			#endregion
		}
	}
}