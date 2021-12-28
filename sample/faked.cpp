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
#include <fakeit/powerfakeit.h>
using namespace fakeit;
#endif

#include "functions.h"
#include "SampleClass.h"

using namespace std;
using namespace PowerFake;
using namespace FakeTest;


TAG_PRIVATE_MEMBER(SamplePrivateValue, SampleClass::private_val);
TAG_PRIVATE_MEMBER(SamplePrivate, SampleClass::SamplePrivate);
TAG_PRIVATE_MEMBER(VirtualPrivate, SampleBase::CallPrivateVirtual);
TAG_PRIVATE_MEMBER(VirtualProtected, SampleBase::CallProtectedVirtual);
TAG_OVERLOADED_PRIVATE(OverloadedPrivateInt, void (int),
    SampleClass::OverloadedPrivate);
TAG_OVERLOADED_PRIVATE(OverloadedPrivateFloat, void (float),
    SampleClass::OverloadedPrivate);

// Sample showing how MakeFake can be used inside struct/classes, since
// auto is not allowed here
struct SampleStruct
{
        FakePtr normal_fake =
                MakeFake(normal_func, [this](int) {});
        FakePtr ov = MakeFake<void (int)>(overloaded, [](int){});
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
    noexcept_func();

    auto oifk = MakeFake<void (int)>(overloaded,
        [](int) { cout << "Fake called for overloaded(int)" << endl; }
    );

    auto offk = MakeFake<void (float)>(overloaded,
        [](float) { cout << "Fake called for overloaded(float)" << endl; }
    );

    auto noexfk = MakeFake(noexcept_func,
        []() { cout << "Fake called for noexcept_func()" << endl; });

    cout << "-> Fake calls" << endl;
    overloaded(5);
    overloaded(6.0F);
    noexcept_func();

    auto internalfk = MakeFake(called_by_normal_func,
        [](int) { cout << "Fake called for called_by_normal_func(int)" << endl; }
    );
    normal_func(3);

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
        sc.CallThisNoExcept();
        sc.OverloadedCall();
        const_cast<const SampleClass &>(sc).OverloadedCall(2);
    }

    auto ccfk = MakeFake(&SampleClass::CallThis,
        []() { cout << "Fake called for SampleClass::CallThis" << endl; }
    );
    auto cnxfk = MakeFake(&SampleClass::CallThisNoExcept,
        []() { cout << "Fake called for SampleClass::CallThisNoExcept" << endl; }
    );
    auto oc1fk = MakeFake<int ()>(&SampleClass::OverloadedCall,
        []() {
            cout << "Fake called for SampleClass::OverloadedCall()" << endl;
            return 0;
        }
    );
    auto oc2fk = MakeFake<int (int) const>(&SampleClass::OverloadedCall,
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
        sc.CallThisNoExcept();
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
        auto ct2fk = MakeFake<void (int)>(&SampleClass2::CallThis,
            [](int) { cout << "Fake called for SampleClass2::CallThis" << endl; }
        );
        sc2.CallThis(4);
        {
            auto ct2fk = MakeFake<void (int)>(&SampleClass2::CallThis,
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

    cout << "\n Private function Tests\n"
            "----------------------------------------------" << endl;
    cout << "-> Real calls" << endl;
    sc2.CallSamplePrivate();
    sc2.CallOverloadedPrivate(4);
    sc2.CallOverloadedPrivate(4.0f);

    cout << "-> Real calls of private functions directly" << endl;
    SamplePrivate::Call(sc2);
    SamplePrivateValue::Value(sc2) = 25;
    SamplePrivate::Call(sc2);
    OverloadedPrivateInt::Call(sc2, 10);
    OverloadedPrivateFloat::Call(sc2, 5);

    cout << "-> Fake calls" << endl;
    auto pfk = MakeFake<SamplePrivate>([]() { cout << "Faked call for SamplePrivate" << endl; });
    auto pfk2 = MakeFake<OverloadedPrivateInt>([](int) { cout << "Faked call for OverloadedPrivate(int)" << endl; });
    auto pfk3 = MakeFake<OverloadedPrivateFloat>([](float) { cout << "Faked call for OverloadedPrivate(float)" << endl; });
    sc2.CallSamplePrivate();
    sc2.CallOverloadedPrivate(4);
    sc2.CallOverloadedPrivate(4.0f);

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

        Verify(Function(pfk, normal_func).Using(100)).Exactly(Once);

        PowerFakeIt<SampleClass> pfk2;
        When(Method(pfk2, CallThis)).Do([]() { cout << "WOW2" << endl; });
        When(OverloadedMethod(pfk2, OverloadedCall, int())).Return(4);
        When(ConstOverloadedMethod(pfk2, OverloadedCall, int(int))).Return(5);
        When(PrivateMethod(pfk2, SamplePrivate)).Do([]() { cout << "Private WOW!" << endl; });
        When(PrivateMethod(pfk2, OverloadedPrivateInt)).Do([](int) { cout << "Overloaded(int) Private WOW!" << endl; });
        When(PrivateMethod(pfk2, OverloadedPrivateFloat)).Do([](float) { cout << "Overloaded(float) Private WOW!" << endl; });

        SampleClass s;
        s.CallThis();
        s.OverloadedCall();
        s.CallSamplePrivate();
        s.CallOverloadedPrivate(4);
        s.CallOverloadedPrivate(4.0f);

        Verify(Method(pfk2, CallThis)).Exactly(Once);
        Using(pfk2).Verify(Method(pfk2, CallThis)
            + OverloadedMethod(pfk2, OverloadedCall, int()));

        VerifyNoOtherInvocations(Method(pfk2, CallThis));

        ExMock<SampleBase> mock;
        Fake(PrivateMethod(mock, VirtualPrivate));
        Fake(PrivateMethod(mock, VirtualProtected));

        mock.get().CallNonPublics(30);
        Verify(PrivateMethod(mock, VirtualProtected),
            PrivateMethod(mock, VirtualPrivate)).Exactly(Once);
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
