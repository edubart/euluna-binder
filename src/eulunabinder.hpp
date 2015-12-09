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

#ifndef EULUNABINDER_HPP
#define EULUNABINDER_HPP

#include "eulunaengine.hpp"
#include "eulunabinderdetail.hpp"

class EulunaBinder
{
public:
    enum KlassType {
        Klass_None,
        Klass_Singleton,
        Klass_Copyable,
        Klass_Shared
    };

private:
    struct Klass {
        Klass() { }
        Klass(const std::string& name, const std::string& base, KlassType type) :
            name(name), base(base), type(type) { }
        std::string name;
        std::string base;
        std::map<std::string,EulunaCppFunctionPtr> funcs;
        KlassType type = Klass_None;
        void *instance = nullptr;
    };

public:
    static EulunaBinder& instance() {
        static EulunaBinder instance;
        return instance;
    }

    static void registerGlobalBindings(EulunaEngine* euluna) {
        instance().registerBindings(euluna);
    }

    // Do the bindings
    void registerBindings(EulunaEngine* euluna) {
        for(auto& it : m_globalFunctions)
            euluna->registerGlobalFunction(it.first, it.second.get());
        for(auto& kit : m_klasses) {
            Klass& klass = kit.second;
            if(klass.type == Klass_Singleton) {
                euluna->registerSingletonClass(klass.name);
                for(auto& fit : klass.funcs)
                    euluna->registerClassFunction(klass.name, fit.first, fit.second.get());
            }
        }
    }

    // Registers
    void registerClass(const std::string& className,
                       const std::string& classBase,
                       KlassType classType) {
        assert(m_klasses.find(className) == m_klasses.end());
        m_klasses[className] = Klass(className,classBase,classType);
        m_klass = &m_klasses[className];
    }
    void registerClassFunction(const std::string& className,
                               const std::string& functionName,
                               EulunaCppFunction function) {
        assert(m_klass && m_klass->name == className);
        assert(m_klass->funcs.find(functionName) == m_klass->funcs.end());
        m_klass->funcs[functionName] = EulunaCppFunctionPtr(new EulunaCppFunction(std::move(function)));
    }
    void registerGlobalFunction(const std::string& functionName,
                                EulunaCppFunction function) {
         assert(m_globalFunctions.find(functionName) == m_globalFunctions.end());
         m_globalFunctions[functionName] = EulunaCppFunctionPtr(new EulunaCppFunction(std::move(function)));
     }


    // Binders
    /*
    template<class C>
    void bindSharedClass() {
        static_assert(std::is_base_of<EulunaSharedObject, C>::value, "Shared classes must be derived from EulunaSharedObject.");
        registerClass(euluna_tools::demangle_type<C>(), std::string(), Klass_Shared);
    }
    */
    void bindSingleton(const std::string& singletonName) {
        registerClass(singletonName, std::string(), Klass_Singleton);
    }

    template<typename C>
    void bindSingletonClass(const std::string& singletonName, C* instance) {
        registerClass(singletonName, std::string(), Klass_Singleton);
        m_klass->instance = instance;
    }

    template<typename F>
    void bindClassStaticFunction(const std::string& functionName, const F& function) {
        registerClassFunction(m_klass->name, functionName, euluna_binder::bind_fun(function));
    }

    template<typename F>
    void bindGlobalFunction(const std::string& functionName, const F& function) {
        registerGlobalFunction(functionName, euluna_binder::bind_fun(function));
    }

    template<class C, typename F>
    void bindSingletonMemberFunction(const std::string& functionName, F C::*function) {
        C* c = static_cast<C*>(m_klass->instance);
        assert(c);
        registerClassFunction(m_klass->name, functionName, euluna_binder::bind_singleton_mem_fun(function, c));
    }

    /*
    template<class C, typename F, class FC>
    void bindClassMemberFunction(const std::string& functionName, F FC::*function) {
        registerClassFunction(euluna_tools::demangle_type<C>(),
                              functionName,
                              euluna_binder::bind_shared_mem_fun<C>(function));
    }
    */

