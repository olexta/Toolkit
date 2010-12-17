**************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		IniFile.cpp													*/
/*																			*/
/*	Content:	Implementation of Adapters::IniFile class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008-2009 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "IniFile.h"

using namespace _SETTINGS::Adapters;


//-----------------------------------------------------------------------------
//					Toolkit::Settings::Adapters::IniFile
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Retrieves a string from the specified section in an initialization
// file.
//
// If 'section' is null reference, function load all section names in
// the ini file.
// If 'key' is null reference, function returns all key names in the
// specified section.
//
//-------------------------------------------------------------------
List<String^>^ IniFile::get_ini_string( String ^section, String ^key )
{
	// allocate memory and initialize it to '\0'
	array<wchar_t>	^buf = gcnew array<wchar_t>(_bufsize);
	unsigned int	ret = 0;

	// now read value/keys/sections from ini file
	// until all data go in allocated buffer
	for( unsigned int nSize = _bufsize;
		 (ret = GetPrivateProfileString( section, key, nullptr,
										 buf, nSize, _filename )
		 ) == nSize - (key ? 1 : 2); ) {
		// and allocate new increazed
		buf = gcnew array<wchar_t>(nSize += _bufsize);
	}
	if( !key && (ret > 0) ) ret--;

	// create List of strings that will contain result
	List<String^>	^list = gcnew List<String^>();
	// and fill it with result
	list->AddRange( safe_cast<IEnumerable<String^>^>(
					(gcnew String(buf, 0, ret))->Split( '\0' )) );

	// if ini string value request returns empty
	// array, then more deep processing is needed:
	// check that section contains specified key
	if( (list->Count == 0) && (section != nullptr) && (key != nullptr ) &&
		get_ini_string( section, nullptr )->Contains( key ) ) {
		// this is not default value (key have
		// empty string value): add it to the list
		list->Add( "" );
	}

	return list;
}


//-------------------------------------------------------------------
//
// Copies a string into the specified section of an initialization
// file.
//
// If 'key' is null reference, the entire section, including all
// entries within the section, is deleted.
// If 'string' is null reference, the key pointed to by the 'key'
// parameter is deleted.
//
// If function succeeded, the return value is zero. In other case,
// function return last error code (from 'GetLastError' function).
//
//-------------------------------------------------------------------
unsigned long IniFile::set_ini_string( String ^section, String ^key,
									   String ^string )
{
	// attempt save changes to specified ini file
	if( !WritePrivateProfileString( section, key, string, _filename ) ) {
		// return last error if unsucceeded
		return Marshal::GetLastWin32Error();
	};
	// all right, return 0
	return 0;
}


//-------------------------------------------------------------------
//
// Check for correct adapter path.
//
// Valid path must starts with adapter's delimeter
//
//-------------------------------------------------------------------
void IniFile::check_path( String ^path )
{
	// check for null reference
	if( path == nullptr ) throw gcnew ArgumentNullException("path");

	// check for valid adapter's path (it must
	// starts with adapter's delimeter)
	if( !path->StartsWith( _del.ToString() ) ) {
		// throw argument exception
		throw gcnew ArgumentException(String::Format(
		ERR_PATH_STARTS, path, "adapter", _del ));
	}
}


//-------------------------------------------------------------------
//
// Check for supported value.
//
// Now, IniFile adapter supports following types: bool, int, double,
// DateTime and String.
//
//-------------------------------------------------------------------
void IniFile::check_value( Object ^value )
{
	// check for initialized reference (accept null
	// references: this may mean that item is section only)
	if( value == nullptr ) return;

	Type	^t = value->GetType();

	// test for all supported types
	if( (t == bool::typeid) || (t == int::typeid) || (t == double::typeid) ||
		(t == DateTime::typeid) || (t == String::typeid) ) {
		// this is supported type
		return;
	} else {
		// we doesn't support this type
		throw gcnew ArgumentException(String::Format(
		ERR_INVALID_TYPE, t->ToString() ));
	}
}


//-------------------------------------------------------------------
//
// Serialize object to string.
//
// This function use predefined culture info and additional format
// specifiers to convert object to string.
//
//-------------------------------------------------------------------
String^ IniFile::obj_to_str( Object ^value )
{
	// check for null reference
	if( value == nullptr ) throw gcnew ArgumentNullException("value");

	// get current value type
	Type	^type = value->GetType();

	// depend on value type make specific formatting
	if( type == bool::typeid ) {
		// convert bool to string using default format
		return safe_cast<bool>( value ).ToString();
	} else if( type == int::typeid ) {
		// convert int to string by own format using modified culture info
		return safe_cast<int>( value ).ToString( INT_F, _ci );
	} else if( type == double::typeid ) {
		// convert double to string by own format using modified culture info
		return safe_cast<double>( value ).ToString( DOUBLE_F, _ci );
	} else if( type == DateTime::typeid ) {
		// convert DateTime to string by own format using modified culture info
		return safe_cast<DateTime>( value ).ToString( DATETIME_F, _ci );
	} else if( type == String::typeid ) {
		// just return specified string
		return safe_cast<String^>( value );
	} else {
		// we doesn't support this type of value
		throw gcnew ArgumentException(String::Format(
		ERR_INVALID_TYPE, type->ToString() ));
	}
}


//-------------------------------------------------------------------
//
// Deserialize object from string.
//
// This function attempts to determine value type by using predefined
// culture info and additional format specifiers. This is sequence of
// the type casts: bool, int, double and DateTime. If no one from this
// conversions was succeeded, then returns value as String.
//
//-------------------------------------------------------------------
Object^ IniFile::str_to_obj( String ^value )
{
	// check for null reference
	if( value == nullptr ) throw gcnew ArgumentNullException("value");

	// try convert specified string to bool value
	bool		b = false;
	if( bool::TryParse( value, b ) ) return b;
	// try convert specified string to int value
	int			i = 0;
	if( int::TryParse( value, NumberStyles::Integer, _ci, i ) ) return i;
	// try convert specified string to double value
	double		d = 0;
	if( double::TryParse( value,
						  NumberStyles::Float ^
						  NumberStyles::AllowExponent,
						  _ci, d ) ) return d;
	// try convert specified string to DateTime value
	DateTime	dt;
	if( DateTime::TryParseExact( value, DATETIME_F, _ci,
								 DateTimeStyles::AllowLeadingWhite |
								 DateTimeStyles::AllowTrailingWhite,
								 dt ) ) return dt;

	// if notone convertion was successful,
	// return value as string
	return value;
}


//-------------------------------------------------------------------
//
// Split specified location to section and key.
//
// Returns true, if function succeeded. Otherwise returns false.
//
//-------------------------------------------------------------------
bool IniFile::split_location( String ^loc, String^ %sec, String^ %key )
{
	int pos = loc->LastIndexOf( _del );
	if( (pos >= 0) && (pos < loc->Length - 1) ) {
		// split location to section and key
		sec = loc->Substring( 0, pos );
		key = loc->Substring( pos + 1 );
		// normalize section if needed
		if( sec == "" ) sec = _del.ToString();
		// return succeded rezult
		return true;
	}
	return false;
}


//-------------------------------------------------------------------
//
// Adapters::IAdapter::default implementation.
//
// Gets/puts setting's value that is located by specified path
// from/to cache.
//
//-------------------------------------------------------------------
Object^ IniFile::get_value( String ^loc )
{ENTER_READ(_lock)

	// check for correct path
	check_path( loc );

	// just return value from cache
	return m_cache[loc];

EXIT_READ(_lock)}

void IniFile::set_value( String ^loc, Object ^val )
{ENTER_WRITE(_lock)

	// check for correct path
	check_path( loc );
	// check for valid value type
	check_value( val );

	// expose location to cache
	for( String ^s = loc; (s != "") && !m_cache->ContainsKey( s );
		 s = s->Substring( 0, s->LastIndexOf( _del ) ) ) {
		// push null reference to cache as empty item
		m_cache[s] = nullptr;
	}

	// store value in cache
	m_cache[loc] = val;

EXIT_WRITE(_lock)}


//-------------------------------------------------------------------
//
// Adapters::IAdapter::Name implementation.
//
// Gets adapter's name.
//
//-------------------------------------------------------------------
String^ IniFile::get_name( void )
{
	return _name;
}


//-------------------------------------------------------------------
//
// Adapters::IAdapter::Delimeter implementation.
//
// Get's string that is used in location pathes as delimiter.
//
//-------------------------------------------------------------------
String^ IniFile::get_delimeter( void )
{
	return _del.ToString();
}


//-------------------------------------------------------------------
//
// Adapters::IAdapter::Remove implementation.
//
// Removes setting value that is located by specified path from
// cache.
//
//-------------------------------------------------------------------
bool IniFile::remove( String ^loc )
{ENTER_WRITE(_lock)

	// check for correct path
	check_path( loc );

	// create new list of locations
	List<String^>	^locs = gcnew List<String^>;
	// and copy there all locations 
	for each( String ^s in m_cache->Keys ) {
		// that are subpathes to specified location
		if( (s + _del)->StartsWith( loc + _del ) ) locs->Add( s );
	}

	// initialize return value
	bool bRes = true;
	// removes all items from cache
	for each( String ^s in locs ) bRes = bRes && m_cache->Remove( s );

	// return combined result
	return bRes;

EXIT_WRITE(_lock)}


//-------------------------------------------------------------------
//
// Adapters::IAdapter::Reload implementation.
//
// Retrieves settings from source to internal cache.
//
//-------------------------------------------------------------------
IEnumerable<String^>^ IniFile::reload( String ^loc )
{ENTER_WRITE(_lock)

	// check for existing file
	if( !IO::File::Exists( _filename ) )
		throw gcnew IO::FileNotFoundException(String::Format(
		ERR_PATH_NOT_FOUND, _filename ));

	// check for correct path
	check_path( loc );
	// modify root location
	if( loc == _del.ToString() ) loc = "";

	// look through all items in cache
	for each( String ^s in m_cache->Keys ) {
		// and remove items beeing subitems for specified location 
		if( (s + _del)->StartsWith( loc + _del ) ) m_cache->Remove( s );
	}

	// get all sections from ini file
	List<String^>	^secs = get_ini_string( nullptr, nullptr );
	// and remove those that are not subitems to specified location
	for( int i = 0; i < secs->Count;
		(secs[i] + _del)->StartsWith( loc + _del ) ?
		i++ : secs->RemoveAt( i ) );
	// sort list of sections
	secs->Sort();

	// create string list that will contain result
	List<String^>	^items = gcnew List<String^>();
	// look through all valid sections in back order
	for( int i = secs->Count - 1; i >= 0; i-- ) {
		// modify section name, if needed
		String	^sec = (secs[i] == _del.ToString() ? "" : secs[i]);

		// expose section to cache
		for( String ^s = sec; (s != "") && !m_cache->ContainsKey( s );
			s = s->Substring( 0, s->LastIndexOf( _del ) ) ) {
			// push null reference to cache as empty item
			m_cache[s] = nullptr;
		}
		// and add it to results list if it is not location
		if( String::Compare( sec, loc + _del ) > 0 )
			items->Add( sec->Substring( (loc + _del)->Length ) );

		// load all keys
		for each( String ^key in get_ini_string( secs[i], nullptr ) ) {
			// check key for valid symbols
			if( key->Contains( _del.ToString() ) )
				throw gcnew IO::InvalidDataException(String::Format(
				ERR_INI_FILE, _del ));

			// produce location
			String		^path = sec + _del + key;
			// add relative path to result list
			items->Add( path->Substring( (loc + _del)->Length ) );
			// extract value from key and put it to the cache
			IList<String^>	^val = get_ini_string( secs[i], key );
			if( val->Count > 0 ) m_cache[path] = str_to_obj( val[0] );
		}
	}

	// now, reload data from location only
	String	^locsec, ^lockey;
	// if split location to section and key succeeded
	if( split_location( loc, locsec, lockey ) ) {
		// extract value from key and put it to cache
		IList<String^>	^val = get_ini_string( locsec, lockey );
		if( val->Count > 0 ) m_cache[lockey] = str_to_obj( val[0] );
	}

	return items;

EXIT_WRITE(_lock)}


//-------------------------------------------------------------------
//
// Adapters::IAdapter::Flush implementation.
//
// Flashes settings from internal cache to source.
//
//-------------------------------------------------------------------
void IniFile::flush( String ^loc )
{ENTER_READ(_lock)

	// check for correct path
	check_path( loc );
	// modify root location
	if( loc == _del.ToString() ) loc = "";

	// empty those sections and keys, that doesn't exists in cache
	// so, look through all sections in file
	for each( String ^sec in get_ini_string( nullptr, nullptr ) ){
		// and if section starts with specified location
		if( (sec + _del)->StartsWith( loc + _del ) ) {
			// and not contains in cache and not root section, then
			if(	!m_cache->ContainsKey( sec ) && (sec != _del.ToString()) ) {
				// delete it
				set_ini_string( sec, nullptr, nullptr );
			}
			// look for it keys
			for each( String ^key in get_ini_string( sec, nullptr ) ) {
				// and if key doesn't exist in the chache
				if( !m_cache->ContainsKey( (sec == _del.ToString() ? "" : sec) +
										   _del + key ) ) {
					// delete it
					set_ini_string( sec, key, nullptr );
				}
			}
		}
	}

	// create new list of locations
	List<String^>	^locs = gcnew List<String^>;
	// and copy there all locations 
	for each( String ^s in m_cache->Keys ) {
		// that are subpathes to specified location
		if( (s + _del)->StartsWith( loc + _del ) ) locs->Add( s );
	}

	// write new data from cache to source file
	// (locs array was copied by enumerator from
	// Map, so this array is sorted already)
	for( int i = 0; i < locs->Count; i++ ) {
		// ok. split location to section and key  
		String	^sec, ^key;
		// if split succeeded
		if( split_location( locs[i], sec, key ) ) {
			// then check for cached value
			if( m_cache[locs[i]] != nullptr ) {
				// and write value to file
				int res = set_ini_string( sec, key, 
										  obj_to_str( m_cache[locs[i]] ) );
				if( res != 0 )
					throw gcnew IO::IOException(String::Format(
					ERR_INI_WRITE, _filename, res ));
			} else {
				// try to delete key-value pair from file
				set_ini_string( sec, key, nullptr );

				// check for this location is empty leaf section
				// (it be leaf, if no subpathes exists)
				if( ((i + 1) >= locs->Count) || 
					!locs[i + 1]->Contains( locs[i] + _del ) ) {
					// create fake key to create section
					String	^guid = Guid::NewGuid().ToString();
					int res = set_ini_string( locs[i], guid, "fake" );
					// and now delete it
					if( res == 0 ) res = set_ini_string( locs[i], guid, nullptr );
					if( res != 0 )
						throw gcnew IO::IOException(String::Format(
						ERR_INI_WRITE, _filename, res ));
				}
			}
		}
	}

	// the last step is removing unneeded empty sections
	// load all sections from source
	List<String^>	^secs = get_ini_string( nullptr, nullptr );
	// sort it
	secs->Sort();
	// and look through all sections
	for( int i = 0; i < secs->Count; i++ ){
		// if this is subpath to specified location
		if( (secs[i] + _del)->StartsWith( loc + _del ) ) {
			// check that section hasn't keys and isn't empty leaf
			if( (get_ini_string( secs[i], nullptr )->Count == 0) &&
				((secs[i] == _del.ToString()) ||
				 (m_cache[secs[i]] != nullptr) ||
				 (((i + 1) < secs->Count) &&
				  secs[i + 1]->Contains(
					(secs[i] == _del.ToString() ? "" : secs[i]) + _del ))) ) {
				// this is unneeded section: remove it
				set_ini_string( secs[i], nullptr, nullptr );
			}
		}
	}

EXIT_READ(_lock)}


//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the IniFile class and initialize it with
/// specified path to file.
/// </summary><remarks>
/// Current adapter name builds by extracting file name without
/// extension from passed path.
/// </remarks>
//-------------------------------------------------------------------
IniFile::IniFile( String ^filename ) :			  \
	_filename(IO::Path::GetFullPath( filename )), \
	_lock(gcnew ReaderWriterLock)
{
	// build adapter's name based on filename
	_name = System::IO::Path::GetFileNameWithoutExtension( _filename );

	INIT_CI(_ci)

	// create cached map of settings
	m_cache = gcnew Map<String^, Object^>();
}


//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the IniFile class and initialize it with
/// specified name and path to file.
/// </summary>
//-------------------------------------------------------------------
IniFile::IniFile( String ^filename, String ^name ) : \
	_filename(IO::Path::GetFullPath( filename )),	 \
	_lock(gcnew ReaderWriterLock), _name(name)
{
	// check for name to be initialized
	if( name == nullptr ) throw gcnew ArgumentNullException("name");

	INIT_CI(_ci)

	// create cached map of settings
	m_cache = gcnew Map<String^, Object^>();
}
