Rookie Programming Language
====

See (rookielang.github.io)(rookielang.github.io)<br>
See (rookielang.github.io)(rookielang.github.io)<br>
See (rookielang.github.io)(rookielang.github.io)<br>

Syntax
----

C++ API
----

C API
----

Syntax
====

overview
----
```ruby
class foo {
  def main() {
    sum(1, 10);
  }
  
  def sum(a, b) {
    return a + b;
  }
}
```

loops
----
```ruby
for (i=0;i<10;i++)
  printf(i);
```

conditions
----
```ruby
if (1 > 5) printf("1 > 5");
```

includes
----
```ruby
include "math.rky";
```


indexer
----
```ruby
def __getitem__ (key) {
    return key;
}
def __setitem__ (key, value) {
   set(key, value);
}
```
