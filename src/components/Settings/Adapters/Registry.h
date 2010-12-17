/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		Registry.h													*/
/*																			*/
/*	Content:	Definition of Adapters::Registry class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008-2009 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "..\Settings.h"
#include "IAdapter.h"

using namespace System;
using namespace Microsoft;
using namespace System::Threading;
using namespace System::Globalization;


_SETTINGS_BEGIN
namespace Adapters {
	/// <summary>
	/// Adapter that provides runtime access to windows registry.
	/// </summary><remarks><para>
	/// Runtime access means that all (get and set value) operations
	/// use direct registry API calls, so force Load/Save calls are
	/// not necessary. Therefore loading and saving hierarchy structure
	/// require manual Load/Save calls.</para><para>
	/// This class provides ability to store following types in
	/// registry: bool, int, double, DateTime and String. Value of
	/// each type except int will be translated to string with
	/// specified format and save. Restore operation will attempt to
	/// determine type of stored value by string format. If no format
	/// is acceptable than value will be restored as String.
	/// </para></remarks>
	public ref class Registry sealed : IAdapter
	{
	private:
		static Char				const _del = '\\';
		ReaderWriterLock^		const _lock;

		initonly String				^_name;
		initonly CultureInfo		^_ci;
		initonly Win32::RegistryKey	^_root;
		initonly String				^_key;

		void check_path( String ^path );
		void check_value( Object ^value );
		String^ obj_to_str( Object ^value );
		Object^ str_to_obj( String ^value );
		bool split_location( String ^loc, String^ %key, String^ %val );
		List<String^>^ load_key( String ^subpath );
		bool empty_key( String ^subpath );

	// Adapters::IAdapter
	private:
		virtual Object^ get_value( String ^loc ) sealed =
			IAdapter::default::get;
		virtual void set_value( String ^loc, Object ^val ) sealed =
			IAdapter::default::set;
		virtual String^ get_name( void ) sealed =
			IAdapter::Name::get;
		virtual String^ get_delimeter( void ) sealed =
			IAdapter::Delimeter::get;

		virtual bool remove( String ^loc ) sealed =
			IAdapter::Remove;
		virtual IEnumerable<String^>^ reload( String ^loc ) sealed =
			IAdapter::Reload;
		virtual void flush( String ^loc ) sealed =
			IAdapter::Flush;

	public:
		Registry( String ^key, String ^name );
};
}_SETTINGS_END
