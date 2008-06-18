using System;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;
using System.Collections.Generic;


namespace Toolkit.Controls.TreeViewer
{
	/// <summary>
	/// ��������� ������� Tab'��
	/// </summary>
	[ CLSCompliantAttribute( true ) ]
	public partial class TreeViewer : Control
	{
		#region private declarations
		// ��������� ���������� � �������� Tab'��
		private TabCollection m_tc;
		// ������ �������� Tab'�
		private int m_selectedIndex = 0;
		// ���������, ������� ��������� ������ Tab'��
		private readonly int BUTTON_HEIGHT = System.Windows.Forms.SystemInformation.CaptionHeight;
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
		/// ������� �����������.
		/// </summary>
		public TreeViewer() : base()
		{
			// ������������� ���������
			m_tc = new TabCollection( this );
			// �������� �� ����������� � ��������� ���������
			m_tc.CollectionChanged += new EventHandler( this.on_tabs_change );
			
			// ������������� ���������� ���������
			InitializeComponent();
			
			// ���������� ������������ ��������� �� ��������
			update_layout( false );
		}
		
		#endregion
		
		#region public properties
		/// <summary>
		/// �������� ��������� � Tab'���, ������� ����������� ������� ��������
		/// </summary>
		public TabCollection Tabs
		{
			get { return m_tc; }
		}

		/// <summary>
		/// ���������/��������� ������� �������� ��������� Tab'�
		/// </summary>
		public int SelectedIndex
		{
			get { return m_selectedIndex; }
			set
			{
				// �������� �������� ���������
				if( (value < 0) || (value > m_tc.Count) ) {
					throw new ArgumentOutOfRangeException( "SelectedIndex" );
				}

				// ��������� ��������� Tab'�
				change_tab( value, m_selectedIndex );
				// ���������� ������������ ��������� �� ��������,
				// � ��������� ���� ���������
				update_layout( true );
			}
		}

		/// <summary>
		/// ���������� ���������� Tab'�� � BrowserControl'�
		/// </summary>
		public int TabCount
		{
			get { return m_tc.Count; }
		}

		/// <summary>
		/// �������������/���������� ������� Tab.
		/// ��� ��������� ���������� �������� �� ������������� ������������� Tab'a
		/// ������� ���������� BrowserControl'�
		/// ��� ����������� ��������� Tab'a ������������ null
		/// � ������ ���������� ������.
		/// </summary>
		public Tab SelectedTab
		{
			get
			{
				// �������� �� ������� Tab'�� � ������� BrowserControl'�
				if( m_tc.Count > 0 ) { return m_tc[m_selectedIndex]; } else { return null; }
			}
			set
			{
				// �������� �������� ���������
				if( (value == null) ) {
					throw new ArgumentNullException( "SelectedTab" );
				}
				if( !m_tc.Contains( value ) ) {
					throw new ArgumentException( "Tab doesn't belong to control" );
				}
				// ��������� ��������� Tab'a
				change_tab( m_tc.IndexOf( value ), m_selectedIndex );
				// ���������� ������������ �������� ��������� �� ������� BrowserControl'�,
				// � ��������� ���� ���������
				update_layout( true );
			}
		}

		/// <summary>
		/// ���������� ������ ���� ToolStrip ����������
		/// </summary>
		public ToolStrip ToolStrip {
			get {
				return tsMain;
			}
		}
		#endregion

