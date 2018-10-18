rookie
====

__[ONLINE PLAYGROUND](https://pjc0247.github.io/try-rookie/)__<br>
__[LANGUAGE DOCUMENTATION - incomplete](docs/)

Concept
----
* Similar to C oriented languages (which means very intuitive)
* Less confusing syntax, unlike modern languages.
* Supports `OOP`.

Usage
----
```cpp
compile_option opts;
compile_output out;

out = compiler::default_compiler()
    .compile("a = 1;b = 4;", opts);

if (out.errors.empty()) {

    printf("DONE!");
}
else {
    printf("Your code has following error(s).\r\n");
    for (auto &err : out.errors)
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
