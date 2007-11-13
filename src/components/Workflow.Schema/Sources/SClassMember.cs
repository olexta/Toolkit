using System;

namespace Workflow.Schema
{
	/// <summary>
	/// Class member information abstract class.
	/// Implement shared properties, such as: Name, Caption and Description.
	/// </summary>
	public abstract class SClassMember : Toolkit.Collections.IKeyedObject<string>
	{
		private string m_Name;
		private string m_Caption;
		private string m_Description;

		/// <summary>
		/// Gets, sets member's name.
		/// <remarks>Setter is internal only.</remarks>		
		/// </summary>
		public virtual string Name
		{
			get { return m_Name; }
			internal set { m_Name = value; }
		}

		/// <summary>
		/// Gets, sets class member user acceptance name.
		/// <remarks>Setter is internal only.</remarks>
		/// </summary>
		public virtual string Caption
		{
			get { return m_Caption; }
			internal set { m_Caption = value; }
		}

		/// <summary>
		/// Gets, sets class member description msg.
		/// <remarks>Setter is internal only.</remarks>
		/// </summary>
		public virtual string Description
		{
			get { return m_Description; }
			internal set { m_Description = value; }
		}

		/// <summary>
		/// Internal ctor.
		/// </summary>
		internal SClassMember()
		{
		}

		/// <summary>
		/// Internal copy ctor.
		/// </summary>
		internal SClassMember( SClassMember member )
		{
			Name = member.Name;
			Caption = member.Caption;
			Description = member.Description;
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
