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
