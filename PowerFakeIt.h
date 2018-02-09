/*
 * PowerFakeIt.h
 *
 *  Created on: ۱۹ بهمن ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2018.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef POWERFAKEIT_H_
#define POWERFAKEIT_H_

#include <fakeit.hpp>
#include "powerfake.h"

namespace PowerFake
{

/**
 * FakeIt integration for PowerFake
 */
class PowerFakeIt: public fakeit::ActualInvocationsSource
{
    public:

        template <typename R , typename ...Args>
        fakeit::MockingContext<R, Args...> stub(R (*func_ptr)(Args...))
        {
            static auto fk = MakeFake(func_ptr,
                [this, func_ptr](Args... args) {
                    fakeit::RecordedMethodBody<R, Args...> &recorder =
                            MethodRecorder<R, Args...>(
                                    invocation_handler[FuncKey(func_ptr)]);
                    recorder.handleMethodInvocation(args...);
            });
            return fakeit::MockingContext<R, Args...>(
                new MethodMockingContextImpl<R, Args...>(*this, func_ptr));
        }

        template <typename T, typename R , typename ...Args>
        fakeit::MockingContext<R, Args...> stub(R (T::*func_ptr)(Args...))
        {
            return fakeit::MockingContext<R, Args...>(
                new MethodMockingContextImpl<R, Args...>(*this, func_ptr));
        }


        void getActualInvocations(
            std::unordered_set<fakeit::Invocation *> &into) const override
        {
            for (auto &method: invocation_handler)
            {
                ActualInvocationsSource &s =
                        dynamic_cast<ActualInvocationsSource &>(*method.second);
                s.getActualInvocations(into);
            }
        }

        template<typename R, typename ...Args>
        static fakeit::RecordedMethodBody<R, Args...> *createRecordedMethodBody(
            PowerFakeIt &mock, std::string method_type_name)
        {
            return new fakeit::RecordedMethodBody<R, Args...>(mock.getFakeIt(),
                method_type_name);
        }

        fakeit::FakeitContext &getFakeIt()
        {
            return Fakeit;
        }

    private:
        std::map<WrapperBase::FunctionKey,
            std::unique_ptr<fakeit::Destructible>> invocation_handler;

        template <typename R, typename ...Args>
        static fakeit::RecordedMethodBody<R, Args...> &MethodRecorder(
            std::unique_ptr<fakeit::Destructible> &r)
        {
            return static_cast<fakeit::RecordedMethodBody<R, Args...>&>(*r);
        }

        template <typename FuncType>
        static WrapperBase::FunctionKey FuncKey(FuncType func_ptr)
        {
#pragma GCC diagnostic ignored "-Wpmf-conversions"
            return std::make_pair(reinterpret_cast<void *>(func_ptr),
                std::type_index(typeid(FuncType)));
#pragma GCC diagnostic warning "-Wpmf-conversions"
        }

        // borrowed from FakeIt MockImpl implementation
        template<typename R, typename ...Args>
        class MethodMockingContextBase:
                public fakeit::MethodMockingContext<R, Args...>::Context
        {
            public:
                MethodMockingContextBase(PowerFakeIt &mock) :
                        _mock(mock)
                {
                }

                virtual ~MethodMockingContextBase() = default;

                void addMethodInvocationHandler(
                    typename fakeit::ActualInvocation<Args...>::Matcher *matcher,
                    fakeit::ActualInvocationHandler<R, Args...> *invocationHandler)
                {
                    getRecordedMethodBody().addMethodInvocationHandler(matcher,
                        invocationHandler);
                }

                void scanActualInvocations(
                    const std::function<void(fakeit::ActualInvocation<Args...> &)> &scanner)
                {
                    getRecordedMethodBody().scanActualInvocations(scanner);
                }

                void setMethodDetails(std::string mockName,
                    std::string methodName)
                {
                    getRecordedMethodBody().setMethodDetails(mockName,
                        methodName);
                }

                bool isOfMethod(fakeit::MethodInfo &method)
                {
                    return getRecordedMethodBody().isOfMethod(method);
                }

                fakeit::ActualInvocationsSource &getInvolvedMock()
                {
                    return _mock;
                }

                std::string getMethodName()
                {
                    return getRecordedMethodBody().getMethod().name();
                }

            protected:
                PowerFakeIt &_mock;

                virtual fakeit::RecordedMethodBody<R, Args...> &getRecordedMethodBody() = 0;
        };

        template<typename R, typename ...Args>
        class MethodMockingContextImpl: public MethodMockingContextBase<R, Args...>
        {
            public:
                MethodMockingContextImpl(PowerFakeIt &mock, R (*func_ptr)(Args...)) :
                        MethodMockingContextBase<R, Args...>(mock),
                        f_key(FuncKey(func_ptr)),
                        method_name(typeid(func_ptr).name())
                {
                }

                template <typename T>
                MethodMockingContextImpl(PowerFakeIt &mock, R (T::*func_ptr)(Args...)) :
                        MethodMockingContextBase<R, Args...>(mock),
                        f_key(FuncKey(func_ptr)),
                        method_name(typeid(func_ptr).name())
                {
                }

            protected:
                WrapperBase::FunctionKey f_key;
                std::string method_name;

                virtual fakeit::RecordedMethodBody<R, Args...> &getRecordedMethodBody() override
                {
                    auto k = this->_mock.invocation_handler.find(f_key);
                    if (k == this->_mock.invocation_handler.end())
                    {
                        auto j = this->_mock.invocation_handler.insert(
                            make_pair(f_key,
                                createRecordedMethodBody<R, Args...>(
                                        this->_mock, method_name)));
                        k = j.first;
                    }
                    return MethodRecorder<R, Args...>(k->second);
                }

                virtual std::function<R(Args&...)> getOriginalMethod() override
                {
                    throw std::runtime_error("Not supported!");
                }
        };
};

} /* namespace PowerFake */

#endif /* POWERFAKEIT_H_ */

