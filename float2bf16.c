#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static FILE *in;
static FILE *out;

static void read(void *buf, size_t n)
{
	if (fread(buf, n, 1, in) != 1) {
		perror("reading");
		exit(1);
	}
}

static void skip(size_t n)
{
#ifdef _WIN32
	_fseeki64(in, n, SEEK_CUR);
#else
	fseek(in, n, SEEK_CUR);
#endif
}

static void copy(size_t n)
{
#define BUF_SIZE 1024
	for (size_t i = 0; i < n; i += BUF_SIZE) {
		size_t m = n - i;
		if (m > BUF_SIZE)
			m = BUF_SIZE;
		short buf[BUF_SIZE * 2];
		read(buf, m * 4);
		for (size_t j = 0; j < m; j++) {
			static bool check = true;
			if (check && buf[j * 2] != 0) {
				fprintf(stderr, "Warning: input precision greater than bfloat16\n");
				check = false;
			}
			buf[j] = buf[j * 2 + 1];
		}
		fwrite(buf, 2, m, out);
	}
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		printf("Usage: float2bf16 model_f32.bin model_bf16\n");
		return 1;
	}
	in = fopen(argv[1], "rb");
	if (in == NULL) {
		perror(argv[1]);
		return 1;
	}
	out = fopen(argv[2], "wb");
	if (out == NULL) {
		perror(argv[2]);
		return 1;
	}
	struct {
		int dim;
		int hidden_dim;
		int n_layers;
		int n_heads;
		int n_kv_heads;
		int vocab_size;
		int seq_len;
	} h;
	read(&h, sizeof(h));
	fwrite(&h, sizeof(h), 1, out);
	bool shared_weights = h.vocab_size > 0;
	int vocab_size = abs(h.vocab_size);
	copy(vocab_size * h.dim);
	size_t n = h.dim * h.n_layers;
	copy(n);
	int head_size = h.dim / h.n_heads;
	copy(h.n_heads * head_size * n);
	copy(h.n_kv_heads * head_size * n);
	copy(h.n_kv_heads * head_size * n);
	copy(h.n_heads * head_size * n);
	copy(n);
	copy(h.hidden_dim * n);
	copy(h.hidden_dim * n);
	copy(h.hidden_dim * n);
	copy(h.dim);
	if (!shared_weights) {
		skip(h.seq_len * head_size >> 1 << 3);
		copy(vocab_size * h.dim);
	}
	fclose(out);
	fclose(in);
	return 0;
}
