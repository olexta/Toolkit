﻿//****************************************************************************
//*	Project		:	Robust Persistence Layer
//*
//*	Module		:	IClassInfo.cs
//*
//*	Content		:	Definition of IClassInfo interface
//*	Author		:	Alexander Kurbatov
//*	Copyright	:	Copyright © 2006 Alexander Kurbatov
//*
//****************************************************************************
using System;
using System.Collections.Generic;
using System.Text;

namespace ABBYY.Toolkit.RPL.Schema
{
	/// <summary>
	/// Interface for bussines classes which allows schema object to be build for
	/// specific instance of bussines class
	/// </summary>
	public interface IClassInfo
	{
		/// <summary>
		/// returns bussines logic name of class
		/// </summary>
		string Type { get;}

		/// <summary>
		/// returs array of strings which are component parts of global state name.
		/// Part with highest priority must be first and so on...
		/// </summary>
		string[] StateComponents { get;}
	}
}