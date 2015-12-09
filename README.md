Euluna
======

This is a WIP project.

Effortless lua integration for your C++ project with a simple
and straightforward way of binding your classes from C++ to Lua.

Quick Example
--------------------

```C++
#include "euluna.hpp"

double myadd(float a, int b) {
  return a+b;
}

EULUNA_BEGIN_SINGLETON("mathex")
EULUNA_SINGLETON_FUNC_NAMED("add", myadd)
EULUNA_END

int main()
{
    // Initialize euluna engine using a new lua state
    EulunaEngine euluna(luaL_newstate());
    // Register C++ bindings to lua
    EulunaBinder::registerGlobalBindings(&euluna);
    // Example of calling a C++ function from lua and returning its result back to C++
    double res = euluna.runBuffer<double>("return mathex.add(1,2)");
    // Outputs 3
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
```C++
std::string myconcat(const std::string& a, const std::string& b) {
    return a + b;
}

EULUNA_BEGIN_GLOBAL_FUNCTIONS()
EULUNA_GLOBAL_NAMED("concat", myconcat)
EULUNA_END
```

Lua code:
```Lua
local str = concat("hello","world!")
print(str) -- hello world!
```

### Global functions as singleton functions

C++ code:
```C++

#include <cstdio>

EULUNA_BEGIN_SINGLETON("cio")
EULUNA_SINGLETON_FUNC_NAMED("puts", std::puts);
EULUNA_SINGLETON_FUNC_NAMED("gets", std::gets);
EULUNA_END
```

Lua code:
```Lua
cio.puts("hello world!") -- hello world!
```

### Singleton

C++ code:
```C++
class Foo {
public:
    static Foo* instance() {
        static Foo instance;
        return &instance;
    }

    void setBoo(const std::string& foo) { m_boo = boo; }
    std::string getBoo() const { return m_boo; }

private:
    std::string m_boo;
}

EULUNA_BEGIN_SINGLETON_CLASS_NAMED("foo", Foo, Foo::instance())
EULUNA_SINGLETON_MEMBER(Foo, setBoo)
EULUNA_SINGLETON_MEMBER(Foo, getBoo)
EULUNA_END
```

Lua code:
```Lua
foo.setBoo('hello world!')
print(foo.getBoo()) -- hello world!
```

### Class with user managed memory (no smart pointers)

This kind of class should be used with care,
the user is reponsible for mantaining the class memory valid
and collecting its C++/lua memory.

C++ code:
```C++

class Dummy {
public:
    Dummy() { }
    ~Dummy() { }

    void setBoo(const std::string& foo) { m_boo = boo; }
    std::string getBoo() const { return m_boo; }

private:
    std::string m_boo;
}

EULUNA_BEGIN_MANAGED_CLASS(Dummy, []((Euluna *euluna, Dummy *dummy, int totalRefs, int refChange) {
    if(totalRefs == 0) {
        delete dummy; // delete C++ memory for this object
        euluna->collectManagedObject(dummy); // collects all lua variables related to this object
    }
})
EULUNA_MANAGED_STATIC_NAMED("new", []{ return new Dummy(); })
EULUNA_MANAGED_MEMBER(Dummy, setBoo)
EULUNA_MANAGED_MEMBER(Dummy, setFoo)
EULUNA_END
```

Lua code:
```Lua
local dummy = Dummy.new()
dummy.setBoo('hello world!)
print(dummy.getBoo()) -- hello world!
```

Recommendations
-------------------------

- Use a single global lua state
- Use shared objects with smart pointers


