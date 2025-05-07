/*
 * powerfake.h
 *
 *  Created on: ۲۷ شهریور ۱۳۹۶
 *
 *  Copyright Hedayat Vatankhah <hedayat.fwd@gmail.com>, 2017-2022.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

/** @file powerfake.h
    @brief PowerFake Main API

    This file provides main user API
*/

#ifndef POWERFAKE_H_
#define POWERFAKE_H_

#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <vector>
#include <functional>
#include <stdexcept>
#include <string_view>
#include <typeindex>
#include <type_traits>
#include <boost/core/demangle.hpp>
#include <array>

namespace PowerFake
{

namespace internal {

class FakeBase;

template <typename FT, typename = void>
struct functor_helper {};

template <typename FT, typename = std::enable_if_t<
        bool(sizeof(typename functor_helper<FT>::std_func_sig_t::member_ptr))>>
struct class_functor_helper;

}

using FakePtr = std::unique_ptr<internal::FakeBase>;

/** @defgroup user_api The User API
 *  This is the basic user API (independent of any macking frameworks)
 *  @{
 */

#if __has_cpp_attribute(nodiscard) >= 201907L
#define MFK_DISCARD_WARNING ("MakeFake is effective until the return value is alive")
#else
#define MFK_DISCARD_WARNING
#endif

/**
 * Creates the fake object for the given function, faked with function object
 * @p f. You can specify the function signature explicitly.
 * @param func_ptr Pointer to the function to be faked
 * @param f the fake function, which should have the same signature as the faked
 * function
 * @return A fake object faking the given function with @p f. Fake is in effect
 * while this object lives
 */
template <typename Signature, typename Functor>
[[nodiscard MFK_DISCARD_WARNING]]
static FakePtr MakeFake(Signature *func_ptr, Functor f);

/**
 * Creates the fake object for the given member function, faked with @p f
 *
 * @param func_ptr Pointer to the function to be faked
 * @param f the fake function, can receive object's this pointer if its first
 * parameter is a pointer to @p Class
 * @return A fake object faking the given function with @p f. Fake is in effect
 * while this object lives
 */
template<typename Signature, typename Class, typename Functor>
[[nodiscard MFK_DISCARD_WARNING]]
static FakePtr MakeFake(Signature Class::*func_ptr, Functor f);

/**
 * Creates a fake object for a private member function tagged with
 * PrivateMemberTag; which should be created using TAG_PRIVATE() macro.
 *
 * @tparam PrivateMemberTag the private member function tag defined using
 * TAG_PRIVATE()
 * @param f the fake function, which can receive object's this pointer as its
 * first parameter
 * @return A fake object faking the function with the given tag with @p f.
 * Fake is in effect while this object lives
 */
template <typename PrivateMemberTag, typename Functor>
[[nodiscard MFK_DISCARD_WARNING]]
static FakePtr MakeFake(Functor f);

/**
 * Creates the fake object for the given function, faked with function object
 * @p f. This overload derives the type of @p func_ptr from the signature
 * of @p f, so you won't need to specify the type of overload you need.
 * @param func_ptr Pointer to the function to be faked
 * @param f the fake function, which should have the same signature as the faked
 * function
 * @return A fake object faking the given function with @p f. Fake is in effect
 * while this object lives
 */
template <typename Functor>
[[nodiscard MFK_DISCARD_WARNING]]
static FakePtr MakeFake(
    typename internal::functor_helper<Functor>::type func_ptr, Functor f);

/**
 * Creates the fake object for the given member function, faked with @p f. It
 * derives the type of @p func_ptr from @p f to select the appropriate overload
 * automatically from available member functions.
 *
 * @param func_ptr Pointer to the function to be faked
 * @param f the fake function, which must receive object's this pointer as its
 * first parameter; which is cv-qualified pointer to @p Class which is used
 * to select appropriate cv-qualified overload
 * @return A fake object faking the given function with @p f. Fake is in effect
 * while this object lives
 */
template<typename Functor>
[[nodiscard MFK_DISCARD_WARNING]]
static FakePtr MakeFake(
    typename internal::class_functor_helper<Functor>::member_ptr func_ptr,
    Functor f);

#undef MFK_DISCARD_WARNING

/**
 * Define a wrapper for the given function. For normal functions, it should be
 * called with the function name, e.g.:
 *
 *      WRAP_FUNCTION(MyNameSpace::MyClass::MyFunction);
 *
 * For overloaded function, you need to specify the function signature so
 * that the target function can be selected among the overloaded ones:
 *
 *      WRAP_FUNCTION(void (int, float), MyNameSpace::MyClass::MyFunction)
 *
 * For static or private functions, you should mark the function as such:
 *
 *      WRAP_FUNCTION(PFK_PRIVATE(MyClass::MyFunction));
 *      WRAP_FUNCTION(PFK_STATIC(MyClass, MyClass::MyFunction));
 *      WRAP_FUNCTION(PFK_STATIC(MyClass, PFK_PRIVATE(MyClass::MyFunction)));
 *
 * #### Usage
 *     WRAP_FUNCTION(FUNC) \n
 *     WRAP_FUNCTION(FSIG, FUNC)
 *
 * @param FSIG function type signature; required for overloaded functions;
 *              e.g. void (int)
 * @param FUNC specifies the function to wrap; for static, private and static
 *              private functions, you should use #PFK_PRIVATE/#PFK_STATIC macros
 *              to mark the function as such
 */
#define WRAP_FUNCTION(...) \
    PFK_SELECT_9TH(__VA_ARGS__, \
        PFK_WRAP_OVERLOADED_STATIC_PRIVATE_MEMBER, PFK_WRAP_STATIC_PRIVATE_MEMBER, \
        CALL_WRAP_OVERLOADED_PRIVATE_MEMBER, CALL_WRAP_PRIVATE_MEMBER, \
        PFK_WRAP_OVERLOADED_STATIC_MEMBER, PFK_WRAP_STATIC_MEMBER, \
        PFK_WRAP_OVERLOADED_FUNCTION, PFK_WRAP_FUNCTION)(WRAPPED, __VA_ARGS__)

#ifndef __MINGW32__
/**
 * Hides the given function, so that it can be replaced with a fake/mock. You
 * cannot access the actual function anymore, but using HIDE_FUNCTION() you can
 * also capture function calls in the same translation unit (unless the call is
 * optimized by the compiler, which can happen if function call is inlined or
 * if it undergoes 'sibling call optimization' (e.g. when the function call is
 * the last statement of the caller function).
 *
 * #### Usage
 *     HIDE_FUNCTION(FUNC) \n
 *     HIDE_FUNCTION(FSIG, FUNC)
 *
 * @param FSIG function type signature; required for overloaded functions;
 *              e.g. void (int)
 * @param FUNC specifies the function to wrap; for static, private and static
 *              private functions, you should use #PFK_PRIVATE/#PFK_STATIC macros
 *              to mark the function as such
 * @see WRAP_FUNCTION()
 */
#define HIDE_FUNCTION(...) \
    PFK_SELECT_9TH(__VA_ARGS__, \
        PFK_WRAP_OVERLOADED_STATIC_PRIVATE_MEMBER, PFK_WRAP_STATIC_PRIVATE_MEMBER, \
        CALL_WRAP_OVERLOADED_PRIVATE_MEMBER, CALL_WRAP_PRIVATE_MEMBER, \
        PFK_WRAP_OVERLOADED_STATIC_MEMBER, PFK_WRAP_STATIC_MEMBER, \
        PFK_WRAP_OVERLOADED_FUNCTION, PFK_WRAP_FUNCTION)(HIDDEN, __VA_ARGS__)

#endif

/**
 * Specifies a static member function for WRAP_FUNCTION/HIDE_FUNCTION macros
 */
#define PFK_STATIC(CLASS, FUNCTION) CLASS, FUNCTION, _1

/**
 * Specifies a private member function for WRAP_FUNCTION/HIDE_FUNCTION macros
 */
#define PFK_PRIVATE(FUNCTION) FUNCTION, _1, _2, _3, _4

#ifndef DISABLE_PFK_SIMPLE_NAMES
/*
 * Define very simple macro names for some macros, for example to specify
 * static/private functions, if DISABLE_PFK_SIMPLE_NAMES is not defined
 */

#define PRIVATE     PFK_PRIVATE
#define STATIC      PFK_STATIC
#define TYPE_HINT   PFK_TYPE_HINT
#endif

/**
 * Creates a tag class for the given private member. For overloaded functions
 * the function type should be explicitly specified so that the correct function
 * can be selected.
 *
 * It is not possible to pass private member functions directly to MakeFake();
 * therefore, we need to create a tag for that member function and pass it to
 * MakeFake().
 *
 * Additionally, you can use this macro to tag private member variables, and
 * access them using the static Value() function of the tag class.
 *
 * @note It should be a top level call and cannot be called inside a block
 *
 * #### Usage
 *     TAG_PRIVATE(TAG, PRIVATE_MEMBER) \n
 *     TAG_PRIVATE(TAG, FSIG, PRIVATE_MEMBER)
 *
 * @param TAG the name of the tag class to be used instead of the private member
 * @param FSIG overloaded function type, which can be specified in function
 * signature format; e.g. void (int)
 * @param PRIVATE_MEMBER the private member which we want to tag
 */
#define TAG_PRIVATE(...) \
    PFK_SELECT_9TH(__VA_ARGS__,,,,,, PFK_TAG_OVERLOADED_PRIVATE, \
        PFK_TAG_PRIVATE_MEMBER)(__VA_ARGS__)

/**@}*/

namespace internal
{

enum Qualifiers: unsigned
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

/**
 * Separates functions base type and its qualifiers
 */
template <typename T>
struct func_cv_processor;

template <typename R , typename ...Args>
struct func_cv_processor<R (*)(Args...)>
{
    typedef R (*base_type)(Args...);
    static const uint32_t q = Qualifiers::NO_QUAL;
};

template <typename R , typename ...Args>
struct func_cv_processor<R (*)(Args..., ...)>
{
    typedef R (*base_type)(Args..., ...);
    static const uint32_t q = Qualifiers::NO_QUAL;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args...)>
{
    typedef R (T::*base_type)(Args...);
    static const uint32_t q = Qualifiers::NO_QUAL;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args..., ...)>
{
    typedef R (T::*base_type)(Args..., ...);
    static const uint32_t q = Qualifiers::NO_QUAL;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args...) const>
{
    typedef R (T::*base_type)(Args...);
    static const uint32_t q = Qualifiers::CONST;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args..., ...) const>
{
    typedef R (T::*base_type)(Args..., ...);
    static const uint32_t q = Qualifiers::CONST;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args...) volatile>
{
    typedef R (T::*base_type)(Args...);
    static const uint32_t q = Qualifiers::VOLATILE;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args..., ...) volatile>
{
    typedef R (T::*base_type)(Args..., ...);
    static const uint32_t q = Qualifiers::VOLATILE;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args...) const volatile>
{
    typedef R (T::*base_type)(Args...);
    static const uint32_t q = Qualifiers::CONST | Qualifiers::VOLATILE;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args..., ...) const volatile>
{
    typedef R (T::*base_type)(Args..., ...);
    static const uint32_t q = Qualifiers::CONST | Qualifiers::VOLATILE;
};

#if __cplusplus >= 201703L
template <typename R , typename ...Args>
struct func_cv_processor<R (*)(Args...) noexcept>
{
    typedef R (*base_type)(Args...);
    static const uint32_t q = Qualifiers::NOEXCEPT;
};

template <typename R , typename ...Args>
struct func_cv_processor<R (*)(Args..., ...) noexcept>
{
    typedef R (*base_type)(Args..., ...);
    static const uint32_t q = Qualifiers::NOEXCEPT;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args...) noexcept>
{
    typedef R (T::*base_type)(Args...);
    static const uint32_t q = Qualifiers::NOEXCEPT;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args..., ...) noexcept>
{
    typedef R (T::*base_type)(Args..., ...);
    static const uint32_t q = Qualifiers::NOEXCEPT;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args...) const noexcept>
{
    typedef R (T::*base_type)(Args...);
    static const uint32_t q = Qualifiers::CONST | Qualifiers::NOEXCEPT;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args..., ...) const noexcept>
{
    typedef R (T::*base_type)(Args..., ...);
    static const uint32_t q = Qualifiers::CONST | Qualifiers::NOEXCEPT;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args...) volatile noexcept>
{
    typedef R (T::*base_type)(Args...);
    static const uint32_t q = Qualifiers::VOLATILE | Qualifiers::NOEXCEPT;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args..., ...) volatile noexcept>
{
    typedef R (T::*base_type)(Args..., ...);
    static const uint32_t q = Qualifiers::VOLATILE | Qualifiers::NOEXCEPT;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args...) const volatile noexcept>
{
        typedef R (T::*base_type)(Args...);
        static const uint32_t q = Qualifiers::CONST | Qualifiers::VOLATILE
                | Qualifiers::NOEXCEPT;
};

template <typename T, typename R , typename ...Args>
struct func_cv_processor<R (T::*)(Args..., ...) const volatile noexcept>
{
        typedef R (T::*base_type)(Args..., ...);
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