		#region private methods
		/// <summary>
		/// ��������� ��������� Tab'�
		/// </summary>
		/// <param name="index">�������������� ����� ������</param>
		/// <param name="prev">������ �����������</param>
		/// <returns>true � ������ ���������� ��������� ��������� Tab'a, 
		/// ����� false</returns>
		private bool change_tab( int index, int prev )
		{
			// �������� ������������� ����������� �� �����������
			// � ������� ����������� Tab'a
			if( Deselecting != null ) {
				// �������� ���������� �������
				BrowserControlCancelEventArgs e =
					new BrowserControlCancelEventArgs(
						m_tc[prev], prev, true, BrowserControlAction.Deselecting );
				// ��������� �������
				Deselecting( this, e );
				// �������� ������������� ����������� ���������� 
				// �������� ��������� ��������� Tab'a
				if( e.Cancel ) { return false; }
			}
			
			// �������� ������������� ����������� �� �����������
			// � ������� ��������� Tab'a
			if( Selecting != null ) {
				// �������� ���������� �������
				BrowserControlCancelEventArgs e =
					new BrowserControlCancelEventArgs(
						m_tc[index], index, true, BrowserControlAction.Selecting );
				// ��������� �������
				Selecting( this, e );
				// �������� ������������� ����������� ���������� 
				// �������� ��������� ��������� Tab'a
				if( e.Cancel ) { return false; }
			}
			
			// ��������� ��������� Tab'a
			m_selectedIndex = index;

			// �������� ������������� ����������� �� �����������
			// � ����������� Tab'a
			if( Deselected != null ) {
				// �������� ���������� �������
				BrowserControlEventArgs e =
						new BrowserControlEventArgs(
							m_tc[prev], prev, BrowserControlAction.Deselected );
				// ��������� �������
				Deselected( this, e );
			}
			
			// �������� ����������� ���������������� ������ 
			m_tc[prev].m_button.Font = 
				new Font( m_tc[prev].m_button.Font, new FontStyle() );
			
			// �������� ������������� ����������� �� �����������
			// � ��������� Tab'a
			if( Selected != null ) {
				// �������� ���������� �������
				BrowserControlEventArgs e =
						new BrowserControlEventArgs(
							m_tc[m_selectedIndex], m_selectedIndex, BrowserControlAction.Selected );
				// ��������� �������
				Selected( this, e );
			}

			// �������� ����������� �������� ������
			m_tc[m_selectedIndex].m_button.Font = 
				new Font( m_tc[m_selectedIndex].m_button.Font, FontStyle.Bold );

			// �������� ������������� ����������� �� �������
			if( SelectedIndexChanged != null ) {
				// ��������� �������
				SelectedIndexChanged( this, new EventArgs() );
			}

			// ������� ���������� ���������� ���������
			return true;
		}
		
