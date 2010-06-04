/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		IniFile.h													*/
/*																			*/
/*	Content:	Definition of Adapters::IniFile class						*/
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
using namespace System::Threading;
using namespace System::Globalization;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace Toolkit::Collections;


_SETTINGS_BEGIN
namespace Adapters {
	/// <summary>
	/// Adapter that provides access to windows INI files.
	/// </summary><remarks>
	/// This class provide ability to store following types in INI
	/// file: bool, int, double, DateTime and String. Each value
	/// will be translated to string with specified format and
	/// save. Restore operation will attempt to determine type of
	/// stored value by string format. If no format is acceptable
	/// than value will be restored as String.
	/// ATTENTION: This class doesn't support transaction approach.
	/// </remarks>
	public ref class IniFile sealed : IAdapter
	{
	private:
		/// <summary>
		/// Retrieves a string from the specified section in an initialization
		/// file.
		/// </summary><returns>
		/// The return value is the number of characters copied to the buffer,
		/// not including the terminating null character. If neither lpAppName
		/// nor lpKeyName is NULL and the supplied destination buffer is too
		/// small to hold the requested string, the string is truncated and
		/// followed by a null character, and the return value is equal to
		/// nSize minus one. If either lpAppName or lpKeyName is NULL and the
		/// supplied destination buffer is too small to hold all the strings,
		/// the last string is truncated and followed by two null characters.
		/// In this case, the return value is equal to nSize minus two.
		///</returns><remarks>
		/// If lpAppName is NULL, GetPrivateProfileString copies all section
		/// names in the specified file to the supplied buffer. If lpKeyName
		/// is NULL, the function copies all key names in the specified
		/// section to the supplied buffer. In either case, each string is
		/// followed by a null character and the final string is followed by a
		/// second null character. If the supplied destination buffer is too
		/// small to hold all the strings, the last string is truncated and
		/// followed by two null characters.
		/// </remarks>
		[DllImport("kernel32.dll", CharSet=CharSet::Auto)]
		static unsigned int GetPrivateProfileString(
													 String^ lpAppName,
													 String^ lpKeyName,
													 String^ lpDefault,
													 array<wchar_t>^ lpReturnedString,
													 unsigned int nSize,
													 String^ lpFileName
												   );
		/// <summary>
		/// Copies a string into the specified section of an initialization
		/// file.
		/// </summary><remarks>
		/// If lpKeyName is NULL, the entire section, including all entries
		/// within the section, is deleted. If lpString is NULL, the key
		/// pointed to by the lpKeyName parameter is deleted.
		/// </remarks>
		[DllImport("KERNEL32.DLL", CharSet=CharSet::Auto)]
		static bool WritePrivateProfileString(
											   String^ lpAppName,
											   String^ lpKeyName,
											   String^ lpString,
											   String^ lpFileName
											 );
	private:
		static Char				const _del = '/';
		static unsigned long	const _bufsize = 1024;
		String^					const _filename;
		ReaderWriterLock^		const _lock;
		initonly String			^_name;
		initonly CultureInfo	^_ci;

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
