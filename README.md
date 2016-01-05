Euluna
======

This is a WIP project.

Effortless lua integration for your C++ project with a simple
and straightforward way of binding your classes from C++ to Lua.

Quick Example
--------------------

```cpp
#include "euluna.hpp"

double mathex_lerp(double a, double b, double t) {
    return a + (b-a)*t;
}

EULUNA_BEGIN_SINGLETON("mathex")
EULUNA_FUNC_NAMED("lerp", mathex_lerp)
EULUNA_END()

int main()
{
    // Initialize euluna engine using a new lua state
    EulunaEngine euluna(luaL_newstate());
    // Register C++ bindings to lua
    EulunaBinder::registerGlobalBindings(&euluna);
    // Example of calling a C++ function from lua and returning its result back to C++
    double res = euluna.runBuffer<double>("return mathex.lerp(0,10,0.5)");
    // Outputs 5
    std::cout << res << std::endl;
    return 0;
}
```

Requeriments
------------------

- C++11 support
- C++ ABI for demangling class names
- RTTI enabled

Features
-----------

- Bind almost any kind of C++ object to lua
- Seamless binding of std::function to/from lua functions
- Simple signal/slot system for emitting events to lua
- Lua exception safe
- EulunaStruct
- Bind shared objects to lua using any kind of smart pointer
- Bind copyable objects to lua
- Bind raw pointer objects to lua

License
---------
MIT

Examples of bindings
----------------------------

### Global functions

C++ code:
```cpp
std::string string_concat(const std::string& a, const std::string& b) {
    return a + b;
}

EULUNA_BEGIN_GLOBAL_FUNCTIONS()
EULUNA_FUNC_NAMED("concat", string_concat)
EULUNA_END()
```

Lua code:
```lua
local str = concat('hello',' world!')
print(str) -- hello world!
```

### Global functions as singleton functions

C++ code:
```C++
namespace stringutil {
std::string concat(const std::string& a, const std::string& b) {
    return a + b;
}
}

EULUNA_BEGIN_SINGLETON("stringutil")
EULUNA_SINGLETON_FUNC_NAMED("concat", stringutil::concat);
EULUNA_END()
```

Lua code:
```lua
local str = stringutil.concat('hello',' world!')
print(str) -- hello world!
```

### Singleton

C++ code:
```cpp
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
```

Lua code:
```lua
foo.setBoo('hello world!')
print(foo.getBoo()) -- hello world!
```

### Class with user managed memory (no smart pointers)

This kind of class should be used with care,
the user is reponsible for mantaining the class memory valid
and collecting its C++/lua memory.

C++ code:
```cpp

class Dummy {
public:
    void setBoo(const std::string& boo) { m_boo = boo; }
    std::string getBoo() const { return m_boo; }

private:
    std::string m_boo;
};

void __handleDummyUse(EulunaInterface* lua, Dummy *dummy) {
    // nothing to do
}
void __handleDummyRelease(EulunaInterface* lua, Dummy *dummy) {
    lua->releaseObject(dummy);
    delete dummy;
}

EULUNA_BEGIN_MANAGED_CLASS(Dummy)
EULUNA_CLASS_REFERENCE_HANDLERS(__handleDummyUse, __handleDummyRelease)
EULUNA_CLASS_STATIC_NAMED("new", []{ return new Dummy; })
EULUNA_CLASS_MEMBER(Dummy, setBoo)
EULUNA_CLASS_MEMBER(Dummy, getBoo)
EULUNA_END()
```

Lua code:
```lua
local dummy = Dummy.new()
dummy:setBoo('hello world!')
print(dummy:getBoo()) -- hello world!
```

### Class with inheritance

C++ code:
```cpp
class Polygon {
public:
    virtual float getArea() = 0;
    void setValues(float a, float b) { m_width = a; m_height = b; }
protected:
    float m_width = 0, m_height = 0;
};

class Rectangle : public Polygon {
public:
    virtual float getArea() { return m_width * m_height; }
};

class Triangle : public Polygon {
public:
    virtual float getArea() { return (m_width * m_height) / 2.0f; }
};

EULUNA_BEGIN_MANAGED_CLASS(Polygon)
EULUNA_CLASS_MEMBER(Polygon, getArea)
EULUNA_CLASS_MEMBER(Polygon, setValues)
EULUNA_END()

EULUNA_BEGIN_MANAGED_DERIVED_CLASS(Rectangle, "Polygon")
EULUNA_CLASS_STATIC_NAMED_EX("new", []{ return new Rectangle; })
EULUNA_CLASS_GENERIC_REFERENCE_HANDLERS(Rectangle)
EULUNA_CLASS_MEMBER(Rectangle, getArea)
EULUNA_END()

EULUNA_BEGIN_MANAGED_DERIVED_CLASS(Triangle, "Polygon")
EULUNA_CLASS_STATIC_NAMED_EX("new", []{ return new Triangle; })
EULUNA_CLASS_GENERIC_REFERENCE_HANDLERS(Triangle)
EULUNA_CLASS_MEMBER(Triangle, getArea)
EULUNA_END()
```

Lua code:
```lua
local t = Triangle.new()
t:setValues(2, 3)
print(t:getArea()) -- 3

local r = Rectangle.new()
r:setValues(2, 3)
print(r:getArea()) -- 6
```

Recommendations
-------------------------

- Use a single global lua state
- Use shared objects with smart pointers


