#pragma once

#include <algorithm>

#include "binding.h"
#include "object.h"
#include "string.h"

class rkarray : public rkobject<rkarray> {
public:
    static void import(binding &b) {
        auto type = type_builder(L"array");

        type.method(rk_id_new, create_array);

        method(type, rk_id_getitem, &rkarray::get);
        method(type, rk_id_setitem, &rkarray::set);
        method(type, rk_id_tostring, &rkarray::to_string);

        method(type, L"__add__", &rkarray::op_add);

        method(type, L"at", &rkarray::get);
        method(type, L"push", &rkarray::push);
        method(type, L"remove", &rkarray::remove);
        method(type, L"remove_at", &rkarray::remove);
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
    value to_string() {
        std::wstring str;

        str += L"[";
        for (auto it = ary.begin(); it != ary.end(); ++it) {
            if (it != ary.begin())
                str += L", ";

            str += rk_towstring(*it);
        }
        str += L"]";

        return str2rk(str);
    }

    value op_add(value_cref other) {
        auto new_ary = new rkarray();

        // FIXME
        if (other.objref->sighash == sig2hash(L"array")) {
            auto other_ary = rk2obj(other, rkarray*);

            new_ary->ary.insert(
                new_ary->ary.end(),
                ary.begin(), ary.end());
            new_ary->ary.insert(
                new_ary->ary.end(),
                other_ary->ary.begin(), other_ary->ary.end());
        }

        return obj2rk(new_ary, L"array");
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
    value remove_at(value_cref v) {
        ary.erase(ary.begin() + rk2int(v));
        return rknull;
    }
    value length() {
        return int2rk(ary.size());
    }

private:
    std::vector<value> ary;
};