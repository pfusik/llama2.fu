# llama2fu.py - Llama 2 LLM command-line interface 
# Copyright (c) 2026 Piotr Fusik
# SPDX-License-Identifier: MIT

import array
import struct
import sys

import llama2cli

class PythonLoader(llama2cli.Loader):
	def open(self, path: str) -> None:
		self.f = open(path, "rb")

	def read_int(self) -> int:
		return struct.unpack("i", self.f.read(4))[0]

	def read_float(self) -> float:
		return struct.unpack("f", self.f.read(4))[0]

	def _read_weights(self, a: array.array, n: int) -> None:
		assert False

	def allocate_and_read_weights(self, n: int) -> array.array:
		a = array.array("h")
		a.fromfile(self.f, n)
		return a

	def read_string(self) -> str:
		return self.f.read(self.read_int()).decode("utf8")

	def close(self) -> None:
		self.f.close()

if __name__ == "__main__":
	args = sys.argv[1:]
	sys.exit(llama2cli.Llama2Cli.run(args, len(args), PythonLoader()))
