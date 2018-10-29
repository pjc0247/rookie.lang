#pragma once

#include <string>
#include <algorithm>

#include "binding.h"
#include "object.h"

class rkreflection : public rkobject<rkreflection> {
public:
    static void import(binding &b) {
        auto type = type_builder(L"reflection");

        static_method(type, L"type", get_type);

        b.add_type(type);
    }

    static value get_type(value_cref name) {
        auto a =  obj2rk_nogc(rkctx()->get_type(rkwstr(name)), L"type");
        return a;
    }
};