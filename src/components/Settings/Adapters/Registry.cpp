/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		Registry.cpp												*/
/*																			*/
/*	Content:	Implementation of Adapters::Registry class					*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "Registry.h"

using namespace _SETTINGS::Adapters;


//-----------------------------------------------------------------------------
//					Toolkit::Settings::Adapters::Registry
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Check for correct adapter path.
//
// Valid path must starts with adapter's delimeter
//
//-------------------------------------------------------------------
void Registry::check_path( String ^path )
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
// Now, Registry adapter supports following types: bool, int, double,
// DateTime and String.
//
//-------------------------------------------------------------------
void Registry::check_value( Object ^value )
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
String^ Registry::obj_to_str( Object ^value )
{
	// check for null reference
	if( value == nullptr ) throw gcnew ArgumentNullException("value");

	// get current value type
	Type	^type = value->GetType();

	// depend on value type make specific formatting
	if( type == bool::typeid ) {
		// convert bool to string using default format
		return static_cast<bool>( value ).ToString();
	} else if( type == double::typeid ) {
		// convert double to string by own format using modified culture info
		return static_cast<double>( value ).ToString( DOUBLE_F, _ci );
	} else if( type == DateTime::typeid ) {
		// convert DateTime to string by own format using modified culture info
		return static_cast<DateTime>( value ).ToString( DATETIME_F, _ci );
	} else if( type == String::typeid ) {
		// just return specified string
		return static_cast<String^>( value );
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
// the type casts: bool, double and DateTime. If no one from this
// conversions was succeeded, then returns value as String.
//
//-------------------------------------------------------------------
Object^ Registry::str_to_obj( String ^value )
{
	// check for null reference
	if( value == nullptr ) throw gcnew ArgumentNullException("value");

	// try convert specified string to bool value
	bool		b = false;
	if( bool::TryParse( value, b ) ) return b;
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
// Split specified location to key and value.
//
// Returns true, if function succeeded. Otherwise returns false.
//
//-------------------------------------------------------------------
bool Registry::split_location( String ^loc, String^ %key, String^ %val )
{
	int pos = loc->LastIndexOf( _del );
	if( (pos >= 0) && (pos < loc->Length - 1) ) {
		// split location to section and key
		key = loc->Substring( 0, pos );
		val = loc->Substring( pos + 1 );
		// normalize section if needed
		if( key == "" ) key = _del.ToString();
		// return succeded result
		return true;
	}
	return false;
}


//-------------------------------------------------------------------
//
// Gets string list that represents registry key hierarchy.
// 
// Returns null reference if key doesn't exist.
//
//-------------------------------------------------------------------
List<String^>^ Registry::load_key( String ^subpath )
{
	// open specified key
	Win32::RegistryKey	^key = _root->OpenSubKey( _key + subpath );
	if( key == nullptr ) return nullptr;

	try {
		// initialize result list
		List<String^>	^items = gcnew List<String^>;

		// load values for specified key
		for each( String ^s in key->GetValueNames() ) {
			// get value type
			Win32::RegistryValueKind	type = key->GetValueKind( s );
			// if this is supported type
			if( (type == Win32::RegistryValueKind::DWord) ||
				(type == Win32::RegistryValueKind::String) ) {
				// add value name to items
				items->Add( s );
			}
		}
		// load subkeys for specified key
		for each( String ^subkey in key->GetSubKeyNames() ) {
			// add subkey only
			items->Add( subkey );
			// and load all subkeys in hierarchy using recursion
			for each( String ^s in load_key( subpath + _del + subkey ) ) {
				// add to list composite name
				items->Add( subkey + _del + s );
			}
		}
		return items;
	} finally {
		// dispose key object
		key->Close();
	}
}


//-------------------------------------------------------------------
//
// Removes content of the specified key recursively.
//
// Returns operation result. This function performs smart deletion:
// it delete only value with known type of. So, returns true if key
// was deleted completely, in other case returns false.
//
//-------------------------------------------------------------------
bool Registry::empty_key( String ^subpath )
{
	// open specified key
	Win32::RegistryKey	^key = _root->OpenSubKey( _key + subpath, true );
	if( key == nullptr ) return true;

	try {
		// look through all values
		for each( String ^s in key->GetValueNames() ) {
			// get type of value
			Win32::RegistryValueKind	type = key->GetValueKind( s );
			// if this is supported value type
			if( (type == Win32::RegistryValueKind::DWord) ||
				(type == Win32::RegistryValueKind::String) ) {
				// remove registry value
				key->DeleteValue( s );
			}
		}
		// look through all subkeys
		for each( String ^s in key->GetSubKeyNames() ) {
			// delete subkey using recursion (if all
			// it's subkeys and values were deleted)
			if( empty_key( subpath + _del + s ) ) key->DeleteSubKey( s );
		}

		// return delete result
		return (key->ValueCount == 0) && (key->SubKeyCount == 0);
	} finally {
		// dispose key object
		key->Close();
	}
}


//-------------------------------------------------------------------
//
// Adapters::IAdapter::default implementation.
//
// Gets/puts setting's value that is located by specified path
// from/to Windows registry.
//
//-------------------------------------------------------------------
Object^ Registry::get_value( String ^loc )
{ENTER_READ(_lock)

	// check for correct path
	check_path( loc );
	// initialize value to null reference
	Object	^value = nullptr;

	// retrieving root node value is unsupported,
	// so split has to complete successfuly
	String	^lockey, ^locval;
	if( split_location( loc, lockey, locval ) ) {
		// try to read value from registry
		value = Win32::Registry::GetValue( _root->Name + _del + _key + lockey,
										   locval, nullptr );
		// depend on value type
		Type	^type = (value != nullptr) ? value->GetType() : nullptr;
		if( type == int::typeid ) {
			// do nothing: this is realy integer
		} else if( type == String::typeid ) {
			// parse string representation for known types
			value = str_to_obj( static_cast<String^>( value ) );
		} else {
			// this is unsupported type - ignore it
			value = nullptr;
		}
	}

	return value;

EXIT_READ(_lock)}

void Registry::set_value( String ^loc, Object ^val )
{ENTER_WRITE(_lock)

	// check for correct path
	check_path( loc );
	// check for valid value type
	check_value( val );

	// if value is null referense
	if( val == nullptr ) {
		// create subkey only
		_root->CreateSubKey( _key + loc );
		// and return
		return;
	}

	// setting root node value is unsupported,
	// so split has to complete successfuly
	String	^lockey, ^locval;
	if( split_location( loc, lockey, locval ) ) {
		// depend on value type
		if( val->GetType() == int::typeid ) {
			// write as DWORD value
			Win32::Registry::SetValue( _root->Name + _del + _key + lockey, locval,
									   val, Win32::RegistryValueKind::DWord );
		} else {
			// write own string representation
			Win32::Registry::SetValue( _root->Name + _del + _key + lockey, locval,
									   obj_to_str( val ),
									   Win32::RegistryValueKind::String );
		}
	}

EXIT_WRITE(_lock)}


//-------------------------------------------------------------------
//
// Adapters::IAdapter::Name implementation.
//
// Gets adapter's name.
//
//-------------------------------------------------------------------
String^ Registry::get_name( void )
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
String^ Registry::get_delimeter( void )
{
	return _del.ToString();
}


//-------------------------------------------------------------------
//
// Adapters::IAdapter::Remove implementation.
//
// Removes setting value that is located by specified path from
// Windows registry.
//
//-------------------------------------------------------------------
bool Registry::remove( String ^loc )
{ENTER_WRITE(_lock)

	// check for correct path
	check_path( loc );
	
	// removing root node is unsupported,
	// so split has to complete successfuly
	String	^lockey, ^locval;
	if( split_location( loc, lockey, locval ) ) {
		// attempt to open key
		Win32::RegistryKey	^key = _root->OpenSubKey( _key + lockey, true );
		if( key == nullptr ) return true;

		try {
			// remove value if it exists
			key->DeleteValue( locval, false );
			// if key content was not fully erased return failed
			if( !empty_key( loc ) ) return false;
			// remove key if exists
			 key->DeleteSubKey( locval, false );
		} finally {
			// dispose key object
			key->Close();
		}
	}
	return true;

EXIT_WRITE(_lock)}


//-------------------------------------------------------------------
//
// Adapters::IAdapter::Reload implementation.
//
// Retrieves settings hierarchy from Windows registry.
//
//-------------------------------------------------------------------
IEnumerable<String^>^ Registry::reload( String ^loc )
{ENTER_READ(_lock)

	// check for correct path
	check_path( loc );

	// create string list that will contain
	// result and load key hierarchy
	List<String^>	^items = load_key( loc );

	// check that corresponding key exists
	if( items != nullptr ) {
		// remove all top-level keys
		items->Sort();
		for( int i = items->Count - 1; i > 0; i-- ) {
			// if current key name contains previous
			if( (items[i] + _del)->StartsWith( items[i - 1] + _del ) ) {
				// remove previous item
				items->RemoveAt( i - 1 );
			}
		}
	} else {
		// key not exists, so value must exists
		String	^lockey, ^locval;
		// if split to key-value failed or value
		// doesn't exists throw exception
		if( !split_location( loc, lockey, locval ) ||
			Win32::Registry::GetValue( _root->Name + _del + _key + lockey,
									   locval, nullptr ) == nullptr )
		   throw gcnew ArgumentException(String::Format(
		   ERR_PATH_NOT_FOUND, _root->Name + _key + loc), "loc");

		// all right: initialize items to empty list
		items = gcnew List<String^>;
	}

	return items;

EXIT_READ(_lock)}


//-------------------------------------------------------------------
//
// Adapters::IAdapter::Flush implementation.
//
//-------------------------------------------------------------------
void Registry::flush( String ^loc )
{
	// this is online adapter, so do nothing
}


//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the Registry class and initialize it with
/// specified name and Windows registry key.
/// </summary><remarks>
/// You cann't open root registry keys.
/// </remarks>
//-------------------------------------------------------------------
Registry::Registry( String ^key, String ^name ) : \
	_lock(gcnew ReaderWriterLock), _name(name)
{
	// check for the registry key and adapter's name were initialized
	if( name == nullptr ) throw gcnew ArgumentNullException("name");
	if( key == nullptr ) throw gcnew ArgumentNullException("key");

	// determine root registry key
	_key = key + _del;
	if( _key->StartsWith( Win32::Registry::ClassesRoot->Name + _del,
						  StringComparison::OrdinalIgnoreCase ) ) {
		// HKEY_CLASSES_ROOT
		_root = Win32::Registry::ClassesRoot;
	} else if( _key->StartsWith( Win32::Registry::CurrentUser->Name + _del,
								 StringComparison::OrdinalIgnoreCase ) ) {
		// HKEY_CURRENT_USER
									 _root = Win32::Registry::CurrentUser;

	} else if( _key->StartsWith( Win32::Registry::LocalMachine->Name + _del,
								 StringComparison::OrdinalIgnoreCase ) ) {
		// HKEY_LOCAL_MACHINE
		_root = Win32::Registry::LocalMachine;
	} else if( _key->StartsWith( Win32::Registry::Users->Name + _del,
								 StringComparison::OrdinalIgnoreCase ) ) {
		// HKEY_USERS
		_root = Win32::Registry::Users;
	} else if( _key->StartsWith( Win32::Registry::CurrentConfig->Name + _del,
								 StringComparison::OrdinalIgnoreCase ) ) {
		// HKEY_CURRENT_CONFIG
		_root = Win32::Registry::CurrentConfig;
	} else {
		// this is not valid registry key
		throw gcnew ArgumentException(String::Format(
		ERR_REGISTRY_KEY, key, "key"));
	}

	// normalize input key value
	while( key->Contains( _del.ToString() + _del.ToString() ) ) {
		// remove duplicated delimeters
		key = key->Replace( _del.ToString() + _del.ToString(),
							_del.ToString() );
	}
	// and initialize _key
	_key = key->Substring( _root->Name->Length )->Trim( _del );

	// check for key is not registry root
	if( _key == "" ) {
		// throw parameter exception
		throw gcnew ArgumentException(String::Format(
		ERR_REGISTRY_ROOT, _root->Name ), "key"); 
	}

	INIT_CI(_ci)
}