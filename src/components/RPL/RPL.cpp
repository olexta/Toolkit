/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		RPL.cpp														*/
/*																			*/
/*	Content:	This is the main DLL file.									*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "RPL.h"

using namespace System;


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