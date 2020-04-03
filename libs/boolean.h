#pragma once

#include <string>
#include <algorithm>

#include "binding.h"
#include "object.h"

class rkboolean : public rkobject<rkboolean> {
public:
    TYPENAME(L"boolean")

    static void _import(binding &b) {
        auto type = type_builder(L"boolean");

        method(type, rk_id_tostring, &rkboolean::to_string);

        b.add_type(type);
    }

    rkboolean() {
    }
    rkboolean(bool v) :
        v(v) {
    }

    value to_string() {
        if (v)
            return str2rk(L"true");
        else
            return str2rk(L"false");
    }

private:
    bool v;
};