#!/bin/sh

./bin/tpcc -ts test.tpc &&
echo "COMPILED SUCCESSFULLY" &&
nasm -f elf64 test.asm &&
gcc test.o -no-pie &&
echo "COMPILED ASM" &&
./a.out ; echo "RETURNED VALUE:" $?