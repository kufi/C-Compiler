Compiler in C
============

A simple compiler in C.

In no way production ready.

make
./bin/compiler input.s > out.dot
dot -Tps out.dot -o out.ps
evince out.ps