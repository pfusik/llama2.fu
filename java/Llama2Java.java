// Llama2Java - Llama 2 LLM command-line interface 
// Copyright (c) 2026 Piotr Fusik
// SPDX-License-Identifier: MIT

import java.io.EOFException;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;

class JavaLoader extends Loader
{
	private FileChannel channel;

	@Override
	public void open(String path)
	{
		try {
			channel = FileChannel.open(Paths.get(path), StandardOpenOption.READ);
		}
		catch (IOException e) {
			throw new RuntimeException(e);
		}
	}

	private ByteBuffer read(int n)
	{
		ByteBuffer buf = ByteBuffer.allocate(n).order(ByteOrder.LITTLE_ENDIAN);
		try {
			if (channel.read(buf) != n)
				throw new EOFException();
		}
		catch (IOException e) {
			throw new RuntimeException(e);
		}
		return buf.flip();
	}

	@Override
	public int readInt()
	{
		return read(4).getInt();
	}

	@Override
	public float readFloat()
	{
		return read(4).getFloat();
	}

	@Override
	protected void readWeights(short[] a, int n)
	{
		final int BUF_SIZE = 4096;
		ByteBuffer buf = ByteBuffer.allocate(BUF_SIZE << 1).order(ByteOrder.LITTLE_ENDIAN);
		for (int i = 0; i < n; i += BUF_SIZE) {
			int m = Math.min(n - i, BUF_SIZE);
			buf.limit(m << 1);
			try {
				if (channel.read(buf) != m << 1)
					throw new EOFException();
			}
			catch (IOException e) {
				throw new RuntimeException(e);
			}
			buf.flip().asShortBuffer().get(a, i, m);
			buf.clear();
		}
	}

	@Override
	public String readString()
	{
		return StandardCharsets.UTF_8.decode(read(readInt())).toString();
	}

	@Override
	public void close()
	{
		try {
			channel.close();
		}
		catch (IOException e) {
			throw new RuntimeException(e);
		}
	}
}

public class Llama2Java
{
	public static void main(String[] args)
	{
		System.exit(Llama2Cli.run(args, args.length, new JavaLoader()));
	}
}
