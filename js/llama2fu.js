// llama2fu.js - Llama 2 LLM command-line interface 
// Copyright (c) 2026 Piotr Fusik
// SPDX-License-Identifier: MIT

import fs from "fs";
import { Loader, Llama2Cli } from "./llama2cli.js";

class NodeLoader extends Loader
{
	#f

	open(path)
	{
		this.#f = fs.openSync(path);
	}

	#read(a)
	{
		if (fs.readSync(this.#f, a) != a.length * a.BYTES_PER_ELEMENT)
			throw new Error();
	}

	readInt()
	{
		const a = new Int32Array(1);
		this.#read(a);
		return a[0];
	}

	readFloat()
	{
		const a = new Float32Array(1);
		this.#read(a);
		return a[0];
	}

	readWeights(a, n)
	{
		this.#read(a);
	}

	readString()
	{
		const b = new Uint8Array(this.readInt());
		this.#read(b);
		return new TextDecoder().decode(b);
	}

	close()
	{
		fs.closeSync(this.#f);
	}
}

const args = process.argv.slice(2);
process.exit(Llama2Cli.run(args, args.length, new NodeLoader()));
