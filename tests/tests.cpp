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
EULUNA_GLOBAL(nop)
EULUNA_GLOBAL(concat)
EULUNA_GLOBAL_EX("div", [](float a, float b) { return a/b; })
EULUNA_GLOBAL_EX("mul", lua_mul)
EULUNA_END

TEST(EulunaBinder, Globals) {
    EXPECT_EQ(g_lua.runBuffer<std::string>("return concat(1,2)", "@eulunabinder_globals.lua"), std::string("12"));
    EXPECT_EQ(g_lua.runBuffer<float>("return div(10,5)", "@eulunabinder_globals.lua"), 2.0f);
    EXPECT_EQ(g_lua.runBuffer<double>("return mul(3,50)", "@eulunabinder_globals.lua"), 150.0);
}

////////////////////
EULUNA_BEGIN_SINGLETON(test)
EULUNA_SINGLETON_STATIC_EX("nop", nop)
EULUNA_SINGLETON_STATIC_EX("concat", concat)
EULUNA_SINGLETON_STATIC_EX("div", [](float a, float b) { return a/b; })
EULUNA_SINGLETON_STATIC_EX("mul", lua_mul)
EULUNA_END

TEST(EulunaBinder, Singletons) {
    EXPECT_EQ(g_lua.runBuffer<std::string>("return test.concat(1,2)", "@eulunabinder_globals.lua"), std::string("12"));
    EXPECT_EQ(g_lua.runBuffer<float>("return test.div(10,5)", "@eulunabinder_globals.lua"), 2.0f);
    EXPECT_EQ(g_lua.runBuffer<double>("return test.mul(3,50)", "@eulunabinder_globals.lua"), 150.0);
}

/*
class Test {
public:
    static std::string concat(const std::string& a, int b) { return a+std::to_string(b); }
    static int lua_mul(EulunaInterface* lua) {
        double a = lua->popNumber();
        double b = lua->popNumber();
        lua->pushNumber(a*b);
        return 1;
    }
    std::string concat2(const std::string& a, int b) { return a+std::to_string(b); }
    int lua_mul2(EulunaInterface* lua) {
        double a = lua->popNumber();
        double b = lua->popNumber();
        lua->pushNumber(a*b);
        return 1;
    }
};
Test g_test;

////////////////////
EULUNA_BEGIN_SINGLETON_CLASS(Test, g_test)
EULUNA_SINGLETON_STATIC_EX("concat", concat)
EULUNA_END

EULUNA_BEGIN_SINGLETON_CLASS_EX("g_test", Test, g_test)
EULUNA_END
*/
