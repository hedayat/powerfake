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
* Currently, it provides CMake integration


## Roadmap
* Provide support for other build systems (e.g. QMake, Autotools, Meson)
* Integrate with some C++ mocking frameworks
