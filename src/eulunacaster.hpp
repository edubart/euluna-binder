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
inline int push(EulunaInterface *lua, bool b) {
    lua->pushBoolean(b);
    return 1;
}
inline bool pull(EulunaInterface *lua, int index, bool& b) {
    b = lua->toBoolean(index);
    return true;
}

// int
inline int push(EulunaInterface *lua, int i) {
    lua->pushInteger(i);
    return 1;
}
inline bool pull(EulunaInterface *lua, int index, int& i) {
    i = lua->toInteger(index);
    if(i == 0 && !lua->isNumber(index) && !lua->isNil())
        return false;
    return true;
}

// double
inline int push(EulunaInterface *lua, double d) {
    lua->pushNumber(d);
    return 1;
}
inline bool pull(EulunaInterface *lua, int index, double& d) {
    d = lua->toNumber(index);
    if(d == 0 && !lua->isNumber(index) && !lua->isNil())
        return false;
    return true;
}

#if !defined(__x86_64) || defined(__MINGW32__) || defined(__APPLE__)
// long
inline int push(EulunaInterface *lua, long l) { push(lua, (double)l); return 1; }
inline bool pull(EulunaInterface *lua, int index, long& l) { double d; bool r = pull(lua, index, d); l = d; return r; }

// unsigned long
inline int push(EulunaInterface *lua, unsigned long l) { push(lua, (double)l); return 1; }
inline bool pull(EulunaInterface *lua, int index, unsigned long& l) { double d; bool r = pull(lua, index, d); l = d; return r; }
#endif

// float
inline int push(EulunaInterface *lua, float f) { push(lua, (double)f); return 1; }
inline bool pull(EulunaInterface *lua, int index, float& f) { double d; bool r = pull(lua, index, d); f = d; return r; }

// int8
inline int push(EulunaInterface *lua, int8_t v) { push(lua, (int)v); return 1; }
inline bool pull(EulunaInterface *lua, int index, int8_t& v) { int i; bool r = pull(lua, index, i); v = i; return r; }

// uint8
inline int push(EulunaInterface *lua, uint8_t v) { push(lua, (int)v); return 1; }
inline bool pull(EulunaInterface *lua, int index, uint8_t& v) { int i; bool r = pull(lua, index, i); v = i; return r; }

// int16
inline int push(EulunaInterface *lua, int16_t v) { push(lua, (int)v); return 1; }
inline bool pull(EulunaInterface *lua, int index, int16_t& v) { int i; bool r = pull(lua, index, i); v = i; return r; }

// uint16
inline int push(EulunaInterface *lua, uint16_t v) { push(lua, (int)v); return 1; }
inline bool pull(EulunaInterface *lua, int index, uint16_t& v) { int i; bool r = pull(lua, index, i); v = i; return r; }

// uint32
inline int push(EulunaInterface *lua, uint32_t v) { push(lua, (double)v); return 1; }
inline bool pull(EulunaInterface *lua, int index, uint32_t& v) { double d; bool r = pull(lua, index, d); v = d; return r; }

// int64
inline int push(EulunaInterface *lua, int64_t v) { push(lua, (double)v); return 1; }
inline bool pull(EulunaInterface *lua, int index, int64_t& v) { double d; bool r = pull(lua, index, d); v = d; return r; }

// uint64
inline int push(EulunaInterface *lua, uint64_t v) { push(lua, (double)v); return 1; }
inline bool pull(EulunaInterface *lua, int index, uint64_t& v) { double d; bool r = pull(lua, index, d); v = d; return r; }

