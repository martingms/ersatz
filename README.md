# ersatz

A simple, small, work in progress SAT-solver, written in ANSI C.

Most of the design is based on the
[MiniSat](http://minisat.se)-[paper](http://minisat.se/downloads/MiniSat.pdf).

## Build

    $ make

## Test

Requires `prove`. See the `Makefile` for an alternative.

    $ make test

## Run

    # To solve a SAT instance
    $ ./ersatz formula.cnf

    # To verify a model
    $ ./ersatz -V model.cnf formula.cnf

## Stuff missing

* Watched literals
* Clause learning
* Branching heuristics
* More documentation
* ...
