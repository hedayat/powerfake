/*
 * SampleClass.h
 *
 *  Created on: ۱۱ دی ۱۳۹۶
 *      Author: Hedayat Vatankhah
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
};

#endif /* SAMPLE_SAMPLECLASS_H_ */
