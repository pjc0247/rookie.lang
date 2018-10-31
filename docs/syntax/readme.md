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

