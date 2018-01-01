/*
 * faked.cpp
 *
 *  Created on: ۱۱ دی ۱۳۹۶
 *      Author: Hedayat Vatankhah
 */

#include <iostream>
#include <powerfake.h>
#include "functions.h"

using namespace std;


DECLARE_WRAPPER(void (*)(int), overloaded, overloaded_int_fake);
DECLARE_WRAPPER(void (*)(float), overloaded, overloaded_float_fake);
DECLARE_WRAPPER(normal_func, normal_func_fake);

void FakeOverloaded()
{
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
}


int main(/*int argc, char **argv*/)
{
    FakeOverloaded();
}
