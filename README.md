PowerFake
=============
PowerFake is a faking library/tool for faking free functions and non-virtual
member functions. It is built on top of GNU Linker's --wrap option.

## Features
* Faking C/C++ free functions
* Faking static class member functions
* Faking member functions of a class
* Provides control over the life time of faking
* Provides integration with FakeIt(https://github.com/eranpeer/FakeIt)

## Limitations
* Cannot fake constructors/destructors
* Cannot fake inlined functions
* Cannot fake function calls in the same translation unit as the target function
* GCC & GNU Linker only
* Cannot work with GCC LTO, since ld's --wrap is not supported in this case
* Currently, it only provides CMake integration

## Usage
Currently, there are no docs for using PowerFake, but you can refer to the sample
directory to see a show case of PowerFake features and how to use it in other
projects. Hopefully, there will be some docs someday!

## Dependencies
* GNU Linker (ld)
* GCC
* CMake
* Boost Core

## Roadmap
* Provide support for other build systems (e.g. QMake, Autotools, Meson)
* Integrate with some C++ mocking frameworks
* Support faking virtual functions with virtual dispatch
