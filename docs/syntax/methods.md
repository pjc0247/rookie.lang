Methods
----

```ruby
class my_first_program {
    @main
    static def main() {
        say_hello();
    }

    static def say_hello() {
        puts("Hello World!");
    }
}
```

__Instance method__
```ruby
class greeter {
    def say_hello() {
        puts("Hello World!");
    }
}

class my_first_program {
    @main
    static def main() {
        greeter.new().say_hello();        
    }
}
```