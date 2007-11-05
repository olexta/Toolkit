/****************************************************************************/
/*																			*/
/*	Project:	Toolkit Collections											*/
/*																			*/
/*	Module:		IKeyedObject.h												*/
/*																			*/
/*	Content:	Definition of IKeyedObject interface						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2006-2007 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "Collections.h"


_COLLECTIONS_BEGIN
///	<summary>
///	Encapsulates the behavior needed to represent object as keyed object.
///	</summary><remarks>
/// Keyed object store it's own key as some property. This property MUST BE NOT
/// CHANGED throught object's life cycle. In other case results of the all
/// generic intrfaces will be unpredictable.
///	</remarks>
generic<typename TKey>
public interface class IKeyedObject
{
	property TKey Key {
		virtual TKey get( void );
	}
};
_COLLECTIONS_END