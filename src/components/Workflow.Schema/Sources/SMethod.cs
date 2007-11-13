//****************************************************************************
//*	Project		:	Robust Persistence Layer
//*
//*	Module		:	SMethod.cs
//*
//*	Content		:	Implementation of CSMethod class
//*	Author		:	Alexander Kurbatov, Nikita Marunyak
//*	Copyright	:	Copyright © 2006, 2007 Alexander Kurbatov, Nikita Marunyak
//*
//*	This class is used to store following some class's method information:
//*		1) isAvailable - Defines whether or not this method may be involved
//*
//*	Class implements ISerializable, so it can serialize/deserialize itself
//*
//****************************************************************************
using System;

namespace Workflow.Schema
{
	/// <remarks/>
	public class SMethod : SClassMember
	{
		/// <summary>
		/// Stores method involve possibility
		/// </summary>
		private bool m_IsAvailable = false;			

		/// <summary>
		/// Property accessor to m_isAvailable.
		/// Set accessor has only internal access modifier
		/// </summary>
		public bool IsAvailable
		{
			get
			{
				return m_IsAvailable;
			}
			// can't change property from out side in security reason
			internal set
			{
				m_IsAvailable = value;
			}
		}

		/// <summary>
		/// Overridden Object.ToString().
		/// </summary>
		public override string ToString()
		{
			return Name;
		}

		/// <summary>
		/// Internal class ctor.
		/// </summary>
		internal SMethod( )
		{
		}

		/// <summary>
		/// Internal copy ctor.
		/// </summary>
		internal SMethod( SMethod method )
			: base( method )
		{
			IsAvailable = method.IsAvailable;
		}
	}
}
