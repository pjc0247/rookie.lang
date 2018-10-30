#pragma once

#include <string>

#include "binding.h"
#include "string.h"
#include "object.h"

class rkkvpair : public rkobject<rkkvpair> {
public:
    static void import(binding &b) {
        auto type = type_builder(L"kvpair");

        method(type, rk_id_getitem, &rkkvpair::get);
        method(type, rk_id_tostring, &rkkvpair::to_string);

        b.add_type(type);
    }

    rkkvpair(value_cref key, value_cref value) {
        k = key; v = value;

        set_property(sig2hash_c(L"key"), key);
        set_property(sig2hash_c(L"value"), value);
    }

    value get(value_cref key) {
        if (rk2int(key) == 0) return k;
        if (rk2int(key) == 1) return v;
        // TODO: EXCEPTION
    }
    value to_string() {
        return str2rk(L"<#kvpair key: " + rk_towstring(k) + L", value: " + rk_towstring(v) + L">");
    }

private:
    value k, v;
};