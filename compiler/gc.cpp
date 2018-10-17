#include "stdafx.h"

#include "gc.h"

void gc::add_object(object *objref) {
    all_objects.insert(objref);
}
void gc::remove_object(object *objref) {
    all_objects.erase(objref);
}

void gc::collect() {
    gc_context ctx;

    // MARK
    for (auto &v : roots)
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
    for (auto objref : all_objects) {
        if (ctx.marks.find(objref) == ctx.marks.end()) {
            remove_object(objref);
            delete objref;
        }
    }
}