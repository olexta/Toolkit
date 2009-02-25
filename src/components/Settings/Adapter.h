/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		Adapter.h													*/
/*																			*/
/*	Content:	Definition of Adapter class									*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "Settings.h"
#include "Adapters\IAdapter.h"
#include "Node.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace _SETTINGS::Adapters;


_SETTINGS_BEGIN
value class ValueBox;

/// <summary>
/// This class encapsulates the behavior needed to save and load
/// values from some adapter.
/// </summary><remarks>
/// This is wrapper around IAdapter interface to represent this
/// class as another one node. So, neither another class can
/// acceess to IAdapter manual. Adapter is private class, so user
/// can access only through common Node functionality. In common
/// hierarhy of nodes it is placed just after root (for example
/// "/ini" - the path to adapter with name "ini".
/// </remarks>
private ref class Adapter sealed : Node
{
private:
	IAdapter^	const _adp;		// interface to adapter instance

	String^ path_to_adp( String ^fullpath );
	String^ adp_to_path( String ^adppath );

protected:
	virtual void OnSetParent( Node ^parent ) override;

public:
	Adapter( Node^ parent, IAdapter ^adapter );
	
	property ValueBox default[String^] {
		virtual void set( String ^path, ValueBox value ) override;
	}
	property ValueBox Value {
		virtual ValueBox get( void ) override;
		virtual void set( ValueBox value ) override;
	}
	
	ValueBox GetValue( String ^fullpath );
	void SetValue( String ^fullpath, ValueBox value );
	IEnumerable<Node^>^ Load( String ^fullpath );
	void Save( String ^fullpath );
	void Remove( String ^fullpath );

	virtual void Load( void ) override;
	virtual void Save( void ) override;
};
_SETTINGS_END