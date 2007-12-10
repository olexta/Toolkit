//****************************************************************************
//*	Project		:	Workflow.Schema
//*
//*	Module		:	Exceptions.cs
//*
//*	Content		:	Custom library exceptions
//*	Author		:	Nikita Marunyak
//*	Copyright	:	Copyright © 2007 Nikita Marunyak
//*
//****************************************************************************

using System;

namespace Toolkit.Workflow.Schema
{
	public class SchemaIsAlreadyInitializedException : ApplicationException
	{
		public SchemaIsAlreadyInitializedException()
			: base( "Schema is already inited." )
		{
		}
	}

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

	public class SchemaIsntInitializedException : ApplicationException
	{
		public SchemaIsntInitializedException()
			: base( "Schema isn't initialized." )
		{
		}
	}
}
