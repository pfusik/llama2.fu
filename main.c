#include <stdio.h>
#include <stdlib.h>

#include "llama2fu.h"

typedef struct {
	int (*readInt)(Loader *self);
	float (*readFloat)(Loader *self);
	void (*readFloats)(Loader *self, float *a, ptrdiff_t n);
	void (*skipBytes)(Loader *self, ptrdiff_t n);
	char *(*readString)(Loader *self);
} LoaderVtbl;

struct Loader {
	const LoaderVtbl *vtbl;
	FILE *fp;
};

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
	Loader_Read(self, a, n * sizeof(float));
}

static void Loader_SkipBytes(Loader *self, ptrdiff_t n)
{
	fseek(self->fp, n, SEEK_CUR);
}

static char *Loader_ReadString(Loader *self)
{
	int n = Loader_ReadInt(self);
	char *s = malloc(n + 1);
	Loader_Read(self, s, n);
	s[n] = '\0';
	return s;
}

static void Loader_Open(Loader *self, const char *filename)
{
	self->fp = fopen(filename, "rb");
	if (self->fp == NULL) {
		perror(filename);
		exit(1);
	}
	static const LoaderVtbl vtbl = {
		Loader_ReadInt,
		Loader_ReadFloat,
		Loader_ReadFloats,
		Loader_SkipBytes,
		Loader_ReadString
	};
	self->vtbl = &vtbl;
}

static void Loader_Close(Loader *self)
{
	fclose(self->fp);
}

int main(int argc, char **argv)
{
	const char *prompt;
	switch (argc) {
	case 2:
		prompt = "";
		break;
	case 3:
		prompt = argv[2];
		break;
	default:
		printf("Usage: run <checkpoint> [<prompt>]\n");
		return 1;
	}
	Loader modelLoader;
	Loader_Open(&modelLoader, argv[1]);
	Loader tokenizerLoader;
	Loader_Open(&tokenizerLoader, "tokenizer.bin");

	Llama2 *obj = Llama2_New();
	Llama2_Load(obj, &modelLoader, &tokenizerLoader);
	Loader_Close(&tokenizerLoader);
	Loader_Close(&modelLoader);
	Llama2_Generate(obj, prompt, 256);
	Llama2_Delete(obj);
	return 0;
}
