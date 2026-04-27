This is a port of [llama2.c](https://github.com/karpathy/llama2.c) to [Fusion](https://fusion-lang.org).
Fusion can be automatically translated to pure C, C++, C#, D, Java,
JavaScript, Python, Swift and TypeScript with no additional dependencies.

It uses a similar model format to llama2.c, but with bfloat16 weights,
so it occupies half the disk space, half the RAM and is twice as fast.
Use `export_meta_llama_bin.py` to convert the original Meta models
or `float2bf16.c` to convert llama2.c models.
