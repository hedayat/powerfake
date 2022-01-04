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


namespace FakeTest
{

class SampleClass
{
    public:
        SampleClass();
        ~SampleClass();

        static void StaticFunc();
        static void CallPrivateStaticFunc() { PrivateStaticFunc(); }

        void CallThis() const;
        void CallThisNoExcept() const noexcept;

        int OverloadedCall();
        int OverloadedCall(int b);
        int OverloadedCall(int b) const;

        std::unique_ptr<int> GetIntPtr();
        std::unique_ptr<int> &GetIntPtrReference();
        const std::unique_ptr<int> &GetIntPtrConstReference();

        void CallSamplePrivate() { SamplePrivate(); }
        void CallOverloadedPrivate(int a) { OverloadedPrivate(a); }
        void CallOverloadedPrivate(float b) { OverloadedPrivate(b); }

    private:
        int private_val = 100;

        static void PrivateStaticFunc();

        void SamplePrivate();
        void OverloadedPrivate(int a);
        void OverloadedPrivate(float b);
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
        virtual ~SampleBase() {}
        virtual void CallVirtual(int b) = 0;
        void CallNonPublics(int b);

    protected:
        virtual void CallProtectedVirtual(int b [[maybe_unused]]) {}

    private:
        virtual void CallPrivateVirtual(int b [[maybe_unused]]) {}
};

class VirtualSample: public SampleClass2, virtual public SampleClass, public SampleBase
{
	public:
        virtual void CallVirtual(int b);
};

}  // namespace FakeTest

#endif /* SAMPLE_SAMPLECLASS_H_ */
