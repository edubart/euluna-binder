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
    class Binder {
    public:
        virtual ~Binder() {}
        virtual void registerBindings(EulunaEngine *euluna) = 0;
    };

    class BinderGlobals : public Binder {
        std::map<std::string,EulunaCppFunctionPtr> m_functions;
    public:
        explicit BinderGlobals() {}
        template<typename F>
        BinderGlobals& def(const std::string& functionName, F function) {
            if(m_functions.find(functionName) != m_functions.end())
                throw EulunaEngineError(euluna_tools::format("Global function '%s' is already defined", functionName));
            m_functions[functionName] = EulunaCppFunctionPtr(new EulunaCppFunction(euluna_binder::bind_fun(std::forward<F>(function))));
            return *this;
        };
        virtual void registerBindings(EulunaEngine *euluna) {
            for(auto& it : m_functions)
                euluna->registerGlobalFunction(it.first, it.second.get());
        }
    };

    class BinderSingleton : public Binder {
        std::string m_name;
        std::map<std::string,EulunaCppFunctionPtr> m_functions;
    public:
        explicit BinderSingleton(const std::string& name) : m_name(name) { }
        template<typename F>
        BinderSingleton& def(const std::string& functionName, F function) {
            if(m_functions.find(functionName) != m_functions.end())
                throw EulunaEngineError(euluna_tools::format("Function '%s' for singleton '%s' is already defined", functionName, m_name));
            m_functions[functionName] = EulunaCppFunctionPtr(new EulunaCppFunction(euluna_binder::bind_fun(std::forward<F>(function))));
            return *this;
        };
        virtual void registerBindings(EulunaEngine *euluna) {
            euluna->registerSingletonClass(m_name);
            for(auto& it : m_functions)
                euluna->registerClassFunction(m_name, it.first, it.second.get());
        }
    };

    class BinderSingletonClass : public Binder {
        std::string m_name;
        void *m_instance = nullptr;
        std::map<std::string,EulunaCppFunctionPtr> m_functions;
    public:
        explicit BinderSingletonClass(const std::string& name, void* instance) : m_name(name), m_instance(instance) {
            if(!instance)
                throw EulunaEngineError(euluna_tools::format("Null singleton instance while binding singleton '%s'", m_name));
        }
        template<typename F>
        BinderSingletonClass& defStatic(const std::string& functionName, F&& function) {
            if(m_functions.find(functionName) != m_functions.end())
                throw EulunaEngineError(euluna_tools::format("Static function '%s' for singleton '%s' is already defined", functionName, m_name));
            m_functions[functionName] = EulunaCppFunctionPtr(new EulunaCppFunction(euluna_binder::bind_fun(std::forward<F>(function))));
            return *this;
        };

        template<class C, typename F>
        BinderSingletonClass& def(const std::string& functionName, F C::*function) {
            if(m_functions.find(functionName) != m_functions.end())
                throw EulunaEngineError(euluna_tools::format("Member function '%s' for singleton '%s' is already defined", functionName, m_name));
            m_functions[functionName] = EulunaCppFunctionPtr(new EulunaCppFunction(euluna_binder::bind_singleton_mem_fun(std::forward<decltype(function)>(function), static_cast<C*>(m_instance))));
            return *this;
        }
        virtual void registerBindings(EulunaEngine *euluna) {
            euluna->registerSingletonClass(m_name);
            for(auto& it : m_functions)
                euluna->registerClassFunction(m_name, it.first, it.second.get());
        }
    };

    class BinderManagedClass : public Binder {
        std::string m_name;
        std::string m_base;
        std::map<std::string,EulunaCppFunctionPtr> m_functions;
        std::function<void(EulunaInterface*,void*)> m_useHandler;
        std::function<void(EulunaInterface*,void*)> m_releaseHandler;
    public:
        explicit BinderManagedClass(const std::string& name, const std::string& base) : m_name(name), m_base(base) { }
        template<typename F>
        BinderManagedClass& defStatic(const std::string& functionName, F&& function) {
            if(m_functions.find(functionName) != m_functions.end())
                throw EulunaEngineError(euluna_tools::format("Static function '%s' for managed class '%s' is already defined", functionName, m_name));
            m_functions[functionName] = EulunaCppFunctionPtr(new EulunaCppFunction(euluna_binder::bind_fun(std::forward<F>(function))));
            return *this;
        };

        template<typename F>
        BinderManagedClass& def(const std::string& functionName, F&& function) {
            if(m_functions.find(functionName) != m_functions.end())
                throw EulunaEngineError(euluna_tools::format("Member function '%s' for managed class '%s' is already defined", functionName, m_name));
            m_functions[functionName] = EulunaCppFunctionPtr(new EulunaCppFunction(euluna_binder::bind_managed_mem_fun(std::forward<F>(function))));
            return *this;
        }

        template<class C>
        BinderManagedClass& useHandler(void (*function)(EulunaInterface*, C*)) {
            m_useHandler = [=](EulunaInterface* lua, void *instance) {
                function(lua, static_cast<C*>(instance));
            };
            return *this;
        };
        template<class C>
        BinderManagedClass& releaseHandler(void (*function)(EulunaInterface*, C*)) {
            m_releaseHandler = [=](EulunaInterface* lua, void *instance) {
                function(lua, static_cast<C*>(instance));
            };
            return *this;
        };
        virtual void registerBindings(EulunaEngine *euluna) {
            euluna->registerManagedClass(m_name, m_base, m_useHandler, m_releaseHandler);
            for(auto& it : m_functions)
                euluna->registerClassFunction(m_name, it.first, it.second.get());
        }
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
        for(auto& binder : m_binders)
            binder->registerBindings(euluna);
    }

    // Binders
    EulunaBinder::BinderGlobals& globals() {
        auto ret = new BinderGlobals();
        m_binders.push_back(std::unique_ptr<Binder>(static_cast<Binder*>(ret)));
        return *ret;
    }
    EulunaBinder::BinderSingleton& singleton(const std::string& name) {
        auto ret = new BinderSingleton(name);
        m_binders.push_back(std::unique_ptr<Binder>(static_cast<Binder*>(ret)));
        return *ret;
    }
    template<class C>
    EulunaBinder::BinderSingletonClass& singletonClass(const std::string& name, C *instance) {
        auto ret = new BinderSingletonClass(name, static_cast<C*>(instance));
        m_binders.push_back(std::unique_ptr<Binder>(static_cast<Binder*>(ret)));
        return *ret;
    }

    template<class C>
    EulunaBinder::BinderManagedClass& managedClass(const std::string& name, const std::string& base = std::string()) {
        auto ret = new BinderManagedClass(name, base);
        m_binders.push_back(std::unique_ptr<Binder>(static_cast<Binder*>(ret)));
        return *ret;
    }

