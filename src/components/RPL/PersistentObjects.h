/****************************************************************************/
/*																			*/
/*	Project:	Robust Persistence Layer									*/
/*																			*/
/*	Module:		PersistentObjects.h											*/
/*																			*/
/*	Content:	Definition of PersistentObjects class						*/
/*																			*/
/*	Author:		Alexey Tkachuk												*/
/*	Copyright:	Copyright © 2007-2008 Alexey Tkachuk						*/
/*				All Rights Reserved											*/
/*																			*/
/****************************************************************************/

#pragma once
#include "RPL.h"

using namespace System;
using namespace System::Collections::Generic;


_RPL_BEGIN
ref class PersistentObject;

/// <summary>
/// This class provide services for collection of objects.
/// </summary><remarks>
/// It provides some usefull routines such as unique object control,
/// null reference control and some others. It implements OnXXX events
/// to control collection access too.
/// </remarks>
public ref class PersistentObjects : MarshalByRefObject,
									 ICollection<PersistentObject^>
{
private:
	// IEnumerable
	virtual System::Collections::IEnumerator^ get_enumerator( void ) sealed =
		System::Collections::IEnumerable::GetEnumerator;
	
	// IEnumerable<PersistenObject^>
	virtual bool items_is_readonly( void ) sealed =
		ICollection<PersistentObject^>::IsReadOnly::get;
	virtual IEnumerator<PersistentObject^>^ items_get_enumerator( void ) sealed =
		IEnumerable<PersistentObject^>::GetEnumerator;

protected:
	List<PersistentObject^>	m_list;

	virtual void OnClear( void );
	virtual void OnInsert( PersistentObject ^prop );
	virtual void OnRemove( PersistentObject ^prop );
	virtual void OnClearComplete( void );
	virtual void OnInsertComplete( PersistentObject ^prop );
	virtual void OnRemoveComplete( PersistentObject ^prop );

public:
	// TODO: понять как будет работать даное решение при тонком клиенте
	// (объект PersistentObjects находится на сервере + мы добавляем объект
	// MarshalByRef, который находится на том-же сервере: вопрос, не будут-ли
	// запросы от PersistentObjects на объект проходить через клиент)

	PersistentObjects( void );
	explicit PersistentObjects( IEnumerable<PersistentObject^> ^e );

	property int Count {
		virtual int get( void );
	}

	virtual void Add( PersistentObject ^obj );
	virtual void Clear( void );
	virtual bool Contains( PersistentObject ^obj );
	virtual void CopyTo( array<PersistentObject^> ^dest, int index );
	virtual bool Remove( PersistentObject ^obj );
};
_RPL_END
