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

#include <memory>


class SampleClass
{
    public:
        SampleClass();
        ~SampleClass();

        void CallThis() const;

        int OverloadedCall();
        int OverloadedCall(int b);

        std::unique_ptr<int> GetIntPtr();
        std::unique_ptr<int> &GetIntPtrReference();
        const std::unique_ptr<int> &GetIntPtrConstReference();
};

class SampleClass2: virtual public SampleClass
{
    public:
        using SampleClass::CallThis;
        virtual ~SampleClass2() {}

        void DerivedFunc();
        void CallThis(int a);

        virtual void CallVirtual(int b);
};

class SampleBase
{
    public:
        virtual void CallVirtual(int b) = 0;
};

class VirtualSample: public SampleClass2, virtual public SampleClass, public SampleBase
{
	public:
        virtual void CallVirtual(int b);
};

#endif /* SAMPLE_SAMPLECLASS_H_ */
