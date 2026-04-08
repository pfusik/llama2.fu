#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <locale.h>
#endif

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

static void usage(void)
{
	fprintf(stderr, "Usage:   llama2fu <checkpoint> [options]\n");
	fprintf(stderr, "Example: llama2fu model.bin -n 256 -i \"Once upon a time\"\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -t <float>  temperature in [0,inf], default 1.0\n");
	fprintf(stderr, "  -s <int>    random seed, default time(NULL)\n");
	fprintf(stderr, "  -n <int>    number of steps to run for, default 256. 0 = max_seq_len\n");
	fprintf(stderr, "  -i <string> input prompt\n");
	fprintf(stderr, "  -z <string> optional path to custom tokenizer\n");
	fprintf(stderr, "  -m <string> mode: generate|chat, default: generate\n");
	fprintf(stderr, "  -y <string> (optional) system prompt in chat mode\n");
}

int main(int argc, char **argv)
{
	float temperature = 1;
	int64_t seed = 0;
	int steps = 256;
	const char *prompt = NULL;
	const char *tokenizerPath = "tokenizer.bin";
	const char *mode = "generate";
	const char *systemPrompt = NULL;

	if (argc < 2 || (argc & 1) != 0) {
		usage();
		return 1;
	}
	const char *modelPath = argv[1];
	for (int i = 2; i < argc; i += 2) {
		const char *opt = argv[i];
		if (opt[0] != '-' || opt[1] == '\0' || opt[2] != '\0') {
			usage();
			return 1;
		}
		const char *value = argv[i + 1];
		switch (opt[1]) {
		case 't':
			temperature = atof(value);
			break;
		case 's':
			seed = atoll(value);
			break;
		case 'n':
			steps = atoi(value);
			break;
		case 'i':
			prompt = value;
			break;
		case 'z':
			tokenizerPath = value;
			break;
		case 'm':
			if (strcmp(value, "generate") != 0 && strcmp(value, "chat") != 0) {
				fprintf(stderr, "unknown mode: %s\n", value);
				usage();
				return 1;
			}
			mode = value;
			break;
		case 'y':
			systemPrompt = value;
			break;
		default:
			usage();
			return 1;
		}
	}

	Loader modelLoader;
	Loader_Open(&modelLoader, modelPath);
	Loader tokenizerLoader;
	Loader_Open(&tokenizerLoader, tokenizerPath);
	Llama2 *obj = Llama2_New();
	Llama2_Load(obj, &modelLoader, &tokenizerLoader);
	Loader_Close(&tokenizerLoader);
	Loader_Close(&modelLoader);

	if (seed <= 0)
		seed = time(NULL);
	if (steps <= 0 || steps > Llama2_GetSeqLen(obj))
		steps = Llama2_GetSeqLen(obj);

#ifdef _WIN32
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
#else
	setlocale(LC_ALL, "C.UTF-8");
#endif
	Llama2_SetRandomSeed(obj, seed);
	if (strcmp(mode, "chat") == 0)
		Llama2_Chat(obj, prompt, systemPrompt, temperature, steps);
	else
		Llama2_Generate(obj, prompt == NULL ? "" : prompt, temperature, steps);
	Llama2_Delete(obj);
	return 0;
}
