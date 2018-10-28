/*  gc.h

    A Garbage collector impl
    with Mark & Sweep.
 */
#pragma once

#include <set>
#include <vector>

const uint32_t gc_grow_size = 50;

class  runner;
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

    uint32_t next_collect;
};