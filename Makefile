CFLAGS = -Wall -O2

ifeq ($(OS),Windows_NT)
EXEEXT = .exe
endif

llama2fu$(EXEEXT): main.c llama2fu.c
	$(CC) -o $@ $(CFLAGS) $^ `pkg-config --cflags --libs glib-2.0`

llama2fu.c: llama2.fu
	fut -o $@ $^

clean:
	$(RM) llama2fu$(EXEEXT) llama2fu.c llama2fu.h
