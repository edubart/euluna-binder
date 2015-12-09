/*
 * Copyright (c) 2016 Euluna <https://github.com/edubart/euluna>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef EULUNABINDERDETAIL_HPP
#define EULUNABINDERDETAIL_HPP

#include "eulunainterface.hpp"

/// This namespace contains some dirty metaprogamming that uses a lot of C++11 features
/// The purpose here is to create templates that can bind any function from C++
/// and expose in lua environment. This is done combining variadic templates,
/// lambdas, tuples and some type traits features to create
/// templates that can detect functions's arguments and then generate lambdas.
/// These lambdas pops arguments from lua stack, call the bound C++ function and then
/// pushes the result to lua.
namespace  euluna_binder {


/// Pack arguments from lua stack into a tuple recursively
template<int N>
struct pack_values_into_tuple {
    template<typename Tuple>
    static void call(Tuple& tuple, EulunaInterface* lua) {
        typedef typename std::tuple_element<N-1, Tuple>::type ValueType;
        if(!euluna_caster::pull(lua, N, std::get<N-1>(tuple)))
            lua->argError(N, euluna_tools::format("%s expected, got %s", euluna_tools::demangle_type<ValueType>(), lua->toTypeName(N)));
        pack_values_into_tuple<N-1>::call(tuple, lua);
    }
};
template<>
struct pack_values_into_tuple<0> {
    template<typename Tuple>
    static void call(Tuple&, EulunaInterface*) { }
};

/// C++ function caller that push results to lua
template<typename Ret, typename F, typename... Args>
typename std::enable_if<!std::is_void<Ret>::value, int>::type
call_fun_and_push_result(const F& f, EulunaInterface* lua, const Args&... args) {
    Ret ret = f(args...);
    return euluna_caster::push(lua, ret);
}

/// C++ void function caller
template<typename Ret, typename F, typename... Args>
typename std::enable_if<std::is_void<Ret>::value, int>::type
call_fun_and_push_result(const F& f, EulunaInterface*, const Args&... args) {
    f(args...);
    return 0;
}

/// Expand arguments from tuple for later calling the C++ function
template<int N, typename Ret>
struct expand_fun_arguments {
    template<typename Tuple, typename F, typename... Args>
    static int call(const Tuple& tuple, const F& f, EulunaInterface* lua, const Args&... args) {
        return expand_fun_arguments<N-1,Ret>::call(tuple, f, lua, std::get<N-1>(tuple), args...);
    }
};
template<typename Ret>
struct expand_fun_arguments<0,Ret> {
    template<typename Tuple, typename F, typename... Args>
    static int call(const Tuple&, const F& f, EulunaInterface* lua, const Args&... args) {
        return call_fun_and_push_result<Ret>(f, lua, args...);
    }
};

/// Bind different types of functions generating a lambda
template<typename Ret, typename F, typename Tuple>
EulunaCppFunction bind_fun_specializer(const F& f) {
    enum { N = std::tuple_size<Tuple>::value };
    return [=](EulunaInterface* lua) -> int {
        lua->ensureStackSize(N);
        Tuple tuple;
        pack_values_into_tuple<N>::call(tuple, lua);
        lua->pop(N);
        return expand_fun_arguments<N,Ret>::call(tuple, f, lua);
    };
}

/// Bind a customized function
inline
EulunaCppFunction bind_fun(std::function<int(EulunaInterface*)>&& f) {
    return f;
}

/// Bind a std::function
template<typename Ret, typename... Args>
EulunaCppFunction bind_fun(const std::function<Ret(Args...)>& f) {
    typedef typename std::tuple<typename euluna_traits::remove_const_ref<Args>::type...> Tuple;
    return bind_fun_specializer<typename euluna_traits::remove_const_ref<Ret>::type,
                                decltype(f),
                                Tuple>(f);
}

/// Bind C++ functions
template<typename Ret, typename... Args>
EulunaCppFunction bind_fun(Ret (*f)(Args...)) {
    return bind_fun(std::function<Ret(Args...)>(f));
}

/// Specialization for lambdas
template<typename F>
struct bind_lambda_fun;

template<typename Lambda, typename Ret, typename... Args>
struct bind_lambda_fun<Ret(Lambda::*)(Args...) const> {
    static EulunaCppFunction call(const Lambda& f) {
        typedef typename std::tuple<typename euluna_traits::remove_const_ref<Args>::type...> Tuple;
        return bind_fun_specializer<typename euluna_traits::remove_const_ref<Ret>::type,
                                    decltype(f),
                                    Tuple>(f);

    }
};

template<typename Lambda>
typename std::enable_if<std::is_constructible<decltype(&Lambda::operator())>::value, EulunaCppFunction>::type bind_fun(const Lambda& f) {
    typedef decltype(&Lambda::operator()) F;
    return bind_lambda_fun<F>::call(f);
}

/*
/// Create member function lambdas for shared classes
template<typename Ret, typename C, typename... Args>
std::function<Ret(const euluna_shared_ptr<C>&, const Args&...)> make_shared_mem_func(Ret (C::* f)(Args...)) {
    auto mf = std::mem_fn(f);
    return [=](const euluna_shared_ptr<C>& obj, const Args&... args) mutable -> Ret {
        if(!obj)
            throw EulunaNullObjectException();
        return mf(obj.get(), args...);
    };
}
template<typename C, typename... Args>
std::function<void(const euluna_shared_ptr<C>&, const Args&...)> make_shared_mem_func(void (C::* f)(Args...)) {
    auto mf = std::mem_fn(f);
    return [=](const euluna_shared_ptr<C>& obj, const Args&... args) mutable -> void {
        if(!obj)
            throw EulunaNullObjectException();
        mf(obj.get(), args...);
    };
}
*/

