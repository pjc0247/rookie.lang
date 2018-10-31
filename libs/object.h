#pragma once

#include <algorithm>

#include "binding.h"

class rkscriptobject : public rkobject<rkscriptobject> {
public:
    static void import(binding &b) {
        auto type = type_builder(L"object");

        method(type, L"properties", &rkscriptobject::all_properties);
        method(type, L"__set_prop", &rkscriptobject::set_property);
        method(type, L"__get_prop", &rkscriptobject::get_property);

        b.add_type(type);
    }

    value all_properties() {
        return value::mkinteger(1);
    }
    value set_property(value_cref key, value_cref value);
    value get_property(value_cref key);
};