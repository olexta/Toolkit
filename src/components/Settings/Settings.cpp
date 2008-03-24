/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Settings											*/
/*																			*/
/*	Module:		Settings.cpp												*/
/*																			*/
/*	Content:	This is the main DLL file.									*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

//#include "windows.h"
//#include "Settings.h"
//#include "IAdapter.h"
//#include "Node.h"
//#include "Nodes.h"
//#include "Adapter.h"
//#include "Item.h"
#include "Item.h"
#include "Manager.h"
#include "adapters\IniFile.h" 

using namespace System;
using namespace _SETTINGS;


void print_node( Node ^node )
{
	if( node->Childs->Count == 0 ) {
		
		Console::WriteLine( node->Path + ": " + node["."] );
		return;
	}
	for each( Node ^child in node->Childs ) print_node( child );
}

int main( array<System::String ^> ^args )
{
// 	wchar_t	*buf = new wchar_t[MAX_PATH];
//	GetPrivateProfileString( L"alexta", L"val_1", NULL, buf, MAX_PATH, L"b:\\test.ini" );

	try {

		//String		^v1 = gcnew String("1");
		//String		^v2 = gcnew String("1");

		//Console::WriteLine( 2 == ValueBox(1) );
		//Console::WriteLine( Object::ReferenceEquals( 1, 1 ) );
		
		Manager	^mng = gcnew Manager(gcnew IniFile("b:\\test.ini", "ini"));
		//b->Load();
		//Console::WriteLine( b["/test/alexta/val_1"] );
		mng["/ini/alexta"] = nullptr;

		print_node( mng );
		Console::WriteLine();

		mng->Save();

	//	Node	^node = b->Childs["test/alexta"];
	//	print_node(node);
	//	Console::WriteLine();

	//	b["/test/../test/alexta/nikita/dinner"] = "WOW";
	//	
	//	Item	^item = gcnew Item("nikita", 8.88);
	//	item->Childs->Add( gcnew Item("dinner") );
	//	node->Childs->Add(item);
	//	print_node(node);

		//Console::WriteLine( b["/test/../test/alexta/nikita"] );

		Console::WriteLine();

		Console::WriteLine( "\nOK" );
	} catch( Exception ^e ) {

		Console::WriteLine( e );
	} finally {
		
		Console::ReadLine();
	}


	return 0;
}