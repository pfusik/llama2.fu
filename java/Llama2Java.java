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
		return read(4).asIntBuffer().get();
	}

	@Override
	public float readFloat()
	{
		return read(4).asFloatBuffer().get();
	}

	@Override
	protected void readWeights(short[] a, int n)
	{
		read(n << 1).asShortBuffer().get(a);
	}

	@Override
	public void skipBytes(int n)
	{
		try {
			channel.position(channel.position() + n);
		}
		catch (IOException e) {
			throw new RuntimeException(e);
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
