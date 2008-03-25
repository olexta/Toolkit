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

#include "Item.h"
#include "Manager.h"
#include "adapters\IniFile.h" 

using namespace System;
using namespace _SETTINGS;


int main( array<System::String ^> ^args )
{
	try {

		Console::WriteLine( "\nOK" );
	} catch( Exception ^e ) {

		Console::WriteLine( e );
	} finally {
		
		Console::ReadLine();
	}

	return 0;
}