/// Create member function lambdas for singleton classes
template<typename Ret, typename C, typename... Args>
std::function<Ret(const Args&...)> make_mem_func_singleton(Ret (C::* f)(Args...), C* instance) {
    auto mf = std::mem_fn(f);
    return [=](Args... args) mutable -> Ret { return mf(instance, args...); };
}
template<typename C, typename... Args>
std::function<void(const Args&...)> make_mem_func_singleton(void (C::* f)(Args...), C* instance) {
    auto mf = std::mem_fn(f);
    return [=](Args... args) mutable -> void { mf(instance, args...); };
}
template<typename Ret, typename C, typename... Args>
std::function<Ret(const Args&...)> make_mem_func_singleton(Ret (C::* f)(Args...) const, C* instance) {
    auto mf = std::mem_fn(f);
    return [=](Args... args) mutable -> Ret { return mf(instance, args...); };
}
template<typename C, typename... Args>
std::function<void(const Args&...)> make_mem_func_singleton(void (C::* f)(Args...) const, C* instance) {
    auto mf = std::mem_fn(f);
    return [=](Args... args) mutable -> void { mf(instance, args...); };
}

/*
/// Bind member functions for shared classes
template<typename C, typename Ret, class FC, typename... Args>
EulunaCppFunction bind_shared_mem_fun(Ret (FC::* f)(Args...)) {
    typedef typename std::tuple<euluna_shared_ptr<FC>, typename euluna_traits::remove_const_ref<Args>::type...> Tuple;
    auto lambda = make_shared_mem_func<Ret,FC>(f);
    return bind_fun_specializer<typename euluna_traits::remove_const_ref<Ret>::type,
                                decltype(lambda),
                                Tuple>(lambda);
}
*/

/// Bind member functions for singleton classes
template<typename C, typename Ret, class FC, typename... Args>
EulunaCppFunction bind_singleton_mem_fun(Ret (FC::*f)(Args...), C *instance) {
    typedef typename std::tuple<typename euluna_traits::remove_const_ref<Args>::type...> Tuple;
    assert(instance);
    auto lambda = make_mem_func_singleton<Ret,FC>(f, static_cast<FC*>(instance));
    return bind_fun_specializer<typename euluna_traits::remove_const_ref<Ret>::type,
                                decltype(lambda),
                                Tuple>(lambda);
}

template<typename C, typename Ret, class FC, typename... Args>
EulunaCppFunction bind_singleton_mem_fun(Ret (FC::*f)(Args...) const, C *instance) {
    typedef typename std::tuple<typename euluna_traits::remove_const_ref<Args>::type...> Tuple;
    assert(instance);
    auto lambda = make_mem_func_singleton<Ret,FC>(f, static_cast<FC*>(instance));
    return bind_fun_specializer<typename euluna_traits::remove_const_ref<Ret>::type,
                                decltype(lambda),
                                Tuple>(lambda);
}

/// Bind customized functions for singleton classes
template<typename C, class FC>
EulunaCppFunction bind_singleton_mem_fun(int (FC::*f)(EulunaInterface*), C *instance) {
    assert(instance);
    auto mf = std::mem_fn(f);
    return [=](EulunaInterface* lua) mutable -> int { return mf(instance, lua); };
}

/*
/// Bind customized member functions for shared classes
template<typename C>
EulunaCppFunction bind_shared_mem_fun(int (C::*f)(EulunaInterface*)) {
    auto mf = std::mem_fn(f);
    return [=](EulunaInterface* lua) mutable -> int {
        auto obj = lua->polymorphicPull<euluna_shared_ptr<C>>(1);
        if(!obj)
            throw EulunaBadArgumentException();
        lua->remove(1);
        return mf(obj, lua);
    };
}
*/

}

#endif // EULUNABINDERDETAIL_HPP

