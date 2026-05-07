// main.cpp - Llama 2 LLM command-line interface 
// Copyright (c) 2026 Piotr Fusik
// SPDX-License-Identifier: MIT

#include <fstream>
#ifdef _WIN32
#include <windows.h>
#else
#include <locale.h>
#endif

#include "llama2cli.hpp"

class CppLoader : public Loader
{
	std::ifstream s;

	template <typename T>
	void read(T *buf, size_t n)
	{
		s.read(reinterpret_cast<char *>(buf), n * sizeof(T));
	}

protected:
	void readWeights(int16_t *a, ptrdiff_t n) override
	{
		read(a, n);
	}

public:
	void open(std::string_view path) override
	{
		s.exceptions(std::ifstream::failbit);
		s.open(std::string(path), std::ios::in | std::ios::binary);
	}

	int readInt() override
	{
		int result;
		read(&result, 1);
		return result;
	}

	float readFloat() override
	{
		float result;
		read(&result, 1);
		return result;
	}

	std::string readString() override
	{
		auto n = readInt();
		std::string result(n, '\0');
		read(result.data(), n);
		return result;
	}

	void close() override
	{
		s.close();
	}
};

#ifdef _WIN32
int wmain(int argc, wchar_t **wargv)
{
	auto argstrs = std::make_unique<std::string[]>(argc - 1);
	auto args = std::make_unique<std::string_view[]>(argc - 1);
	for (int i = 1; i < argc; i++) {
		int size = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, NULL, 0, NULL, NULL) - 1;
		argstrs[i - 1].resize(size);
		WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, argstrs[i - 1].data(), size, NULL, NULL);
		args[i - 1] = argstrs[i - 1];
	}
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
#else
int main(int argc, char **argv)
{
	setlocale(LC_ALL, "C.UTF-8");
	auto args = std::make_unique<std::string_view[]>(argc - 1);
	for (int i = 1; i < argc; i++)
		args[i - 1] = argv[i];
#endif
	CppLoader loader;
	return Llama2Cli::run(args.get(), argc - 1, &loader);
}
