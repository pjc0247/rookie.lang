#include "stdafx.h"

#include "runner.h"
#include "libs/exception.h"

#include "gc.h"

gc::gc(runner &r) :
    r(r) {

    next_collect = gc_grow_size;
}
gc::~gc() {
    for (auto obj : all_objects) {
        if (obj != nullptr)
            delete obj;
    }
}

void gc::add_object(object *objref) {
    all_objects.insert(objref);

    if (all_objects.size() >= next_collect) {
        collect();

        if (all_objects.size() >= next_collect * 0.7) {
            if (_overflow(next_collect, gc_grow_size))
                rk_throw(new memory_overflow_exception());
            else 
                next_collect += gc_grow_size;
        }
    }
}
void gc::remove_object(object *objref) {
    all_objects.erase(objref);
}

void gc::collect() {
    gc_context ctx;

    rklog("[GC] before : %d\n", all_objects.size());

    // MARK
    for (int i=0;i<r.stack.size();i++)
        mark(r.stack[i], ctx);

    // SWEEP
    sweep(ctx);

    shrink_if_possible();

    rklog("[GC] after : %d\n", all_objects.size());
}

int gc::object_count() {
    return all_objects.size();
}

void gc::mark(value &v, gc_context &ctx) {
    if (v.type != value_type::object)
        return;

    ctx.marks.insert(v.objref);

    for (auto &prop : v.objref->properties) {
        if (ctx.marks.find(prop.second.objref) !=
            ctx.marks.end())
            continue;

        mark(prop.second, ctx);
    }
    for (auto &ref : v.objref->gc_refs) {
        if (ctx.marks.find(ref.objref) !=
            ctx.marks.end())
            continue;

        mark(ref, ctx);
    }
}
void gc::sweep(gc_context &ctx) {
    for (auto it = all_objects.begin(); it != all_objects.end();) {
        auto objref = *it;

        if (ctx.marks.find(objref) == ctx.marks.end()) {
            it = all_objects.erase(it);
            delete objref;
        }
        else
            ++it;
    }
}
void gc::shrink_if_possible() {
    if (next_collect < object_count() * 2.5f)
        return;

    next_collect *= 0.8f;
}