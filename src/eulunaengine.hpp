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

    void registerManagedClass(const std::string& className, const std::string& baseClass = std::string()) {
        // creates the class table (that it's also the class methods table)
        getOrCreateGlobalTable(className);
        const int klass = getTop();

        // creates the class metatable
        getOrCreateRegistryTable(className + "_mt");
        int klass_mt = getTop();

        // save reference for the methods table in the metamethods table for faster access
        pushValue(klass);
        rawSeti(1, klass_mt);

        // get event
        pushCppFunction([](EulunaInterface *lua) {
            // stack: obj, key
            EulunaObject* obj = lua->toObject(-2);
            std::string key = lua->toString(-1);
            assert(obj);

            // if the field for this key exists, returns it
            obj->lua_rawGet(lua);
            if(!lua->isNil()) {
                lua->remove(-2); // removes the obj
                // field value is on the stack
                return 1;
            }
            lua->pop(); // pops the nil field

            // pushes the method assigned by this key
            lua->getMetatable();  // pushes obj metatable
            lua->rawGeti(1); // push obj methods table
            lua->getField(key); // pushes obj method
            lua->insert(-4); // move value to bottom
            lua->pop(3); // pop obj methods, obj metatable, obj

            // the result value is on the stack
            return 1;
        });
        setField("__index", klass_mt);

        // set event
        pushCppFunction([](EulunaInterface *lua) {
            // stack: obj, key, value
            EulunaObject *obj = lua->toObject(-3);
            assert(obj);
            lua->remove(-3);
            obj->lua_rawSet(lua);
            return 0;
        });
        setField("__newindex", klass_mt);

        // equal event
        pushCppFunction([](EulunaInterface *lua) {
            // stack: obj1, obj2
            void *objA = lua->toUserdata(-2);
            void *objB = lua->toUserdata(-1);
            bool ret = (objA == objB);
            lua->pop(2);
            lua->pushBoolean(ret);
            return 1;
        });
        setField("__eq", klass_mt);

        // collect event
        pushCppFunction([](EulunaInterface *lua) {
            // gets object pointer
            EulunaObject* obj = lua->toObject(-1);
            lua->pop(1);
            assert(obj);
            // resets pointer to decrease object use count
            obj->lua_unref(lua);
            return 0;
        });
        setField("__gc", klass_mt);

        // redirect methods to the base class ones
        if(!className.empty()) {
          // the following code is what create classes hierarchy for lua, by reproducing:
          // DerivedClass = { __index = BaseClass }

          // redirect the class methods to the base methods
          pushValue(klass);
          newTable();
          getOrCreateGlobalTable(baseClass);
          setField("__index");
          setMetatable();
          pop();
        }

        // pops klass, klass_mt
        pop(2);
    }

    void registerGlobalFunction(const std::string& functionName, EulunaCppFunction* function) {
        pushCppFunction(function);
        setGlobal(functionName);
    }

    void registerClassFunction(const std::string& className, const std::string& functionName, EulunaCppFunction* function) {
        getGlobal(className);
        assert(isTable());
        pushCppFunction(function);
        setField(functionName);
        pop();
    }

    // aliases
    template<typename R>
    R runBuffer(const std::string& buffer, const std::string& source = "") { return polymorphicSafeDoBuffer<R>(buffer, source); }

private:
    std::unordered_map<void*,EulunaObject> m_objects;
};

#endif // EULUNAENGINE_HPP
