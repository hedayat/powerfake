/*
 * powerfake.h
 *
 *  Created on: ۲۷ شهریور ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah 2017-2020.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef POWERFAKE_H_
#define POWERFAKE_H_

#include <array>
#include <map>
#include <memory>
#include <vector>
#include <functional>
#include <stdexcept>
#include <typeindex>
#include <type_traits>
#include <boost/core/demangle.hpp>

namespace PowerFake
{

namespace internal {
class FakeBase;
}

using FakePtr = std::unique_ptr<internal::FakeBase>;

/**
 * Creates the fake object for the given function, faked with function object
 * @p f
 * @param func_ptr Pointer to the function to be faked
 * @param f the fake function
 * @return A fake object faking the given function with @p f. Fake is in effect
 * while this object lives
 */
template <typename Signature, typename Functor>
static FakePtr MakeFake(Signature *func_ptr, Functor f);

/**
 * Creates the fake object for the given member function, faked with @p f
 * @param func_ptr Pointer to the function to be faked
 * @param f the fake function
 * @return A fake object faking the given function with @p f. Fake is in effect
 * while this object lives
 */
template<typename Signature, typename Class, typename Functor>
static FakePtr MakeFake(Signature Class::*func_ptr, Functor f);

/**
 * Creates a fake object for a private member function tagged with
 * PrivateMemberTag; which should be created using TAG_PRIVATE_MEMBER() or
 * TAG_OVERLOADED_PRIVATE() macros
 * @param f the fake function
 * @return A fake object faking the function with the given tag with @p f.
 * Fake is in effect while this object lives
 */
template <typename PrivateMemberTag, typename Functor>
static FakePtr MakeFake(Functor f);

/**
 * Define a wrapper for the given function. For normal functions, it should be
 * called with the function name, e.g.:
 *      WRAP_FUNCTION(MyNameSpace::MyClass::MyFunction);
 * For overloaded function, you need to specify the function signature so
 * that the target function can be selected among the overloaded ones:
 *      WRAP_FUNCTION(void (MyNameSpace::MyClass::*)(int, float),
 *          MyNameSpace::MyClass::MyFunction)
 */
#define WRAP_FUNCTION(...) \
    SELECT_3RD(__VA_ARGS__, WRAP_FUNCTION_2, WRAP_FUNCTION_1)(__VA_ARGS__)

#define WRAP_STATIC_MEMBER(...) \
    SELECT_4TH(__VA_ARGS__, WRAP_STATIC_MEMBER_2, WRAP_STATIC_MEMBER_1)(__VA_ARGS__)

#define WRAP_PRIVATE_MEMBER(...) \
    SELECT_3RD(__VA_ARGS__, WRAP_PRIVATE_MEMBER_2, WRAP_PRIVATE_MEMBER_1)(__VA_ARGS__)


/**
 * It is not possible to pass private member functions directly to MakeFake(),
 * Therefore, we need to create a tag for that member function and pass it to
 * MakeFake().
 *
 * Note that it cannot be called inside a block
 */
#define TAG_PRIVATE_MEMBER(TAG, MEMBER_FUNCTION) \
    struct TAG: public PowerFake::internal::TagBase<TAG> { \
        static constexpr const char *member_name = #MEMBER_FUNCTION; \
    }; \
    template struct PowerFake::internal::PrivateFunctionExtractor<TAG, \
                                                            &MEMBER_FUNCTION>

#define TAG_OVERLOADED_PRIVATE(TAG, CLASS, FTYPE, MEMBER_FUNCTION) \
    struct TAG: public PowerFake::internal::TagBase<TAG> { \
        static constexpr const char *member_name = #MEMBER_FUNCTION; \
    }; \
    template struct PowerFake::internal::PrivateFunctionExtractor<TAG, \
        static_cast<decltype( \
            PowerFake::internal::FuncType<FTYPE, CLASS>(nullptr))>( \
                    &MEMBER_FUNCTION)>

