**************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		RPL.cpp														*/
/*																			*/
/*	Content:	This is the main DLL file.									*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "RPL.h"
#include "RetrieveCriteria.h"

using namespace System;
using namespace _RPL;


int main( array<System::String ^> ^args )
{
	try {
		//
		// put test code here
		//
		RetrieveCriteria	^rc = gcnew RetrieveCriteria(
			"", gcnew Where::Clause("oops", 1),
			gcnew OrderBy::Clause("oops"));

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
