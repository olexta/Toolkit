//****************************************************************************
//*	Project		:	Workflow.Schema
//*
//*	Module		:	SMethod.cs
//*
//*	Content		:	Implementation of SMethod class
//*	Author		:	Alexander Kurbatov, Nikita Marunyak
//*	Copyright	:	Copyright © 2006, 2007 Alexander Kurbatov, Nikita Marunyak
//*
//*	This class is used to store following some class's method information:
//*		1) isAvailable - Defines whether or not this method may be involved
//*
//****************************************************************************
using System;

namespace Toolkit.Workflow.Schema
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
			get	{ return m_IsAvailable;	}
			internal set { m_IsAvailable = value; }
		}

		/// <summary>
		/// Ctor.
		/// </summary>
		public SMethod(
			string name,
			string caption,
			string description,
			int displayOrder,
			bool isAvailable)
			: base( name, caption, description, displayOrder )
		{
			m_IsAvailable = isAvailable;
		}

		/// <summary>
		/// Internal copy ctor.
		/// </summary>
		public SMethod( SMethod method )
			: base( method )
		{
			IsAvailable = method.IsAvailable;
		}
	}
}