namespace internal
{

enum Qualifiers
{
    NO_QUAL = 0,
    CONST = 1,
    VOLATILE = 2,
    NOEXCEPT = 4,
    // TODO: test these
    LV_REF = 8,
    RV_REF = 16,
    CONST_REF = 32
};

#if __cplusplus > 201703L
using std::type_identity;
#else
template<class T>
struct type_identity
{
    using type = T;
};
#endif

template <typename T>
struct func_cv_processor;

template <typename R , typename ...Args>
struct func_cv_processor<R (*)(Args...)>
{
    typedef R (*base_type)(Args...);
    static const uint32_t q = Qualifiers::NO_QUAL;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args...)>
{
    typedef R (T::*base_type)(Args...);
    static const uint32_t q = Qualifiers::NO_QUAL;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args...) const>
{
    typedef R (T::*base_type)(Args...);
    static const uint32_t q = Qualifiers::CONST;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args...) volatile>
{
    typedef R (T::*base_type)(Args...);
    static const uint32_t q = Qualifiers::VOLATILE;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args...) const volatile>
{
    typedef R (T::*base_type)(Args...);
    static const uint32_t q = Qualifiers::CONST | Qualifiers::VOLATILE;
};

#if __cplusplus >= 201703L
template <typename R , typename ...Args>
struct func_cv_processor<R (*)(Args...) noexcept>
{
    typedef R (*base_type)(Args...);
    static const uint32_t q = Qualifiers::NOEXCEPT;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args...) noexcept>
{
    typedef R (T::*base_type)(Args...);
    static const uint32_t q = Qualifiers::NOEXCEPT;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args...) const noexcept>
{
    typedef R (T::*base_type)(Args...);
    static const uint32_t q = Qualifiers::CONST | Qualifiers::NOEXCEPT;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args...) volatile noexcept>
{
    typedef R (T::*base_type)(Args...);
    static const uint32_t q = Qualifiers::VOLATILE | Qualifiers::NOEXCEPT;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args...) const volatile noexcept>
{
        typedef R (T::*base_type)(Args...);
        static const uint32_t q = Qualifiers::CONST | Qualifiers::VOLATILE
                | Qualifiers::NOEXCEPT;
};
#endif

template <typename T>
using remove_func_cv_t = typename func_cv_processor<T>::base_type;

template <typename T>
uint32_t func_qual_v = func_cv_processor<T>::q;

template <typename FuncType>
remove_func_cv_t<FuncType> unify_pmf(FuncType f)
{
    return reinterpret_cast<remove_func_cv_t<FuncType>>(f);
}

/*
 * FuncType template functions used to return function pointer type from
 * function signature. Providing Signature template parameter, it can be
 * used to return pointer to an overloaded function using signature syntax,
 * which is specially useful for class member functions.
 * The last variant receives a function pointer type, so the user can also
 * pass a pointer type rather than function type itself.
 */
template <typename Signature, typename C>
Signature C::* FuncType(Signature C::*);
template <typename Signature>
Signature *FuncType(Signature *);
template <typename FuncPtr>
FuncPtr FuncType(FuncPtr);

std::string ToStr(uint32_t q, bool mangled = false);

#if __cplusplus >= 201703L
// Provide access to private member functions. Inspired by:
// http://bloglitb.blogspot.com/2011/12/access-to-private-members-safer.html
template<typename Tag, auto PrivMemfuncPtr>
struct PrivateFunctionExtractor
{
    friend auto GetAddress(Tag) { return PrivMemfuncPtr; }
};
#endif

/**
 * Base class for tags used to refer to private class members. It also enables
 * calling of private functions & access to private member variables using
 * Call() & Value() functions
 */
template <typename Tag>
struct TagBase {
    template <typename Class, typename ...Args>
    static decltype(auto) Call(Class &obj, Args... args)
    {
        return (obj.*GetAddress(Tag()))(args...);
    }

    template <typename Class>
    static auto &Value(Class &obj)
    {
        return obj.*GetAddress(Tag());
    }

