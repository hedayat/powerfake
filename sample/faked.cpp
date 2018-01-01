/*
 * faked.cpp
 *
 *  Created on: ۱۱ دی ۱۳۹۶
 *      Author: Hedayat Vatankhah
 */

#include <iostream>
#include <powerfake.h>

#include "functions.h"
#include "SampleClass.h"

using namespace std;


DECLARE_WRAPPER(void (*)(int), overloaded, overloaded_int_fake);
DECLARE_WRAPPER(void (*)(float), overloaded, overloaded_float_fake);
DECLARE_WRAPPER(normal_func, normal_func_fake);

DECLARE_WRAPPER(SampleClass::CallThis, SampleClass_CallThis_fk);
DECLARE_WRAPPER(int (SampleClass::*)(), SampleClass::OverloadedCall,
    SampleClass_OverloadedCall_fk);
DECLARE_WRAPPER(int (SampleClass::*)(int), SampleClass::OverloadedCall,
    SampleClass_OverloadedCall2_fk);

DECLARE_WRAPPER(void (SampleClass2::*)(int), SampleClass2::CallThis,
    SampleClass2_CallThis2_fk);


void FakeOverloaded()
{
    overloaded(5);
    overloaded(6.0F);

    auto oifk = MakeFake(overloaded_int_fake,
        [](int) { cout << "Fake called for overloaded(int)" << endl; }
    );

    auto offk = MakeFake(overloaded_float_fake,
        [](float) { cout << "Fake called for overloaded(float)" << endl; }
    );

    overloaded(5);
    overloaded(6.0F);

    auto normalfk = MakeFake(normal_func_fake,
        [](int) { cout << "Fake called for normal_func(int)" << endl; }
    );
    normal_func(3);

    {
        SampleClass sc;
        sc.CallThis();
        sc.OverloadedCall();
        sc.OverloadedCall(2);
    }

    auto ccfk = MakeFake(SampleClass_CallThis_fk,
        []() { cout << "Fake called for SampleClass::CallThis" << endl; }
    );
    auto oc1fk = MakeFake(SampleClass_OverloadedCall_fk,
        []() {
            cout << "Fake called for SampleClass::OverloadedCall()" << endl;
            return 0;
        }
    );
    auto oc2fk = MakeFake(SampleClass_OverloadedCall2_fk,
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

    SampleClass2 sc2;
    sc2.CallThis();

    sc2.OverloadedCall(3);

    {
        auto ct2fk = MakeFake(SampleClass2_CallThis2_fk,
            [](int) { cout << "Fake called for SampleClass2::CallThis" << endl; }
        );
        sc2.CallThis(4);
    }
    sc2.CallThis(4);
}


int main(/*int argc, char **argv*/)
{
    FakeOverloaded();
}
