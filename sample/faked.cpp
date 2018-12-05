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
    cout << "-> Real calls" << endl;
    overloaded(5);
    overloaded(6.0F);

    auto oifk = MakeFake((void (*)(int)) overloaded,
        [](int) { cout << "Fake called for overloaded(int)" << endl; }
    );

    auto offk = MakeFake(static_cast<void (*)(float)>(overloaded),
        [](float) { cout << "Fake called for overloaded(float)" << endl; }
    );

    cout << "-> Fake calls" << endl;
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
        cout << "-> Real calls" << endl;
        SampleClass sc;
        sc.CallThis();
        sc.OverloadedCall();
        const_cast<const SampleClass &>(sc).OverloadedCall(2);
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
        static_cast<int (SampleClass::*)(int) const>(&SampleClass::OverloadedCall),
        [](int) {
            cout << "Fake called for SampleClass::OverloadedCall(int) const" << endl;
            return 0;
        }
    );

    std::unique_ptr<int> a;
    auto oc3fk = MakeFake(&SampleClass::GetIntPtrReference, [&a]() -> std::unique_ptr<int> &{
        cout << "Fake called for SampleClass::GetIntPtrReference()" << endl;
        return a;
    });

    {
        SampleClass sc;
        cout << "-> Fake calls" << endl;
        sc.CallThis();
        sc.OverloadedCall();
        const_cast<const SampleClass &>(sc).OverloadedCall(2);
        sc.GetIntPtrReference();
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
    cout << "-> Real calls" << endl;
    SampleClass2 sc2;
    cout << "-> Fake calls" << endl;
    sc2.CallThis();

    const_cast<const SampleClass2 &>(sc2).OverloadedCall(3);

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
            cout << "-> Nested fake call" << endl;
            sc2.CallThis(4);
        }
        cout << "-> Fake call" << endl;
        sc2.CallThis(4);
    }
    cout << "-> Real call" << endl;
    sc2.CallThis(4);

    auto vfk = MakeFake(&SampleClass2::CallVirtual, [](int) { cout << "Faked called for SampleClass2::CallVirtual" << endl; });
    cout << "-> Fake call" << endl;
    sc2.CallVirtual(6);

    cout << "\n VirtualSample Tests\n"
            "----------------------------------------------" << endl;
    VirtualSample vs;
    cout << "-> Real calls" << endl;
    vs.CallVirtual(3);
    auto vfk2 = MakeFake(&VirtualSample::CallVirtual, [](int) { cout << "Faked called for VirtualSample::CallVirtual" << endl; });
    cout << "-> Fake calls" << endl;
    vs.CallVirtual(3);

    cout << "\n VirtualDispatch test!\n"
            "----------------------------------------------" << endl;
    cout << "-> Real calls although faked, as we currently do not support "
            "virtual dispatch" << endl;
    call_virtual_func(&vs);
}


void FakeItSamples()
{
#ifdef ENABLE_FAKEIT
    try
    {

        cout << "\n PowerFakeIt tests\n"
                "----------------------------------------------" << endl;
        PowerFakeIt<> pfk;

        When(Function(pfk, normal_func)).Do([](int ){ cout << "WOW :) " << endl; });


        normal_func(100);

        Verify(Function(pfk, normal_func).Using(100)).Exactly(1);

        PowerFakeIt<SampleClass> pfk2;
        When(Method(pfk2, CallThis)).Do([]() { cout << "WOW2" << endl; });
        When(OverloadedMethod(pfk2, OverloadedCall, int())).Return(4);
        When(ConstOverloadedMethod(pfk2, OverloadedCall, int(int))).Return(5);

        SampleClass s;
        s.CallThis();
        s.OverloadedCall();

        Verify(Method(pfk2, CallThis)).Exactly(1);
        Using(pfk2).Verify(Method(pfk2, CallThis)
            + OverloadedMethod(pfk2, OverloadedCall, int()));

        VerifyNoOtherInvocations(Method(pfk2, CallThis));
    }
    catch (std::exception& e)
    {
        cerr << "Error: " << e.what() << endl;
    }
    catch (fakeit::UnexpectedMethodCallException &)
    {
        cerr << "Unexpected method call! You should fix your code to expect "
                "for more calls" << endl;
    }
    catch (...)
    {
        cerr << "Unknown error" << endl;
    }

#endif
}