    template <typename Class>
    static const auto &Value(const Class &obj)
    {
        return obj.*GetAddress(Tag());
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
    friend auto GetAddress(Tag);
#pragma GCC diagnostic pop
};

/**
 * A base class for all Fake<> classes, so that we can store them inside a
 * container
 */
class FakeBase
{
    public:
        FakeBase() = default;
        FakeBase(const FakeBase &) = delete;
        FakeBase(FakeBase &&) = default;
        virtual ~FakeBase() {}
};

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
class Fake: public FakeBase
{
    public:
        Fake(Fake &&) = default;
        template <typename Functor>
        Fake(Wrapper<T> &o, Functor fake): o(o), orig_fake(o.fake) { o.fake = fake; }
        ~Fake() { o.fake = orig_fake; }

    private:
        Wrapper<T> &o;
        typename Wrapper<T>::FakeFunction orig_fake;
};

/**
 * Fake<> specialization for member functions, allowing fakes which does not
 * receive the original object pointer as their first parameter in addition to
 * normal fakes which do.
 */
template <typename T, typename R , typename ...Args>
class Fake<R (T::*)(Args...)>: public FakeBase
{
    private:
        typedef Wrapper<R (T::*)(Args...)> WT;

    public:
        Fake(Fake &&) = default;
        Fake(WT &o, std::function<R(T *, Args...)> fake) :
                o(o), orig_fake(o.fake)
        {
            o.fake = fake;
        }
        Fake(WT &o, std::function<R (Args...)> fake): o(o), orig_fake(o.fake) {
            o.fake = [fake](T *, Args... a) -> R { return fake(a...); };
        }
        ~Fake() { o.fake = orig_fake; }

    private:
        WT &o;
        typename WT::FakeFunction orig_fake;
};


/**
 * Stores components of a function prototype and the function alias
 */
struct FunctionPrototype
{
    FunctionPrototype(std::string ret, std::string name, std::string params,
        uint32_t qual, std::string alias = "") :
            return_type(ret), name(name), params(params), qual(qual),
            alias(alias)
    {
    }

    std::string Str() const
    {
        return return_type + ' ' + name + params + ' ' + internal::ToStr(qual);
    }

    std::string return_type;
    std::string name;
    std::string params;
    uint32_t qual;
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
    typedef std::function<R (T *o, Args... args)> FakeFunction;

    static FunctionPrototype Extract(const std::string &func_name,
        uint32_t fq = internal::Qualifiers::NO_QUAL);
};

/**
 * PrototypeExtractor specialization for normal functions and static member
 * functions
 */
template <typename R , typename ...Args>
struct PrototypeExtractor<R (*)(Args...)>
{
    typedef std::function<R (Args... args)> FakeFunction;
    typedef R (*FuncPtrType)(Args...);

    static FunctionPrototype Extract(const std::string &func_name,
        uint32_t fq = internal::Qualifiers::NO_QUAL);

    template <typename Class>
    static FunctionPrototype Extract(const std::string &func_name,
        uint32_t fq = internal::Qualifiers::NO_QUAL);
};


/**
 * Collects prototypes of all wrapped functions, to be used by bind_fakes
 */
class WrapperBase
{
    public:
        typedef std::multimap<std::string, FunctionPrototype> Prototypes;
        typedef std::pair<void *, std::type_index> FunctionKey;
        typedef std::map<FunctionKey, WrapperBase *> FunctionWrappers;

    public:
        /**
         * @return function prototype of all wrapped functions
         */
        static const Prototypes &WrappedFunctions();

        /**
         * Add wrapped function prototype and alias
         */
        WrapperBase(std::string alias, FunctionKey key,
            FunctionPrototype prototype)
        {
            prototype.alias = alias;
            AddFunction(key, prototype);
        }

    protected:
        template <typename RetType>
        static RetType *WrapperObject(FunctionKey key)
        {
            auto w = wrappers->find(key);
            if (w == wrappers->end())
                throw std::invalid_argument("Wrapped function with the given "
                        "key not found");
            return static_cast<RetType *>(w->second);
        }

