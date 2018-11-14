#pragma once

#include <string>

#include "binding.h"
#include "string.h"
#include "object.h"

class rkkvpair : public rkobject<rkkvpair> {
public:
    TYPENAME(L"kvpair")

    static void import(binding &b) {
        auto type = type_builder(L"kvpair");

        method(type, rk_id_getitem, &rkkvpair::get);
        method(type, rk_id_tostring, &rkkvpair::to_string);

        b.add_type(type);
    }

    rkkvpair(value_cref key, value_cref value) {
        k = key; v = value;

        set_property(rkid(L"key"), key);
        set_property(rkid(L"value"), value);
    }

    value get(uint32_t key) {
        if (key == 0) return k;
        if (key == 1) return v;
        
		throw new out_of_range_exception();
    }
    value to_string() {
        return str2rk(L"<#kvpair key: " + rk_call_tostring(k) + L", value: " + rk_call_tostring(v) + L">");
    }

private:
    value k, v;
};