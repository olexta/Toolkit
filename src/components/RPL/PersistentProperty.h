/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentProperty.h										*/
/*																			*/
/*	Content:	Definition of CPersistentProperty class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"

using namespace System;


_RPL_BEGIN
ref class StreamWrapper;
ref class CPersistentProperty;
delegate void PP_CHANGE( CPersistentProperty^ sender,
						 Object^ oldValue, Object ^newValue );

/// <sumamry>
/// Provide internal access to the CPersistentProperty class.
/// </sumamry><remarks>
/// This is more flexible realisation of a "internal" access modifier. This
/// interface can be used in .NET Remoting.
/// </remarks>
private interface class IIPersistentProperty
{
	event PP_CHANGE ^OnChange {
		void add( PP_CHANGE ^d );
		void remove( PP_CHANGE ^d );
	}
	void SetChanged( bool value );
};

/// <summary>
/// Implement object's property functionality.
/// </summary><remarks>
/// Now only followed property types is supported: bool, int, double, DateTime,
/// String, Stream and DBNull.
/// </remarks>
[Serializable]
public ref class CPersistentProperty sealed : /*MarshalByRefObject,*/
											  IIPersistentProperty
{
private:
	String		^m_name;
	Object		^m_value;
	bool		m_changed;
	
	PP_CHANGE	^m_on_change;
	Object		^_lock_this;

	void set_value( Object ^value );
	void on_change( StreamWrapper^ sender );

// IIPersistentProperty
private:
	event PP_CHANGE ^OnChange {
		virtual void add( PP_CHANGE ^d ) sealed =
			IIPersistentProperty::OnChange::add;
		virtual void remove( PP_CHANGE ^d ) sealed =
			IIPersistentProperty::OnChange::remove;
		virtual void raise( CPersistentProperty ^sender,
							Object ^oldValue, Object ^newValue ) sealed;
	}
	virtual void SetChanged( bool value ) sealed =
		IIPersistentProperty::SetChanged;

protected:
	void OnValidate( Object^ value );

public:
	CPersistentProperty( String ^name );
	CPersistentProperty( String ^name, Object ^value );
	explicit CPersistentProperty( const CPersistentProperty %prop );
	~CPersistentProperty( void );

	CPersistentProperty% operator=( Object^ value );
	CPersistentProperty% operator=( const CPersistentProperty %value );
	bool operator==( const CPersistentProperty %value );
	bool operator!=( const CPersistentProperty %value );
	operator String^( void );
	
	property String^ Name {
		String^ get( void );
	}
	property Object^ Value {
		Object^ get( void );
		void set( Object ^value );
	}
	property bool IsChanged {
		bool get( void );
	}
	property Object^ SyncRoot {
		Object^ get( void );
	}

	virtual String^ ToString( void ) override;
};
_RPL_END