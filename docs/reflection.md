reflection
====

```cpp
value your_func(value &foo) {
  auto is_foo = rk_istypeof(rktype("foo"), foo);
  if (is_foo)
    printf("input value is foo!");
  else
    printf("nooooo");
}
```
