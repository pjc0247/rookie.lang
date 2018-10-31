#pragma once

#include <string>
#include <algorithm>

#include "binding.h"
#include "object.h"

class rkinteger : public rkobject<rkinteger> {
public:
    TYPENAME(L"integer")

    static void import(binding &b) {
        auto type = type_builder(L"integer");

        method(type, rk_id_tostring, &rkinteger::to_string);

        b.add_type(type);
    }

    value to_string(value_cref n) {
        return value::mkstring2(std::to_wstring(rkint(n)).c_str());
    }
};