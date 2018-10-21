Rookie::Syntax
====

Inheritance
----
```ruby
class person {
    def greeting() { print("HELLO PERSON"); }
}
class engineer : person {
    def greeting() { print("HELLO ENGINEER"); }
}
```
```ruby
// HELLO PERSON
person.new().greeting();

// HELLO ENGINNEER
engineer.new().greeting();
```

Indexer
----
```ruby
class foo {
    def __get_item__ (key) {
        /* ... */
    }
    def __set_item__ (key, value) {
        /* ... */
    }
}
```
```ruby
f = foo.new();

f["a"] = 1234;
print(f["a"]);
```
