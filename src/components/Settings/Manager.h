/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		Manager.h													*/
/*																			*/
/*	Content:	Definition of Manager class									*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright @ 2007-2008 Alexey Tkachuk						*/
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
/// This class provide main functionality to manage settings.
/// </summary><remarks>
/// In many ways this class is the key to the settings. It must
/// be created first and is used to manage adapters to different
/// settings storage mechanisms (there are no other methods to
/// add adapter except 'Add' function). Manager inherits from
/// Node class and is root to all tree of settings. To access
/// Manager from any other node you can use "/" path. Root is
/// only one node that have empty name.
/// </remarks>
public ref class Manager : Node
{
protected:
	virtual void OnSetParent( Node ^parent ) override;

public:
	Manager( void );
	Manager( IAdapter ^adapter );
	Manager( IEnumerable<IAdapter^> ^adapters );
	
	property ValueBox default[String^] {
		virtual void set( String ^path, ValueBox value ) override;
	}
	property ValueBox Value {
		virtual ValueBox get( void ) override;
		virtual void set( ValueBox value ) override;
	}

	void Add( IAdapter ^adapter );
	void Remove( String ^adapter );
	
	virtual void Load( void ) override;
	virtual void Save( void ) override;
};
_SETTINGS_END