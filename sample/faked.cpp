/*
 * faked.cpp
 *
 *  Created on: ۱۱ دی ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2017.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#include <iostream>
#include <powerfake.h>

#ifdef ENABLE_FAKEIT
#include <fakeit.hpp>
#include <fakeit/powerfakeit.h>
using namespace fakeit;
#endif

#include "functions.h"
#include "SampleClass.h"

using namespace std;
using namespace PowerFake;

// Sample showing how MakeFake can be used inside struct/classes, since
// auto is not allowed here
struct SampleStruct
{
        FakeType<decltype(&normal_func)> normal_fake =
                MakeFake(normal_func, [this](int) {});
        FakeType<void (*)(int)> ov = MakeFake((void (*)(int)) overloaded, [](int){});
};


void FakeOverloaded();
void FakeItSamples();

int main(/*int argc, char **argv*/)
{
    FakeOverloaded();
    FakeItSamples();
}



void FakeOverloaded()
{
    cout << "\n Free Function Tests\n"
            "----------------------------------------------" << endl;
    overloaded(5);
    overloaded(6.0F);

    auto oifk = MakeFake((void (*)(int)) overloaded,
        [](int) { cout << "Fake called for overloaded(int)" << endl; }
    );

    auto offk = MakeFake(static_cast<void (*)(float)>(overloaded),
        [](float) { cout << "Fake called for overloaded(float)" << endl; }
    );

    overloaded(5);
    overloaded(6.0F);

    auto normalfk = MakeFake(normal_func,
        [](int) { cout << "Fake called for normal_func(int)" << endl; }
    );
    normal_func(3);

    auto refret_fk = MakeFake(non_copyable_ref, []() -> unique_ptr<int> & {
        static unique_ptr<int> aa(new int(2));
        return aa;
    });

    cout << "\n SampleClass Tests\n"
            "----------------------------------------------" << endl;
    {
        SampleClass sc;
        sc.CallThis();
        sc.OverloadedCall();
        sc.OverloadedCall(2);
    }

    auto ccfk = MakeFake(&SampleClass::CallThis,
        []() { cout << "Fake called for SampleClass::CallThis" << endl; }
    );
    auto oc1fk = MakeFake(
        static_cast<int (SampleClass::*)()>(&SampleClass::OverloadedCall),
        []() {
            cout << "Fake called for SampleClass::OverloadedCall()" << endl;
            return 0;
        }
    );
    auto oc2fk = MakeFake(
        static_cast<int (SampleClass::*)(int)>(&SampleClass::OverloadedCall),
        [](int) {
            cout << "Fake called for SampleClass::OverloadedCall(int)" << endl;
            return 0;
        }
    );

    {
        SampleClass sc;
        sc.CallThis();
        sc.OverloadedCall();
        sc.OverloadedCall(2);
    }

    auto intptr_fk = MakeFake(&SampleClass::GetIntPtr,
        []() { return make_unique<int>(5); }
    );

    auto intptrref_fk = MakeFake(&SampleClass::GetIntPtrReference,
        []() -> std::unique_ptr<int> & {
            static auto myuniq = make_unique<int>(4);
            return myuniq;
        }
    );
    auto intptr_constref_fk = MakeFake(&SampleClass::GetIntPtrConstReference,
        []() -> const std::unique_ptr<int> & {
            static auto myuniq = make_unique<int>(4);
            return myuniq;
        }
    );

    cout << "\n SampleClass2 Tests\n"
            "----------------------------------------------" << endl;
    SampleClass2 sc2;
    sc2.CallThis();

    sc2.OverloadedCall(3);

    {
        auto ct2fk = MakeFake(
            static_cast<void (SampleClass2::*)(int)>(&SampleClass2::CallThis),
            [](int) { cout << "Fake called for SampleClass2::CallThis" << endl; }
        );
        sc2.CallThis(4);
        {
            auto ct2fk = MakeFake(
                static_cast<void (SampleClass2::*)(int)>(&SampleClass2::CallThis),
                [](int) { cout << "Nested Fake called for SampleClass2::CallThis" << endl; }
            );
            sc2.CallThis(4);
        }
        sc2.CallThis(4);
    }
    sc2.CallThis(4);

    auto vfk = MakeFake(&SampleClass2::CallVirtual, [](int) { cout << "Faked called for SampleClass2::CallVirtual" << endl; });
    sc2.CallVirtual(6);

    cout << "\n VirtualSample Tests\n"
            "----------------------------------------------" << endl;
    VirtualSample vs;
    vs.CallVirtual(3);
    auto vfk2 = MakeFake(&VirtualSample::CallVirtual, [](int) { cout << "Faked called for VirtualSample::CallVirtual" << endl; });
    vs.CallVirtual(3);

    cout << "\n VirtualDispatch test!\n"
            "----------------------------------------------" << endl;
    call_virtual_func(&vs);
}


void FakeItSamples()
{
#ifdef ENABLE_FAKEIT
    try
    {

        PowerFakeIt<> pfk;

        When(Function(pfk, normal_func)).Do([](int ){ cout << "WOW :) " << endl; });


        normal_func(100);

        Verify(Function(pfk, normal_func).Using(100)).Exactly(1);

        PowerFakeIt<SampleClass> pfk2;
        When(Method(pfk2, CallThis)).Do([]() { cout << "WOW2" << endl; });

        SampleClass s;
        s.CallThis();

        Verify(Method(pfk2, CallThis)).Exactly(1);
    }
    catch (std::exception& e)
    {
        cerr << "Error: " << e.what() << endl;
    }
    catch (...)
    {
        cerr << "Unknown error" << endl;
    }

#endif
}
