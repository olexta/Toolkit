/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		Item.h														*/
/*																			*/
/*	Content:	Definition of Item class									*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008-2009 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "Settings.h"
#include "Node.h"
#include "ValueBox.h"

using namespace System;
using namespace System::Collections::Generic;


_SETTINGS_BEGIN
ref class Adapter;

/// <summary>
/// Class Item implements simple setting functionality.
/// </summary><remarks>
/// It is the only one class that get access to values. Item is
/// derived from Node class, so it provide all it functionality.
/// Each Item can be in one of the two states: online and offline.
/// Online state means that item get and set value to adapter
/// directly. In offline, mode value is cached to the item's
/// internal storage. When some item is removed from Childs
/// collection it cache data from adapter. After attach item to
/// online childs it flushes internal storage to adapter.
/// </remarks>
public ref class Item sealed : Node
{
public:
	enum class STATE {Unknown, Offline, Online};

private:
	STATE		m_state;		// connection state to adapter
	ValueBox	m_value;		// store value in standalone mode

	STATE check_parent( Node ^parent );
	Adapter^ get_adapter( void );
	bool set_state( STATE state, bool sync );
	//IEnumerable<Item^>^ expand_childs( void );

	Item( Item ^parent, String ^path, Item^ *leaf );

internal:
	static Item^ create_chain( Item ^parent, String ^path );
	static Item^ create_chain( String ^path, ValueBox value );

protected:
	virtual void OnSetParent( Node ^parent ) override;
	virtual void OnSetParentComplete( Node ^parent ) override;

public:
	Item( String ^name );
	Item( String ^name, ValueBox value );
	Item( Item ^item );
	virtual ~Item( void );

	property STATE State {
		STATE get( void );
	}
	property ValueBox default[String^] {
		virtual void set( String ^path, ValueBox value ) override;
	}
	property ValueBox Value {
		virtual ValueBox get( void ) override;
		virtual void set( ValueBox value ) override;
	}

	virtual void Load( void ) override;
	virtual void Save( void ) override;
};
_SETTINGS_END
