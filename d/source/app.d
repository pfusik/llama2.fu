// app.d - Llama 2 LLM command-line interface 
// Copyright (c) 2026 Piotr Fusik
// SPDX-License-Identifier: MIT

import std.stdio;

import llama2cli;

class DLoader : Loader
{
	private File f;

	override void open(string path) => f.open(path);

	override int readInt()
	{
		int result;
		f.rawRead((&result)[0 .. 1]);
		return result;
	}

	override float readFloat()
	{
		float result;
		f.rawRead((&result)[0 .. 1]);
		return result;
	}

	override void readWeights(short[] a, ptrdiff_t n)
	{
		f.rawRead(a);
	}

	override string readString()
	{
		ubyte[] b = new ubyte[readInt()];
		f.rawRead(b);
		return cast(string) b;
	}

	override void close() => f.close();
}

int main(string[] args)
{
	return Llama2Cli.run(args[1 .. $], cast(int) args.length - 1, new DLoader());
}
