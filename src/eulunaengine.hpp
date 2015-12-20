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

        // save class table in index 1 of the class metatable
        pushValue(klass);
        rawSeti(1);

        // get event
        pushCppFunction([](EulunaInterface* lua) {
            // stack: obj, key
            void* obj = lua->toObject<void*>(-2);
            std::string key = lua->toString(-1);
            assert(obj);

            // check for the field in the obj table
            lua->pushLightUserdata(obj); // push obj pointer
            lua->getRegistry(); // get obj table
            if(!lua->isNil()) {
                lua->insert(-2); // move key value to top
                lua->rawGet(); // get key from obj table
                if(!lua->isNil()) {
                    lua->insert(-3); // move value to bottom
                    lua->pop(2); // pop obj table, obj
                    return 1;
                }
            }
            lua->pop(2); // leave only obj in the stack

            // pushes the method assigned by this key
            lua->getMetatable();  // pushes obj metatable
            lua->rawGeti(1); // push obj methods table
            lua->getField(key); // pushes obj method
            lua->insert(-4); // move value to bottom
            lua->pop(3); // pop obj methods, obj metatable, obj

            // the result value is on the stack
            return 1;
        }, className + ":__index");
        setField("__index");

        // set event
        pushCppFunction([](EulunaInterface* lua) {
            //stack: obj, key, value
            void* obj = lua->toObject<void*>(-3);
            assert(obj);
            lua->remove(-3); // remove obj from stack
            lua->pushLightUserdata(obj); // push obj pointer
            lua->getRegistry(); // get obj table
            if(lua->isNil()) {
                lua->pop(); // pops the nil field
                lua->newTable(); // create a new obj table
                lua->pushLightUserdata(obj); // push obj pointer
                lua->pushValue(-2); // push obj table
                lua->setRegistry(); // save obj table in registry
            }
            lua->insert(-3); // move obj table to bottom
            lua->rawSet(); // set the obj table field
            lua->pop();
            return 0;
        }, className + ":__newindex");
        setField("__newindex");

        // use event
        if(useHandler) {
            pushCppFunction([&](EulunaInterface *lua) {
                void *obj = lua->toObject<void*>();
                assert(obj);
                lua->pop();
                useHandler(lua,obj);
                return 0;
            }, className + ":__use");
            setField("__use");
        }

        // release event
        if(releaseHandler) {
            pushCppFunction([&](EulunaInterface *lua) {
                void *obj = lua->toObject<void*>();
                lua->pop();
                if(obj)
                    releaseHandler(lua,obj);
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
