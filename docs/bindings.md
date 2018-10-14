bindings
====

There are 2 kinds of exportations in __rookie__.<br>
First, you can export a __function__. It's a simply works as global functions. No callees required.<br>
Second one is __class__. You can create a type and fill out some methods.

functions
----
You can export your C++ functions into __rookie__.<br>
```cpp
_rookie_library(rookie_stdlib)
_rookie_function("print", (value v) {
    if (is_rkint(v)) printf("%d\n", rkint(v));
    else if (is_rkstr(v)) printf("%s\n", rkcstr(v));
});
_end_rookie_library
```

classes
----
Please refer the [stdlibs](https://github.com/pjc0247/rookie.lang/tree/master/libs) for a while.
```cpp
class your_object : public rkobject<rkarray> {
public:
    
}
```
