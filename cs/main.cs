// main.cs - Llama 2 LLM command-line interface 
// Copyright (c) 2026 Piotr Fusik
// SPDX-License-Identifier: MIT

using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

class DotNetLoader : Loader
{
	BinaryReader BR;

	public override void Open(string path)
	{
		BR = new BinaryReader(File.OpenRead(path));
	}

	public override int ReadInt() => BR.ReadInt32();

	public override float ReadFloat() => BR.ReadSingle();

	protected override void ReadWeights(short[] a, int n)
	{
		const int bufSize = 4096;
		byte[] buf = new byte[bufSize << 1];
		for (int i = 0; i < n; i += bufSize) {
			int m = Math.Min(n - i, bufSize) << 1;
			if (BR.Read(buf, 0, m) != m)
				throw new IOException("Truncated read");
			buf.AsSpan(0, m).CopyTo(MemoryMarshal.AsBytes(a.AsSpan(i, m >> 1)));
		}
	}

	public override string ReadString() => Encoding.UTF8.GetString(BR.ReadBytes(BR.ReadInt32()));

	public override void Close() => BR.Close();
}

public static class Llama2DotNet
{
	public static int Main(string[] args)
	{
		return Llama2Cli.Run(args, args.Length, new DotNetLoader());
	}
}
