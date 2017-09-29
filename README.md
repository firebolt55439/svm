# Sumer's VM
A lightweight VM that compiles an Assembly-like language to bytecode. Documentation is rather sparse, but there is a list of language operators and instructions in `Instructions.h`.

Please forgive my `#include` of cpp files directly. I was 14 when I wrote it. I am more enlightened now.

## Compilation
Simply execute `CC=g++ $CC VM.cpp -o vm`. Replace `g++` with the C++ compiler of your choice (in my case, I symlinked clang to g++).