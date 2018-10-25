#pragma once

#include <algorithm>

#include "binding.h"
#include "value_object.h"
#include "object.h"

class rkarray : public rkobject<rkarray> {
public:
    static void import(binding &b) {
        auto type = type_builder(L"array");

        type.method(rk_new, create_array);

        method(type, rk_getitem, &rkarray::get);
        method(type, rk_setitem, &rkarray::set);

        method(type, L"at", &rkarray::get);
        method(type, L"push", &rkarray::push);
        method(type, L"remove", &rkarray::remove);
        method(type, L"length", &rkarray::length);

        b.add_type(type);
    }

    rkarray(int n) {
        for (int i = 0; i < n; i++) {
            value v = rkctx()->next_param();
            push(v);
        }
    }

    value static create_array(value &idx) {
        return value::mkobjref(new rkarray(0));
    }

    value get(value &idx) {
        return ary[rkint(idx)];
    }
    value set(value &idx, value &v) {
        ary[rkint(idx)] = v;
        return rknull;
    }
    value push(value &v) {
        ary.push_back(v);
        return rknull;
    }
    value remove(value &v) {
        ary.erase(std::remove(
            ary.begin(), ary.end(), v),
            ary.end());
        return rknull;
    }
    value length() {
        return value::mkinteger(ary.size());
    }

private:
    std::vector<value> ary;
};