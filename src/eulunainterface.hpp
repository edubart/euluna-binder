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

#ifndef EULUNAINTERFACE_HPP
#define EULUNAINTERFACE_HPP

#include "eulunaprereqs.hpp"
#include "eulunatools.hpp"
#include "eulunaexception.hpp"

namespace euluna_caster {

template<typename T>
typename std::enable_if<!std::is_scalar<T>::value,int>::type push(EulunaInterface *lua, const T& v);
template<typename T>
typename std::enable_if<std::is_scalar<T>::value,int>::type push(EulunaInterface *lua, T v);
template<typename T>
bool pull(EulunaInterface *lua, int index, T& v);

}

// Interface for managing lua state
class EulunaInterface {
public:
    EulunaInterface() : L(luaL_newstate()), m_ownState(true) {
        openLibs();
        if(!L)
            throw EulunaRuntimeError();
    }

    explicit EulunaInterface(lua_State *L) : L(L), m_ownState(false) {
        if(!L)
            throw EulunaRuntimeError();
    }

    ~EulunaInterface() {
        if(m_ownState)
            lua_close(L);
    }

    // load and call with exception
    int safeCall(int numArgs = 0, int numRets = 0) {
        // saves the current stack size for calculating the number of results later
        int savedStackSize = stackSize() - numArgs - 1;
        // pushes error function
        int errorFuncIndex = savedStackSize + 1;
        pushCFunction([](lua_State* L) -> int {
            EulunaInterface lua(L);
            // pops the error message
            std::string error = lua.popString();
            // push error with traceback information
            lua.traceback(error, 1);
            return 1;
        });
        insert(errorFuncIndex);
        // call the function
        int err = pcall(numArgs, numRets, errorFuncIndex);
        //  remove error func
        remove(errorFuncIndex);
        // throw if any error
        handleLuaError(err);
        // calculate the number of results
        int rets = stackSize() - savedStackSize;
        // adjust stack size if needed
        if(numRets >= 0 && rets != numRets)
            ensureStackSize(savedStackSize + numRets);
        // return the number of results
        return rets;
    }

    void safeLoadBuffer(const std::string& buffer, const std::string& source = "") {
        // parse lua code
        int err = loadBuffer(buffer, source);
        // throw if any error
        handleLuaError(err);
    }

    int safeDoBuffer(const std::string& buffer, const std::string& source = "", int numRets = 0) {
        // parse lua code
        safeLoadBuffer(buffer, source);
        // call parsed lua code
        return safeCall(0, numRets);
    }

    // state manipulation
    lua_State* newThread() { return lua_newthread(L); }
    lua_State *luaState() { return L; }
    void setLuaState(lua_State *state) { L = state; m_ownState = false; }

    // basic stack manipulation
    int getTop() const { return lua_gettop(L); }
    int stackSize() const { return lua_gettop(L); }
    void setTop(int index) { lua_settop(L, index); }
    void pushValue(int index = -1) { lua_pushvalue(L, index); }
    void insert(int index) { lua_insert(L, index); }
    void remove(int index) { lua_remove(L, index); }
    void replace(int index) { lua_replace(L, index); }
    void xmove(lua_State *state, int index) { lua_xmove(L, state, index); }
    void ensureStackSize(int n) {
        while(stackSize() != n) {
            if(stackSize() < n)
                pushNil();
            else
                pop();
        }
    }
    void clearStack() { pop(stackSize()); }

    // access functions
    bool isNil(int index = -1) const { return lua_isnil(L, index); }
    bool isBoolean(int index = -1) const { return lua_isboolean(L, index); }
    bool isNumber(int index = -1) const { return lua_isnumber(L, index); }
    bool isString(int index = -1) const { return lua_isstring(L, index); }
    bool isTable(int index = -1) const { return lua_istable(L, index); }
    bool isFunction(int index = -1) const { return lua_isfunction(L, index); }
    bool isCFunction(int index = -1) const { return lua_iscfunction(L, index); }
    bool isLuaFunction(int index = -1) const { return (isFunction(index) && !isCFunction(index)); }
    bool isUserdata(int index = -1) const { return lua_isuserdata(L, index); }
    bool isThread(int index = -1) const { return lua_isthread(L, index); }
    bool isLightUserdata(int index = -1) const { return lua_islightuserdata(L, index); }
    int type(int index = -1) const { return lua_type(L, index); }

