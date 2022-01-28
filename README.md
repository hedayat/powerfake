PowerFake
=============
PowerFake is a C++ faking library/tool for faking free functions and
non-virtual member functions. Using its FakeIt integration support, it allows
mocking non-virtual member functions and free functions.

It is built on top of GNU Linker's --wrap option and linker scripts.

## Features
* Faking C/C++ free functions
* Faking static class member functions
* Faking non-virtual member functions of a class
* Faking non-virtual dispatch of virtual functions
* Faking private and protected member functions
* Provides access to private & protected member variables
* Provides control over the life time of faking
* Provides integration with FakeIt(https://github.com/eranpeer/FakeIt)

## Limitations
* Cannot fake constructors/destructors
* Cannot fake inlined functions
* Faking function calls in the same translation unit as the target function has
some limitations
* Supported compilers: GCC, MinGW, and Clang (experimental)
* Supported linkers: LLD, GNU Linker
* Cannot work with GCC LTO, since ld's --wrap is not supported in this case
* Currently, it only provides CMake integration

## Usage
Currently, there are no docs for using PowerFake, but you can refer to the sample
directory to see a show case of PowerFake features and how to use it in other
projects. Hopefully, there will be some docs someday!

## Dependencies
* GNU Linker (ld) or LLVM Linker (lld)
* GCC or Clang with C++17 support
* CMake
* Boost Core

## Roadmap
* Provide support for other build systems (e.g. QMake, Autotools, Meson)
* Integrate with some C++ mocking frameworks
* Support faking virtual functions with virtual dispatch