    // for static private member functions
    template <typename ...Args>
    static decltype(auto) Call(Args... args)
    {
        return GetAddress(Tag())(args...);
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

    // for static private member variables
    static auto &Value()
    {
        return *GetAddress(Tag());
    }

#if !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
#endif
    friend auto GetAddress(Tag);
#if !defined(__clang__)
#pragma GCC diagnostic pop
#endif
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
    FunctionPrototype() = default;
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
    uint32_t qual = Qualifiers::NO_QUAL;
    std::string alias;
};

enum class FakeType
{
    WRAPPED,
    HIDDEN
};

struct FunctionInfo
{
    FunctionPrototype prototype;
    FakeType fake_type = FakeType::WRAPPED;
    std::string symbol = "";
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
    typedef std::function<R (T *o, Args...)> FakeFunction;

    static FunctionPrototype Extract(const std::string &func_name,
        uint32_t fq = internal::Qualifiers::NO_QUAL);
};

/**
 * PrototypeExtractor specialization for variadic member functions
 */
template <typename T, typename R , typename ...Args>
struct PrototypeExtractor<R (T::*)(Args..., ...)>
{
    typedef std::function<R (T *o, Args..., va_list)> FakeFunction;

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
    typedef std::function<R (Args...)> FakeFunction;
    typedef R (*FuncPtrType)(Args...);

