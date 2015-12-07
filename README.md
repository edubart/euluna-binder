# Euluna

This is a WIP project.

Effortless lua integration for your C++11 project with a simple
and straightforward way of binding for your classes from C++ to Lua.


### Quick Example

```C++
#include "euluna.hpp"

double add(float a, int b) {
  return a+b;
}

EULUNA_BEGIN_SINGLETON("mathex")
EULUNA_SINGLETON_FUNC("add", add)
EULUNA_END

int main()
{
    double res = EulunaEngine::instance()->runBuffer<double>("return mathex.add(1,2)");
    std::cout << res << std::endl; // 3
    return 0;
}
```

### Requeriments

- C++11 support
- C++ ABI for demangling class names
- RTTI enabled


### Features

- Bind almost any kind of C++ object to lua
- Seemless binding of std::function to/from lua functions
- Simple signal/slot system for emitting events to lua
- Lua exception safe
- EulunaStruct
- Bind shared objects to lua using any kind of smart pointer
- Bind copyable objects to lua
- Bind raw pointer objects to lua ()

### Tips

- Usage of a single global lua state
- Use shared objects with smart pointers when possible

### License
MIT
