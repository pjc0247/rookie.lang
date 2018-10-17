#pragma once

#include <set>
#include <vector>

#include "value_object.h"

struct gc_context {
    std::set<object*> marks;
};

class gc {
public:
    void add_object(object *objref);
    void remove_object(object *objref);

    void collect();

    int object_count();

protected:
    void mark(value &v, gc_context &ctx);
    void sweep(gc_context &ctx);

private:
    std::set<object*> all_objects;
    std::vector<value> roots;
};