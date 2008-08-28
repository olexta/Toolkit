/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		IAdapter.h													*/
/*																			*/
/*	Content:	Definition of IAdapter interface							*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright @ 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "..\Settings.h"

using namespace System;
using namespace System::Collections::Generic;


_SETTINGS_BEGIN
namespace Adapters {
	/// <summary>
	/// Encapsulates the behavior needed for working with
	/// storage adapters.
	/// </summary><remarks>
	/// Each adapter class must implement this interface.
	/// </remarks>
	public interface class IAdapter
	{
		/// <summary>
		/// Gets/puts setting's value that is located by specified path from/to
		/// cache.
		/// </summary>
		property Object^ default[String^] {
			virtual Object^ get( String ^location );
			virtual void set( String ^location, Object ^value );
		}
		/// <summary>
		/// Gets adapter's name.
		/// </summary><remarks>
		/// This have not be class name, but value that will be node's name in
		/// settings tree structure for this instance of Adapter class.
		/// </remarks>
		property String^ Name {
			virtual String^ get( void );
		}
		/// <summary>
		/// Get's string that is used in location pathes as delimiter.
		/// </summary>
		property String^ Delimeter {
			virtual String^ get( void );
		}
		
		/// <summary>
		/// Removes setting value that is located by specified path from cache.
		/// </summary>
		/// <param name="location">Fullpath data location.</param>
		/// <returns>
		/// true if the path is successfully found and value was removed;
		/// otherwise, false.
		/// </returns><remarks>
		/// Must remove all children settings also.
		/// </remarks>
		virtual bool Remove( String ^location );
		
		/// <summary>
		/// Retrieves settings from source to internal cache.
		/// </summary>
		/// <param name="location">Fullpath data location.</param>
		/// <returns>
		/// List of the setting's fullnames that were retrieved.
		/// </returns><remarks>
		/// This function must load all children settings also.
		/// </remarks>
		virtual IEnumerable<String^>^ Reload( String ^location );
		/// <summary>
		/// Flashes settings from internal cache to source.
		/// </summary>
		/// <param name="location">Fullpath data location.</param>
		/// <remarks>
		/// This function must flash all children settings also.
		/// </remarks>
		virtual void Flush( String ^location );
	};
}
_SETTINGS_END