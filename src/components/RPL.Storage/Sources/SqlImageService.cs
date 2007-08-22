//****************************************************************************
//*	Project		:	Robust Persistence Layer
//*
//*	Module		:	ISqlStream.cs
//*
//*	Content		:	Interface for writing/reading image data from/to MS SqlServer
//*	Author		:	Alexander Kurbatov
//*	Copyright	:	Copyright © 2006-2007 Alexander Kurbatov
//*
//****************************************************************************
using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace ABBYY.Toolkit.RPL.Storage
{
	public abstract class SqlImageService : MarshalByRefObject
	{
		/// <summary>
		/// Save stream in case that is in  new property
		/// </summary>
		public abstract void Save( ref int id, int ObjectID, string PropertyName, Stream stream );
		/// <summary>
		/// update stream for existing property
		/// </summary>
		public abstract void Save( int id, Stream stream );
		/// <summary>
		/// Reads a sequence of bytes from specific property id
		/// </summary>
		public abstract int Read( int id, byte[] buffer, int offset, int count, long position );
		/// <summary>
		/// gets length of image property
		/// </summary>
		public abstract long Length( int id );
	}
}
