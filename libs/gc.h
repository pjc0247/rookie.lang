#pragma once

#include "binding.h"

class rkgc : public rkobject<rkgc> {
public:
    TYPENAME(L"gc")

    static void _import(binding &b) {
        auto type = type_builder(L"gc");

        static_method(type, L"object_count", object_count);
        static_method(type, L"collect", collect);

        b.add_type(type);
    }

    static value object_count() {
        return int2rk(rkctx()->gc().object_count());
    }
    static value collect() {
        rkctx()->gc().collect();
        return rknull;
    }
};