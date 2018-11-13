#include "stdafx.h"

#include "runner.h"
#include "libs/exception.h"

#include "gc.h"

gc::gc(runner &r) :
    r(r) {

    force_no_gc = 0;
    next_collect = gc_grow_size;
}
gc::~gc() {
    for (auto obj : all_objects) {
        if (obj != nullptr)
            delete obj;
    }
}

void gc::add_object(object *objref) {
    if (objref == nullptr) return;

    all_objects.insert(objref);

    if (force_no_gc == 0 &&
        all_objects.size() >= next_collect) {
        collect();

        if (all_objects.size() >= next_collect * 0.7) {
            rklog("[GC] increased %d", next_collect);

            if (_overflow(next_collect, gc_grow_size))
                rk_throw(new memory_overflow_exception());
            else
                grow_single();
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

void gc::mark(const value &v, gc_context &ctx) {
    if (v.type != value_type::object)
        return;

    ctx.marks.insert(v.objref);

    for (auto &prop : v.objref->properties) {
        if (ctx.marks.find(prop.second.objref) !=
            ctx.marks.end())
            continue;

        mark(prop.second, ctx);
    }

    v.objref->gc_visit([this, &ctx](value_cref v) { mark(v, ctx); });
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

inline
void gc::grow_single() {
    if (_overflow(next_collect, gc_grow_size))
        throw memory_overflow_exception();
    
    next_collect += gc_grow_size;
}
inline
void gc::grow_until_fit() {
    while (all_objects.size() * 1.1 >= next_collect)
        grow_single();
}

void gc::begin_no_gc() {
    assert(force_no_gc != UINT32_MAX);

    force_no_gc++;
}
void gc::end_no_gc() {
    assert(force_no_gc != 0);

    force_no_gc--;

    if (force_no_gc == 0)
        grow_until_fit();
}