private:
    std::vector<std::unique_ptr<Binder>> m_binders;
};

class EulunaAutoBinder {
public:
    template<typename T>
    explicit EulunaAutoBinder(T&& f) { std::move(f)(); }
};

// utility macro
#define EULUNA_AUTO_BIND(name) void __euluna_bind_##name(); \
                          EulunaAutoBinder __euluna_bindings_##name([] { __euluna_bind_##name(); }); \
                          void __euluna_bind_##name()
// bind functions
#define EULUNA_FUNC(func) .def(#func, func)
#define EULUNA_FUNC_NAMED(name,func) .def(name, func)
#define EULUNA_CLASS_STATIC(klass,func) .defStatic(#func, &klass::func)
#define EULUNA_CLASS_STATIC_NAMED(name,klass,func) .defStatic(name, &klass::func)
#define EULUNA_CLASS_STATIC_NAMED_EX(name,func) .defStatic(name, func)
#define EULUNA_CLASS_MEMBER(klass,func) .def(#func, &klass::func)
#define EULUNA_CLASS_MEMBER_NAMED(name,klass,func) .def(name, &klass::func)
#define EULUNA_CLASS_MEMBER_NAMED_EX(name,func) .def(name, func)

// bind globals
#define EULUNA_BEGIN_GLOBAL_FUNCTIONS(name) EulunaAutoBinder __euluna_bindings_##name([] { EulunaBinder::instance().globals()

// bind singletons with no C++ class
#define EULUNA_BEGIN_SINGLETON(klass) EulunaAutoBinder __euluna_binding_##klass([] { EulunaBinder::instance().singleton(#klass)
#define EULUNA_BEGIN_SINGLETON_NAMED(name,klass) EulunaAutoBinder __euluna_binding_##klass([] { EulunaBinder::instance().singleton(name)

// bind singletons with C++ class
#define EULUNA_BEGIN_SINGLETON_CLASS(klass,ptr) EulunaAutoBinder __euluna_binding_##klass([] { EulunaBinder::instance().singletonClass(#klass, ptr)
#define EULUNA_BEGIN_SINGLETON_CLASS_NAMED(name,klass,ptr) EulunaAutoBinder __euluna_binding_##klass([] { EulunaBinder::instance().singletonClass(name, ptr)

// bind managed C++ classes
#define EULUNA_BEGIN_MANAGED_CLASS(klass) EulunaAutoBinder __euluna_binding_##klass([] { EulunaBinder::instance().managedClass<klass>(#klass)
#define EULUNA_BEGIN_MANAGED_CLASS_NAMED(name,klass) EulunaAutoBinder __euluna_binding_##klass([] { EulunaBinder::instance().managedClass<klass>(name)
#define EULUNA_BEGIN_MANAGED_DERIVED_CLASS(klass,base) EulunaAutoBinder __euluna_binding_##klass([] { EulunaBinder::instance().managedClass<klass>(#klass,base)
#define EULUNA_BEGIN_MANAGED_DERIVED_CLASS_NAMED(name,klass,base) EulunaAutoBinder __euluna_binding_##klass([] { EulunaBinder::instance().managedClass<klass>(name,base)
#define EULUNA_CLASS_REFERENCE_HANDLERS(use,release) .useHandler(use).releaseHandler(release)

// bind ending
#define EULUNA_END() ;});

#endif // EULUNABINDER_HPP
