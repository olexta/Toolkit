//----------------------------------------------------------------------
//
// SVN:			$Id$
//
//----------------------------------------------------------------------

using System;
using System.Reflection;
using System.Resources;
using System.Globalization;
using System.Drawing;

namespace Toolkit.Controls
{
	/// <summary>
	/// Клас для спрощення роботи з ресурсами
	/// </summary>
	public class ResourceHelper
	{
		private ResourceManager m_rsmgr;

		/// <summary>
		/// Контруктор, для використання ресурсів, що зберігаються
		/// окрумо від збірок(assembly)
		/// </summary>
		/// <param name="baseName">
		/// Базове ім'я реурсу. Наприклад для App.MyResource.uk-UA.resource,
		/// базовим буде App.MyResource
		/// </param>
		/// <param name="dir">
		/// Шлях до директорії в якій знаходяться файли ресурсів
		/// </param>
		public ResourceHelper( string baseName, string dir )
		{
			m_rsmgr = ResourceManager.
				CreateFileBasedResourceManager( baseName, dir, null );
		}

		/// <summary>
		/// Конструктор для використання ресурсів,
		/// що інтегровані в збірку(assembly)
		/// </summary>
		/// <param name="source">
		/// Тип, інформація про який буде використана для знаходження ресурсів
		/// </param>
		public ResourceHelper( Type source )
		{
			m_rsmgr = new ResourceManager( source );
		}

		/// <summary>
		/// Конструктор для використання ресурсів,
		/// що інтегровані в збірку(assembly)
		/// </summary>
		/// <param name="baseName">
		/// Базове ім'я реурсу. Наприклад для App.MyResource.uk-UA.resource,
		/// базовим буде App.MyResource
		/// </param>
		/// <param name="assembly">
		/// Збірка(assembly) в якій зберігаються ресурси
		/// </param>
		public ResourceHelper( string baseName, Assembly assembly )
		{
			m_rsmgr = new ResourceManager( baseName, assembly );
		}

		/// <summary>
		/// Повертає строкове значення за вказаним ідентифікатором
		/// </summary>
		/// <param name="name">
		/// Ідентифікатор
		/// </param>
		public string GetString( string name )
		{
			return m_rsmgr.GetString( name );
		}

		/// <summary>
		/// Повертає строкове значення за вказаним ідентифікатором
		/// </summary>
		/// <param name="name">
		/// Ідентифікатор
		/// </param>
		/// <param name="ci">
		/// Локаль
		/// </param>
		public string GetString( string name, CultureInfo ci )
		{
			return m_rsmgr.GetString( name, ci );
		}

		/// <summary>
		/// Повертає об'єкт за вказаним ідентифікатором
		/// </summary>
		/// <param name="name">
		/// Ідентифікатор
		/// </param>
		public object GetObject( string name )
		{
			return m_rsmgr.GetObject( name );
		}

		/// <summary>
		/// Повертає об'єкт за вказаним ідентифікатором
		/// </summary>
		/// <param name="name">
		/// Ідентифікатор
		/// </param>
		/// <param name="ci">
		/// Локаль
		/// </param>
		public object GetObject( string name, CultureInfo ci )
		{
			return m_rsmgr.GetObject( name, ci );
		}

		/// <summary>
		/// Повертає інонку за вказаним ідентифікатором
		/// </summary>
		/// <param name="name">
		/// Ідентифікатор
		/// </param>
		/// <remarks>
		/// Повертає null, якщо під вказаним ідентифікатором
		/// буде зберагітися об'єкт іншого типу
		/// </remarks>
		public Icon GetIcon( string name )
		{
			return GetObject( name ) as Icon;
		}

		/// <summary>
		/// Повертає інонку за вказаним ідентифікатором
		/// </summary>
		/// <param name="name">
		/// Ідентифікатор
		/// </param>
		/// <param name="ci">
		/// Локаль
		/// </param>
		/// <remarks>
		/// Повертає null, якщо під вказаним ідентифікатором
		/// буде зберагітися об'єкт іншого типу
		/// </remarks>
		public Icon GetIcon( string name, CultureInfo ci )
		{
			return GetObject( name, ci ) as Icon;
		}

		/// <summary>
		/// Повертає зображення за вказаним ідентифікатором
		/// </summary>
		/// <param name="name">
		/// Ідентифікатор
		/// </param>
		/// <remarks>
		/// Повертає null, якщо під вказаним ідентифікатором
		/// буде зберагітися об'єкт іншого типу
		/// </remarks>
		public Image GetImage( string name )
		{
			return GetObject( name ) as Image;
		}

		/// <summary>
		/// Повертає зображення за вказаним ідентифікатором
		/// </summary>
		/// <param name="name">
		/// Ідентифікатор
		/// </param>
		/// <param name="ci">
		/// Локаль
		/// </param>
		/// <remarks>
		/// Повертає null, якщо під вказаним ідентифікатором
		/// буде зберагітися об'єкт іншого типу
		/// </remarks>
		public Image GetImage( string name, CultureInfo ci )
		{
			return GetObject( name, ci ) as Image;
		}

		/// <summary>
		/// Повертає ResourceManager, який використовується для роботи
		/// </summary>		
		public ResourceManager Manager
		{
			get {
				return m_rsmgr;
			}
		}
	}
}