    bool toBoolean(int index = -1) { return lua_toboolean(L, index); }
    int toInteger(int index = -1) { return lua_tointeger(L, index); }
    double toNumber(int index = -1) { return lua_tonumber(L, index); }
    const char* toCString(int index = -1) { return lua_tostring(L, index); }
    std::string toString(int index = -1) {
        size_t len = 0;
        const char *cstr = lua_tolstring(L, index, &len);
        return (cstr && len > 0) ? std::string(cstr, len) : std::string();
    }
    LuaCFunction toCFunction(int index = -1) { return lua_tocfunction(L, index); }
    const void *toPointer(int index = -1) { return lua_topointer(L, index); }
    void* toUserdata(int index = -1) { return lua_touserdata(L, index); }
    lua_State* toThread(int index = -1) { return lua_tothread(L, index); }
    const char* toTypeName(int index = -1) { return lua_typename(L, lua_type(L, index)); }

    // convert with check functions
    bool checkStack(int size) { return lua_checkstack(L, size); }
    void checkType(int type, int index = -1) { return luaL_checktype(L, index, type); }
    void checkAny(int index = -1) { return luaL_checkany(L, index); }
    long checkInteger(int index = -1) { return luaL_checkinteger(L, index); }
    double checkNumber(int index = -1) { return luaL_checknumber(L, index); }
    const char* checkCString(int index = -1) { return luaL_checkstring(L, index); }
    std::string checkString(int index = -1) {
        size_t len = 0;
        const char *cstr = luaL_checklstring(L, index, &len);
        return (cstr && len > 0) ? std::string(cstr, len) : std::string();
    }
    void* checkUserData(const char* name, int index = -1) { return luaL_checkudata(L, index, name); }
    void* checkUserData(const std::string& name, int index = -1) { return luaL_checkudata(L, index, name.c_str()); }

    // push functions
    void pushNil() { lua_pushnil(L); }
    void pushInteger(long v) { lua_pushinteger(L, v); }
    void pushNumber(double v) { lua_pushnumber(L, v); }
    void pushBoolean(bool v) { lua_pushboolean(L, v); }
    void pushCString(const char* v) { lua_pushstring(L, v); }
    void pushString(const std::string& v) { lua_pushlstring(L, v.c_str(), v.length()); }
    void pushLightUserdata(void* p) { lua_pushlightuserdata(L, p); }
    void pushThread() { lua_pushthread(L); }
    void pushCFunction(LuaCFunction func, int n = 0) { lua_pushcclosure(L, func, n); }
    void pushCppFunction(EulunaCppFunction* func) {
        // create a pointer to func (this pointer DOESN'T hold the function existence)
        pushLightUserdata(func);
        assert(func);
        // actually pushes a C function callback that will call the cpp function
        pushCFunction([](lua_State* L) -> int {
            EulunaInterface lua(L);
            // retrieves function pointer from userdata
            auto funcPtr = static_cast<EulunaCppFunction*>(lua.toUserdata(lua.upvalueIndex(1)));
            assert(funcPtr);
            int numRets;
            // do the call
            try {
                numRets = (*funcPtr)(&lua);
                assert(numRets == lua.stackSize());
            } catch(std::exception& e) {
                numRets = 0;
                lua.clearStack();
                lua.traceback(euluna_tools::format("C++ exception %s: %s", euluna_tools::demangle_type(e), e.what()));
                lua.error();
            }
            return numRets;
        }, 1);
    }
    void pushCppFunction(EulunaCppFunction func) {
        // create a pointer to func (this pointer holds the function existence)
        new(newUserdata(sizeof(EulunaCppFunctionPtr))) EulunaCppFunctionPtr(new EulunaCppFunction(std::move(func)));
        // sets the userdata __gc metamethod, needed to free the function pointer when it gets collected
        newTable();
        pushCFunction([](lua_State* L) -> int {
            EulunaInterface lua(L);
            auto funcPtr = static_cast<EulunaCppFunctionPtr*>(lua.popUserdata());
            assert(funcPtr);
            funcPtr->reset();
            return 0;
        });
        setField("__gc");
        setMetatable();
        // actually pushes a C function callback that will call the cpp function
        pushCFunction([](lua_State* L) -> int {
            EulunaInterface lua(L);
            // retrieves function pointer from userdata
            auto funcPtr = static_cast<EulunaCppFunctionPtr*>(lua.toUserdata(lua.upvalueIndex(1)));
            assert(funcPtr);
            int numRets;
            // do the call
            try {
                numRets = (*(funcPtr->get()))(&lua);
                assert(numRets == lua.stackSize());
            } catch(std::exception& e) {
                numRets = 0;
                lua.clearStack();
                lua.traceback(euluna_tools::format("C++ exception %s: %s", euluna_tools::demangle_type(e), e.what()));
                lua.error();
            }
            return numRets;
        }, 1);
    }

