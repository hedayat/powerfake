/*
 * SampleClass.h
 *
 *  Created on: ۱۱ دی ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2017.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef SAMPLE_SAMPLECLASS_H_
#define SAMPLE_SAMPLECLASS_H_

class SampleClass
{
    public:
        SampleClass();
        ~SampleClass();

        void CallThis();

        int OverloadedCall();
        int OverloadedCall(int b);
};

class SampleClass2: public SampleClass
{
    public:
        using SampleClass::CallThis;

        void DerivedFunc();
        void CallThis(int a);

	virtual void CallVirtual(int b);
};

#endif /* SAMPLE_SAMPLECLASS_H_ */