        void AddFunction(FunctionKey func_key, FunctionPrototype sig);

    private:
        static Prototypes *wrapped_funcs;
        static FunctionWrappers *wrappers;
};


/**
 * Objects of this type are called 'alias'es for wrapped function, as it stores
 * the function object which will be called instead of the wrapped function.
 *
 * To make sure that function objects are managed properly, the user should use
 * Fake class and MakeFake() function rather than using the object of this class
 * directly.
 */
template <typename FuncType>
class Wrapper: public WrapperBase
{
    public:
        typedef typename PrototypeExtractor<FuncType>::FakeFunction FakeFunction;

    public:
        /**
         * Add wrapped function prototype and alias
         */
        Wrapper(std::string alias, FuncType func_ptr, uint32_t fq,
            std::string func_name) :
                WrapperBase(alias, FuncKey(func_ptr),
                    PrototypeExtractor<FuncType>::Extract(func_name, fq))
        {
        }

        template<typename Class>
        Wrapper(internal::type_identity<Class>, std::string alias,
            FuncType func_ptr, uint32_t fq, std::string func_name) :
                WrapperBase(alias, FuncKey(func_ptr),
                    PrototypeExtractor<FuncType>::template Extract<Class>(
                        func_name, fq))
        {
        }

        bool Callable() const { return static_cast<bool>(fake); }

        template <typename ...Args>
        typename FakeFunction::result_type Call(Args&&... args) const
        {
            return fake(std::forward<Args>(args)...);
        }

        static Wrapper &WrapperObject(FuncType func)
        {
            return *WrapperBase::WrapperObject<Wrapper>(FuncKey(func));
        }

    private:
        FakeFunction fake;
        friend class internal::Fake<FuncType>;

        static FunctionKey FuncKey(FuncType func_ptr)
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
#pragma GCC diagnostic ignored "-Wpedantic"
            return std::make_pair(reinterpret_cast<void *>(func_ptr),
                std::type_index(typeid(FuncType)));
#pragma GCC diagnostic pop
        }
};

} // namespace internal


// helper macors
#define TMP_POSTFIX         __end__
#define TMP_WRAPPER_PREFIX  __wrap_function_
#define TMP_REAL_PREFIX     __real_function_
#define BUILD_NAME_HELPER(A,B,C) A##B##C
#define BUILD_NAME(A,B,C) BUILD_NAME_HELPER(A,B,C)
#define TMP_REAL_NAME(base)  BUILD_NAME(TMP_REAL_PREFIX, base, TMP_POSTFIX)
#define TMP_WRAPPER_NAME(base)  BUILD_NAME(TMP_WRAPPER_PREFIX, base, TMP_POSTFIX)
// select macro based on the number of args
#define SELECT_3RD(_1, _2, NAME,...) NAME
#define SELECT_4TH(_1, _2, _3, NAME,...) NAME
#define PRIVATE_TAG(ALIAS) ALIAS##PowerFakePrivateTag
#define PRIVMEMBER_ADDR(ALIAS) GetAddress(PRIVATE_TAG(ALIAS)())


/// If you use WRAP_FUNCTION() macros in more than a single file, you should
/// define a different namespace for each file, otherwise 'multiple definition'
/// errors can happen
#ifndef POWRFAKE_WRAP_NAMESPACE
#define POWRFAKE_WRAP_NAMESPACE PowerFakeWrap
#endif

#define CREATE_WRAPPER_FUNCTION(FTYPE, ALIAS) \
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
    template class wrapper_##ALIAS<PowerFake::internal::remove_func_cv_t<FTYPE>>


