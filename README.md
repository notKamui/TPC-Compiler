# TPC Compiler

## A very small subset of C (Très Petit C)

by notKamui & ZwenDo, syntax defined by the Université Gustave Eiffel

### Summary

-   [The Language](#The-Language)
-   [Requirements](#Requirements)
-   [Compiling the Compiler](#Compiling-the-Compiler)
-   [Usage](#Usage)
-   [Sample](#Sample)

### The Language

TPC, which stands for "Très Petit C", is an extremely small and unusable subset of C.

It only exists as a very interesting exercise on syntax analysis, finite state machines and compilation to nasm x86-64.

The grammar of the language is defined in `./src/parser.y`.

It has no data structures except for `struct`s (which are non recursive nor nested), and has no `for` loops, nor pointers.
The only two primitives are `int` and `char`, and the only control flow instructions are `if`, `else`, and `while`. The rest of the syntax is vastly equivalent to that of C.

### Requirements

-   Make sure you're on a \*nix system (Linux, MacOS, etc)
-   Make sure you have a recent version of `gcc` installed
-   Make sure you have `make` installed

### Compiling the Compiler

Just `make` in the root of this project. The binary of the compiler will be located in `./bin/`.

### Usage

```
Usage: tpcc [OPTIONS] [file]

OPTIONS:
-t: Prints the AST of the selected program
-s: Prints the symbol table of the selected program
-n: No output (prevents creating an asm file)
-x: produces an executable file
-h: Prints this message
tpcc can also receive its input feed from stdin
```

### Sample

A good sample is given in `./sample/mini_game.tpc`. It's a valid tpc program that you can compile and execute (it's a Guess The Number game).

Compile and execute it with the following commands:

```sh
tpcc -nx sample/mini_game.tpc
./mini_game
```
