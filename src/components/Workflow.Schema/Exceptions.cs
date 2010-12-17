****************************************************************************
//*	Project		:	Workflow.Schema
//*
//*	Module		:	Exceptions.cs
//*
//*	Content		:	Custom library exceptions
//*	Author		:	Nikita Marunyak
//*	Copyright	:	Copyright © 2008 - 2009 Nikita Marunyak
//*
//* SVN			:	$Id$
//*
//****************************************************************************

using System;

namespace Toolkit.Workflow.Schema
{
	public class NoClassInformationException : ApplicationException
	{
		private IClassInfo m_Obj;

		public NoClassInformationException( IClassInfo obj )
			: base( "There's no information about specified object in the schema." )
		{
			m_Obj = obj;
		}

		public IClassInfo Obj
		{
			get { return m_Obj; }
		}
	}

	public class NoMemberInformationException : ApplicationException
	{
		private string m_MemberName;

		public NoMemberInformationException( string memberName )
			: base( "There's no information about specified class member in the schema." )
		{
			m_MemberName = memberName;
		}

		public string MemberName
		{
			get { return m_MemberName; }
		}
	}

	public class EnumTypeIsNotFoundException: ApplicationException
	{
		private string m_TypeName;

		/// <summary>
		/// Creates instance.
		/// </summary>
		public EnumTypeIsNotFoundException( string typeName )
			: base( string.Format( "Enum type {0} is not found.", typeName ) )
		{
			m_TypeName = typeName;
		}
		
		/// <remarks>
		/// Returns type name.
		/// </remarks>
		public string TypeName
		{
			get {
				return m_TypeName;
			}
		}
	}
}
