/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		Nodes.h														*/
/*																			*/
/*	Content:	Definition of Node::Nodes class								*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008-2009 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "Settings.h"
#include "Node.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace Toolkit::Collections;


_SETTINGS_BEGIN
/// <summary>
/// Represents a collection of subnodes.
/// </summary><remarks>
/// This class inherits from the generic KeyedMap. Each node have
/// it's own unique name, so this name is used as key. Nodes class
/// override some On... routines to handle collection changes. Also,
/// default indexer and Contains is overriden to accept relative
/// pathes.
/// </remarks>
ref class Node::Nodes sealed : KeyedMap<String^, Node^>
{
private:
	Node^		const _parent;	// parent node

protected:
	virtual void OnClear( void ) override;
	virtual void OnInsertComplete( Node ^node ) override;
	virtual void OnRemoveComplete( Node ^node ) override;

public:
	Nodes( Node ^parent );
	explicit Nodes( Node ^parent, Node ^child );
	explicit Nodes( Node ^parent, IEnumerable<Node^> ^childs );

	property Node^ default[String^] {
		virtual Node^ get( String^ path ) override;
	}

	virtual bool Contains( String ^path ) override;
	virtual bool Remove( String ^path ) override;

	void Add( Node ^node, bool force );
	void Clear( bool force );
	bool Remove( String ^name, bool force );

	void Revert( void );
};
_SETTINGS_END
