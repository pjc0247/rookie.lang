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

	uint32_t object_count();

    void begin_no_gc();
    void end_no_gc();

protected:
    void mark(const value &v, gc_context &ctx);
    void sweep(gc_context &ctx);

    void shrink_if_possible();

private:
    void grow_single();
    void grow_until_fit();

private:
    runner &r;

    std::set<object*> all_objects;
    std::vector<value> roots;

    uint32_t next_collect;

    uint32_t force_no_gc;
};