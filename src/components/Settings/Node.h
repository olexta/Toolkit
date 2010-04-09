/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		Node.h														*/
/*																			*/
/*	Content:	Definition of Node class									*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2008-2009 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "Settings.h"

using namespace System;
using namespace System::Threading;
using namespace Toolkit::Collections;


_SETTINGS_BEGIN
ref class Nodes;

/// <summary>
/// This class encapsulates tree representation of settings.
/// </summary><remarks>
/// This is the class from which all classes that accept tree
/// approach inherit from. Each node have it's own key - name,
/// so class implements Toolkit::Collections::IKeyedObject
/// interface. Name is const during node's lifecycle. Also, Path
/// property returns path to root node (where each node is
/// separated by delimeter). Default accessor gets or sets value
/// by specified relative path (this node value can be modified
/// by using of property Value). By using Load/Save functions
/// you can reload/flush data of this node and all it's subnodes.
/// </remarks>
public ref class Node abstract: IKeyedObject<String^>
{
protected:
	ref class Nodes;

public:
	value class ValueBox;

private:
	Node			^m_parent;					// reference to parent node

	void check_name( String ^name );
	void set_parent( Node ^parent );
	Node^ get_child( String ^path );

protected:
	static	String^	const _delimeter = "/";		// delimeter
	static	String^	const _cur_path	 = ".";		// path to current item
	static	String^	const _par_path  = "..";	// path to parent item

	String^				const _name;			// name of the node
	ReaderWriterLock^	const _lock;			// lock object
	initonly Nodes^		_childs;				// subnodes collection

	static String^ ParsePath( String^ %path );

	String^ RootTraverse( Node^ *root );
	Node^ Find( String^ %path );

	virtual void OnSetParent( Node ^parent );
	virtual void OnSetParentComplete( Node ^parent );

public:
	Node( String ^name );
	Node( String ^name, Node ^parent );
	virtual ~Node( void );

	property String^ Delimeter {
		static String^ get( void );
	}
	property String^ Name {
		virtual String^ get( void ) = IKeyedObject<String^>::Key::get;
	}
	property String^ Path {
		String^ get( void );
	}
	property KeyedMap<String^, Node^>^ Childs {
		KeyedMap<String^, Node^>^ get( void );
	}
	property ValueBox default[String^] {
		virtual ValueBox get( String ^path );
		virtual void set( String ^path, ValueBox value );
	}
	property ValueBox Value {
		virtual ValueBox get( void ) abstract;
		virtual void set( ValueBox value ) abstract;
	}

	virtual void Load( void ) abstract;
	virtual void Save( void ) abstract;
};
_SETTINGS_END
