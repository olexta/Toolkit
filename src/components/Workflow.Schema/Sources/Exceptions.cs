using System;

namespace Workflow.Schema
{
	public class SchemaIsAlreadyInitedException : ApplicationException
	{
		public SchemaIsAlreadyInitedException()
			: base( "Schema is already inited." )
		{
		}
	}

	public class CantChangeValueException : ApplicationException
	{
		public CantChangeValueException()
			: base( "Can't change value because schema is already inited." )
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
}
