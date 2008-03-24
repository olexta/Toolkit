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
		property Object^ default[String^] {
			virtual Object^ get( String ^location );
			virtual void set( String ^location, Object ^value );
		}
		property String^ Name {
			virtual String^ get( void );
		}
		property String^ Delimeter {
			virtual String^ get( void );
		}
		
		virtual bool Remove( String ^location );
		
		virtual IEnumerable<String^>^ Reload( String ^location );
		virtual void Flush( String ^location );
	};
}
_SETTINGS_END