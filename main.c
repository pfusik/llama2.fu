#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
	void (*readFloats)(Loader *self, float *a, ptrdiff_t n);
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

static void Loader_ReadFloats(Loader *self, float *a, ptrdiff_t n)
{
#define BUF_SIZE 1024
	for (ptrdiff_t i = 0; i < n; i += BUF_SIZE) {
		char buf[BUF_SIZE * 2];
		ptrdiff_t m = n - i;
		if (m > BUF_SIZE)
			m = BUF_SIZE;
		Loader_Read(self, buf, m * 2);
		for (ptrdiff_t j = 0; j < m; j++) {
			char f[4] = { 0, 0, buf[j * 2], buf[j * 2 + 1] };
			a[i + j] = *(const float *) f;
		}
	}
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

int main(int argc, char **argv)
{
#ifdef _WIN32
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
#else
	setlocale(LC_ALL, "C.UTF-8");
#endif
	static const LoaderVtbl vtbl = {
		Loader_Open,
		Loader_ReadInt,
		Loader_ReadFloat,
		Loader_ReadFloats,
		Loader_SkipBytes,
		Loader_ReadString,
		Loader_Close
	};
	Loader loader = { &vtbl };
	return Llama2Cli_Run((const char *const *) argv + 1, argc - 1, &loader, time(NULL));
}
