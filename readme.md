rookie
====

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