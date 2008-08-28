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
#include "adapters\Registry.h"

using namespace System;
using namespace _SETTINGS;


int main( array<System::String ^> ^args )
{
	try {
		//
		// put test code here
		//

		//-----------------------------------------------------------
		Console::WriteLine( "\nOK" );
	} catch( Exception ^e ) {
		// print exception
		Console::WriteLine( e );
	} finally {
		// wait for user input in any case
		Console::ReadLine();
	}

	return 0;
}