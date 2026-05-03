// main.c - Llama 2 LLM command-line interface 
// Copyright (c) 2023 Andrej (original C code: https://github.com/karpathy/llama2.c)
// Copyright (c) 2026 Piotr Fusik
// SPDX-License-Identifier: MIT

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <locale.h>
#endif

#include "llama2cli.h"

typedef struct {
	void (*open)(Loader *self, const char *path);
	int (*readInt)(Loader *self);
	float (*readFloat)(Loader *self);
	void (*readWeights)(Loader *self, short *a, ptrdiff_t n);
	void (*skipBytes)(Loader *self, ptrdiff_t n);
	char *(*readString)(Loader *self);
	void (*close)(Loader *self);
} LoaderVtbl;

struct Loader {
	const LoaderVtbl *vtbl;
	FILE *fp;
};

static void Loader_Open(Loader *self, const char *path)
{
	self->fp = fopen(path, "rb");
	if (self->fp == NULL) {
		perror(path);
		exit(1);
	}
}

static void Loader_Read(Loader *self, void *buf, size_t size)
{
	if (fread(buf, size, 1, self->fp) != 1) {
		perror("Error reading file");
		exit(1);
	}
}

static int Loader_ReadInt(Loader *self)
{
	int result;
	Loader_Read(self, &result, sizeof(result));
	return result;
}

static float Loader_ReadFloat(Loader *self)
{
	float result;
	Loader_Read(self, &result, sizeof(result));
	return result;
}

static void Loader_ReadWeights(Loader *self, short *a, ptrdiff_t n)
{
	return Loader_Read(self, a, n * sizeof(short));
}

static void Loader_SkipBytes(Loader *self, ptrdiff_t n)
{
#ifdef _WIN32
	_fseeki64(self->fp, n, SEEK_CUR);
#else
	fseek(self->fp, n, SEEK_CUR);
#endif
}

static char *Loader_ReadString(Loader *self)
{
	int n = Loader_ReadInt(self);
	char *s = malloc(n + 1);
	Loader_Read(self, s, n);
	s[n] = '\0';
	return s;
}

static void Loader_Close(Loader *self)
{
	fclose(self->fp);
}

#ifdef _WIN32
int wmain(int argc, wchar_t **wargv)
{
	char **argv = malloc(argc * sizeof(char *));
	for (int i = 1; i < argc; i++) {
		int size = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, NULL, 0, NULL, NULL);
		argv[i] = malloc(size);
		WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, argv[i], size, NULL, NULL);
	}
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
#else
int main(int argc, char **argv)
{
	setlocale(LC_ALL, "C.UTF-8");
#endif
	static const LoaderVtbl vtbl = {
		Loader_Open,
		Loader_ReadInt,
		Loader_ReadFloat,
		Loader_ReadWeights,
		Loader_SkipBytes,
		Loader_ReadString,
		Loader_Close
	};
	Loader loader = { &vtbl };
	return Llama2Cli_Run((const char *const *) argv + 1, argc - 1, &loader);
}
