#pragma once

#include <set>
#include <vector>

#include "value_object.h"

struct runner;
struct gc_context {
    std::set<object*> marks;
};

class gc {
public:
    gc(runner &r);
    virtual ~gc();  

    void add_object(object *objref);
    void remove_object(object *objref);

    void collect();

    int object_count();

protected:
    void mark(value &v, gc_context &ctx);
    void sweep(gc_context &ctx);

private:
    runner &r;

    std::set<object*> all_objects;
    std::vector<value> roots;
};