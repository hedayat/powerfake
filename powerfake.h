/*
 * powerfake.h
 *
 *  Created on: ۲۷ شهریور ۱۳۹۶
 *      Author: Hedayat Vatankhah <hedayat.fwd@gmail.com>
 */

#ifndef POWERFAKE_H_
#define POWERFAKE_H_

#include <array>
#include <vector>
#include <iostream>
#include <functional>
#include <boost/core/demangle.hpp>

namespace PowerFake
{


template <typename T>
class Mock
{
    public:
        template <typename Functor>
        Mock(T &o, Functor fake): o(o) { o.mock = fake; }
        ~Mock() { o.mock = typename T::MockType(); }
    private:
        T &o;
};

template <typename T>
class Wrapper;

template <typename T, typename R , typename ...Args>
class Mock<Wrapper<R (T::*)(Args...)>>
{
    private:
        typedef Wrapper<R (T::*)(Args...)> WT;

    public:
        Mock(WT &o, std::function<R (T *, Args...)> fake): o(o) {
            o.mock = fake;
        }
        Mock(WT &o, std::function<R (Args...)> fake): o(o) {
            o.mock = [fake](T *, Args... a) { return fake(a...); };
        }
        ~Mock() { o.mock = typename WT::MockType(); }
    private:
        WT &o;
};

template <typename T, typename Functor>
Mock<T> MakeMock(T &name, Functor f)
{
    return Mock<T>(name, f);
}

struct FunctionPrototype
{
    FunctionPrototype(std::string ret, std::string name, std::string params) :
            return_type(ret), name(name), params(params)
    {
    }
    std::string return_type;
    std::string name;
    std::string params;
    std::string alias;
};

template <typename T> struct PrototypeExtractor;

template <typename T, typename R , typename ...Args>
struct PrototypeExtractor<R (T::*)(Args...)>
{
    typedef std::function<R (T *o, Args... args)> MockType;
    typedef R (T::*MemFuncPtrType)(Args...);

    static FunctionPrototype Extract(const std::string &func_name);
};

template <typename R , typename ...Args>
struct PrototypeExtractor<R (*)(Args...)>
{
    typedef std::function<R (Args... args)> MockType;
    typedef R (*FuncPtrType)(Args...);

    static FunctionPrototype Extract(const std::string &func_name);
};

class WrapperBase
{
    public:
        static const std::vector<FunctionPrototype> &WrappedFunctions();

        WrapperBase(std::string alias, FunctionPrototype prototype)
        {
            prototype.alias = alias;
            AddFunction(prototype);
        }

    protected:
        static void AddFunction(FunctionPrototype sig);

    private:
        static std::vector<FunctionPrototype> wrapped_funcs;
};


template <typename T>
class Wrapper: public WrapperBase
{
    public:
        typedef typename PrototypeExtractor<T>::MockType MockType;

        using WrapperBase::WrapperBase;

        bool HasMock() const { return static_cast<bool>(mock); }
        const MockType &Call() const { return mock; }

    private:
        MockType mock;
        template <typename Y>
        friend class Mock;
};

#define TMP_POSTFIX         __end__
#define TMP_WRAPPER_PREFIX  __proxy_function_
#define TMP_REAL_PREFIX     __real_function_
#define BUILD_NAME_HELPER(A,B,C) A##B##C
#define BUILD_NAME(A,B,C) BUILD_NAME_HELPER(A,B,C)
#define TMP_REAL_NAME(base)  BUILD_NAME(TMP_REAL_PREFIX, base, TMP_POSTFIX)
#define TMP_WRAPPER_NAME(base)  BUILD_NAME(TMP_WRAPPER_PREFIX, base, TMP_POSTFIX)

#define DECLARE_WRAPPER(FN, ALIAS) extern PowerFake::Wrapper<decltype(&FN)> ALIAS

#define WRAP_FUNCTION(FN, ALIAS) \
    PowerFake::Wrapper<decltype(&FN)> ALIAS(#ALIAS, \
        PowerFake::PrototypeExtractor<decltype(&FN)>::Extract(#FN)); \
    template <typename T> struct proxy_##ALIAS; \
    template <typename T, typename R , typename ...Args> \
    struct proxy_##ALIAS<R (T::*)(Args...)> \
    { \
        static R TMP_WRAPPER_NAME(ALIAS)(T *o, Args... args) \
        { \
            R TMP_REAL_NAME(ALIAS)(T *o, Args... args); \
            if (ALIAS.HasMock()) \
                return ALIAS.Call()(o, args...); \
            return TMP_REAL_NAME(ALIAS)(o, args...); \
        } \
    }; \
    template <typename R , typename ...Args> \
    struct proxy_##ALIAS<R (*)(Args...)> \
    { \
        static R TMP_WRAPPER_NAME(ALIAS)(Args... args) \
        { \
            R TMP_REAL_NAME(ALIAS)(Args... args); \
            if (ALIAS.HasMock()) \
                return ALIAS.Call()(args...); \
            return TMP_REAL_NAME(ALIAS)(args...); \
        } \
    }; \
    template class proxy_##ALIAS<decltype(&FN)>;

template<typename T, typename R, typename ...Args>
FunctionPrototype PrototypeExtractor<R (T::*)(Args...)>::Extract(
    const std::string &func_name)
{
    const std::string class_type = boost::core::demangle(typeid(T).name());
    const std::string ret_type = boost::core::demangle(typeid(R).name());
    const std::string ptr_type = boost::core::demangle(
        typeid(MemFuncPtrType).name());

    std::string params = ptr_type.substr(ptr_type.rfind('('));
    std::string f = func_name.substr(func_name.rfind("::"));
    for (auto sp = f.find(' '); sp != std::string::npos; sp = f.find(' '))
        f.erase(sp, 1);
    return FunctionPrototype(ret_type, class_type + f, params);
}

template<typename R, typename ...Args>
FunctionPrototype PrototypeExtractor<R (*)(Args...)>::Extract(
    const std::string &func_name)
{
    const std::string ret_type = boost::core::demangle(typeid(R).name());
    const std::string ptr_type = boost::core::demangle(
        typeid(FuncPtrType).name());

    std::cout << "PTR TYPE:" << ptr_type << std::endl;
    std::string params = ptr_type.substr(ptr_type.rfind('('));
    return FunctionPrototype(ret_type, func_name, params);
}


}  // namespace PowerFake

#endif /* POWERFAKE_H_ */