    // bindClassStaticFunction
    // bindClassMemberFunction
    // bindClassConstructorFunction
    // bindClassDestructorFunction
    // bindGlobalFunction
    // bindSingleton
    // bindSingletonFunction


private:
    std::map<std::string, Klass> m_klasses;
    Klass *m_klass = nullptr;
    std::map<std::string,EulunaCppFunctionPtr> m_globalFunctions;
};

class EulunaBinderFunction {
public:
    template<typename T>
    explicit EulunaBinderFunction(T&& f) { std::move(f)(); }
};

// utility macro
#define E_CONCAT_HELPER(a, b) a ## b
#define E_CONCAT(a, b) E_CONCAT_HELPER(a, b)
#define E_UNIQUE_NAME(str) E_CONCAT(base, __COUNTER__)

// bind globals
#define EULUNA_BEGIN_GLOBAL_FUNCTIONS() EulunaBinderFunction E_UNIQUE_NAME(__euluna_binding_)([] {
#define EULUNA_GLOBAL(func) EulunaBinder::instance().bindGlobalFunction(#func, func);
#define EULUNA_GLOBAL_NAMED(name,func) EulunaBinder::instance().bindGlobalFunction(name, func);

// bind singletons with no C++ class
#define EULUNA_BEGIN_SINGLETON(klass) EulunaBinderFunction E_UNIQUE_NAME(__euluna_binding_)([] { EulunaBinder::instance().bindSingleton(klass);
#define EULUNA_SINGLETON_FUNC(func) EulunaBinder::instance().bindClassStaticFunction(#func, func);
#define EULUNA_SINGLETON_FUNC_NAMED(name,func) EulunaBinder::instance().bindClassStaticFunction(name, func);

// bind singletons with C++ class
#define EULUNA_BEGIN_SINGLETON_CLASS(klass,ptr) EulunaBinderFunction __euluna_binding_##klass([] { EulunaBinder::instance().bindSingletonClass<klass>(#klass, ptr);
#define EULUNA_BEGIN_SINGLETON_CLASS_NAMED(name,klass,ptr) EulunaBinderFunction __euluna_binding_##klass([] { EulunaBinder::instance().bindSingletonClass<klass>(name, ptr);
#define EULUNA_SINGLETON_STATIC(klass,func) EulunaBinder::instance().bindClassStaticFunction(#func, &klass::func);
#define EULUNA_SINGLETON_STATIC_NAMED(name,klass,func) EulunaBinder::instance().bindClassStaticFunction(name, func);
#define EULUNA_SINGLETON_MEMBER(klass,func) EulunaBinder::instance().bindSingletonMemberFunction(#func, &klass::func);
#define EULUNA_SINGLETON_MEMBER_NAMED(name,klass,func) EulunaBinder::instance().bindSingletonMemberFunction(name, &klass::func);

/*
#define EULUNA_BEGIN_SHARED_CLASS(a) EulunaBinderFunction __euluna_binding_##a([] { EulunaBinder::instance().bindSharedClass<a>();
#define EULUNA_SHARED_STATIC(a,b) EulunaBinder::instance().bindClassStaticFunction<a>(#b, &a::b);
#define EULUNA_SHARED_MEMBER(a,b) EulunaBinder::instance().bindSharedMemberFunction<a>(#b, &a::b);

#define EULUNA_BEGIN_COPYABLE_CLASS(a) EulunaBinderFunction __euluna_binding_##a([] { EulunaBinder::instance().bindSharedClass<a>();
#define EULUNA_COPYABLE_STATIC(a,b) EulunaBinder::instance().bindClassStaticFunction<a>(#b, &a::b);
#define EULUNA_COPYABLE_MEMBER(a,b) EulunaBinder::instance().bindSharedMemberFunction<a>(#b, &a::b);
*/

#define EULUNA_END });

#endif // EULUNABINDER_HPP
