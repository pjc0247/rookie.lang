#pragma once

#include <string>
#include <algorithm>

#include "binding.h"
#include "object.h"

class rkboolean : public rkobject<rkboolean> {
public:
    TYPENAME(L"boolean")

        static void import(binding &b) {
        auto type = type_builder(L"boolean");

        method(type, rk_id_tostring, &rkboolean::to_string);

        b.add_type(type);
    }

    value to_string(value_cref n) {
        if (n.integer)
            return str2rk(L"true");
        else
            return str2rk(L"false");
    }
};