#define DEFINE_WRAPPER_OBJECT(FTYPE, FNAME, FADDR, ALIAS) \
    static PowerFake::internal::Wrapper<PowerFake::internal::remove_func_cv_t<FTYPE>> \
        ALIAS(#ALIAS, PowerFake::internal::unify_pmf<FTYPE>(FADDR), \
            PowerFake::internal::func_qual_v<FTYPE>, #FNAME);

#define DEFINE_WRAPPER_OBJECT2(FCLASS, FTYPE, FNAME, ALIAS) \
    static PowerFake::internal::Wrapper<PowerFake::internal::remove_func_cv_t<FTYPE>> \
        ALIAS(PowerFake::internal::type_identity<FCLASS>(), \
                #ALIAS, PowerFake::internal::unify_pmf<FTYPE>(&FNAME), \
                PowerFake::internal::func_qual_v<FTYPE>, #FNAME);

#ifndef BIND_FAKES

/**
 * Define wrapper for function FNAME with type FTYPE and alias ALIAS. Must be
 * used only once for each function in a cpp file.
 */
#define WRAP_FUNCTION_BASE(FTYPE, FNAME, FADDR, ALIAS) \
    DEFINE_WRAPPER_OBJECT(FTYPE, FNAME, FADDR, ALIAS) \
    CREATE_WRAPPER_FUNCTION(FTYPE, ALIAS)

/**
 * Define wrapper for static member function FNAME of class FCLASS with type
 * FTYPE and alias ALIAS.
 */
#define WRAP_STATIC_MEMBER_BASE(FCLASS, FTYPE, FNAME, ALIAS) \
    DEFINE_WRAPPER_OBJECT2(FCLASS, FTYPE, FNAME, ALIAS) \
    CREATE_WRAPPER_FUNCTION(FTYPE, ALIAS)

#else // BIND_FAKES

#define WRAP_FUNCTION_BASE(FTYPE, FNAME, FADDR, ALIAS) \
    DEFINE_WRAPPER_OBJECT(FTYPE, FNAME, nullptr, ALIAS)

#define WRAP_STATIC_MEMBER_BASE(FCLASS, FTYPE, FNAME, ALIAS) \
    DEFINE_WRAPPER_OBJECT2(FCLASS, FTYPE, FNAME, ALIAS)

#endif


#define WRAP_PRIVATE_BASE(FTYPE, FNAME, ALIAS) \
    WRAP_FUNCTION_BASE(decltype( \
        PowerFake::internal::FuncType<FTYPE>(PRIVMEMBER_ADDR(ALIAS))), FNAME, \
        PRIVMEMBER_ADDR(ALIAS), ALIAS)

/**
 * Wrap a private member function with alias ALIAS.
 */
#define WRAP_PRIVATE_MEMBER_IMPL(FTYPE, FNAME, ALIAS) \
    TAG_PRIVATE_MEMBER(PRIVATE_TAG(ALIAS), FNAME); \
    WRAP_PRIVATE_BASE(FTYPE, FNAME, ALIAS)

#define WRAP_PRIVATE_MEMBER_IMPL_2(CLASS, FTYPE, FNAME, ALIAS) \
    TAG_OVERLOADED_PRIVATE(PRIVATE_TAG(ALIAS), CLASS, FTYPE, FNAME); \
    WRAP_PRIVATE_BASE(FTYPE, FNAME, ALIAS)

/**
 * Define a wrapper for function with type FTYPE and name FNAME.
 */
#define WRAP_FUNCTION_2(FTYPE, FNAME) \
    WRAP_FUNCTION_BASE(decltype(PowerFake::internal::FuncType<FTYPE>(&FNAME)), \
        FNAME, &FNAME, BUILD_NAME(POWRFAKE_WRAP_NAMESPACE, _alias_, __LINE__))

/**
 * Define a wrapper for function named FNAME.
 */
#define WRAP_FUNCTION_1(FNAME) WRAP_FUNCTION_2(decltype(&FNAME), FNAME)

/**
 * Define a wrapper for static member function of class FCLASS with type
 * FTYPE and name FNAME.
 */
#define WRAP_STATIC_MEMBER_2(FCLASS, FTYPE, FNAME) \
    WRAP_STATIC_MEMBER_BASE(FCLASS, decltype( \
            PowerFake::internal::FuncType<FTYPE>(&FNAME)), FNAME, \
            BUILD_NAME(POWRFAKE_WRAP_NAMESPACE, _alias_, __LINE__))

/**
 * Define a wrapper for static member function FNAME of class FCLASS.
 */
#define WRAP_STATIC_MEMBER_1(FCLASS, FNAME) \
    WRAP_STATIC_MEMBER_2(FCLASS, decltype(&FNAME), FNAME)

/**
 * Define a wrapper for private member function with type FTYPE and name FNAME
 */
#define WRAP_OVERLOADED_PRIVATE(CLASS, FTYPE, FNAME) \
        WRAP_PRIVATE_MEMBER_IMPL_2(CLASS, FTYPE, FNAME, \
            BUILD_NAME(POWRFAKE_WRAP_NAMESPACE, _alias_, __LINE__))

#define WRAP_PRIVATE_MEMBER_2(FTYPE, FNAME) \
    WRAP_PRIVATE_MEMBER_IMPL_2(FTYPE, FNAME, \
        BUILD_NAME(POWRFAKE_WRAP_NAMESPACE, _alias_, __LINE__))

#define WRAP_PRIVATE_MEMBER_1_HELPER(FNAME, ALIAS) \
    WRAP_PRIVATE_MEMBER_IMPL(decltype(PRIVMEMBER_ADDR(ALIAS)), FNAME, ALIAS)

/**
 * Define a wrapper for private member function with name FNAME
 */
#define WRAP_PRIVATE_MEMBER_1(FNAME) \
    WRAP_PRIVATE_MEMBER_1_HELPER(FNAME, \
        BUILD_NAME(POWRFAKE_WRAP_NAMESPACE, _alias_, __LINE__))

// MakeFake implementations
template <typename Signature, typename Functor>
static FakePtr MakeFake(Signature *func_ptr, Functor f)
{
    typedef internal::remove_func_cv_t<Signature *> FuncType;
    return std::make_unique<internal::Fake<FuncType>>(
            internal::Wrapper<FuncType>::WrapperObject(func_ptr), f);
}

template<typename Signature, typename Class, typename Functor>
static FakePtr MakeFake(Signature Class::*func_ptr, Functor f)
{
    typedef internal::remove_func_cv_t<Signature Class::*> FuncType;
    return std::make_unique<internal::Fake<FuncType>>(
        internal::Wrapper<FuncType>::WrapperObject(internal::unify_pmf(func_ptr)), f);
}

template <typename PrivateMemberTag, typename Functor>
static FakePtr MakeFake(Functor f)
{
    return MakeFake(GetAddress(PrivateMemberTag()), f);
}

namespace internal
{

// PrototypeExtractor implementations
template<typename T, typename R, typename ...Args>
FunctionPrototype PrototypeExtractor<R (T::*)(Args...)>::Extract(
    const std::string &func_name, uint32_t fq)
{
    // strip scoping from func_name, we retrieve complete scoping (including
    // namespace(s) if available) from class_type
    std::string f = func_name.substr(func_name.rfind("::"));
    const std::string class_type = boost::core::demangle(typeid(T).name());
    return PrototypeExtractor<R (*)(Args...)>::Extract(class_type + f, fq);
}

template<typename R, typename ...Args>
FunctionPrototype PrototypeExtractor<R (*)(Args...)>::Extract(
    const std::string &func_name, uint32_t fq)
{
    const std::string ret_type = boost::core::demangle(typeid(R).name());
    const std::string ptr_type = boost::core::demangle(
        typeid(FuncPtrType).name());

    // ptr_type example: char* (*)(int const*, int, int)
    std::string params = ptr_type.substr(ptr_type.rfind('('));
    return FunctionPrototype(ret_type, func_name, params, fq);
}

template<typename R, typename ...Args>
template<typename Class>
FunctionPrototype PrototypeExtractor<R (*)(Args...)>::Extract(
    const std::string &func_name, uint32_t fq)
{
    return PrototypeExtractor<R (Class::*)(Args...)>::Extract(func_name, fq);
}

} // namespace internal

}  // namespace PowerFake

#endif /* POWERFAKE_H_ */
