#include <gtest/gtest.h>
#include "../src/euluna.hpp"
#include <iostream>

EulunaBinder& g_binder = EulunaBinder::instance();
EulunaEngine& g_lua = EulunaEngine::instance();

TEST(DemanglerTest, BasicTypes) {
    EXPECT_EQ("int", euluna_tools::demangle_type<int>());
    EXPECT_EQ("bool", euluna_tools::demangle_type<bool>());
}

TEST(DemanglerTest, Classes) {
    //EXPECT_EQ("EulunaSharedObject", EulunaSharedObject().euluna_getCppClassName());
    //EXPECT_EQ("std::string", euluna_tools::demangle_type<std::string>());
    EXPECT_EQ(EulunaRuntimeError().name(), "EulunaRuntimeError");
    EulunaRuntimeError derivedException;
    EulunaException& e = derivedException;
    EXPECT_EQ(e.name(), "EulunaRuntimeError");
    EXPECT_EQ(euluna_tools::demangle_type(e), "EulunaRuntimeError");
}

TEST(EulunaBinder, RegisterBindings)
{
    g_binder.registerBindings(&g_lua);
    EXPECT_EQ(g_lua.stackSize(), 0);
}

////////////////////
void gnop() {
    // do nothing
}

std::string gconcat(const std::string& a, int b) {
    return a+std::to_string(b);
}

int lua_gmul(EulunaInterface* lua) {
    double a = lua->popNumber();
    double b = lua->popNumber();
    lua->pushNumber(a*b);
    return 1;
}

EULUNA_BEGIN_GLOBAL_FUNCTIONS(myglobals)
EULUNA_FUNC(gconcat)
EULUNA_FUNC(gnop)
EULUNA_FUNC_NAMED("gdiv", [](float a, float b) { return a/b; })
EULUNA_FUNC_NAMED("gmul", lua_gmul)
EULUNA_END()

TEST(EulunaBinder, Globals) {
    EXPECT_EQ(g_lua.runBuffer<std::string>("return gconcat(1,2)"), std::string("12"));
    EXPECT_EQ(g_lua.runBuffer<float>("return gdiv(10,5)"), 2.0f);
    EXPECT_EQ(g_lua.runBuffer<double>("return gmul(3,50)"), 150.0);
    EXPECT_EQ(g_lua.stackSize(), 0);
}

////////////////////
EULUNA_BEGIN_SINGLETON(test)
EULUNA_FUNC(gconcat)
EULUNA_FUNC(gnop)
EULUNA_FUNC_NAMED("div", [](float a, float b) { return a/b; })
EULUNA_FUNC_NAMED("mul", lua_gmul)
EULUNA_END()

class Test {
public:
    static std::string concat(const std::string& a, int b) { return a+std::to_string(b); }
    static int lua_mul(EulunaInterface* lua) {
        double a = lua->popNumber();
        double b = lua->popNumber();
        lua->pushNumber(a*b);
        return 1;
    }
    static void nop() { }
    void mynop() { }
    uint setFoo(const std::string& foo) { m_foo = foo; return 0xdeadbeaf; }
    std::string getMyFoo() const { return m_foo; }
private:
    std::string m_foo;
};
Test g_test;

EULUNA_BEGIN_SINGLETON_CLASS_NAMED("g_test", Test, &g_test)
EULUNA_CLASS_STATIC(Test, concat)
EULUNA_CLASS_STATIC(Test, nop)
EULUNA_CLASS_STATIC_NAMED("mul", Test, lua_mul)
EULUNA_CLASS_MEMBER(Test, mynop)
EULUNA_CLASS_MEMBER(Test, setFoo)
EULUNA_CLASS_MEMBER_NAMED("getFoo", Test, getMyFoo)
EULUNA_END()

TEST(EulunaBinder, Singletons) {
    EXPECT_EQ(g_lua.runBuffer<std::string>("return test.gconcat(1,2)"), "12");
    EXPECT_EQ(g_lua.runBuffer<float>("return test.div(10,5)"), 2.0f);
    EXPECT_EQ(g_lua.runBuffer<double>("return test.mul(3,50)"), 150.0);
    EXPECT_EQ(g_lua.runBuffer<std::string>("return g_test.concat(1,2)"), "12");
    EXPECT_EQ(g_lua.runBuffer<double>("return g_test.mul(3,50)"), 150.0);
    EXPECT_EQ(g_lua.runBuffer<uint>("return g_test.setFoo('boo')"), 0xdeadbeaf);
    EXPECT_EQ(g_lua.runBuffer<std::string>("return g_test.getFoo()"), "boo");
    EXPECT_EQ(g_lua.stackSize(), 0);
}



///////////////////////////
class Dummy {
public:
    Dummy() { m_dummyCounter++; }
    ~Dummy() { m_dummyCounter--; }

