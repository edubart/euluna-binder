#include <gtest/gtest.h>
#include <euluna.hpp>
#include <iostream>

EulunaBinder& g_binder = EulunaBinder::instance();
EulunaEngine& g_lua = EulunaEngine::instance();

TEST(DemanglerTest, BasicTypes) {
    EXPECT_EQ("int", euluna_tools::demangle_type<int>());
    EXPECT_EQ("bool", euluna_tools::demangle_type<bool>());
}

TEST(DemanglerTest, Classes) {
    //EXPECT_EQ("EulunaSharedObject", EulunaSharedObject().euluna_getCppClassName());
    EXPECT_EQ("std::string", euluna_tools::demangle_type<std::string>());
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
void nop() {
    // do nothing
}

std::string concat(const std::string& a, int b) {
    return a+std::to_string(b);
}

int lua_mul(EulunaInterface* lua) {
    double a = lua->popNumber();
    double b = lua->popNumber();
    lua->pushNumber(a*b);
    return 1;
}

EULUNA_BEGIN_GLOBAL_FUNCTIONS()
EULUNA_GLOBAL(concat)
EULUNA_GLOBAL(nop)
EULUNA_GLOBAL_NAMED("div", [](float a, float b) { return a/b; })
EULUNA_GLOBAL_NAMED("mul", lua_mul)
EULUNA_END

TEST(EulunaBinder, Globals) {
    EXPECT_EQ(g_lua.runBuffer<std::string>("return concat(1,2)", "@eulunabinder_globals.lua"), std::string("12"));
    EXPECT_EQ(g_lua.runBuffer<float>("return div(10,5)", "@eulunabinder_globals.lua"), 2.0f);
    EXPECT_EQ(g_lua.runBuffer<double>("return mul(3,50)", "@eulunabinder_globals.lua"), 150.0);
    EXPECT_EQ(g_lua.stackSize(), 0);
}

////////////////////
EULUNA_BEGIN_SINGLETON("test")
EULUNA_SINGLETON_FUNC(concat)
EULUNA_SINGLETON_FUNC(nop)
EULUNA_SINGLETON_FUNC_NAMED("div", [](float a, float b) { return a/b; })
EULUNA_SINGLETON_FUNC_NAMED("mul", lua_mul)
EULUNA_END

class Test {
public:
    static std::string concat(const std::string& a, int b) { return a+std::to_string(b); }
    static int lua_mul(EulunaInterface* lua) {
        double a = lua->popNumber();
        double b = lua->popNumber();
        lua->pushNumber(a*b);
        return 1;
    }
    uint setFoo(const std::string& foo) { m_foo = foo; return 0xdeadbeaf; }
    std::string getMyFoo() const { return m_foo; }
private:
    std::string m_foo;
};
Test g_test;

EULUNA_BEGIN_SINGLETON_CLASS_NAMED("g_test", Test, &g_test)
EULUNA_SINGLETON_STATIC(Test, concat)
EULUNA_SINGLETON_STATIC_NAMED("mul", Test, lua_mul)
EULUNA_SINGLETON_MEMBER(Test, setFoo)
EULUNA_SINGLETON_MEMBER_NAMED("getFoo", Test, getMyFoo)
EULUNA_END

TEST(EulunaBinder, Singletons) {
    EXPECT_EQ(g_lua.runBuffer<std::string>("return test.concat(1,2)", "@eulunabinder_globals.lua"), "12");
    EXPECT_EQ(g_lua.runBuffer<float>("return test.div(10,5)", "@eulunabinder_globals.lua"), 2.0f);
    EXPECT_EQ(g_lua.runBuffer<double>("return test.mul(3,50)", "@eulunabinder_globals.lua"), 150.0);
    EXPECT_EQ(g_lua.runBuffer<std::string>("return g_test.concat(1,2)", "@eulunabinder_globals.lua"), "12");
    EXPECT_EQ(g_lua.runBuffer<double>("return g_test.mul(3,50)", "@eulunabinder_globals.lua"), 150.0);
    EXPECT_EQ(g_lua.runBuffer<uint>("return g_test.setFoo('boo')", "@eulunabinder_globals.lua"), 0xdeadbeaf);
    EXPECT_EQ(g_lua.runBuffer<std::string>("return g_test.getFoo()", "@eulunabinder_globals.lua"), "boo");
    EXPECT_EQ(g_lua.stackSize(), 0);
}


//////////////////////

double myadd(float a, int b) {
  return a+b;
}

EULUNA_BEGIN_SINGLETON("mathex")
EULUNA_SINGLETON_FUNC_NAMED("add", myadd)
EULUNA_END

TEST(Euluna, Example) {
    // create a new lua state
    auto L = luaL_newstate();
    // initialize euluna engine using the supplied state
    EulunaEngine euluna(L);
    // register C++ bindings in the supplied lua state
    EulunaBinder::registerGlobalBindings(&euluna);
    // example calling a C++ function from lua then return its value back to C++
    double res = euluna.runBuffer<double>("return mathex.add(1,2)");
    EXPECT_EQ(res, 3);
}