// string
inline int push(EulunaInterface *lua, const char* cstr) {
    lua->pushCString(cstr);
    return 1;
}
inline int push(EulunaInterface *lua, const std::string& str) {
    lua->pushString(str);
    return 1;
}
inline bool pull(EulunaInterface *lua, int index, std::string& str) {
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

// enum
template<class T>
typename std::enable_if<std::is_enum<T>::value, int>::type
push(EulunaInterface *lua, T e) { return push(lua, (int)e); }

template<class T>
typename std::enable_if<std::is_enum<T>::value, bool>::type pull(EulunaInterface *lua, int index, T& myenum) {
    int i;
    if(pull(lua, index, i)) {
        myenum = (T)i;
        return true;
    }
    return false;
}

// std::function
template<typename Ret, typename... Args>
int push(EulunaInterface *lua, const std::function<Ret(Args...)>& func);

class LuaFunctionHolder {
public:
    LuaFunctionHolder() = delete;
    LuaFunctionHolder(const LuaFunctionHolder&) = delete;
    LuaFunctionHolder& operator=(const LuaFunctionHolder&) = delete;

    LuaFunctionHolder(EulunaInterface *lua, int ref) : m_lua(lua), m_ref(ref) { }
    ~LuaFunctionHolder() { m_lua->unref(m_ref); }
    void pushFunc() const { m_lua->getRef(m_ref); }
    EulunaInterface *getLua() { return m_lua; }
private:
    EulunaInterface *m_lua;
    int m_ref;
};

// note that std::function callbacks can throw exceptions
template<typename... Args>
bool pull(EulunaInterface *lua, int index, std::function<void(Args...)>& func) {
    if(lua->isFunction(index)) {
        lua->pushValue(index);
        int ref = lua->ref();
        std::shared_ptr<LuaFunctionHolder> holder(new LuaFunctionHolder(lua, ref));
        func = [holder](Args... args) {
            EulunaInterface *lua = holder->getLua();
            holder->pushFunc();
            if(lua->isFunction()) {
                int numArgs = lua->polymorphicPush(args...);
                lua->safeCall(numArgs, 0);
            } else
                throw EulunaRuntimeError("Attempt to call an expired lua function from C++");
        };
        return true;
    } else if(lua->isNil(index)) {
        func = std::function<void(Args...)>();
        return true;
    }
    return false;
}
template<typename Ret, typename... Args>
typename std::enable_if<!std::is_void<Ret>::value, bool>::type
pull(EulunaInterface *lua, int index, std::function<Ret(Args...)>& func) {
    if(lua->isFunction(index)) {
        lua->pushValue(index);
        int ref = lua->ref();
        std::shared_ptr<LuaFunctionHolder> holder(new LuaFunctionHolder(lua, ref));
        func = [holder](Args... args) -> Ret {
            EulunaInterface *lua = holder->getLua();
            holder->pushFunc();
            if(lua->isFunction()) {
                int numArgs = lua->polymorphicPush(args...);
                lua->safeCall(numArgs, 1);
                return lua->polymorphicPop<Ret>();
            } else
                throw EulunaRuntimeError("Attempt to call an expired lua function from C++");
            return Ret();
        };
        return true;
    } else if(lua->isNil(index)) {
        func = std::function<Ret(Args...)>();
        return true;
    }
    return false;
}

// lambda
template<typename Lambda>
typename std::enable_if<std::is_constructible<decltype(&Lambda::operator())>::value, int>::type
push(EulunaInterface *lua, const Lambda& func) {
    typedef typename euluna_traits::lambda_to_stdfunction<Lambda>::value F;
    return push(lua, F(func));
}

// list
template<typename T>
int push(EulunaInterface *lua, const std::list<T>& list) {
    lua->createTable(list.size(), 0);
    int i = 1;
    for(const T& v : list) {
        push(lua, v);
        lua->rawSeti(i);
        i++;
    }
    return 1;
}

template<typename T>
bool pull(EulunaInterface *lua, int index, std::list<T>& list) {
    if(lua->isTable(index)) {
        lua->pushNil();
        while(lua->next(index < 0 ? index-1 : index)) {
            T value;
            if(pull(lua, -1, value))
                list.push_back(value);
            lua->pop();
        }
        return true;
    } else if(lua->isNil()) {
        list.clear();
        return true;
    }
    return false;
}

// vector
template<typename T>
int push(EulunaInterface *lua, const std::vector<T>& vector) {
    lua->createTable(vector.size(), 0);
    int i = 1;
    for(const T& v : vector) {
        push(lua, v);
        lua->rawSeti(i);
        i++;
    }
    return 1;
}

template<typename T>
bool pull(EulunaInterface *lua, int index, std::vector<T>& vector) {
    if(lua->isTable(index)) {
        lua->pushNil();
        while(lua->next(index < 0 ? index-1 : index)) {
            T value;
            if(pull(lua, -1, value))
                vector.push_back(value);
            lua->pop();
        }
        return true;
    } else if(lua->isNil()) {
        vector.clear();
        return true;
    }
    return false;
}

// deque
template<typename T>
int push(EulunaInterface *lua, const std::deque<T>& deque) {
    lua->createTable(deque.size(), 0);
    int i = 1;
    for(const T& v : deque) {
        push(lua, v);
        lua->rawSeti(i);
        i++;
    }
    return 1;
}

template<typename T>
bool pull(EulunaInterface *lua, int index, std::deque<T>& deque) {
    if(lua->isTable(index)) {
        lua->pushNil();
        while(lua->next(index < 0 ? index-1 : index)) {
            T value;
            if(pull(lua, -1, value))
                deque.push_back(value);
            lua->pop();
        }
        return true;
    } else if(lua->isNil()) {
        deque.clear();
        return true;
    }
    return false;
}

// map
template<class K, class V>
int push(EulunaInterface *lua, const std::map<K, V>& map) {
    lua->newTable();
    for(auto& it : map) {
        push(lua, it.first);
        push(lua, it.second);
        lua->rawSet();
    }
    return 1;
}

template<class K, class V>
bool pull(EulunaInterface *lua, int index, std::map<K, V>& map) {
    if(lua->isTable(index)) {
        lua->pushNil();
        while(lua->next(index < 0 ? index-1 : index)) {
            K key;
            V value;
            if(pull(lua, -1, value) && pull(lua, -2, key))
                map[key] = value;
            lua->pop();
        }
        return true;
    }
    return false;
}

// unordered_map
template<class K, class V>
int push(EulunaInterface *lua, const std::unordered_map<K, V>& map) {
    lua->newTable();
    for(auto& it : map) {
        push(lua, it.first);
        push(lua, it.second);
        lua->rawSet();
    }
    return 1;
}

template<class K, class V>
bool pull(EulunaInterface *lua, int index, std::unordered_map<K, V>& map) {
    if(lua->isTable(index)) {
        lua->pushNil();
        while(lua->next(index < 0 ? index-1 : index)) {
            K key;
            V value;
            if(pull(lua, -1, value) && pull(lua, -2, key))
                map[key] = value;
            lua->pop();
        }
        return true;
    }
    return false;
}

// set
template<class K>
int push(EulunaInterface *lua, const std::set<K>& set) {
    lua->createTable(set.size(), 0);
    int i=1;
    for(auto it = set.cbegin(); it != set.cend(); ++it) {
        push(lua, *it);
        lua->rawSeti(i);
        i++;
    }
    return 1;
}

template<class K>
bool pull(EulunaInterface *lua, int index, std::set<K>& set) {
    if(lua->isTable(index)) {
        lua->pushNil();
        while(lua->next(index < 0 ? index-1 : index)) {
            K key;
            if(pull(lua, -1, key))
                set.insert(key);
            lua->pop();
        }
        return true;
    }
    return false;
}

// unordered_set
template<class K>
int push(EulunaInterface *lua, const std::unordered_set<K>& set) {
    lua->createTable(set.size(), 0);
    int i=1;
    for(auto it = set.cbegin(); it != set.cend(); ++it) {
        push(lua, *it);
        lua->rawSeti(i);
        i++;
    }
    return 1;
}

template<class K>
bool pull(EulunaInterface *lua, int index, std::unordered_set<K>& set) {
    if(lua->isTable(index)) {
        lua->pushNil();
        while(lua->next(index < 0 ? index-1 : index)) {
            K key;
            if(pull(lua, -1, key))
                set.insert(key);
            lua->pop();
        }
        return true;
    }
    return false;
}

// tuple
template<int N>
struct push_tuple_luavalue {
    template<typename Tuple>
    static void call(EulunaInterface *lua, const Tuple& tuple) {
        push(lua, std::get<N-1>(tuple));
        lua->rawSeti(N);
        push_tuple_luavalue<N-1>::call(lua, tuple);
    }
};
template<>
struct push_tuple_luavalue<0> {
    template<typename Tuple>
    static void call(EulunaInterface *lua, const Tuple& tuple) { }
};
template<typename... Args>
int push(EulunaInterface *lua, const std::tuple<Args...>& tuple) {
    lua->newTable();
    push_tuple_luavalue<sizeof...(Args)>::call(lua, tuple);
    return 1;
}

template<int N>
struct pull_tuple_luavalue {
    template<typename Tuple>
    static bool call(EulunaInterface *lua, int index, Tuple& tuple) {
        lua->rawGeti(std::tuple_size<Tuple>::value - N + 1, index);
        bool ok = pull(lua, -1, std::get<std::tuple_size<Tuple>::value - N>(tuple));
        lua->pop();
        return ok && pull_tuple_luavalue<N-1>::call(lua, index, tuple);
    }
};
template<>
struct pull_tuple_luavalue<0> {
    template<typename Tuple>
    static bool call(EulunaInterface *lua, int index, Tuple& tuple) { return true; }
};
template<typename... Args>
bool pull(EulunaInterface *lua, int index, std::tuple<Args...>& tuple) {
    if(lua->isTable(index))
        return pull_tuple_luavalue<sizeof...(Args)>::call(lua, index, tuple);
    return false;
}

}

#endif // EULUNACASTER_HPP