		/// <summary>
		/// ��������� ����������� �������� ���������� ���������
		/// </summary>
		/// <param name="fullUpdate">���� true, �� ����� ����������� ��������
		/// ��������� ��� �������� ���������� ��������, ����� ���������� 
		/// ������������ �������</param>
		private void update_layout( bool fullUpdate )
		{
			// �������� �� ������������� �������� �������� ���������
			if( fullUpdate ) {
				// � ����� ������������ �������� ��������
				for( int i = Controls.Count -1; i >= 0; i-- ) {
					// ������� ���������� ������ �������� 2-� �����
					if( ( Controls[i] is Tab ) || ( Controls[i] is TreeView ) ) {
						// ������� �������
						this.Controls.Remove( Controls[i] );
					}
				}
			}

			// �������� �� ��, ��� ���� Tab'�, ������� ����� ����������
			if( m_tc.Count > 0 ) {
				// ������� ������� ������������ ������
				int treeTop = tsMain.Bottom;
				// ������ ������� -||-
				int treeBottom = Bottom;

				int index = 0;

				// ������� ������ "��������" ������� � ������ � �� ������� ��������
				for( int i = 0; i < m_tc.Count && i <= m_selectedIndex; i++ ) {
					// ��� ���������� ������� ���������� �������� ������������ ������
					int top;

					// ���� ������ ������ ������, �� ������� ���������� -
					// ��� Toolbar'a, ����� - ��� ���������� ������
					if( i > 0 ) {
						top = m_tc[i - 1].Bottom;
					} else {
						top = tsMain.Bottom;
					}

					// ������� �������, ������������ � ������� ��� Tab'a
					m_tc[i].Location = new Point( 0, top );
					m_tc[i].Size = new Size( Width, BUTTON_HEIGHT );
					m_tc[i].TabIndex = index;
					
					// �������� ������������� ���������� ��������� ��������
					if( fullUpdate ) {
						// ��������� �������� �������
						this.Controls.Add( m_tc[i] );
					}

					// ���������� ������� ������� ��� "������"
					treeTop = m_tc[i].Bottom;
				}
				
				// ������������� ����������� ������� �������� ���������
				index = m_tc.Count;
				// ������� Tab'�� � �������� ������� �� ������� ���������
				for( int i = m_tc.Count - 1; i > m_selectedIndex; i-- ) {
					int top;
					// ���� ��������� Tab, �� ���� �������� Tab'a ���� �� 
					// BUTTON_HEIGHT ����������� ���� ��������, ����� ������������ ����� 
					// ���������� Tab'a
					if( i + 1 < m_tc.Count ) {
						top = Math.Max( m_tc[i + 1].Top - BUTTON_HEIGHT,
								   m_tc[m_selectedIndex].Bottom + BUTTON_HEIGHT );
					} else {
						top = Math.Max( Bottom - BUTTON_HEIGHT,
									m_tc[m_selectedIndex].Bottom + BUTTON_HEIGHT );
					}

					// ������� �������, ������������ � ������� ��� Tab'a
					m_tc[i].Location = new Point( 0, top );
					m_tc[i].Size = new Size( Width, BUTTON_HEIGHT );
					m_tc[i].TabIndex = index;
					
					// �������� ������������� ���������� ��������� ��������
					if( fullUpdate ) {
						// ��������� �������� �������
						this.Controls.Add( m_tc[i] );
					}
					// ���������� �������
					index--;
					// ����������� ������ ������� ��� "������"
					treeBottom = top;
				}

				// �������� ������������� ���������� ��������� ��������
				if( fullUpdate ) {
					// ��������� �������� �������
					this.Controls.Add( m_tc[m_selectedIndex].TreeView );
				}
				// ������� �������, ������������ � ������� ��� "������"
				m_tc[m_selectedIndex].TreeView.Location =
					new System.Drawing.Point( 0, treeTop );
				m_tc[m_selectedIndex].TreeView.Size =
					new System.Drawing.Size( Width, treeBottom - treeTop );
				m_tc[m_selectedIndex].TreeView.TabIndex = m_selectedIndex + 1;
			}
		}

		// ��������� ������� ��������� ��������� Tab'a.
		private void on_tab_select( Object sender, EventArgs e )
		{
			// �������� ������� ����������
			if( sender == null ) {
				throw new ArgumentNullException( "sender" );
			}
			// �������� ��������� ��������� �������
			if( !(sender is Tab) ) {
				throw new ArgumentException( "Sender is not Tab" );
			}
			// Tab, ������ ������������ ������� ������ ������������ �������� BrowserControl'�
			if( !m_tc.Contains( (Tab) sender ) ) {
				throw new ArgumentException( "Sender doesn't belong to control" );
			}
			
			// ��������� ��������� Tab'a
			change_tab( m_tc.IndexOf( (Tab) sender ), m_selectedIndex );
			// ���������� ������������ �������� ��������� �� ������� BrowserControl'�,
			// � ��������� ���� ���������
			update_layout( true );
		}
		
		//
		// ��������� ������� � ��������� ��������� Tab'��
		//
		private void on_tabs_change( Object sender, EventArgs e )
		{
			// ���������� ������� ������ ���� ���������, ��� ����������� ��������
			// BrowserControl'�
			if( sender.Equals( m_tc ) ) {
				// ���������� ������������ �������� ��������� �� ������� BrowserControl'�,
				// � ��������� ���� ���������
				update_layout( true ); 
			}
		}
		#endregion

		#region parent override 
		//
		// ���������� ������, ������������� ������� � ��������� ������� ��������
		//
		protected override void OnResize( EventArgs e )
		{
			// ����� ������������� ������
			base.OnResize( e );

			// ���������� ������������ ��������� �� ��������
			update_layout( false );
		}
		#endregion

		/// <summary>
		/// ����������� ��������, ������� ����������� ������� BrowserControl'a
		/// </summary>
		public enum BrowserControlAction
		{
			/// <summary>
			/// ������������� ������� BrowserControl.Deselected.
			/// </summary>
			Deselected,
			/// <summary>
			/// ������������� ������� BrowserControl.Deselecting.
			/// </summary>
			Deselecting,
			/// <summary>
			/// ������������� ������� BrowserControl.Selected.
			/// </summary>
			Selected,
			/// <summary>
			/// ������������� ������� BrowserControl.Selecting.
			/// </summary>
			Selecting
		}

