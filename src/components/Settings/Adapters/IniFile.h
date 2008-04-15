﻿/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		IniFile.h													*/
/*																			*/
/*	Content:	Definition of Adapters::IniFile class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright @ 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "..\Settings.h"
#include "IAdapter.h"

using namespace System;
using namespace System::Threading;
using namespace System::Globalization;
using namespace System::Collections::Generic;
using namespace Toolkit::Collections;


_SETTINGS_BEGIN
namespace Adapters {
	///	<summary>
	///	Adapter that provide access to windows INI files.
	///	</summary><remarks>
	/// This class provide ability to store following types in INI
	/// file: bool, int, double, DateTime and String. Each value
	/// will be translated to string with specified format and
	/// save. Restore operation will attempt to determine type of
	/// stored value by string format. If no format is acceptable
	/// than value will be restored as String.
	/// ATTENTION: This class doesn't support transaction approach.
	///	</remarks>
	public ref class IniFile : IAdapter
	{
	private:
		static String^			const _int_f = "0";
		static String^			const _double_f = "0.0##############";
		static String^			const _DateTime_f = "dd/MM/yyyy HH:mm:ss";
		initonly CultureInfo	^_ci;

	private:
		static String^			const _del = "/";
		static unsigned long	const _bufsize = 1024;
		String^					const _filename;
		ReaderWriterLock^		const _lock;

		String					^m_name;
		Map<String^, Object^>	^m_cache;
		
		List<String^>^ get_ini_string( String ^section, String ^key );
		unsigned long set_ini_string( String ^section, String ^key,
									  String ^string );
		void check_path( String ^path );
		void check_value( Object ^value );
		String^ obj_to_str( Object ^value );
		Object^ str_to_obj( String ^value );
		bool split_location( String ^loc, String^ %sec, String^ %key );

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
		IniFile( String ^filename );
		IniFile( String ^filename, String ^name );
};
}_SETTINGS_END