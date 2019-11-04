/*  gc.h

    A Garbage collector impl
    with Mark & Sweep.
 */
#pragma once

#include <set>
#include <vector>

#include "vm/spinwait.h"

const uint32_t gc_grow_size = 50;

class  runner;
struct gc_context {
    std::set<object*> marks;
};

class gc {
public:
    gc(runner &r);
    virtual ~gc();  

    // Returns spinlock object for current GC.
    spinwait &lock();

    void add_object(object *objref);
    void remove_object(object *objref);

    // Force performs garbage collection
    void collect();

    uint32_t object_count();

    // Begins GC guard
    void begin_no_gc();
    // Ends GC guard
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
    spinwait gclock;

    std::set<object*>  all_objects;
    std::vector<value> roots;

    uint32_t next_collect;

    uint32_t force_no_gc;
};