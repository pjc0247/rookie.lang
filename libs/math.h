#pragma once

#include <algorithm>

#include "binding.h"
#include "object.h"

class rkmath : public rkobject<rkmath> {
public:
    static void import(binding &b) {
        auto type = type_builder(L"math");

        static_method(type, L"abs", abs);

        b.add_type(type);
    }

    static value abs(value &v) {
        return int2rk(std::abs(rk2int(v)));
    }
};