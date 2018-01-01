/*
 * powerfake.h
 *
 *  Created on: ۲۷ شهریور ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2017.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
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
class Wrapper;

/**
 * This class should be used to assign fake functions. It'll be released
 * automatically when destructed.
 *
 * It takes Wrapper<> classes as its template type, and the general form is
 * used for free functions and class static member functions.
 */
template <typename T>
class Fake
{
    public:
        template <typename Functor>
        Fake(T &o, Functor fake): o(o), orig_fake(o.fake) { o.fake = fake; }
        ~Fake() { o.fake = orig_fake; }

    private:
        T &o;
        typename T::FakeType orig_fake;
};

/**
 * Fake<> specialization for member functions, allowing fakes which does not
 * receive the original object pointer as their first parameter in addition to
 * normal fakes which do.
 */
template <typename T, typename R , typename ...Args>
class Fake<Wrapper<R (T::*)(Args...)>>
{
    private:
        typedef Wrapper<R (T::*)(Args...)> WT;

    public:
        Fake(WT &o, std::function<R(T *, Args...)> fake) :
                o(o), orig_fake(o.fake)
        {
            o.fake = fake;
        }
        Fake(WT &o, std::function<R (Args...)> fake): o(o), orig_fake(o.fake) {
            o.fake = [fake](T *, Args... a) { return fake(a...); };
        }
        ~Fake() { o.fake = orig_fake; }

    private:
        WT &o;
        typename WT::FakeType orig_fake;
};

/**
 * Helper function to make creating Fakes more conveniently.
 * (Can be eliminated in C++17)
 * @param alias the alias of the method to be faked
 * @param f the fake function
 * @return Fake object for faking the given function
 */
template <typename T, typename Functor>
Fake<T> MakeFake(T &alias, Functor f)
{
    return Fake<T>(alias, f);
}

/**
 * Stores components of a function prototype and the function alias
 */
struct FunctionPrototype
{
    FunctionPrototype(std::string ret, std::string name, std::string params,
        std::string alias = "") :
            return_type(ret), name(name), params(params), alias(alias)
    {
    }
    std::string return_type;
    std::string name;
    std::string params;
    std::string alias;
};

/**
 * This class provides an Extract() method which extracts the FunctionPrototype
 * for a given function. Can be used for both normal functions and member
 * functions through the specializations.
 *
 * We cannot use class names as used in our code (e.g. std::string), as they
 * are not necessarily the same name used when compiled. Therefore, we get
 * the mangled name of the type using typeid (in GCC), and demangle that name
 * to reach the actual name (e.g. std::basic_string<...> rather than
 * std::string).
 */
template <typename T> struct PrototypeExtractor;

/**
 * PrototypeExtractor specialization for member functions
 */
template <typename T, typename R , typename ...Args>
struct PrototypeExtractor<R (T::*)(Args...)>
{
    typedef std::function<R (T *o, Args... args)> FakeType;
    typedef R (T::*MemFuncPtrType)(Args...);

    static FunctionPrototype Extract(const std::string &func_name);
};

/**
 * PrototypeExtractor specialization for normal functions and static member
 * functions
 */
template <typename R , typename ...Args>
struct PrototypeExtractor<R (*)(Args...)>
{
    typedef std::function<R (Args... args)> FakeType;
    typedef R (*FuncPtrType)(Args...);

    static FunctionPrototype Extract(const std::string &func_name);
};

/**
 * Collects prototypes of all wrapped functions, to be used by bind_fakes
 */
class WrapperBase
{
    public:
        typedef std::vector<FunctionPrototype> Prototypes;

    public:
        /**
         * @return function prototype of all wrapped functions
         */
        static const Prototypes &WrappedFunctions();

        /**
         * Add wrapped function prototype and alias
         */
        WrapperBase(std::string alias, FunctionPrototype prototype)
        {
            prototype.alias = alias;
            AddFunction(prototype);
        }

    protected:
        static void AddFunction(FunctionPrototype sig);

    private:
        static Prototypes wrapped_funcs;
};


/**
 * Objects of this type are called 'alias'es for wrapped function, as it stores
 * the function object which will be called instead of the wrapped function.
 *
 * To make sure that function objects are managed properly, the user should use
 * Fake class and MakeFake() function rather than using the object of this class
 * directly.
 */
template <typename T>
class Wrapper: public WrapperBase
{
    public:
        typedef typename PrototypeExtractor<T>::FakeType FakeType;

        using WrapperBase::WrapperBase;

        bool Callable() const { return static_cast<bool>(fake); }

        template <typename ...Args>
        typename FakeType::result_type Call(Args&&... args) const
        {
            return fake(std::forward<Args>(args)...);
        }

    private:
        FakeType fake;
        friend class Fake<Wrapper>;
};