		/// <summary>
		/// ������������� ����������� ��� BrowserControl.Selecting � BrowserControl.Deselecting ������� TabControl'a
		/// </summary>
		public class BrowserControlCancelEventArgs : CancelEventArgs
		{

			private BrowserControlAction m_action;
			private Tab m_tab;
			private int m_tabIndex;

			/// <summary>
			/// ����������� ��-���������
			/// </summary>
			/// <param name="tab">������ �� ������ ���� Tab, � �������� 
			/// ��������� �������</param>
			/// <param name="tabIndex">������������ � 0, ������ Tab'a,
			/// ������� ���������� �������</param>
			/// <param name="cancel">true, ����� �������� ��������,
			/// ����� false</param>
			/// <param name="action">���� �� �������� BrowserControlAction</param>
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
		/// ������������� ����������� ��� BrowserControl.Selected � BrowserControl.Deselected ������� TabControl'a
		/// </summary>
		public class BrowserControlEventArgs : EventArgs
		{

			private BrowserControlAction m_action;
			private Tab m_tab;
			private int m_tabIndex;

			/// <summary>
			/// ����������� ��-���������
			/// </summary>
			/// <param name="tab">������ �� ������ ���� Tab, � �������� 
			/// ��������� �������</param>
			/// <param name="tabIndex">������������ � 0, ������ Tab'a,
			/// ������� ���������� �������</param>
			/// <param name="action">���� �� �������� BrowserControlAction</param>
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
		/// �������� ��������� �������� ���� Tab.
		/// </summary>
		public class TabCollection : IList<Tab>
		{
			// ������ � Tab'���
			List<Tab> m_Tabs = new List<Tab>();
			// ������ �� ������������ �������
			TreeViewer m_bc;
			
			/// <summary>
			/// ������� ������������ ��� �������� ��������� � ���������
			/// </summary>
			internal event EventHandler CollectionChanged;

			/// <summary>
			/// ����������� ��-���������.
			/// </summary>
			/// <param name="bc">������ �� ������������ ������ ���� BrowserConrtrol</param>
			public TabCollection( TreeViewer bc )
			{
				// �������� ������� ����������
				if( bc == null )
					throw new ArgumentNullException( "bc" );
				// ���������� ������ �� ������������ ������
				m_bc = bc;
			}

			#region IList<Tab> Members
			/// <summary>
			/// ���������� ������ ���������� Tab'a � ���������
			/// </summary>
			/// <param name="item">������ ���� Tab ������� 
			/// ���������� ����� � ���������</param>
			/// <returns>������������ � 0 ������ Tab'a � ���������; -1, ���� �����������</returns>
			public int IndexOf( Tab item )
			{
				return m_Tabs.IndexOf( item );
			}

			/// <summary>
			/// ��������� ������������ ������ Tab � ��������� �� ��������� �������
			/// </summary>
			/// <param name="index">Tab, ������� ���������� �������� � ���������</param>
			/// <param name="item">������������ � 0 ������, �� �������� ���������� 
			/// ���������� ������� � ���������</param>
			public void Insert( int index, Tab item )
			{
				// ������� Tab'� � ���������
				m_Tabs.Insert( index, item );
				// �������� �� ����������� � ����� �������� ��������
				item.OnTabSelect += new Tab.ON_TAB_SELECT( m_bc.on_tab_select );
				
				// �������� ������������� ���������� �� ������� � ��������� ���������
				if( CollectionChanged != null ) { 
					// ��������� �������
					CollectionChanged( this, new EventArgs() ); 
				}
			}

