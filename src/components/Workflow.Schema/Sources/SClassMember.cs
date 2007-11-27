//****************************************************************************
//*	Project		:	Workflow.Schema
//*
//*	Module		:	SClassMember.cs
//*
//*	Content		:	Abstract class that represent type members.
//*	Author		:	Nikita Marunyak
//*	Copyright	:	Copyright © 2007 Nikita Marunyak
//*
//****************************************************************************

using System;

namespace Workflow.Schema
{
	/// <summary>
	/// Class member information abstract class.
	/// Implement shared properties, such as: Name, Caption and Description.
	/// </summary>
	public class SClassMember : Toolkit.Collections.IKeyedObject<string>
	{
		private string m_Name;
		private string m_Caption;
		private string m_Description;
		private int m_DisplayOrder;

		/// <summary>
		/// Gets, sets member's name.
		/// <remarks>Setter is internal only.</remarks>		
		/// </summary>
		public string Name
		{
			get { return m_Name; }
			internal set { m_Name = value; }
		}

		/// <summary>
		/// Gets, sets class member user acceptance name.
		/// <remarks>Setter is internal only.</remarks>
		/// </summary>
		public string Caption
		{
			get { return m_Caption; }
			internal set { m_Caption = value; }
		}

		/// <summary>
		/// Gets, sets class member description msg.
		/// <remarks>Setter is internal only.</remarks>
		/// </summary>
		public string Description
		{
			get { return m_Description; }
			internal set { m_Description = value; }
		}

		/// <summary>
		/// Gets, sets class display position.
		/// <remarks>Setter is internal only.</remarks>
		/// </summary>
		public int DisplayOrder
		{
			get { return m_DisplayOrder; }
			internal set { m_DisplayOrder = value; }
		}

		/// <summary>
		/// Overridden Object.ToString().
		/// </summary>
		public override string ToString()
		{
			return Name;
		}

		/// <summary>
		/// Ctor.
		/// </summary>
		protected SClassMember(
			string name,
			string caption,
			string description,
			int displayOrder)
		{
			m_Name = name;
			m_Caption = caption;
			m_Description = description;
			m_DisplayOrder = displayOrder;
		}

		/// <summary>
		/// Copy ctor.
		/// </summary>
		protected SClassMember( SClassMember member )
		{
			Name = member.Name;
			Caption = member.Caption;
			Description = member.Description;
			DisplayOrder = member.DisplayOrder;
		}		
		
		/// <summary>
		/// Realization of KeyedObject;ltT;gt.Key property.
		/// </summary>
		string Toolkit.Collections.IKeyedObject<string>.Key
		{
			get { return Name; }
		}
	}
}
