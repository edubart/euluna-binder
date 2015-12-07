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
        Klass_Copyable,
        Klass_Shared,
        Klass_Singleton
    };

private:
    struct Klass {
        Klass() { }
        Klass(const std::string& name, const std::string& base, KlassType type) :
            name(name), base(base), type(type) { }
        std::string name;
        std::string base;
        KlassType type;
        std::map<std::string,EulunaCppFunctionPtr> funcs;
    };

public:
    static EulunaBinder& instance() {
        static EulunaBinder instance;
        return instance;
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

    template<typename F>
    void bindClassStaticFunction(const std::string& functionName, const F& function) {
        registerClassFunction(m_klass->name, functionName, euluna_binder::bind_fun(function));
    }

    template<typename F>
    void bindGlobalFunction(const std::string& functionName, const F& function) {
        registerGlobalFunction(functionName, euluna_binder::bind_fun(function));
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

#define EULUNA_BEGIN_GLOBAL_FUNCTIONS() EulunaBinderFunction __euluna_binding_##__FILE__##_LINE ([] {
#define EULUNA_GLOBAL(a) EulunaBinder::instance().bindGlobalFunction(#a, a);
#define EULUNA_GLOBAL_EX(a,b) EulunaBinder::instance().bindGlobalFunction(a, b);

#define EULUNA_BEGIN_SINGLETON(a) EulunaBinderFunction __euluna_binding_##a([] { EulunaBinder::instance().bindSingleton(#a);
#define EULUNA_SINGLETON_FUNC(a) EulunaBinder::instance().bindClassStaticFunction(a, b);

#define EULUNA_BEGIN_SINGLETON_CLASS(a,b) EulunaBinderFunction __euluna_binding_##a([] { EulunaBinder::instance().bindSingletonClass(#a, b);
//#define EULUNA_BEGIN_SINGLETON_CLASS_EX(a,b,c) EulunaBinderFunction __euluna_binding_##a([] { EulunaBinder::instance().bindSingletonClass<b>(a, c);
//#define EULUNA_SINGLETON_STATIC(a,b) EulunaBinder::instance().bindClassStaticFunction(#b, &a::b);
#define EULUNA_SINGLETON_STATIC_EX(a,b) EulunaBinder::instance().bindClassStaticFunction(a, b);
//#define EULUNA_SINGLETON_MEMBER(a,b) EulunaBinder::instance().bindSingletonMemberFunction<a>(#b, &a::b);
//#define EULUNA_SINGLETON_MEMBER_EX(a,b) EulunaBinder::instance().bindSingletonMemberFunction<a>(a, b);

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
