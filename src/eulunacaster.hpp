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

#ifndef EULUNACASTER_HPP
#define EULUNACASTER_HPP

#include "eulunainterface.hpp"

namespace euluna_caster {

// bool
template<> inline int push(EulunaInterface *lua, bool b) {
    lua->pushBoolean(b);
    return 1;
}
template<> inline bool pull(EulunaInterface *lua, int index, bool& b) {
    b = lua->toBoolean(index);
    return true;
}

// int
template<> inline int push(EulunaInterface *lua, int i) {
    lua->pushInteger(i);
    return 1;
}
template<> inline bool pull(EulunaInterface *lua, int index, int& i) {
    i = lua->toInteger(index);
    if(i == 0 && !lua->isNumber(index) && !lua->isNil())
        return false;
    return true;
}

// double
template<> inline int push(EulunaInterface *lua, double d) {
    lua->pushNumber(d);
    return 1;
}
template<> inline bool pull(EulunaInterface *lua, int index, double& d) {
    d = lua->toNumber(index);
    if(d == 0 && !lua->isNumber(index) && !lua->isNil())
        return false;
    return true;
}

// long
//template<> inline int push(EulunaInterface *lua, long l) { push(lua, (double)l); return 1; }
//template<> inline bool pull(EulunaInterface *lua, int index, long& l) { double d; bool r = pull(lua, index, d); l = d; return r; }
// unsigned long
//template<> inline int push(EulunaInterface *lua, unsigned long l) { push(lua, (double)l); return 1; }
//template<> inline bool pull(EulunaInterface *lua, int index, unsigned long& l) { double d; bool r = pull(lua, index, d); l = d; return r; }

// float
template<> inline int push(EulunaInterface *lua, float f) { push(lua, (double)f); return 1; }
template<> inline bool pull(EulunaInterface *lua, int index, float& f) { double d; bool r = pull(lua, index, d); f = d; return r; }
// int8
template<> inline int push(EulunaInterface *lua, int8_t v) { push(lua, (int)v); return 1; }
template<> inline bool pull(EulunaInterface *lua, int index, int8_t& v) { int i; bool r = pull(lua, index, i); v = i; return r; }
// uint8
template<> inline int push(EulunaInterface *lua, uint8_t v) { push(lua, (int)v); return 1; }
template<> inline bool pull(EulunaInterface *lua, int index, uint8_t& v) { int i; bool r = pull(lua, index, i); v = i; return r; }
// int16
template<> inline int push(EulunaInterface *lua, int16_t v) { push(lua, (int)v); return 1; }
template<> inline bool pull(EulunaInterface *lua, int index, int16_t& v) { int i; bool r = pull(lua, index, i); v = i; return r; }
// uint16
template<> inline int push(EulunaInterface *lua, uint16_t v) { push(lua, (int)v); return 1; }
template<> inline bool pull(EulunaInterface *lua, int index, uint16_t& v) { int i; bool r = pull(lua, index, i); v = i; return r; }
// uint32
template<> inline int push(EulunaInterface *lua, uint32_t v) { push(lua, (double)v); return 1; }
template<> inline bool pull(EulunaInterface *lua, int index, uint32_t& v) { double d; bool r = pull(lua, index, d); v = d; return r; }
// int64
template<> inline int push(EulunaInterface *lua, int64_t v) { push(lua, (double)v); return 1; }
template<> inline bool pull(EulunaInterface *lua, int index, int64_t& v) { double d; bool r = pull(lua, index, d); v = d; return r; }
// uint64
template<> inline int push(EulunaInterface *lua, uint64_t v) { push(lua, (double)v); return 1; }
template<> inline bool pull(EulunaInterface *lua, int index, uint64_t& v) { double d; bool r = pull(lua, index, d); v = d; return r; }

// string
template<> inline int push(EulunaInterface *lua, const char* cstr) {
    lua->pushCString(cstr);
    return 1;
}
template<> inline int push(EulunaInterface *lua, const std::string& str) {
    lua->pushString(str);
    return 1;
}
template<> inline bool pull(EulunaInterface *lua, int index, std::string& str) {
    str = lua->toString(index);
    return true;
}


// class pointer
template<class C> typename std::enable_if<std::is_class<C>::value, int>::type push(EulunaInterface* lua, C* obj) {
    lua->pushObject(obj);
    return true;
}
template<class C> typename std::enable_if<std::is_class<C>::value, int>::type pull(EulunaInterface* lua, int index, C*& obj) {
    obj = lua->toObject<C>(index);
    return true;
}

}

#endif // EULUNACASTER_HPP
