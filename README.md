================================
Author: Chris Bak (February 4th 2015)
GP 2: Graph Programming
================================

GP 2 is a graph programming language. Graph programs consist of two text files. The first is a specification of the graph program, namely a set of graph transformation rules and a procedural-style command sequence to organise rule application. The second describes an initial _host graph_ to which the graph program is applied. These text files can be written in a text editor. In the future, a graphical editor can be used to generate the text files internally. Graph programs are executed by either passing them to an interpreter, or by compiling to native code.


Compiler
========

The Compiler subdirectory contains a lexer, a parser and a code generator for GP 2 programs. The generated runtime system files are placed in the directory Compiler/runtime.

Build Instructions
---------------------

To build the GP 2 compiler, run

> make build

This creates the executable GP2-compile which is called in one of three ways:

> ./GP2-compile /path/to/program-file /path/to/host-graph-file
> ./GP2-compile -p /path/to/program-file
> ./GP2-compile -h /path/to/host-graph-file

The last two options allow the user to compile a modified or new program while retaining a previously-built host graph, or to compile a new host graph with an existing program.

Running

> make F1=/path/to/program-file F2=/path/to/host-graph-file

builds the compiler, calls it on the passed program and host graph files, and builds the runtime system.

To execute the compiled GP 2 program, type `./GP2-run` in the runtime directory.

Haskell
=======

The Haskell subdirectory contains a reference interpreter for GP2 implemented in Haskell, a compiler that generates OILR Machine instructions (see below), and several ancillary tools for viewing and testing GP2 host graphs.

Typing `make` in the Haskell directory produces the following binaries:

`gp2`: The GP2 reference interpreter.

`gp2c`: The OILR machine compiler, which generates a C file that can be compiled and linked against the OILR Machine runtime.

`ViewGraph`: A host graph visualiser (using GraphViz as a back-end).

`IsoChecker`: A standalone host graph comparison tool, which checks whether two host graphs are isomorphic.



OilrMachine
===========

The OILR machine is a specialised graph language abstract machine.


Build instructions (basic graph data structure)
-----------------------------------------------

Typing `make` in the OilrMachine directory builds the OILR runtime, `oilrrt.a` using a simplistic graph representation.


Build instructions (full graph data structure)
-----------------------------------------------

To build the OILR runtime using the full graph data structure:

> make -f Makefile.gp2


Compiling GP2 programs
----------------------

Compile a GP2 program to C:

> gp2c prog.gp2 graph.host


To compile a GP2 program generated by `gp2c`, you will need the runtime static library, and the two header files `oilrrt.h` and `oilrinst.h`. 

> gcc -o prog prog.c oilrrt.a

