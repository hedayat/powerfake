PowerFake
=============
PowerFake is a faking library/tool for faking free functions and non-virtual
member functions. It is built on top of GNU Linker's --wrap option.

## Features
* Faking C/C++ free functions
* Faking static class member functions
* Faking member functions of a class
* Provides control over the life time of faking

## Limitations
* Cannot fake constructors/destructors
* Cannot fake inlined functions
* Cannot fake function calls in the same translation unit as the target function
* GCC & GNU Linker only
* Cannot work with GCC LTO, since ld's --wrap is not supported in this case
* Currently, it only provides CMake integration

## Dependencies
* GNU Linker (ld)
* GCC
* CMake
* Boost Core

## Roadmap
* Provide support for other build systems (e.g. QMake, Autotools, Meson)
* Integrate with some C++ mocking frameworks
