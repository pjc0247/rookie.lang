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

    rkinteger() {
        n = 0;
    }
    rkinteger(int n) :
        n(n) {
    }
    
    value to_string() {
        return value::mkstring2(std::to_wstring(n).c_str());
    }

private:
    int n;
};