    // get functions
    void rawGet(int index = -2) { lua_rawget(L, index); }
    void rawGeti(int n, int index = -1) { lua_rawgeti(L, index, n); }
    void getField(const char* key, int index = -1) { lua_getfield(L, index, key); }
    void getField(const std::string& key, int index = -1) { lua_getfield(L, index, key.c_str()); }
    void getMetaField(const char* key, int index = -1) { luaL_getmetafield(L, index, key); }
    void getMetaField(const std::string& key, int index = -1) { luaL_getmetafield(L, index, key.c_str()); }
    void getTable(int index = -2) { lua_gettable(L, index); }
    void getMetatable(int index = -1) { lua_getmetatable(L, index); }
    void getGlobal(const char* key) { lua_getglobal(L, key); }
    void getGlobal(const std::string& key) {  lua_getglobal(L, key.c_str()); }
    void getRegistry() { lua_rawget(L, LUA_REGISTRYINDEX); }
    void getRegistryField(const std::string& key) {  lua_getfield(L, LUA_REGISTRYINDEX, key.c_str()); }
    void getRegistryField(const char* key) { lua_getfield(L, LUA_REGISTRYINDEX, key); }
    void createTable(int narr, int nrec) { lua_createtable(L, narr, nrec); }
    void newTable() { lua_newtable(L); }
    void* newUserdata(size_t size) { return lua_newuserdata(L, size); }
    void newMetatable(const std::string& name) { luaL_newmetatable(L, name.c_str()); }

    // set functions
    void rawSet(int index = -3) { lua_rawset(L, index); }
    void rawSeti(int n, int index = -2) { lua_rawseti(L, index, n); }
    void setField(const char* key, int index = -2) { lua_setfield(L, index, key); }
    void setField(const std::string& key, int index = -2) { lua_setfield(L, index, key.c_str()); }
    void setTable(int index = -3) { lua_settable(L, index); }
    void setMetatable(int index = -2) { lua_setmetatable(L, index); }
    void setGlobal(const char* key) { lua_setglobal(L, key); }
    void setGlobal(const std::string& key) { lua_setglobal(L, key.c_str()); }
    void setRegistry() { lua_rawset(L, LUA_REGISTRYINDEX); }
    void setRegistryField(const char* key) { lua_setfield(L, LUA_REGISTRYINDEX, key); }
    void setRegistryField(const std::string& key) { lua_setfield(L, LUA_REGISTRYINDEX, key.c_str()); }

    // load and call
    int pcall(int numArgs = 0, int numRets = 0, int errorFuncIndex = 0) { return lua_pcall(L, numArgs, numRets, errorFuncIndex); }
    void call(int numArgs = 0, int numRets = 0) { lua_call(L, numArgs, numRets); }
    int loadBuffer(const std::string& buffer, const std::string& source = "") { return luaL_loadbuffer(L, buffer.c_str(), buffer.length(), source.c_str()); }
    int doBuffer(const std::string& buffer, const std::string& source = "") {
        int err = luaL_loadbuffer(L, buffer.c_str(), buffer.length(), source.c_str());
        if(err == 0)
            err = lua_pcall(L, 0, LUA_MULTRET, 0);
        return err;
    }

    // garbage collector
    int gc(int what, int data) { return lua_gc(L, what, data); }

    // miscellaneous functions
    void error() { lua_error(L); }
    int argError(int arg, const char* extramsg) { return luaL_argerror(L, arg, extramsg); }
    int argError(int arg, const std::string& extramsg) { return luaL_argerror(L, arg, extramsg.c_str()); }
    bool next(int index = -2) { return lua_next(L, index); }
    void concat(int index = -1) { return lua_concat(L, index); }
    int upvalueIndex(int index) { return lua_upvalueindex(index); }
    const char* typeName(int type) { return lua_typename(L, type); }
    void traceback(const char* msg, int level = 0) { luaL_traceback(L, L, msg, level); }
    void traceback(const std::string& msg, int level = 0) { luaL_traceback(L, L, msg.c_str(), level); }
    void getOrCreateGlobalTable(const std::string& name) {
        getGlobal(name);
        if(isNil()) {
            pop();
            newTable();
            pushValue();
            setGlobal(name);
        } else {
            assert(isTable());
        }
    }
    void getOrCreateRegistryTable(const std::string& name) {
        getRegistryField(name);
        if(isNil()) {
            pop();
            newTable();
            pushValue();
            setRegistryField(name);
        } else {
            assert(isTable());
        }
    }