			/// <summary>
			/// ������� Tab � ��������� �������� �� ���������
			/// </summary>
			/// <param name="index">������������ � 0 ������ Tab'a � ���������,
			/// ������� ���������� �������</param>
			public void RemoveAt( int index )
			{
				// �������� ��������� ����������� �������
				if( (index < 0) || (index > m_Tabs.Count) )
					throw new ArgumentOutOfRangeException( "index" );
				// ����������� �� ������ ���������� Tab'a
				m_Tabs[index].OnTabSelect -= new Tab.ON_TAB_SELECT( m_bc.on_tab_select );
				// ������� �� ���������
				m_Tabs.RemoveAt( index );
				
				// �������� ������������� ���������� �� ������� � ��������� ���������
				if( CollectionChanged != null ) {
					// ��������� ������� 
					CollectionChanged( this, new EventArgs() ); 
				}
			}

			/// <summary>
			/// ����������/������������� Tab � ���������
			/// </summary>
			/// <param name="index">������������ � 0 ������ Tab'a, 
			/// ������� ������������/���������������</param>
			/// <returns>Tab �� ���������� �������</returns>
			public Tab this[int index]
			{
				get {
					// �������� ��������� ����������� �������
					if( (index < 0) || (index > m_Tabs.Count) )
						throw new ArgumentOutOfRangeException( "index" );
					// ����������� Tab'a �� ���������
					return m_Tabs[index];
				} set {
					// �������� �������� ���������
					if( value == null ) {
						throw new ArgumentNullException( "value" );
					}
					// �������� ��������� ����������� �������
					if( (index < 0) || (index > m_Tabs.Count) ) {
						throw new ArgumentOutOfRangeException( "index" );
					}
					// ������� �� ������� "�������" Tab'a
					m_Tabs[index].OnTabSelect -= new Tab.ON_TAB_SELECT( m_bc.on_tab_select );
					// ������������ ������ Tab'a
					m_Tabs[index] = value;
					// �������� �� ������� ������ Tab'a
					m_Tabs[index].OnTabSelect += new Tab.ON_TAB_SELECT( m_bc.on_tab_select );
					
					// �������� ������������� ���������� �� ������� � ��������� ���������
					if( CollectionChanged != null ) { 
						// ��������� �������
						CollectionChanged( this, new EventArgs() ); 
					}
				}
			}

			#endregion

			#region ICollection<Tab> Members
			/// <summary>
			/// ���������� Tab'a � ���������
			/// </summary>
			/// <param name="item">Tab, ������� ���������� ��������</param>
			public void Add( Tab item )
			{
				// �������� �������� ���������
				if( item == null )
					throw new ArgumentNullException( "item" );
				
				// ��������� ����� Tab
				m_Tabs.Add( item );

				// �������� �� ����������� � ����� �������� ��������
				item.OnTabSelect += new Tab.ON_TAB_SELECT( m_bc.on_tab_select );
				
				// �������� ���, ���� �� ������������
				if( m_Tabs.Count == 1 ) {
					item.select();
				}
				
				// �������� ������������� ���������� �� ������� � ��������� ���������
				if( CollectionChanged != null ) {
					// ��������� ������� 
					CollectionChanged( this, new EventArgs() ); 
				}
			}

			/// <summary>
			/// �������� ���� Tab'�� �� ���������
			/// </summary>
			public void Clear()
			{
				// �������� �� ���� ���������
				foreach( Tab tab in m_Tabs ) {
					// ������� �� ������� Tab'��
					tab.OnTabSelect -= new Tab.ON_TAB_SELECT( m_bc.on_tab_select );
				}
				// ������� ���������
				m_Tabs.Clear();
				
				// �������� ������������� ���������� �� ������� � ��������� ���������
				if( CollectionChanged != null ) {
					// ��������� ������� 
					CollectionChanged( this, new EventArgs() ); 
				}
			}

			/// <summary>
			/// ��������� ������������� Tab'a���������
			/// </summary>
			/// <param name="item">Tab, ������ ���������� ����� � ���������</param>
			/// <returns>true, ���� ��������� �������� Tab, ����� false</returns>
			public bool Contains( Tab item )
			{
				return m_Tabs.Contains( item );
			}


