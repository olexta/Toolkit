/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		Node.cpp													*/
/*																			*/
/*	Content:	Implementation of Node class								*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright @ 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "Nodes.h"
#include "Node.h"

using namespace _SETTINGS;


//
// Define double static cast operation
//
#define SCAST_(to, from, value)													\
static_cast<to>( static_cast<from>( value ) )

//
// Define macro for implicit cast operator from specified type to ValueBox.
//
#define OP_IMP_FROM_(type)														\
Node::ValueBox::operator Node::ValueBox( type v )								\
{																				\
	return ValueBox( v );														\
}

//
// Define macro for explicit cast operator from ValueBox to specified
// native type.
//
#define OP_EXP_TO_(type)														\
Node::ValueBox::operator type( ValueBox box )									\
{																				\
	Type	^r = (box.m_value != nullptr ? box.m_value->GetType() : nullptr);	\
	Type	^t = type::typeid;													\
																				\
	if( r == t ) return static_cast<type>( box.m_value );						\
																				\
	throw gcnew InvalidCastException(String::Format(							\
	ERR_CAST_FROM_TO, (r != nullptr ? r->ToString() : "null"),					\
					  (t != nullptr ? t->ToString() : "null") ));				\
}

//
// Define macro for explicit cast operator from ValueBox (being in 'from'
// type) to specified 'to' type.
//
#define OP_EXP_(to, from)														\
Node::ValueBox::operator to( ValueBox box )										\
{																				\
	Type	^r = (box.m_value != nullptr ? box.m_value->GetType() : nullptr);	\
	Type	^f = from::typeid;													\
	Type	^t = to::typeid;													\
																				\
	if( r == f ) return SCAST_(to, from, box.m_value );							\
																				\
	throw gcnew InvalidCastException(String::Format(							\
	ERR_CAST_FROM_TO, (r != nullptr ? r->ToString() : "null"),					\
					  (t != nullptr ? t->ToString() : "null") ));				\
}


//----------------------------------------------------------------------------
//					Toolkit::Settings::Node::ValueBox
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the ValueBox class containing specified
/// object as internal data.
/// </summary><remarks>
/// Now following types are supported as native: bool, int, double,
/// DateTime and String. And char, unsigned char, short, unsigned
/// short, unsigned int, float are convertible to native.
/// </remarks>
//-------------------------------------------------------------------
Node::ValueBox::ValueBox( Object ^value ): \
	m_value(nullptr)
{
	// m_value was already initialized with null reference
	if( value == nullptr ) return;

	// get type of value
	Type	^type = value->GetType();
	
	// check this type through all supported types (native 
	// types checks first, so it minimize processing of
	// cast operators, because of they convert values to
	// such types, then process all convertible)
	if( type == bool::typeid ) {
		// native bool
		m_value = static_cast<bool>( value );
	} else if( type == int::typeid ) {
		// native int
		m_value = static_cast<int>( value );
	} else if( type == double::typeid ) {
		// native double
		m_value = static_cast<double>( value );
	} else if( type == DateTime::typeid ) {
		// native DateTime
		m_value = static_cast<DateTime>( value );
	} else if( type == String::typeid ) {
		// native String
		m_value = static_cast<String^>( value );
	} else if( type == char::typeid ) {
		// convertible to int
		m_value = SCAST_(int, char, value);
	} else if( type == unsigned char::typeid ) {
		// convertible to int
		m_value = SCAST_(int, unsigned char, value);
	} else if( type == short::typeid ) {
		// convertible to int
		m_value = SCAST_(int, short, value);
	} else if( type == unsigned short::typeid ) {
		// convertible to int
		m_value = SCAST_(int, unsigned short, value);
	} else if( type == unsigned int::typeid) {
		// convertible to int
		m_value = SCAST_(int, unsigned int, value);
	} else if( type == float::typeid ) {
		// convertible to double
		m_value = SCAST_(double, float, value );
	} else {
		//unsupported type: throw exception
		throw gcnew ArgumentException(String::Format(
		ERR_INVALID_TYPE, type->ToString() ));
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Implicit cast operator from bool value to ValueBox. 
/// </summary>
//-------------------------------------------------------------------
OP_IMP_FROM_(bool)


//-------------------------------------------------------------------
/// <summary>
/// Implicit cast operator from int value to ValueBox. 
/// </summary>
//-------------------------------------------------------------------
OP_IMP_FROM_(int)


//-------------------------------------------------------------------
/// <summary>
/// Implicit cast operator from double value to ValueBox. 
/// </summary>
//-------------------------------------------------------------------
OP_IMP_FROM_(double)


//-------------------------------------------------------------------
/// <summary>
/// Implicit cast operator from DateTime value to ValueBox. 
/// </summary>
//-------------------------------------------------------------------
OP_IMP_FROM_(DateTime)


//-------------------------------------------------------------------
/// <summary>
/// Implicit cast operator from String value to ValueBox. 
/// </summary>
//-------------------------------------------------------------------
OP_IMP_FROM_(String^)


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to bool value (native).
/// </summary><remarks>
/// If internal data is not bool value then InvalidCastException will
/// be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_TO_(bool)


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to int value (native).
/// </summary><remarks>
/// If internal data is not int value then InvalidCastException will
/// be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_TO_(int)


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to double value (native).
/// </summary><remarks>
/// If internal data is not double value then InvalidCastException
/// will be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_TO_(double)


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to DateTime value (native).
/// </summary><remarks>
/// If internal data is not DateTime value then InvalidCastException
/// will be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_TO_(DateTime)


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to String value (native).
/// </summary><remarks>
/// This cast operator differs from other: it doesn't requires
/// internal data to be in valid type. Since this is settings, so any
/// setting can be converted to string value. This function is
/// similar to ToString() method.
/// </remarks>
//-------------------------------------------------------------------
Node::ValueBox::operator String^( ValueBox box )
{
	return box.ToString();
}


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to char value.
/// </summary><remarks>
/// If internal data is not int value then InvalidCastException will
/// be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_(char, int)


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to unsigned char value.
/// </summary><remarks>
/// If internal data is not int value then InvalidCastException will
/// be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_(unsigned char, int);


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to short value.
/// </summary><remarks>
/// If internal data is not int value then InvalidCastException will
/// be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_(short, int);


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to unsigned short value.
/// </summary><remarks>
/// If internal data is not int value then InvalidCastException will
/// be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_(unsigned short, int);


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to unsigned int value.
/// </summary><remarks>
/// If internal data is not int value then InvalidCastException will
/// be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_(unsigned int, int);


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to long long value.
/// </summary><remarks>
/// If internal data is not int value then InvalidCastException will
/// be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_(long long, int);


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to unsigned long long value.
/// </summary><remarks>
/// If internal data is not int value then InvalidCastException will
/// be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_(unsigned long long, int);


//-------------------------------------------------------------------
/// <summary>
/// Explicit cast operator from ValueBox to float value.
/// </summary><remarks>
/// If internal data is not double value then InvalidCastException
/// will be raised.
/// </remarks>
//-------------------------------------------------------------------
OP_EXP_(float, double);


//-------------------------------------------------------------------
/// <summary>
/// Operator ==. Check for two instances of ValueBox to be equal.
/// </summary><remarks>
/// Checks internal m_value members to be equivalent.
/// </remarks>
//-------------------------------------------------------------------
bool Node::ValueBox::operator ==( ValueBox value1, ValueBox value2 )
{
	return Object::Equals( value1.m_value, value2.m_value );
}


//-------------------------------------------------------------------
/// <summary>
/// Operator !=. Check for two instances of ValueBox to be not equal.
/// </summary><remarks>
/// It uses Equal operator in implementation.
/// </remarks>
//-------------------------------------------------------------------
bool Node::ValueBox::operator !=( ValueBox value1, ValueBox value2 )
{
	return !(value1 == value2);
}


//-------------------------------------------------------------------
/// <summary>
/// Returns a value indicating whether this instance and a specified
/// ValueBox object represent the same value.
/// </summary><remarks>
/// It uses Equal operator in implementation.
/// </remarks>
//-------------------------------------------------------------------
bool Node::ValueBox::Equals( ValueBox value )
{
	return (*this == value);
}


//-------------------------------------------------------------------
/// <summary>
/// Determines whether the specified Object is equal to the current
/// ValueBox instance.
/// </summary><remarks>
/// Try build new instance of the ValueBox class by specified Object
/// parameter. If custructor fails, returns false. In other case uses
/// Equal operator.
/// </remarks>
//-------------------------------------------------------------------
bool Node::ValueBox::Equals( Object ^object )
{
	// try to build new ValueBox instance
	try {
		// if succeeded use Equal operator
		return (*this == ValueBox(object));
	} catch( ArgumentException^ ) {
		// if constructor failed return false
		return false;
	}
}


//-------------------------------------------------------------------
/// <summary>
/// Returns a handle to the internal data.
/// </summary>
//-------------------------------------------------------------------
Object^ Node::ValueBox::ToObject( void )
{
	return m_value;
}


//-------------------------------------------------------------------
/// <summary>
/// Returns a String that represents the current ValueBox.
/// </summary><remarks>
/// This function call the internal data ToString() method or return
/// "null" for internal null reference.
/// </remarks>
//-------------------------------------------------------------------
String^ Node::ValueBox::ToString( void )
{
	return (m_value != nullptr ? m_value->ToString() : "null");
}


//----------------------------------------------------------------------------
//						Toolkit::Settings::Node
//----------------------------------------------------------------------------

//-------------------------------------------------------------------
//
// Check node name to be in the correct format.
//
//-------------------------------------------------------------------
void Node::check_name( String ^name )
{
	// check for null reference
	if( name == nullptr ) throw gcnew ArgumentNullException("name");

	// check for illegal names ('path to current' and 'path to parent')
	if( (name == _cur_path) || (name == _par_path) ) {
		// throw reserved names exception 
		throw gcnew ArgumentException(String::Format(
		ERR_RESERVED_NAME, _cur_path, _par_path ));
	}

	// check for illegal symbols (name cann't contains delimeter)
	if( name->Contains( _delimeter ) ) {
		// throw illegal symbol exception
		throw gcnew ArgumentException(String::Format(
		ERR_STR_CONTAINS, "A name", _delimeter ));
	}
}


//-------------------------------------------------------------------
//
// Set new parent for current node.
//
// By doing this two events will be raised: 'OnSetParent' and
// 'OnSetParentComplete'. This function is used by Nodes class to
// notify about direct collection actions.
//
//-------------------------------------------------------------------
void Node::set_parent( Node ^parent )
{ENTER_WRITE(_lock)

	// backup old parent reference
	Node	^oldParent = m_parent;

	// in case of having parent and specified parent is
	// not null reference - remove this from parent's
	// child collection
	if( (m_parent != nullptr) && (parent != nullptr) ) {

		m_parent->_childs->Remove( this );
	}

	// now m_parent is null reference, so all actions
	// go through exception control (if error will be
	// raised all changes will be rolled back)
	try {
		// fire event before the action
		OnSetParent( parent );

		// save new value
		m_parent = parent;

		// fire event after the action 
		OnSetParentComplete( parent );
	} catch( Exception^ ) {
		// roll back changes
		m_parent = oldParent;
		// if remove operation was done
		if( (m_parent != nullptr) && (parent != nullptr) ) {
			// roll back remove request
			m_parent->_childs->Revert();
		}
		// restore exception
		throw;
	}

EXIT_WRITE(_lock)}


//-------------------------------------------------------------------
//
// Returns child node that is located on specified relative path.
//
// This function is used by Nodes class to search child if default
// accessor or Contains gets relative path as parameter.
//
//-------------------------------------------------------------------
Node^ Node::get_child( String ^path )
{ENTER_READ(_lock)

	Node	^node = Find( path );

	// if specified item was not found return null reference
	return (path == "" ? node : nullptr);

EXIT_READ(_lock)}


//-------------------------------------------------------------------
/// <summary>
/// Split input string to next subnode and tail.
/// </summary><remarks>
/// Subnode name returns as function result, and tail is returned
/// through input string.
/// </remarks>
//-------------------------------------------------------------------
String^ Node::ParsePath( String^ %path )
{
	// search for delimeter
	int		pos = path->IndexOf( _delimeter, 0 );
	
	// split string by this delimeter
	String	^res = pos < 0 ? path : path->Substring( 0, pos );
	path = pos < 0 ? "" : path->Substring( pos + _delimeter->Length );

	return res;
}


//-------------------------------------------------------------------
/// <summary>
/// Returns path and reference to root node.
/// </summary><remarks>
/// This function proceed only current subnode, so founded node may
/// be not root in common sense (like "/" in UNIX). To determine this
/// situation you can use check for 'Delimeter' at the begining of
/// the returned path ("real" root node have "" as name, so path will
/// start with 'Delimeter').
/// </remarks>
//-------------------------------------------------------------------
String^ Node::RootTraverse( Node^ *root )
{
	// if this node is root - return it's name and
	// reference to this
	if( m_parent == nullptr ) {
		// check for null pointer
		if( root != nullptr ) *root = this;
		// and return path as name
		return _name;
	}

	// use recurse to get path from root (this is not efficient
	// approach, but we don't use deep tree hierarche)
	return m_parent->RootTraverse( root ) + _delimeter + _name;
}


//-------------------------------------------------------------------
/// <summary>
/// Find node that is located on specified relative path.
/// </summary><remarks>
/// In case of unsuccessful result return as deep founded node as
/// possible. Path will contain subpath that was not found.
/// </remarks>
//-------------------------------------------------------------------
Node^ Node::Find( String^ %path )
{
	// recurse stop point
	if( path == "" ) return this;

	Node	^node = nullptr;
	String	^subnode = ParsePath( path );
	
	// select node to recursive request
	if( subnode == "" ) {
		// this is root node request, so search current subnode
		// for root and check that founded local root to be
		// "real" root (have empty name)
		if( RootTraverse( &node ), (node->_name != "") ) {
			// throw exception
			throw gcnew InvalidOperationException(ERR_ROOT_LOCATE);
		}
	} else if( subnode == _cur_path ) {
		// just return this
		node = this;
	} else if( subnode == _par_path ) {
		// return reference to parent, or ignore
		// if we are in the root node
		node = (m_parent == nullptr) ? this : m_parent;
	} else {
		// at the end, return subnode if it exists
		if( _childs->Contains( subnode ) ) node = _childs[subnode];
	}

	// in case of unsuccessful search we must return
	// last founded node and tail of unprocessed path
	if( node == nullptr ) {
		// restore original path
		path = subnode + _delimeter + path;
		// and return this as last founded node
		return this;
	}

	// return recursive request to founded node
	return node->Find( path );
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes before setting a new parent
/// to the current Node instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action before the
/// specified parent is set.
/// </remarks>
//-------------------------------------------------------------------
void Node::OnSetParent( Node ^parent )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Performs additional custom processes after setting a new parent
/// to the current Node instance.
/// </summary><remarks>
/// The default implementation of this method is intended to be
/// overridden by a derived class to perform some action after the
/// specified parent is set.
/// </remarks>
//-------------------------------------------------------------------
void Node::OnSetParentComplete( Node ^parent )
{
}


//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the Node class, and initialise it with
/// specified name.
/// </summary>
//-------------------------------------------------------------------
Node::Node( String ^name ) : \
	_name(name), _lock(gcnew ReaderWriterLock())
{
	// check for the correct name
	check_name( name );

	// create subnodes (i use initonly modifier
	// to be able use 'this' pointer)
	_childs = gcnew Nodes(this);

	// initialize reference to parent node
	m_parent = nullptr;
}


//-------------------------------------------------------------------
/// <summary>
/// Creates new instance of the Node class and initialise it with
/// specified name and parent.
/// </summary>
//-------------------------------------------------------------------
Node::Node( String ^name, Node ^parent ) : \
	_name(name), _lock(gcnew ReaderWriterLock())
{
	// check for the correct name
	check_name( name );

	// create subnodes (i use initonly modifier
	// to be able use 'this' pointer)
	_childs = gcnew Nodes(this);

	// set reference to parent node
	m_parent = parent;
}


//-------------------------------------------------------------------
/// <summary>
/// Class disposer.
/// </summary><remarks>
/// Set parent reference to null, dispose all childs and removes it
/// from collection.
/// </remarks>
//-------------------------------------------------------------------
Node::~Node( void )
{ENTER_WRITE(_lock)

	// set parent to null reference
	m_parent = nullptr;

	// call destructor to all childs
	for each( Node ^node in _childs ) delete node;
	// and force clear collection
	_childs->Clear( true );

EXIT_WRITE(_lock)}


//-------------------------------------------------------------------
/// <summary>
/// Gets string that is used as delimeter in node path.
/// </summary>
//-------------------------------------------------------------------
String^ Node::Delimeter::get( void )
{
	// return without any thread synchronization
	// because of using static const string
	return _delimeter;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets name of the node.
/// </summary>
//-------------------------------------------------------------------
String^ Node::Name::get( void )
{
	// return without any thread synchronization
	// because of using const string (name is read-only)
	return _name;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets full path to this node.
/// </summary><remarks>
/// Full path include name of this node at the end. This property
/// proceed current subnode only, so root node may be not "root" in
/// common sence (like "/" in UNIX). To determine this situation you
/// can use check for 'Delimeter' at the begining of the returned
/// path ("real" root node have "" as name, so path will start with
/// 'Delimeter').
/// </remarks>
//-------------------------------------------------------------------
String^ Node::Path::get( void )
{ENTER_READ(_lock)

	return RootTraverse( nullptr );

EXIT_READ(_lock)}


//-------------------------------------------------------------------
/// <summary>
/// Gets subnodes collection.
/// </summary><remarks>
/// This collection can be empty, but never nullptr.
/// </remarks>
//-------------------------------------------------------------------
KeyedMap<String^, Node^>^ Node::Childs::get( void )
{
	return _childs;
}


//-------------------------------------------------------------------
/// <summary>
/// Gets or sets node value that is located on specified relative
/// path.
/// </summary><remarks>
/// Default implementation searchs subnode with specified relative
/// path and call apropriate get or set Value handler. In case of
/// unssuccesful search ArgumentException will be raised.
/// </remarks>
//-------------------------------------------------------------------
Node::ValueBox Node::default::get( String ^path )
{ENTER_READ(_lock)

	// check for null reference
	if( path == nullptr ) throw gcnew ArgumentNullException("path");

	// copy path because of, it will be modified by Find request 
	String	^relpath = path;
	// search apropriate node	
	Node	^node = Find( relpath );
	
	// if specified item was not found, throw exception
	if( relpath != "" )
		throw gcnew ArgumentException(String::Format(
		ERR_PATH_NOT_FOUND, path ));

	// all right, pas call to derived class
	return node->Value;

EXIT_READ(_lock)}

void Node::default::set( String ^path, ValueBox value )
{ENTER_WRITE(_lock)
	
	// check for null reference
	if( path == nullptr ) throw gcnew ArgumentNullException("path");

	// copy path because of, it will be modified by Find request 
	String	^relpath = path;
	// search apropriate node	
	Node	^node = Find( relpath );
	
	// if specified item was not found, throw exception
	if( relpath != "" )
		throw gcnew ArgumentException(String::Format(
		ERR_PATH_NOT_FOUND, path ));

	// all right, pas call to derived object
	node->Value = value;

EXIT_WRITE(_lock)}