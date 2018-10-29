#pragma once

#include <algorithm>

#include "binding.h"
#include "object.h"

class rkarray : public rkobject<rkarray> {
public:
    static void import(binding &b) {
        auto type = type_builder(L"array");

        type.method(rk_id_new, create_array);

        method(type, rk_id_getitem, &rkarray::get);
        method(type, rk_id_setitem, &rkarray::set);

        method(type, L"at", &rkarray::get);
        method(type, L"push", &rkarray::push);
        method(type, L"remove", &rkarray::remove);
        method(type, L"length", &rkarray::length);

        b.add_type(type);
    }

    rkarray() {
    }
    rkarray(int n) {
        for (int i = 0; i < n; i++) {
            value v = rkctx()->next_param();
            push(v);
        }
    }
    
    value static create_array(const value &idx) {
        return value::mkobjref(new rkarray(0));
    }

    value get(value_cref idx) {
        return ary[rkint(idx)];
    }
    value set(value_cref idx, value_cref v) {
        ary[rkint(idx)] = v;
        return rknull;
    }

    value push(value_cref v) {
        ary.push_back(v);
        return rknull;
    }
    value remove(value_cref v) {
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