// helper macors
#define TMP_POSTFIX         __end__
#define TMP_WRAPPER_PREFIX  __wrap_function_
#define TMP_REAL_PREFIX     __real_function_
#define BUILD_NAME_HELPER(A,B,C) A##B##C
#define BUILD_NAME(A,B,C) BUILD_NAME_HELPER(A,B,C)
#define TMP_REAL_NAME(base)  BUILD_NAME(TMP_REAL_PREFIX, base, TMP_POSTFIX)
#define TMP_WRAPPER_NAME(base)  BUILD_NAME(TMP_WRAPPER_PREFIX, base, TMP_POSTFIX)
// select macro based on the number of args (2 or 3)
#define SELECT_MACRO(_1, _2, _3, NAME,...) NAME

/**
 * Declare wrapper for function FN with alias ALIAS. Should be used before
 * creating fake functions using MakeFake or Fake<> constructor
 */
#define DECLARE_WRAPPER_2(FTYPE, FNAME, ALIAS) \
    extern PowerFake::Wrapper<FTYPE> ALIAS

#define DECLARE_WRAPPER_1(FN, ALIAS) DECLARE_WRAPPER_2(decltype(&FN), FN, ALIAS)

#define DECLARE_WRAPPER(...) \
    SELECT_MACRO(__VA_ARGS__, DECLARE_WRAPPER_2, DECLARE_WRAPPER_1)(__VA_ARGS__)


/**
 * Define wrapper for function FN with alias ALIAS. Must be used only once for
 * each function in a cpp file.
 */
#define WRAP_FUNCTION_2(FTYPE, FNAME, ALIAS) \
    PowerFake::Wrapper<FTYPE> ALIAS(#ALIAS, \
        PowerFake::PrototypeExtractor<FTYPE>::Extract(#FNAME)); \
    /* Fake functions which will be called rather than the real function.
     * They call the function object in the alias Wrapper object
     * if available, otherwise it'll call the real function. */  \
    template <typename T> struct wrapper_##ALIAS; \
    template <typename T, typename R , typename ...Args> \
    struct wrapper_##ALIAS<R (T::*)(Args...)> \
    { \
        static R TMP_WRAPPER_NAME(ALIAS)(T *o, Args... args) \
        { \
            R TMP_REAL_NAME(ALIAS)(T *o, Args... args); \
            if (ALIAS.Callable()) \
                return ALIAS.Call(o, args...); \
            return TMP_REAL_NAME(ALIAS)(o, args...); \
        } \
    }; \
    template <typename R , typename ...Args> \
    struct wrapper_##ALIAS<R (*)(Args...)> \
    { \
        static R TMP_WRAPPER_NAME(ALIAS)(Args... args) \
        { \
            R TMP_REAL_NAME(ALIAS)(Args... args); \
            if (ALIAS.Callable()) \
                return ALIAS.Call(args...); \
            return TMP_REAL_NAME(ALIAS)(args...); \
        } \
    }; \
    /* Explicitly instantiate the wrapper_##ALIAS struct, so that the appropriate
     * wrapper function and real function symbol is actually generated by the
     * compiler. These symbols will be renamed to the name expected by 'ld'
     * linker by bind_fakes binary. */ \
    template class wrapper_##ALIAS<FTYPE>;

#define WRAP_FUNCTION_1(FN, ALIAS) WRAP_FUNCTION_2(decltype(&FN), FN, ALIAS)

#define WRAP_FUNCTION(...) \
    SELECT_MACRO(__VA_ARGS__, WRAP_FUNCTION_2, WRAP_FUNCTION_1)(__VA_ARGS__)

template<typename T, typename R, typename ...Args>
FunctionPrototype PrototypeExtractor<R (T::*)(Args...)>::Extract(
    const std::string &func_name)
{
    const std::string class_type = boost::core::demangle(typeid(T).name());
    const std::string ret_type = boost::core::demangle(typeid(R).name());
    const std::string ptr_type = boost::core::demangle(
        typeid(MemFuncPtrType).name());

    // ptr_type example: char* (Folan::*)(int const*, int, int)
    std::string params = ptr_type.substr(ptr_type.rfind('('));

    // strip scoping from func_name, we retrieve complete scoping (including
    // namespace(s) if available) from class_type
    std::string f = func_name.substr(func_name.rfind("::"));
    return FunctionPrototype(ret_type, class_type + f, params);
}

template<typename R, typename ...Args>
FunctionPrototype PrototypeExtractor<R (*)(Args...)>::Extract(
    const std::string &func_name)
{
    const std::string ret_type = boost::core::demangle(typeid(R).name());
    const std::string ptr_type = boost::core::demangle(
        typeid(FuncPtrType).name());

    // ptr_type example: char* (*)(int const*, int, int)
    std::string params = ptr_type.substr(ptr_type.rfind('('));
    return FunctionPrototype(ret_type, func_name, params);
}


}  // namespace PowerFake

#endif /* POWERFAKE_H_ */
