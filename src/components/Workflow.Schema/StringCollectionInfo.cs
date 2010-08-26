//****************************************************************************
//*	Project		:	Workflow.Schema
//*
//*	Module		:	StringCollectionInfo.cs
//*
//*	Author		:	Alexander Kurbatov
//*	Copyright	:	Copyright � 2008 Alexander Kurbatov
//*
//* SVN			:	$Id$	
//*
//****************************************************************************

using System;
using System.Collections.Generic;
using Toolkit.Collections;
using System.Xml;

namespace Toolkit.Workflow.Schema
{
/// <summary>
/// ��� �������� ��������� �������� ��������.
/// ������� ����������� �������� �������� � ����� �������������� ������ 
/// ��������� �������� ��� ��������.
/// </summary>
public class StringCollectionInfo : IKeyedObject<string>
{
	private string m_name;
	private List<string> m_values;

	internal StringCollectionInfo( XmlNode node )
	{
		m_values = new List<string>();
		// ��������� ����� ���������
		m_name = node.Attributes.GetNamedItem( "ws:name" ).Value;
		// ��������� �������� ���������
		foreach( XmlNode keyNode in node.SelectNodes( "ws:items/ws:item",
													  MetaData.Instance.XMLNsMgr ) )
		{
			// ��������� ����������������� ��������
			m_values.Add( Tools.GetLocalizedString( keyNode, "value" ) );
		}
	}

	string IKeyedObject<string>.Key
	{
		get { return Name; }
	}

	/// <summary>
	/// ��������� ����� ����������(��������)
	/// </summary>
	public string Name
	{
		get { return m_name; }
	}

	/// <summary>
	/// ��������� ������ ��������
	/// </summary>
	public IEnumerable<string> Values
	{
		get { return m_values; }
	}
}
}
