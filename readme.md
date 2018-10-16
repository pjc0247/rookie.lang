rookie
====

__rookie__ is a asfdlnsagklnsdafklnsdanfkwelg language that lnsdaflngnfanskefwoelinf for lskdnflsdngflf use.

```cpp
program p;
std::vector<compile_error> errors;

if (compiler::default_compiler()
    .compile("a = 1;b = 4;", p, errors)) {

    printf("DONE!");
}
else {
    printf("Your code has following error(s).\r\n");
    for (auto &err : errors)
        printf("%s\r\n", err.message.c_str());
}
```

built-in runner
----
```cpp
program p; // program you built

runner().execute(p);
```

WebAssembly backend
----
__WORK IN PROGRESS__
```cpp
program p; // program you built

// This generates WAST expressions
//   We need more works to do with it.
p2wast().convert(p);
```

Compling
----
* __rookie__ requires C++17.

__MSVC__<br>
Just open the `.sln` file and build it.

__WebAssembly(emcc)__<br>
```
make
```