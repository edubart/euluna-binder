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

struct EulunaObject {
    int fieldsTableRef = -1;
    int refs = 1;
};

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
        getGlobal(className);
        if(isNil()) {
            pop();
            newTable();
            setGlobal(className);
        } else {
            assert(isTable());
            pop();
        }
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

    void useObject(void *object) {
        m_objects[object] = EulunaObject();
    }

    void releaseManagedObject(void *object) {
        auto it = m_objects.find(object);
        assert(it != m_objects.end());
        m_objects.erase(it);
    }

private:
    std::unordered_map<void*,EulunaObject> m_objects;
};

#endif // EULUNAENGINE_HPP
