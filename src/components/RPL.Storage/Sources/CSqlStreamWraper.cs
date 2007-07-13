//****************************************************************************
//*   Project:   RPL.Storage
//*
//*   Module:      ISqlStream.cs
//*
//*   Content:   Interface for writing/reading image data from/to MS SqlServer
//*   Author:      Alexander Kurbatov
//*   Copyright:   Copyright © 2006-2007 Alexander Kurbatov
//*
//****************************************************************************
using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace RPL.Storage {
	public abstract class CSqlImageService : MarshalByRefObject
	{
		//Save stream in case that is in  new property
		public abstract void Save( ref int id, int ObjectID, string PropertyName, Stream stream );
		//update stream for existing property
		public abstract void Save( int id, Stream stream );
		//Reads a sequence of bytes from specific property id
		public abstract int Read( int id, byte[] buffer, int offset, int count, long position );
		//gets length of image property
		public abstract long Length( int id );
	}
}
