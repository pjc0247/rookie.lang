#pragma once

#include "binding.h"

class rkmath : public rkobject<rkmath> {
public:
    static void _import(binding &b) {
        auto type = type_builder(L"math");

        static_method(type, L"abs", abs);

        b.add_type(type);
    }

    static value abs(value_cref v) {
        return int2rk(std::abs(rk2int(v)));
    }
};