    static FunctionPrototype Extract(const std::string &func_name,
        uint32_t fq = internal::Qualifiers::NO_QUAL);

    template <typename Class>
    static FunctionPrototype Extract(const std::string &func_name,
        uint32_t fq = internal::Qualifiers::NO_QUAL);
};

/**
 * PrototypeExtractor specialization for variadic normal functions and static
 * member functions
 */
template <typename R , typename ...Args>
struct PrototypeExtractor<R (*)(Args..., ...)>
{
    typedef std::function<R (Args..., va_list)> FakeFunction;
    typedef R (*FuncPtrType)(Args..., ...);

    static FunctionPrototype Extract(const std::string &func_name,
        uint32_t fq = internal::Qualifiers::NO_QUAL);

    template <typename Class>
    static FunctionPrototype Extract(const std::string &func_name,
        uint32_t fq = internal::Qualifiers::NO_QUAL);
};

template <typename T>
size_t HashFunctionPtr(T *func_ptr);

template <typename Signature, typename Class>
size_t HashFunctionPtr(Signature Class::*func_ptr);

#if defined(__GNUC__) && !defined(__clang__)
// todo lets keep using pmf address for now. If the alternative impmlmementation
// works well, we can switch using it under GCC too.
#define USE_PMF_ADDR
#endif

/**
 * Collects prototypes of all wrapped functions, to be used by bind_fakes
 */
class WrapperBase
{
    private:
#ifdef USE_PMF_ADDR
        typedef void *FuncPtrID;
#else
        typedef size_t FuncPtrID;
#endif

