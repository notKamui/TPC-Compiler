#!/bin/sh

./bin/tpcc -ts test.tpc
nasm -f elf64 test.asm
gcc test.o -no-pie
./a.out
echo "RETURNED VALUE:" $?