			/// <summary>
			/// �������� ��������� � ���������� �����
			/// </summary>
			/// <param name="array">������ � ������� ���������� ����������� ���������</param>
			/// <param name="arrayIndex">������������ � 0 ������ ������ � �������,
			/// � �������� ����������� �������</param>
			public void CopyTo( Tab[] array, int arrayIndex )
			{
				m_Tabs.CopyTo( array, arrayIndex );
			}
			
			/// <summary>
			/// ���������� ���������� Tab'�� � ���������
			/// </summary>
			public int Count
			{
				get { return m_Tabs.Count; }
			}

			/// <summary>
			/// ���������� ��������, ������� ��������� ����� �� �������� ���������
			/// </summary>
			public bool IsReadOnly
			{
				get { return false; }
			}

			/// <summary>
			/// ������� Tab �� ���������
			/// </summary>
			/// <param name="item">Tab, ������� ���������� �������</param>
			/// <returns>���������� ���������� ��������</returns>
			public bool Remove( Tab item )
			{
				// �������� �������� ���������
				if( item == null )
					throw new ArgumentNullException( "item" );

				// ������� �� ����������� � ����� �������� ��������
				item.OnTabSelect -= new Tab.ON_TAB_SELECT( m_bc.on_tab_select );
				
				// ������� �� ��������� � ���������� ��������� ��������
				bool result = m_Tabs.Remove( item );

				// �������� ������������� ���������� �� ������� � ��������� ���������
				if( CollectionChanged != null ) { 
					// ��������� �������
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
		/// ���������� "��������" ��� BrowserControl'�
		/// </summary>
		public class Tab : UserControl
		{
			private	string		m_tooltip = "";
			private	string		m_caption = "";
			private	TreeView	m_TreeView = new TreeView();
			// ����� ������� ���� �������� ����� �����������
			internal Button		m_button = new Button();
			
			internal delegate void ON_TAB_SELECT( Object sender, EventArgs e );

			// ������� ������������ ��� ������� �� �������������� ������
			internal event ON_TAB_SELECT OnTabSelect;

			/// <summary>
			/// ����������/������������� TreeView ����������
			/// </summary>
			public TreeView TreeView
			{
				get { return m_TreeView; }
			}

			/// <summary>
			/// ����������/������������� �������� ��������� ����������
			/// </summary>
			public string ToolTip
			{
				get { return m_tooltip; }
				set { m_tooltip = value; }
			}

			/// <summary>
			/// ����������� �� ���������
			/// </summary>
			/// <param name="caption">��������� ����������</param>
			public Tab( string caption )
				: base()
			{
				// ���������� ���������
				m_caption = caption;
				
				// ������������� ����������
				InitializeComponent();
				// �������� �� ������� ������� ������
				m_button.Click += new System.EventHandler( this.button_click );
			}
			
			//
			// ��� ������ ������ ������
			//
			internal void select()
			{
				button_click( this, new EventArgs());	
			}
			

			//
			// ��������� ������� ��� ������� �� �������������� ������
			//
			private void button_click( Object sender, EventArgs e )
			{
				// ���� ���������� ���������� - ������� ������������
				if( OnTabSelect != null ) { OnTabSelect( this, new EventArgs() ); }
			}

			//
			// ������� ��������� ������ ("��������")
			//
			// TODO: ������� ������������ ���������� ��������� �� �������
			private void set_caption()
			{
				m_button.Text =
					string.Format( "{0} ({1})",
								   m_caption,
								   m_TreeView.Nodes.Count );
			}

			/// <summary>
			///  �������� ��� ��������/������������ ���������
			/// </summary>
			public string Caption
			{
				get { return m_caption; }
				set {
					// �������� �������� ���������
					if( string.IsNullOrEmpty( value ) )
						throw new ArgumentNullException( "Caption" );
					// ����������� �������� ���������
					m_caption = value;
					// �������������� ���������
					set_caption();
				}
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

				// ������������� ������� ������
				m_button.Text = m_caption;
				m_button.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
				m_button.Dock = DockStyle.Fill;
				m_button.FlatStyle = FlatStyle.Popup;

				this.Controls.Add( m_button );
				this.Name = "TabControl";
				this.Size = new System.Drawing.Size( 203, 315 );
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