/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentProperty.h										*/
/*																			*/
/*	Content:	Definition of PersistentProperty class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"

using namespace System;
using namespace _COLLECTIONS;


_RPL_BEGIN
ref class StreamWrapper;
ref class PersistentProperty;
delegate void PP_CHANGE( PersistentProperty ^sender,
						 Object ^oldValue, Object ^newValue );

/// <sumamry>
/// Provide internal access to the PersistentProperty class.
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
public ref class PersistentProperty sealed : //MarshalByRefObject,
											 IKeyedObject<String^>,
											 IIPersistentProperty
{
private:
	String			^m_name;
	Object			^m_value;
	bool			m_changed;
	PP_CHANGE		^m_on_change;

	void set_value( Object ^value );
	void stream_change( StreamWrapper ^sender );

// IIPersistentProperty
private:
	event PP_CHANGE ^on_change {
		virtual void add( PP_CHANGE ^d ) sealed =
			IIPersistentProperty::OnChange::add;
		virtual void remove( PP_CHANGE ^d ) sealed =
			IIPersistentProperty::OnChange::remove;
		virtual void raise( PersistentProperty ^sender,
							Object ^oldValue, Object ^newValue ) sealed;
	}

	virtual void set_changed( bool value ) sealed =
		IIPersistentProperty::SetChanged;

protected:
	void OnValidate( Object^ value );

public:
	PersistentProperty( String ^name );
	PersistentProperty( String ^name, Object ^value );
	explicit PersistentProperty( const PersistentProperty %prop );
	~PersistentProperty( void );

	PersistentProperty% operator=( Object ^value );
	PersistentProperty% operator=( const PersistentProperty %value );
	bool operator==( const PersistentProperty %value );
	bool operator!=( const PersistentProperty %value );
	operator String^( void );
	
	property String^ Name {
		virtual String^ get( void ) = IKeyedObject<String^>::Key::get;
	}
	property Object^ Value {
		Object^ get( void );
		void set( Object ^value );
	}
	property bool IsChanged {
		bool get( void );
	}

	virtual bool Equals( Object ^obj ) override;
	virtual String^ ToString( void ) override;
};
_RPL_END