    // pop functions
    void pop(int n = 1) { lua_pop(L, n); }
    long popInteger() { long v = toInteger(); pop(); return v; }
    double popNumber() { double v = toNumber(); pop(); return v; }
    bool popBoolean() { bool v = toBoolean(); pop(); return v; }
    std::string popString() { std::string v = toString(); pop(); return v; }
    void* popUserdata() { void* v = toUserdata(); pop(); return v; }

    // open libs
    void openLibs() { luaL_openlibs(L); }

    // ref
    int ref() { return luaL_ref(L, LUA_REGISTRYINDEX); }
    void unref(int ref) { luaL_unref(L, LUA_REGISTRYINDEX, ref); }
    void getRef(int ref) { lua_rawgeti(L, LUA_REGISTRYINDEX, ref); }


    // object related
    template<class C>
    void useObject(C* instance);
    template<class C>
    void releaseObject(C* instance);
    template<class C>
    void pushObject(C *instance);
        useObject(ptr);
        new(newUserdata(sizeof))
        pushL
    }

    EulunaObject* toObject(int index = -1) {
        if(!isUserdata(index))
          return nullptr;
        pushValue(index);
        getRegistry();
        EulunaObject *object = static_cast<EulunaObject*>(toUserdata());
        return object;
    }

    // polymorphic
    template<typename T, typename... Args>
    int polymorphicPush(const T& v, const Args&... args) {
        int r = euluna_caster::push(this, v);
        return r + polymorphicPush(args...);
    }
    int polymorphicPush() { return 0; }

    template<typename R>
    R polymorphicPop() {
        R ret;
        if(!polymorphicPull(ret)) {
            traceback(euluna_tools::format("bad argument or return (%s expected, got %s)", euluna_tools::demangle_type<R>(), toTypeName()));
            remove(-2);
            handleLuaError(LUA_ERRRUN);
        } else
            pop();
        return ret;
    }

    template<class T>
    bool polymorphicPull(T& v, int index = -1) { return euluna_caster::pull(this, index, v); }

    template<typename R, typename... T>
    R polymorphicSafeCall(const T&... args) {
        int numArgs = polymorphicPush(args...);
        safeCall(numArgs, 1);
        return polymorphicPop<R>();
    }

    template<typename R>
    R polymorphicSafeDoBuffer(const std::string& buffer, const std::string& source = "") {
        // parse lua code
        safeLoadBuffer(buffer, source);
        return polymorphicSafeCall<R>();
    }

protected:
    void handleLuaError(int err) {
        if(err == LUA_OK)
            return;
        std::string msg = popString();
        switch(err) {
        case LUA_ERRSYNTAX:
            throw EulunaSyntaxError(msg);
            break;
        case LUA_ERRMEM:
            throw EulunaMemoryError(msg);
            break;
        case LUA_ERRERR:
            throw EulunaErrorError(msg);
            break;
        case LUA_ERRRUN:
        default:
            throw EulunaRuntimeError(msg);
            break;
        }
    }

    lua_State *L;
    bool m_ownState;
};

#include "eulunacaster.hpp"
#include "eulunaobject.hpp"

template<class C>
EulunaObject *EulunaInterface::useObject(C* instance) {
    assert(instance);
    pushLightUserdata(instance);
    getRegistry();
    EulunaObject *obj;
    if(isNil()) {
        pushLightUserdata(instance);
        obj = new EulunaObject();
        pushLightUserdata(obj);
        setRegistry();
    } else
        obj = static_cast<EulunaObject*>(popUserdata());
    obj->lua_ref(this);
    return obj;
}

template<class C>
void EulunaInterface::releaseObject(C* instance) {
    assert(instance);
    pushLightUserdata(instance);
    getRegistry();
    EulunaObject *obj = static_cast<EulunaObject*>(popUserdata());
    assert(obj);
    obj->lua_release(this);
    pushLightUserdata(instance);
    pushNil();
    setRegistry();
    delete obj;
}

template<class C>
void EulunaInterface::pushObject(C *instance);
    EulunaObject *obj = useObject(ptr);
    new(newUserdata(sizeof(void*))) void*(instance);
    obj->lua_getMetatable(this, euluna_tools::demangle_type(*instance));
    assert(!isNil());
    setMetatable();
}

#endif // EULUNAINTERFACE_HPP