    public:
        typedef std::vector<FunctionInfo> Functions;
        typedef std::pair<FuncPtrID, std::type_index> FunctionKey;
        typedef std::map<FunctionKey, WrapperBase *> FunctionWrappers;

    public:
        /**
         * @return function prototype of all wrapped functions
         */
        static Functions &WrappedFunctions();

        /**
         * Add wrapped function prototype and alias
         */
        WrapperBase(std::string alias, FunctionKey key, FakeType fake_type,
            FunctionPrototype prototype)
        {
            prototype.alias = alias;
            AddFunction(key, prototype, fake_type);
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

        void AddFunction(FunctionKey func_key, FunctionPrototype sig,
            FakeType fake_type);

    private:
        static Functions *wrapped_funcs;
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
            std::string func_name, FakeType fake_type = FakeType::WRAPPED) :
                WrapperBase(alias, FuncKey(func_ptr), fake_type,
                    PrototypeExtractor<FuncType>::Extract(func_name, fq))
        {
        }

        template<typename Class>
        Wrapper(internal::type_identity<Class>, std::string alias,
            FuncType func_ptr, uint32_t fq, std::string func_name,
            FakeType fake_type = FakeType::WRAPPED) :
                WrapperBase(alias, FuncKey(func_ptr), fake_type,
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
#ifdef USE_PMF_ADDR
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
#pragma GCC diagnostic ignored "-Wpedantic"
            return std::make_pair(reinterpret_cast<void *>(func_ptr),
                std::type_index(typeid(FuncType)));
#pragma GCC diagnostic pop
#else
            return { HashFunctionPtr(func_ptr),
                std::type_index(typeid(FuncType)) };
#endif
        }
};

// constexpr string concat
// https://stackoverflow.com/a/62823211/3936307
template <std::string_view const&... Strs>
struct CStringJoin
{
    static constexpr auto impl() noexcept
    {
        constexpr std::size_t len = (Strs.size() + ... + 0);
        std::array<char, len + 1> arr{};
        auto append = [i = 0UL, &arr](auto const& s) mutable {
            for (unsigned j = 0; j < s.size(); j++) arr[i++] = s[j];
        };
        (append(Strs), ...);
        arr[len] = 0;
        return arr;
    }
    static constexpr auto arr = impl();
    static constexpr std::string_view value {arr.data(), arr.size() - 1};
};

// Helper to get the value out
template <std::string_view const&... Strs>
static constexpr auto CStringJoin_v = CStringJoin<Strs...>::value;

#ifdef __clang__
#define PFK_CTYPE_PREFIX "[T = "sv
#else
#define PFK_CTYPE_PREFIX "[with T = "sv
#endif
#define PFK_TAG_PREFIX      "PFK"
#define PFK_TAG_START       "Start: "
#define PFK_TAG_END         "End"
#define PFK_TYPEHINT_PREFIX PFK_TAG_PREFIX      "TypeHint"
#define PFK_PROTO_PREFIX    PFK_TAG_PREFIX      "Prototype"

#define PFK_PROTO_START     PFK_PROTO_PREFIX    PFK_TAG_START
#define PFK_PROTO_END       PFK_PROTO_PREFIX    PFK_TAG_END
#define PFK_TYPEHINT_START  PFK_TYPEHINT_PREFIX PFK_TAG_START
#define PFK_TYPEHINT_END    PFK_TYPEHINT_PREFIX PFK_TAG_END
constexpr static std::string_view STR_SEPARATOR = " | ";
constexpr static std::string_view PROTO_END = " | " PFK_PROTO_END;

// borrowed (& modified) from
// https://en.cppreference.com/w/cpp/utility/source_location/source_location
template <typename T>
constexpr auto TypeName()
{
    using std::operator""sv;
    constexpr std::string_view func_name { __PRETTY_FUNCTION__ };
    constexpr auto prefix { PFK_CTYPE_PREFIX };
    constexpr auto type_begin { func_name.find(prefix) };
    static_assert(type_begin != std::string_view::npos, "Cannot determine type name!");
    static_assert(func_name.back() == ']', "Cannot determine type name!");
    const std::size_t first { type_begin + prefix.length() };
    return std::string_view { func_name.cbegin() + first, func_name.cend()
                                      - func_name.cbegin() - first - 1 };
}

// Helper types to extract signature from a functor
template <typename Sig, typename Class, typename = void>
struct member_type_helper { using type = void; };

template <typename Sig, typename Class>
struct member_type_helper<Sig, Class, std::enable_if_t<std::is_class_v<Class>>>
{
    using type = Sig Class::*;
};

template <typename T>
struct std_func_signature;

template <typename R, typename... Args>
struct std_func_signature<std::function<R(Args...)>> {
    using type = R(Args...);
};

template <typename R, typename Class, typename... Args>
struct std_func_signature<std::function<R(Class *, Args...)>> {
    using type = R(Args...);
    using cv_qualified_type = std::conditional_t<std::is_const_v<Class>,
            std::conditional_t<std::is_volatile_v<Class>, R(Args...) const volatile, R(Args...) const>,
            std::conditional_t<std::is_volatile_v<Class>, R(Args...) volatile, type>>;
    using member_ptr = typename member_type_helper<cv_qualified_type, Class>::type;
};

template <typename FT>
struct functor_helper<FT, std::enable_if_t<bool(sizeof(decltype(std::function{std::declval<FT>()})))>>
{
    using std_func_sig_t = std_func_signature<decltype(std::function{std::declval<FT>()})>;
    using type = typename std_func_sig_t::type*;
};

template <typename FT, typename>
struct class_functor_helper: functor_helper<FT>
{
    using member_ptr = typename functor_helper<FT>::std_func_sig_t::member_ptr;
};

} // namespace internal


// -----------------------------------------------------------------------------
// Helper macors
// -----------------------------------------------------------------------------
#define ALIAS_MARKER        __pfkalias__
#define TMP_POSTFIX         __end__
#define TMP_WRAPPER_PREFIX  __wrap_function_
#define TMP_REAL_PREFIX     __real_function_
#define PFK_BUILD_NAME_HELPER(A,B,C) A##B##C
#define PFK_BUILD_NAME(A,B,C) PFK_BUILD_NAME_HELPER(A,B,C)
#define TMP_REAL_NAME(base)  PFK_BUILD_NAME(TMP_REAL_PREFIX, base, TMP_POSTFIX)
#define TMP_WRAPPER_NAME(base)  PFK_BUILD_NAME(TMP_WRAPPER_PREFIX, base, TMP_POSTFIX)
// select macro based on the number of args
#define PFK_SELECT_9TH(_1, _2, _3, _4, _5, _6, _7, _8, NAME,...) NAME
#define PRIVATE_TAG(ALIAS) ALIAS##PowerFakePrivateTag
#define PRIVMEMBER_ADDR(ALIAS) GetAddress(PRIVATE_TAG(ALIAS)())
#define PFK_GET_FIRST_ARG(FIRST_ARG, ...) FIRST_ARG

/// If you use WRAP_FUNCTION() macros in more than a single file, you should
/// define a different namespace for each file, otherwise 'multiple definition'
/// errors might happen
#ifndef POWRFAKE_WRAP_NAMESPACE
#define POWRFAKE_WRAP_NAMESPACE PowerFakeWrap
#endif

#define PFK_TYPE_HINT(CTNAME, DEMANGLED_NAME) \
    auto PFK_BUILD_NAME(POWRFAKE_WRAP_NAMESPACE, __pfktypehint__, __LINE__) = \
        PFK_TYPEHINT_START CTNAME " | " DEMANGLED_NAME " | " PFK_TYPEHINT_END

// -----------------------------------------------------------------------------
// Main implementation for wrapping/hiding functions
// -----------------------------------------------------------------------------
#define CREATE_WRAPPER_FUNCTION(FTYPE, FNAME, ALIAS, FAKE_TYPE) \
    /* Fake functions which will be called rather than the real function.
     * They call the function object in the alias Wrapper object
     * if available, otherwise it'll call the real function.
     *
     * For hidden functions, the real function is not accessible,
     * so we throw an exception if no fakes are available to be called */  \
    template <typename T> struct wrapper_##ALIAS; \
    template <typename T, typename R , typename ...Args> \
    struct wrapper_##ALIAS<R (T::*)(Args...)> \
    { \
        static R TMP_WRAPPER_NAME(ALIAS)(T *o, Args... args) \
        { \
            if (ALIAS.Callable()) \
                return ALIAS.Call(o, args...); \
            if constexpr (PowerFake::internal::FakeType::FAKE_TYPE \
                == PowerFake::internal::FakeType::WRAPPED) \
            { \
                R TMP_REAL_NAME(ALIAS)(T *o, Args... args); \
                return TMP_REAL_NAME(ALIAS)(o, args...); \
            } \
            else \
                throw std::logic_error("PowerFake: No implementation provided " \
                    "for hidden function: " #FNAME); \
        } \
    }; \
    template <typename R , typename ...Args> \
    struct wrapper_##ALIAS<R (*)(Args...)> \
    { \
        static R TMP_WRAPPER_NAME(ALIAS)(Args... args) \
        { \
            if (ALIAS.Callable()) \
                return ALIAS.Call(args...); \
            if constexpr (PowerFake::internal::FakeType::FAKE_TYPE \
                == PowerFake::internal::FakeType::WRAPPED) \
            { \
                R TMP_REAL_NAME(ALIAS)(Args... args); \
                return TMP_REAL_NAME(ALIAS)(args...); \
            } \
            else \
                throw std::logic_error("PowerFake: No implementation provided " \
                    "for hidden function: " #FNAME); \
        } \
    }; \
    template <typename R , typename ...Args> \
    struct wrapper_##ALIAS<R (*)(Args..., ...)> \
    { \
        static R TMP_WRAPPER_NAME(ALIAS)(Args... args, ...) \
        { \
            if (ALIAS.Callable()) \
            { \
                va_list vargs; \
                va_start(vargs, (args, ...)); \
                if constexpr (std::is_same_v<R, void>) \
                { \
                    ALIAS.Call(args..., vargs); \
                    va_end(vargs); \
                    return; \
                } \
                else \
                { \
                    R result = ALIAS.Call(args..., vargs); \
                    va_end(vargs); \
                    return result; \
                } \
            } \
            if constexpr (PowerFake::internal::FakeType::FAKE_TYPE \
                == PowerFake::internal::FakeType::WRAPPED) \
            { \
                R TMP_REAL_NAME(ALIAS)(Args... args); \
                return TMP_REAL_NAME(ALIAS)(args...); \
            } \
            else \
                throw std::logic_error("PowerFake: No implementation provided " \
                    "for hidden function: " #FNAME); \
        } \
    }; \
    auto PROTO_INSTANTIATE_##ALIAS = SPROTO_##ALIAS; \
    /* Explicitly instantiate the wrapper_##ALIAS struct, so that the appropriate
     * wrapper function and real function symbol is actually generated by the
     * compiler. These symbols will be renamed to the name expected by 'ld'
     * linker by bind_fakes binary. */ \
    template class wrapper_##ALIAS<PowerFake::internal::remove_func_cv_t<FTYPE>>

/*
 * Define an instance of PowerFake::internal::Wrapper<> class for the given
 * function
 */
#define DEFINE_WRAPPER_OBJECT(...) \
        PFK_SELECT_9TH(__VA_ARGS__,,, DEFINE_WRAPPER_OBJECT_2, \
            DEFINE_WRAPPER_OBJECT_1)(__VA_ARGS__)

#define DEFINE_WRAPPER_OBJECT_1(FTYPE, FNAME, FADDR, ALIAS, FAKE_TYPE) \
    STATIC_PROTOTYPE_HDR(FTYPE, FNAME, ALIAS, FAKE_TYPE); \
    constexpr static auto SPROTO_##ALIAS = PowerFake::internal::CStringJoin_v< \
            PREFIX_##ALIAS, FUNCTYPE_##ALIAS, PowerFake::internal::PROTO_END>; \
    static PowerFake::internal::Wrapper<PowerFake::internal::remove_func_cv_t<FTYPE>> \
        ALIAS(#ALIAS, PowerFake::internal::unify_pmf<FTYPE>(FADDR), \
            PowerFake::internal::func_qual_v<FTYPE>, #FNAME, \
            PowerFake::internal::FakeType::FAKE_TYPE);

#define DEFINE_WRAPPER_OBJECT_2(FTYPE, FNAME, FADDR, ALIAS, FAKE_TYPE, FCLASS) \
    STATIC_PROTOTYPE_HDR(FTYPE, FNAME, ALIAS, FAKE_TYPE); \
    constexpr static auto CTYPE_##ALIAS = PowerFake::internal::TypeName<FCLASS>(); \
    constexpr static auto SPROTO_##ALIAS = PowerFake::internal::CStringJoin_v< \
            PREFIX_##ALIAS, FUNCTYPE_##ALIAS, PowerFake::internal::STR_SEPARATOR, \
            CTYPE_##ALIAS, PowerFake::internal::PROTO_END>; \
    static PowerFake::internal::Wrapper<PowerFake::internal::remove_func_cv_t<FTYPE>> \
        ALIAS(PowerFake::internal::type_identity<FCLASS>(), \
                #ALIAS, PowerFake::internal::unify_pmf<FTYPE>(FADDR), \
                PowerFake::internal::func_qual_v<FTYPE>, #FNAME, \
                PowerFake::internal::FakeType::FAKE_TYPE);

#ifndef BIND_FAKES

/**
 * Define wrapper for function FNAME with type FSIG and alias ALIAS. Must be
 * used only once for each function in a cpp file. For static members, the class
 * must be specified as the last argument
 */
#define PFK_WRAP_FUNCTION_BASE(FTYPE, FNAME, FADDR, ALIAS, FAKE_TYPE, ...) \
    DEFINE_WRAPPER_OBJECT(FTYPE, FNAME, FADDR, ALIAS, FAKE_TYPE \
        __VA_OPT__(, PFK_GET_FIRST_ARG(__VA_ARGS__))) \
    CREATE_WRAPPER_FUNCTION(FTYPE, FNAME, ALIAS, FAKE_TYPE)

#else // BIND_FAKES

/*
 * This allows re-compiling the file containing WRAP_FUNCTION/HIDE_FUNCTION
 * macros with BIND_FAKES defined specifically to be linked against bind_fakes
 * without extra dependencies
 *
 * It is not used in the current CMake integration, but might become handy for
 * some users or might be used in future by default
 */

#define PFK_WRAP_FUNCTION_BASE(FTYPE, FNAME, FADDR, ALIAS, FAKE_TYPE, ...) \
    DEFINE_WRAPPER_OBJECT(FTYPE, FNAME, nullptr, ALIAS, FAKE_TYPE \
        __VA_OPT__(, PFK_GET_FIRST_ARG(__VA_ARGS__)))

#endif

#define STATIC_PROTOTYPE_HDR(FTYPE, FNAME, ALIAS, FAKE_TYPE) \
    constexpr static auto FUNCTYPE_##ALIAS = PowerFake::internal::TypeName<FTYPE>(); \
    constexpr static std::string_view PREFIX_##ALIAS = PFK_PROTO_START \
        #FAKE_TYPE " | " #FNAME " | " #ALIAS " | "

// -----------------------------------------------------------------------------
// Helper macros for wrapping private members
// -----------------------------------------------------------------------------
#define PFK_WRAP_PRIVATE_BASE(FSIG, FNAME, ALIAS, FAKE_TYPE, ...) \
    PFK_WRAP_FUNCTION_BASE(decltype( \
        PowerFake::internal::FuncType<FSIG>(PRIVMEMBER_ADDR(ALIAS))), FNAME, \
        PRIVMEMBER_ADDR(ALIAS), ALIAS, FAKE_TYPE __VA_OPT__(,) __VA_ARGS__)

/**
 * Wrap a private member function with alias ALIAS.
 */
#define PFK_WRAP_PRIVATE_MEMBER_IMPL(FSIG, FNAME, ALIAS, FAKE_TYPE, ...) \
    TAG_PRIVATE(PRIVATE_TAG(ALIAS), FNAME); \
    PFK_WRAP_PRIVATE_BASE(FSIG, FNAME, ALIAS, FAKE_TYPE __VA_OPT__(,) __VA_ARGS__)

#define PFK_WRAP_OVERLOADED_PRIVATE_MEMBER_IMPL(FSIG, FNAME, ALIAS, FAKE_TYPE, ...) \
    TAG_PRIVATE(PRIVATE_TAG(ALIAS), FSIG, FNAME); \
    PFK_WRAP_PRIVATE_BASE(FSIG, FNAME, ALIAS, FAKE_TYPE __VA_OPT__(,) __VA_ARGS__)

/**
 * Define a wrapper for private member function with type FSIG and name FNAME
 */
#define PFK_WRAP_OVERLOADED_PRIVATE_MEMBER(FAKE_TYPE, FSIG, FNAME, ...) \
    PFK_WRAP_OVERLOADED_PRIVATE_MEMBER_IMPL(FSIG, FNAME, \
        PFK_BUILD_NAME(POWRFAKE_WRAP_NAMESPACE, ALIAS_MARKER, __LINE__), FAKE_TYPE \
        __VA_OPT__(,) __VA_ARGS__)

/**
 * Define a wrapper for private member function with name FNAME
 */
#define PFK_WRAP_PRIVATE_MEMBER(FAKE_TYPE, FNAME, ...) \
    PFK_WRAP_PRIVATE_MEMBER_HELPER(FNAME, \
        PFK_BUILD_NAME(POWRFAKE_WRAP_NAMESPACE, ALIAS_MARKER, __LINE__), FAKE_TYPE \
        __VA_OPT__(,) __VA_ARGS__)

#define PFK_WRAP_PRIVATE_MEMBER_HELPER(FNAME, ALIAS, FAKE_TYPE, ...) \
    PFK_WRAP_PRIVATE_MEMBER_IMPL(decltype(PRIVMEMBER_ADDR(ALIAS)), FNAME, \
        ALIAS, FAKE_TYPE __VA_OPT__(,) __VA_ARGS__)

// -----------------------------------------------------------------------------
// Macros called by main WRAP_FUNCTION/HIDE_FUNCTION macros
// -----------------------------------------------------------------------------
/**
 * Define a wrapper for function named FNAME.
 */
#define PFK_WRAP_FUNCTION(FAKE_TYPE, FNAME, ...) \
    PFK_WRAP_OVERLOADED_FUNCTION(FAKE_TYPE, decltype(&FNAME), FNAME \
        __VA_OPT__(,) __VA_ARGS__)

/**
 * Define a wrapper for function with type FSIG and name FNAME.
 */
#define PFK_WRAP_OVERLOADED_FUNCTION(FAKE_TYPE, FSIG, FNAME, ...) \
    PFK_WRAP_FUNCTION_BASE(decltype(PowerFake::internal::FuncType<FSIG>(&FNAME)), \
        FNAME, &FNAME, PFK_BUILD_NAME(POWRFAKE_WRAP_NAMESPACE, ALIAS_MARKER, __LINE__), \
        FAKE_TYPE __VA_OPT__(,) __VA_ARGS__)

/**
 * Define a wrapper for static member function of class FCLASS with type
 * FSIG and name FNAME.
 */
#define PFK_WRAP_OVERLOADED_STATIC_MEMBER(FAKE_TYPE, FSIG, FCLASS, FNAME, _1) \
    PFK_WRAP_OVERLOADED_FUNCTION(FAKE_TYPE, FSIG, FNAME, FCLASS)

/**
 * Define a wrapper for static member function FNAME of class FCLASS.
 */
#define PFK_WRAP_STATIC_MEMBER(FAKE_TYPE, FCLASS, FNAME, _1) \
    PFK_WRAP_FUNCTION(FAKE_TYPE, FNAME, FCLASS)

/**
 * Define a wrapper for private member function with type FSIG and name FNAME
 */
#define CALL_WRAP_OVERLOADED_PRIVATE_MEMBER(FAKE_TYPE, FSIG, FNAME, ...) \
    PFK_WRAP_OVERLOADED_PRIVATE_MEMBER(FAKE_TYPE, FSIG, FNAME)

/**
 * Define a wrapper for private member function with name FNAME
 */
#define CALL_WRAP_PRIVATE_MEMBER(FAKE_TYPE, FNAME, ...) \
    PFK_WRAP_PRIVATE_MEMBER(FAKE_TYPE, FNAME)

/**
 * Define a wrapper for static private member function of class FCLASS with
 * type FSIG and name FNAME
 */
#define PFK_WRAP_OVERLOADED_STATIC_PRIVATE_MEMBER(FAKE_TYPE, FCLASS, FSIG, FNAME, ...) \
    PFK_WRAP_OVERLOADED_PRIVATE_MEMBER(FAKE_TYPE, FSIG, FNAME, FCLASS)

/**
 * Define a wrapper for static private member function of class FCLASS with name
 * FNAME
 */
#define PFK_WRAP_STATIC_PRIVATE_MEMBER(FAKE_TYPE, FCLASS, FNAME, ...) \
    PFK_WRAP_PRIVATE_MEMBER(FAKE_TYPE, FNAME, FCLASS)

// -----------------------------------------------------------------------------
// Macros called by TAG_PRIVATE()
// -----------------------------------------------------------------------------
/**
 * Tag a private class member
 */
#define PFK_TAG_PRIVATE_MEMBER(TAG, PRIVATE_MEMBER) \
    struct TAG: public PowerFake::internal::TagBase<TAG> { \
        static constexpr const char *member_name = #PRIVATE_MEMBER; \
    }; \
    template struct PowerFake::internal::PrivateFunctionExtractor<TAG, \
                                                            &PRIVATE_MEMBER>

