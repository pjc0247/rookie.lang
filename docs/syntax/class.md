Class
----

__Inheritance__
```ruby
class person {
    def say_hello() {
        puts("Hello");
    }
}
class engineer : person {
    def say_hello() {
        puts("HelloWorld");
    }
}
```
```ruby
// Hello
person.new().say_hello();

// HelloWorld
engineer.new().say_hello();
```

__Static method__
```ruby
class dog {
    static def kind() {
        puts("animal");
    }
}
class anchovy {
    static def kind() {
        puts("fish");
    }
}
```
```ruby
// animal
dog.kind();

// fish
anchovy.kind();
```