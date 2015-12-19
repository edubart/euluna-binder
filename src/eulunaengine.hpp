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

#ifndef EULUNAENGINE_HPP
#define EULUNAENGINE_HPP

#include "eulunainterface.hpp"

// Euluna engine
class EulunaEngine : public EulunaInterface {
public:
    EulunaEngine() : EulunaInterface() { }
    explicit EulunaEngine(lua_State *L) : EulunaInterface(L)  { }

    static EulunaEngine& instance() {
        static EulunaEngine instance;
        return instance;
    }

    void registerSingletonClass(const std::string& className) {
        getOrCreateGlobalTable(className);
        pop(1);
    }

    void registerManagedClass(const std::string& className,
                              const std::string& baseClass,
                              std::function<void(EulunaInterface*,void*)>& useHandler,
                              std::function<void(EulunaInterface*,void*)>& releaseHandler) {
        // creates the class table (that it's also the class methods table)
        getOrCreateGlobalTable(className);
        int klass = getTop();

        // creates the class metatable
        getOrCreateRegistryTable(className + "_mt");

        // index methamethod
        pushValue(klass);
        setField("__index");

        // use event
        if(useHandler) {
            pushCppFunction([&](EulunaInterface *lua) {
                void *obj = lua->toObject<void*>();
                assert(obj);
                useHandler(lua,obj);
                lua->pop();
                return 0;
            }, className + ":__use");
            setField("__use");
        }
 
        // release event
        if(releaseHandler) {
            pushCppFunction([&](EulunaInterface *lua) {
                void *obj = lua->toObject<void*>();
                std::cout << "gc" << obj << std::endl;
                if(obj)
                    releaseHandler(lua,obj);
                lua->pop();
                return 0;
            }, className + ":__gc");
            setField("__gc");
        }

        // redirect methods to the base class ones
        if(!baseClass.empty()) {
            pushValue(klass);
            newTable();
            getOrCreateGlobalTable(baseClass);
            setField("__index");
            setMetatable();
            pop();
        }

        pop(2);
        assert(stackSize() == 0);
    }

    void registerClassFunction(const std::string& className, const std::string& functionName, EulunaCppFunction* function) {
        getGlobal(className);
        assert(isTable());
        pushCppFunction(function, className + ":" + functionName);
        setField(functionName);
        pop();
    }

    void registerGlobalFunction(const std::string& functionName, EulunaCppFunction* function) {
        pushCppFunction(function, functionName);
        setGlobal(functionName);
    }

    // aliases
    template<typename R>
    R runBuffer(const std::string& buffer, const std::string& source = "") { return polymorphicSafeDoBuffer<R>(buffer, source); }
};

#endif // EULUNAENGINE_HPP
