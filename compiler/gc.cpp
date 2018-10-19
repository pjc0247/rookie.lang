#include "stdafx.h"

#include "runner.h"

#include "gc.h"

gc::gc(runner &r) :
    r(r) {
}
gc::~gc() {
    for (auto obj : all_objects) {
        if (obj != nullptr)
            delete obj;
    }
}

void gc::add_object(object *objref) {
    all_objects.insert(objref);
}
void gc::remove_object(object *objref) {
    all_objects.erase(objref);
}

void gc::collect() {
    gc_context ctx;

    // MARK
    for (auto &v : r.stack)
        mark(v, ctx);

    // SWEEP
    sweep(ctx);
}

int gc::object_count() {
    return all_objects.size();
}

void gc::mark(value &v, gc_context &ctx) {
    if (v.type != value_type::object)
        return;

    ctx.marks.insert(v.objref);

    for (auto &prop : v.objref->properties)
        mark(prop.second, ctx);
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