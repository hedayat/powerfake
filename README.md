PowerFake
=============
PowerFake is a C++ library/tool for faking & mocking free functions and
non-virtual member functions. Using PowerFake, you can mock non-virtual member
functions and free functions with existing mocking frameworks. Currently,
it provides seamless integration with FakeIt and Google Mock frameworks and
makes all of their facilities available.

It is built on top of GNU Linker's --wrap option and linker scripts.

## Features
* Faking C/C++ free functions
* Faking static class member functions
* Faking non-virtual member functions of a class
* Faking non-virtual dispatch of virtual functions
* Faking private and protected member functions
* Provides access to private & protected member variables
* Provides control over the life time of faking/mocking
* Provides integration with FakeIt(https://github.com/eranpeer/FakeIt)
* Provides integration with Google Mock(https://github.com/google/googletest/)

## At a Glance

### Mark Target Functions
Every function we want to override in tests should be marked as such for PowerFake,
which can happen in a single source file, which we call `wrap.cpp` using
WRAP_FUNCTION() and HIDE_FUNCTION() macros. Using HIDE_FUNCTION(), you'll lose
access to the original function, but you can also capture calls in the same
translation unit.

```
#include <powerfake.h>
#include <functinos_header.h>

WRAP_FUNCTION(std::string (float), overloaded2);
WRAP_FUNCTION(normal_func);

#ifdef HIDE_FUNCTION
HIDE_FUNCTION(called_by_normal_func);
#endif

WRAP_FUNCTION(STATIC(SampleClass, SampleClass::StaticFunc));
WRAP_FUNCTION(STATIC(SampleClass, PRIVATE(SampleClass::PrivateStaticFunc)));
WRAP_FUNCTION(SampleClass::CallThis);
WRAP_FUNCTION(int (int) const, SampleClass::OverloadedCall);
WRAP_FUNCTION(PFK_PRIVATE(SampleClass::SamplePrivate));
WRAP_FUNCTION(void (int), PRIVATE(SampleClass::OverloadedPrivate));
```

### Build System Setup
PowerFake needs build system integration. It provides CMake support itself
through `bind_fakes()` function:

```
add_library(mainlib STATIC ${main_sources}) # production code (test subject) as a library

add_library(wrap_lib STATIC wrap.cpp) # library containing WRAP_FUNCTION()/HIDE_FUNCTION() macros

# Test runner build
add_executable(test_runner ${test_sources})
target_link_libraries(samples wrap_lib mainlib)
bind_fakes(samples SUBJECT mainlib WRAPPERS wrap_lib CACHE)

```

### Use with FakeIt
```
TAG_PRIVATE(SamplePrivate, SampleClass::SamplePrivate);
TAG_PRIVATE(VirtualPrivate, SampleBase::CallPrivateVirtual);
TAG_PRIVATE(OverloadedPrivateInt, void (int), SampleClass::OverloadedPrivate);

void UseFakeIt()
{
    PowerFakeIt<> pfk;

    When(Function(pfk, normal_func)).Do([](int, const char **const *,
            const std::string &, const char *(*)(const char *))
            {
                cout << "WOW :) " << endl;
            });

    normal_func(100, nullptr, string(), nullptr);

    Verify(Function(pfk, normal_func).Using(100, nullptr, string(), nullptr)).Exactly(Once);

    PowerFakeIt<SampleClass> pfk2;
    When(Method(pfk2, CallThis)).Do([]() { cout << "WOW2" << endl; });
    When(OverloadedMethod(pfk2, OverloadedCall, int())).Return(4);
    When(ConstOverloadedMethod(pfk2, OverloadedCall, int(int))).Return(5);
    When(PrivateMethod(pfk2, SamplePrivate)).Do([]() { cout << "Private WOW!" << endl; });
    When(PrivateMethod(pfk2, OverloadedPrivateInt)).Do([](int) { cout << "Overloaded(int) Private WOW!" << endl; });

    SampleClass s;
    s.CallThis();
    s.OverloadedCall();
    s.CallSamplePrivate();
    s.CallOverloadedPrivate(4);

    Verify(Method(pfk2, CallThis)).Exactly(Once);
    Using(pfk2).Verify(Method(pfk2, CallThis)
        + OverloadedMethod(pfk2, OverloadedCall, int()));

    VerifyNoOtherInvocations(Method(pfk2, CallThis));

    ExMock<SampleBase> mock;
    Fake(PrivateMethod(mock, VirtualPrivate));

    mock.get().CallNonPublics(30);
    Verify(PrivateMethod(mock, VirtualPrivate)).Exactly(Once);
}
```

### Use with Google Mock (gMock)
```
class MockFreeFunctions
{
    public:
        GPFK_MOCK_FUNCTION(void, normal_func, (int b, const char **const *c,
                const std::string &d, const char *(*e)(const char *)));
        GPFK_MOCK_FUNCTION(std::string, overloaded2, (int b));
};

class MockSample: public GPowerFake<SampleClass>
{
    public:
        GPFK_MOCK_METHOD(void, CallThis, (), (const));
        GPFK_MOCK_METHOD(int, OverloadedCall, ());

        // free functions can be mocked here too
        GPFK_MOCK_FUNCTION(void, noexcept_func, ());
};

void GMockSamples()
{
    MockSample mock;
    MockFreeFunctions freemock;
    EXPECT_CALL(mock, CallThis()).Times(AtLeast(1));
    EXPECT_CALL(mock, OverloadedCall()).Times(AtLeast(1));
    EXPECT_CALL(freemock, normal_func(1, nullptr, "", nullptr))
        .Times(AtLeast(1));
    EXPECT_CALL(freemock, overloaded2(_)).Times(AtLeast(1));
    EXPECT_CALL(mock, noexcept_func()).Times(AtLeast(1));

    normal_func(1, nullptr, "", nullptr);
    noexcept_func();
    SampleClass s;
    s.CallThis();
    s.OverloadedCall();
    overloaded2(60);
}
```

### Independent use
```
TAG_PRIVATE(SamplePrivate, SampleClass::SamplePrivate);

void PowerFakeSamples()
{
    overloaded(6.0F); // Real call
    auto offk = MakeFake<void (float)>(overloaded,
        [](float) { cout << "Fake called for overloaded(float)" << endl; }
    );
    overloaded(6.0F); // Fake call for overloaded free function

    // For member functions, you can optionally access object's this ptr
    auto ccfk = MakeFake(&SampleClass::CallThis,
        [](SampleClass *this_ptr) { cout << "Fake called for SampleClass::CallThis for object: " << this_ptr << endl; }
    );
    auto oc2fk = MakeFake<int (int) const>(&SampleClass::OverloadedCall,
        [](int) {
            cout << "Fake called for SampleClass::OverloadedCall(int) const" << endl;
            return 0;
        }
    );

    SampleClass sc;
    sc.CallThis();
    sc.OverloadedCall();

    SampleClass2 sc2;
    sc2.CallThis(); // Real call
    {
        auto ct2fk = MakeFake<void (int)>(&SampleClass2::CallThis,
            [](int) { cout << "Fake called for SampleClass2::CallThis" << endl; }
        );
        sc2.CallThis(4); // Fake call
        {
            auto ct2fk = MakeFake<void (int)>(&SampleClass2::CallThis,
                [](int) { cout << "Nested Fake called for SampleClass2::CallThis" << endl; }
            );
            sc2.CallThis(4); // Nested fake call
        }
        sc2.CallThis(4); // Fake call
    }

    auto stfk = MakeFake(&SampleClass2::StaticFunc, []() { cout << "Faked call for StaticFunc" << endl; });
    SampleClass::StaticFunc();

    auto pfk = MakeFake<SamplePrivate>([]() { cout << "Faked call for SamplePrivate" << endl; });
    sc2.CallSamplePrivate();
}
```

## Installation
PowerFake can be either installed into an address and be used as an external
dependency by projects; or be copied into another project's source directory and
be used directly without any installation.

### Using Embedded PowerFake
You can place powerfake source directory inside your project (e.g. in third_party/
directory), and then `include()` powerfake's `EmbedPowerFake.cmake` into your
own project's `CMakeLists.txt`:

```
include(${CMAKE_SOURCE_DIR}/third_party/powerfake/EmbedPowerFake.cmake)
```

### System Wide Installation
You can also install PowerFake and use it using CMake's `find_package(PowerFake)`
in the `CMakeLists.txt` of your project. You can install PowerFake using
conventional cmake commands:

```
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build . -j
$ cmake --install .
```

## Limitations
* Cannot fake constructors/destructors
* Cannot fake inlined functions
* Faking function calls in the same translation unit as the target function has
some limitations
* Supported compilers: GCC, MinGW, and Clang (experimental)
* Supported linkers: LLD, GNU Linker
* Cannot work with GCC LTO, since ld's --wrap is not supported in this case
* Currently, it only provides CMake integration

## Dependencies
* GNU Linker (ld) or LLVM Linker (lld)
* GCC or Clang with C++17 support
* CMake
* Boost Core

## Roadmap
* Provide support for other build systems (e.g. QMake, Autotools, Meson)
* Integrate with more C++ mocking frameworks
* Support faking virtual functions with virtual dispatch
