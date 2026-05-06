CXXFLAGS = -Wall -O2

ifeq ($(OS),Windows_NT)
EXEEXT = .exe
CXXFLAGS += -fopenmp -municode
endif

llama2fu$(EXEEXT): main.cpp llama2cli.cpp
	$(CXX) -o $@ $(CXXFLAGS) $^

llama2cli.cpp: ../llama2.fu ../llama2cli.fu
	fut -o $@ -D OPENMP $^

clean:
	$(RM) llama2fu$(EXEEXT) llama2cli.cpp llama2cli.hpp
