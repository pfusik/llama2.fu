CFLAGS = -Wall -O2

ifeq ($(OS),Windows_NT)
EXEEXT = .exe
endif

llama2fu$(EXEEXT): main.c llama2cli.c
	$(CC) -o $@ $(CFLAGS) $^ `pkg-config --cflags --libs glib-2.0`

llama2cli.c: llama2.fu llama2cli.fu
	fut -o $@ -D OPENMP $^

clean:
	$(RM) llama2fu$(EXEEXT) llama2cli.c llama2cli.h
