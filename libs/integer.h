#pragma once

#include <string>
#include <algorithm>

#include "binding.h"
#include "value_object.h"
#include "object.h"

class rkinteger : public rkobject<rkinteger> {
public:
    static void import(binding &b) {
        auto type = type_builder(L"integer");

        method(type, rk_tostring, &rkinteger::to_string);

        b.add_type(type);
    }

    value to_string(value &n) {
        return value::mkstring2(std::to_wstring(rkint(n)).c_str());
    }
};