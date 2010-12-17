**************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentProperties.cpp									*/
/*																			*/
/*	Content:	Implementation of PersistentProperties class				*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#include "PersistentProperties.h"

using namespace _RPL;


//-----------------------------------------------------------------------------
//					Toolkit::RPL::PersistentProperties
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------
/// <summary>
/// Default class constructor.
/// </summary>
//-------------------------------------------------------------------
PersistentProperties::PersistentProperties( void )
{
	// do nothing
}


//-------------------------------------------------------------------
/// <summary>
/// Create instance of the PersistentProperties class initialized
/// with KeyValue pairs in the given collection.
/// </summary><remarks>
/// If pairs in collection have not unique keys then only the last
/// value will be stored. All null references will be ignored and no
/// events will be faired.
/// </remarks>
//-------------------------------------------------------------------
PersistentProperties::PersistentProperties(								   \
						IEnumerable<KeyValuePair<String^, ValueBox>> ^e ): \
	Map<String^, ValueBox>(e)
{
	// do nothing
}