    Dummy(const Dummy&) = delete;
    Dummy& operator=(const Dummy&) = delete;

    void setBoo(const std::string& boo) { m_boo = boo; }
    std::string getBoo() const { return m_boo; }

    static Dummy* create() { return new Dummy; }
    void destroy() {
        g_lua.releaseObject(this);
        delete this;
    }

    static int getDummyCounter() { return m_dummyCounter; }

private:
    std::string m_boo;
    static int m_dummyCounter;
};
int Dummy::m_dummyCounter = 0;

void __handleDummyUse(EulunaInterface* lua, Dummy *dummy) {
    // nothing to do
}
void __handleDummyRelease(EulunaInterface* lua, Dummy *dummy) {
    std::cout << "relase" << dummy->getBoo() << std::endl;
    // releases all lua variables related to this object
    lua->releaseObject(dummy);
    // delete C++ memory for this object
    delete dummy;
}

EULUNA_BEGIN_MANAGED_CLASS(Dummy)
EULUNA_CLASS_REFERENCE_HANDLERS(__handleDummyUse, __handleDummyRelease)
EULUNA_CLASS_STATIC(Dummy, create)
EULUNA_CLASS_MEMBER(Dummy, destroy)
EULUNA_CLASS_STATIC_NAMED_EX("new", []{ return new Dummy; })
EULUNA_CLASS_STATIC_NAMED_EX("delete", [](Dummy *dummy) { g_lua.releaseObject(dummy); delete dummy; })
EULUNA_CLASS_MEMBER(Dummy, setBoo)
EULUNA_CLASS_MEMBER(Dummy, getBoo)
EULUNA_END()

TEST(Euluna, ManagedClass) {
    std::string script = R"(
        local res = ''
        local dummy = Dummy.create()
        dummy:setBoo("hello")
        res = res .. dummy:getBoo()
        dummy:destroy()
        dummy = nil
        collectgarbage("collect")

        dummy = Dummy.new()
        dummy:setBoo(" world")
        res = res .. dummy:getBoo()
        Dummy.delete(dummy)
        dummy = nil

        --dummy = Dummy.new()
        --dummy:setBoo("!")
        --res = res .. dummy:getBoo()
        --dummy = nil
        Dummy.new()

        collectgarbage("collect")
        collectgarbage("collect")

        return res
    )";
    EXPECT_EQ(g_lua.runBuffer<std::string>(script), "hello world!");
    EXPECT_EQ(Dummy::getDummyCounter(), 0);
}

//////////////////////
double mathex_lerp(double a, double b, double t) {
    return a + (b-a)*t;
}

std::string string_concat(const std::string& a, const std::string& b) {
    return a + b;
}

namespace stringutil {
std::string concat(const std::string& a, const std::string& b) {
    return a + b;
}
}

EULUNA_BEGIN_SINGLETON(mathex)
EULUNA_FUNC_NAMED("lerp", mathex_lerp)
EULUNA_END()

EULUNA_BEGIN_GLOBAL_FUNCTIONS(myglobals2)
EULUNA_FUNC_NAMED("concat", string_concat)
EULUNA_END()

EULUNA_BEGIN_SINGLETON(stringutil)
EULUNA_FUNC_NAMED("concat", stringutil::concat);
EULUNA_END()

class Foo {
public:
    static Foo* instance() {
        static Foo instance;
        return &instance;
    }

    void setBoo(const std::string& boo) { m_boo = boo; }
    std::string getBoo() const { return m_boo; }

private:
    std::string m_boo;
};

EULUNA_BEGIN_SINGLETON_CLASS_NAMED("foo", Foo, Foo::instance())
EULUNA_CLASS_MEMBER(Foo, setBoo)
EULUNA_CLASS_MEMBER(Foo, getBoo)
EULUNA_END()

TEST(Euluna, Examples) {
    // create a new lua state
    auto L = luaL_newstate();
    // initialize euluna engine using the supplied state
    EulunaEngine euluna(L);
    // register C++ bindings in the supplied lua state
    EulunaBinder::registerGlobalBindings(&euluna);
    // example calling a C++ function from lua then return its value back to C++
    EXPECT_EQ(euluna.runBuffer<double>("return mathex.lerp(0,10,0.5)"), 5.0);
    EXPECT_EQ(euluna.runBuffer<std::string>("return concat('hello',' world!')"), "hello world!");
    EXPECT_EQ(euluna.runBuffer<std::string>("return stringutil.concat('hello',' world!')"), "hello world!");
    EXPECT_EQ(euluna.runBuffer<std::string>("foo.setBoo('hello world!') return foo.getBoo()"), "hello world!");
    EXPECT_EQ(g_lua.stackSize(), 0);
}
