/*
 * powerfakeit.h
 *
 *  Created on: ۱۹ بهمن ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah <hedayat.fwd@gmail.com>, 2018-2022.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef MOCKING_FAKEIT_POWERFAKEIT_H_
#define MOCKING_FAKEIT_POWERFAKEIT_H_

#include <powerfake.h>
#include <fakeit.hpp>


#define Function(power_mock, function) \
        power_mock.stub(&function).setMethodDetails(#power_mock, #function)

#define PrivateMethod(mock, method_tag) \
    mock.template stub<__COUNTER__, method_tag>().setMethodDetails(#mock, \
        method_tag::member_name)


namespace PowerFake
{

/**
 * FakeIt integration for PowerFake
 */
template <typename Class = void>
class PowerFakeIt: public fakeit::ActualInvocationsSource
{
    private:
        class FakeData
        {
            public:
                FakeData(std::unique_ptr<internal::FakeBase> fake,
                    fakeit::Destructible *recorder) :
                        fake(std::move(fake)), recorder(recorder)
                {}

                template <typename R, typename ...Args>
                fakeit::RecordedMethodBody<R, Args...> &MethodRecorder()
                {
                    return static_cast<fakeit::RecordedMethodBody<R, Args...>&>(
                            *recorder);
                }

                const fakeit::ActualInvocationsSource &InvocationSource() const
                {
                    return dynamic_cast<fakeit::ActualInvocationsSource&>(
                            *recorder);
                }

            private:
                std::unique_ptr<internal::FakeBase> fake;
                std::unique_ptr<fakeit::Destructible> recorder;
        };

    public:
        template <typename R , typename ...Args>
        fakeit::MockingContext<R, Args...> stub(R (*func_ptr)(Args...))
        {
            auto f_key = FuncKey(func_ptr);
            auto mocked_it = mocked.find(f_key);
            if (mocked_it == mocked.end())
            {
                auto recorder = createRecordedMethodBody<R, Args...>(*this,
                        std::string(typeid(func_ptr).name()));
                auto fake_ptr = MakeFake(func_ptr,
                    [recorder](Args... args) {
                        return recorder->handleMethodInvocation(args...);
                    });
                auto ins = mocked.emplace(f_key, FakeData(std::move(fake_ptr),
                    recorder));
                mocked_it = ins.first;
            }
            return fakeit::MockingContext<R, Args...>(
                new MethodMockingContextImpl<R, Args...>(*this, mocked_it->second));
        }

        template <typename T, typename R , typename ...Args>
        fakeit::MockingContext<R, Args...> stub(R (T::*func_ptr)(Args...))
        {
            auto f_key = FuncKey(func_ptr);
            auto mocked_it = mocked.find(f_key);
            if (mocked_it == mocked.end())
            {
                auto recorder = createRecordedMethodBody<R, Args...>(*this,
                        std::string(typeid(func_ptr).name()));
                auto fake_ptr = MakeFake(func_ptr,
                    [recorder](Args... args) {
                        return recorder->handleMethodInvocation(args...);
                    });
                auto ins = mocked.emplace(f_key, FakeData(std::move(fake_ptr),
                    recorder));
                mocked_it = ins.first;
            }
            return fakeit::MockingContext<R, Args...>(
                new MethodMockingContextImpl<R, Args...>(*this, mocked_it->second));
        }

        template <int id, typename Tag>
        auto stub()
        {
            return stub(internal::unify_pmf(GetAddress(Tag())));
        }

        // swallow cv-qualified PMF types
        template <typename T, typename R>
        auto stub(R (T::*func_ptr))
        {
            return stub(internal::unify_pmf(func_ptr));
        }

        void getActualInvocations(
            std::unordered_set<fakeit::Invocation *> &into) const override
        {
            for (auto &method: mocked)
            {
                const ActualInvocationsSource &s = method.second.InvocationSource();
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

        // Method() macro compatibility
        Class get();

        template<int id, typename R, typename C,
            typename = typename std::enable_if_t<std::is_same<Class, C>::value>>
        auto stub(R (C::*func_ptr))
        {
            return stub(internal::unify_pmf(func_ptr));
        }

    private:
        std::map<internal::WrapperBase::FunctionKey, FakeData> mocked;

        template <typename FuncType>
        static internal::WrapperBase::FunctionKey FuncKey(FuncType func_ptr)
        {
#ifdef USE_PMF_ADDR
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wpmf-conversions"
            return std::make_pair(reinterpret_cast<void *>(func_ptr),
                std::type_index(typeid(FuncType)));
#pragma GCC diagnostic pop
#else
            return { PowerFake::internal::HashFunctionPtr(func_ptr),
                std::type_index(typeid(FuncType)) };
#endif
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
                MethodMockingContextImpl(PowerFakeIt &mock, FakeData &fake) :
                        MethodMockingContextBase<R, Args...>(mock),
                        fake(fake)
                {
                }

            protected:
                FakeData &fake;

                virtual fakeit::RecordedMethodBody<R, Args...> &getRecordedMethodBody() override
                {
                    return fake.template MethodRecorder<R, Args...>();
                }

                virtual std::function<R(Args&...)> getOriginalMethodCopyArgs() override
                {
                    throw std::runtime_error("Not supported!");
                }

                virtual std::function<R(Args&...)> getOriginalMethodForwardArgs() override
                {
                    throw std::runtime_error("Not supported!");
                }
        };
};

/**
 * Extends fakeit::Mock<> to support mocking private virtual member functions
 */
template<typename C, typename ... baseclasses>
class ExMock: public fakeit::Mock<C, baseclasses...> {
    public:
        using fakeit::Mock<C, baseclasses...>::Mock;

        using fakeit::Mock<C, baseclasses...>::stub;

        template <int id, typename Tag>
        auto stub()
        {
            return stub<id>(PowerFake::internal::unify_pmf(GetAddress(Tag())));
        }
};

} /* namespace PowerFake */

#endif /* MOCKING_FAKEIT_POWERFAKEIT_H_ */

