using System;
using System.Collections.Generic;
using System.Text;

using Toolkit.Workflow.Schema;

namespace Toolkit.Workflow.Schema.Test
{
	public class TestObject : IClassInfo
	{
		public string[] StateComponents
		{
			get
			{
				return new string[] { "true", "i'am" };
			}
		}

		public string Type
		{
			get
			{
				return MetaData.GetObjectType( this );
			}
		}
	}
}