 /**
  * Tag a private class overloaded function
  */
#define PFK_TAG_OVERLOADED_PRIVATE(TAG, FSIG, MEMBER_FUNCTION) \
    struct TAG: public PowerFake::internal::TagBase<TAG> { \
        static constexpr const char *member_name = #MEMBER_FUNCTION; \
    }; \
    template struct PowerFake::internal::PrivateFunctionExtractor<TAG, \
        static_cast<decltype( \
            PowerFake::internal::FuncType<FSIG>(&MEMBER_FUNCTION))>( \
                    &MEMBER_FUNCTION)>

// -----------------------------------------------------------------------------
// MakeFake implementation
// -----------------------------------------------------------------------------
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

template <typename Functor>
static FakePtr MakeFake(
    typename internal::functor_helper<Functor>::type func_ptr, Functor f)
{
    typedef internal::remove_func_cv_t<decltype(func_ptr)> FuncType;
    return std::make_unique<internal::Fake<FuncType>>(
        internal::Wrapper<FuncType>::WrapperObject(func_ptr), f);
}

template<typename Functor>
static FakePtr MakeFake(
    typename internal::class_functor_helper<Functor>::member_ptr func_ptr,
    Functor f)
{
    typedef internal::remove_func_cv_t<decltype(func_ptr)> FuncType;
    return std::make_unique<internal::Fake<FuncType>>(
        internal::Wrapper<FuncType>::WrapperObject(internal::unify_pmf(func_ptr)), f);
}

// -----------------------------------------------------------------------------
// PrototypeExtractor implementation
// -----------------------------------------------------------------------------
namespace internal
{

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

template<typename T, typename R, typename ...Args>
FunctionPrototype PrototypeExtractor<R (T::*)(Args..., ...)>::Extract(
    const std::string &func_name, uint32_t fq)
{
    return PrototypeExtractor<R (T::*)(Args...)>::Extract(func_name, fq);
}

template<typename R, typename ...Args>
FunctionPrototype PrototypeExtractor<R (*)(Args...)>::Extract(
    const std::string &func_name, uint32_t fq)
{
    const std::string ret_type = boost::core::demangle(typeid(R).name());
    const std::string ptr_type = boost::core::demangle(
        typeid(FuncPtrType).name());

    // ptr_type example: char* (*)(int const*, int, int)
    unsigned num_pr = 1;
    std::string::size_type i;
    for (i = ptr_type.size() - 2; i > 0; --i)
    {
        if (ptr_type[i] == ')')
            ++num_pr;
        else if (ptr_type[i] == '(')
            --num_pr;
        if (num_pr == 0) break;
    }
    std::string params = ptr_type.substr(i);
    return FunctionPrototype(ret_type, func_name, params, fq);
}

template<typename R, typename ...Args>
template<typename Class>
FunctionPrototype PrototypeExtractor<R (*)(Args...)>::Extract(
    const std::string &func_name, uint32_t fq)
{
    return PrototypeExtractor<R (Class::*)(Args...)>::Extract(func_name, fq);
}

template<typename R, typename ...Args>
FunctionPrototype PrototypeExtractor<R (*)(Args..., ...)>::Extract(
    const std::string &func_name, uint32_t fq)
{
    return PrototypeExtractor<R (*)(Args...)>::Extract(func_name, fq);
}

template<typename R, typename ...Args>
template<typename Class>
FunctionPrototype PrototypeExtractor<R (*)(Args..., ...)>::Extract(
    const std::string &func_name, uint32_t fq)
{
    return PrototypeExtractor<R (Class::*)(Args...)>::Extract(func_name, fq);
}

template <typename T>
size_t HashFunctionPtr(T *func_ptr)
{
    return std::hash<T*>{}(func_ptr);
}

template <typename Signature, typename Class>
size_t HashFunctionPtr(Signature Class::*func_ptr)
{
    char buf[sizeof(func_ptr)];
    std::memcpy(&buf, &func_ptr, sizeof(func_ptr));
    return std::hash<std::string_view>{}(std::string_view(buf, sizeof buf));
}

} // namespace internal

}  // namespace PowerFake

#endif /* POWERFAKE_H_ */
